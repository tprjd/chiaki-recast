#include <QtTest>
#include <SDL.h>
#include <cmath>
#include <QTemporaryDir>
#include <QUuid>
#include <limits>
#include "keyboardmousemapper.h"
#include "settings.h"
#include "qmlsettings.h"

class MouseResponseTest : public QObject
{
    Q_OBJECT
private slots:
    void promptStartStopAndReversal()
    {
        KeyboardMouseMapper mapper;
        mapper.mouseMove(4, 0, 1000);
        mapper.advanceMouse(8000);
        QCOMPARE(mapper.state().right_x, 8000);
        mapper.mouseMove(-4, 0, 9000);
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, -8000);
        mapper.advanceMouse(24000);
        QCOMPARE(mapper.state().right_x, 0);
    }
    void slowMovementPrecision()
    {
        KeyboardMouseMapper mapper;
        MouseResponse response;
        response.horizontalSensitivity = response.verticalSensitivity = MouseResponse::MinimumSensitivity;
        QVERIFY(mapper.setMouseResponse(response));
        mapper.mouseMove(0.25, -0.25, 1000);
        mapper.advanceMouse(8000);
        QCOMPARE(mapper.state().right_x, 25);
        QCOMPARE(mapper.state().right_y, -25);
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, 0);
    }
    void timedTraces()
    {
        for (int batchUs : {1000, 2000, 4000, 8000}) {
            for (int phaseUs : {0, 1000, 3500}) {
                KeyboardMouseMapper mapper;
                qint64 integral = 0;
                int peak = 0;
                for (int now = 500; now <= 80000; now += 500) {
                    if (now > phaseUs && now <= phaseUs + 32000 && (now - phaseUs) % batchUs == 0)
                        mapper.mouseMove(batchUs / 1000, 0, now);
                    if (now % 8000 == 0) {
                        mapper.advanceMouse(now);
                        integral += mapper.state().right_x * 8;
                        peak = std::max(peak, int(mapper.state().right_x));
                    }
                }
                QCOMPARE(integral, qint64(512000));
                QCOMPARE(peak, 16000);
                QCOMPARE(mapper.state().right_x, 0);
                qInfo("batch=%d us phase=%d us integral=%lld stick*ms peak=%d idle=0", batchUs, phaseUs, static_cast<long long>(integral), peak);
            }
        }
    }
    void timerJitter()
    {
        KeyboardMouseMapper mapper;
        qint64 now = 0, nextSample = 1000, integral = 0;
        for (int interval : {8000, 11000, 5000, 9000, 7000}) {
            now += interval;
            while (nextSample <= now && nextSample <= 32000) {
                mapper.mouseMove(1, 0, nextSample);
                nextSample += 1000;
            }
            mapper.advanceMouse(now);
            integral += qint64(mapper.state().right_x) * interval;
        }
        QVERIFY(std::abs(integral - qint64(512000000)) <= 20000);
        mapper.advanceMouse(now + 8000);
        QCOMPARE(mapper.state().right_x, 0);
    }
    void inverseGameResponse()
    {
        for (double speed : {0.01, 0.1, 0.25, 0.5, 1.0}) {
            KeyboardMouseMapper mapper;
            MouseResponse response;
            response.responseExponent = 0.5;
            response.deadZoneCompensation = 0.1;
            QVERIFY(mapper.setMouseResponse(response));
            mapper.mouseMove(speed * 32767 / 2000, 0, 1000);
            mapper.advanceMouse(8000);
            const double stick = mapper.state().right_x / 32767.0;
            const double simulatedGameSpeed = std::pow((stick - 0.1) / 0.9, 2);
            QVERIFY(std::abs(simulatedGameSpeed - speed) < 0.0001);
            mapper.advanceMouse(16000);
            QCOMPARE(mapper.state().right_x, 0);
        }
    }
    void staleMovementAndReset()
    {
        KeyboardMouseMapper mapper;
        mapper.mouseMove(100, 0, 1000);
        mapper.advanceMouse(200000);
        QCOMPARE(mapper.state().right_x, 0);
        mapper.mouseMove(1, 0, 201000);
        mapper.advanceMouse(208000);
        QCOMPARE(mapper.state().right_x, 2000);
        QVERIFY(!mapper.advanceMouse(208000));
        mapper.mouseMove(1, 0, 209000);
        mapper.reset();
        mapper.advanceMouse(8000);
        QCOMPARE(mapper.state().right_x, 0);
    }
    void persistenceAndInvalidValues()
    {
        Settings store("response");
        store.SetKeyboardEnabled(false);
        QmlSettings bridge(&store);
        QVERIFY(bridge.setProperty("mouseHorizontalSensitivity", 2.5));
        QVERIFY(bridge.setProperty("mouseVerticalSensitivity", 0.75));
        QVERIFY(bridge.setProperty("mouseDeadZoneCompensation", 0.2));
        QVERIFY(bridge.setProperty("mouseResponseExponent", 0.5));
        QVERIFY(bridge.setProperty("mouseInvertVertical", true));
        Settings restarted("response");
        QCOMPARE(restarted.GetMouseResponse().horizontalSensitivity, 2.5);
        QCOMPARE(restarted.GetMouseResponse().verticalSensitivity, 0.75);
        QCOMPARE(restarted.GetMouseResponse().deadZoneCompensation, 0.2);
        QCOMPARE(restarted.GetMouseResponse().responseExponent, 0.5);
        QVERIFY(restarted.GetMouseResponse().invertVertical);
        QVERIFY(!restarted.GetKeyboardEnabled());
        for (double bad : {-1.0, 0.0, 100.0, std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::quiet_NaN()}) {
            bridge.setProperty("mouseHorizontalSensitivity", bad);
            QCOMPARE(bridge.property("mouseHorizontalSensitivity").toDouble(), 2.5);
        }
        bridge.setProperty("mouseDeadZoneCompensation", 0.51);
        QCOMPARE(bridge.property("mouseDeadZoneCompensation").toDouble(), 0.2);
        for (double bad : {0.0, -1.0, 3.1, std::numeric_limits<double>::quiet_NaN()}) {
            bridge.setProperty("mouseResponseExponent", bad);
            QCOMPARE(bridge.property("mouseResponseExponent").toDouble(), 0.5);
        }
        QSettings damaged(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-response");
        damaged.setValue("settings/keyboard_mouse/horizontal_sensitivity", "nan");
        damaged.setValue("settings/keyboard_mouse/vertical_sensitivity", 500);
        damaged.setValue("settings/keyboard_mouse/dead_zone_compensation", -1);
        damaged.setValue("settings/keyboard_mouse/response_exponent", "nan");
        damaged.setValue("settings/keyboard_mouse/invert_vertical", "invalid");
        damaged.sync();
        Settings recovered("response");
        const auto response = recovered.GetMouseResponse();
        QCOMPARE(response.horizontalSensitivity, 1.0);
        QCOMPARE(response.verticalSensitivity, 1.0);
        QCOMPARE(response.deadZoneCompensation, 0.0);
        QCOMPARE(response.responseExponent, 1.0);
        QVERIFY(!response.invertVertical);
    }
    void responseAndValidation()
    {
        KeyboardMouseMapper mapper;
        MouseResponse response;
        response.horizontalSensitivity = 2;
        response.verticalSensitivity = 0.5;
        response.invertVertical = true;
        QVERIFY(mapper.setMouseResponse(response));
        mapper.mouseMove(2, 4, 0);
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, 4000);
        QCOMPARE(mapper.state().right_y, -2000);
        response.horizontalSensitivity = std::numeric_limits<double>::quiet_NaN();
        QVERIFY(!mapper.setMouseResponse(response));
        mapper.reset();
        mapper.mouseMove(1, 0, 0);
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, 2000);
        response = {};
        response.deadZoneCompensation = 0.25;
        QVERIFY(mapper.setMouseResponse(response));
        mapper.reset();
        mapper.advanceMouse(0);
        QCOMPARE(mapper.state().right_x, 0);
        mapper.mouseMove(1, 0, 0);
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, 8942);
        mapper.advanceMouse(32000);
        QCOMPARE(mapper.state().right_x, 0);
        mapper.mouseMove(1e-320, 0, 33000);
        mapper.advanceMouse(49000);
        QCOMPARE(mapper.state().right_x, 8192);
        mapper.mouseMove(10000, 10000, 50000);
        mapper.advanceMouse(66000);
        QVERIFY(std::hypot(mapper.state().right_x, mapper.state().right_y) <= 32768);
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir directory;
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, directory.path());
    app.setOrganizationName("Chiaki response tests");
    app.setApplicationName("Chiaki response tests");
    MouseResponseTest test;
    return QTest::qExec(&test, argc, argv);
}
#include "mouseresponse.moc"
