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
    void timedTraces()
    {
        for (int batchUs : {1000, 2000, 4000, 8000}) {
            for (int renderUs : {8000, 16000, 33000}) {
                KeyboardMouseMapper mapper;
                qint64 integral = 0;
                int peak = 0;
                for (int now = 0; now < 64000; now += 1000) {
                    if (now < 32000 && now % batchUs == 0)
                        mapper.mouseMove(batchUs / 1000, 0, now);
                    if (now % 4000 == 0) {
                        mapper.advanceMouse(now);
                        integral += mapper.state().right_x * 4;
                        peak = std::max(peak, int(mapper.state().right_x));
                    }
                    if (now % renderUs == 0) mapper.state();
                }
                QCOMPARE(integral, qint64(512000));
                QCOMPARE(peak, 16000);
                QCOMPARE(mapper.state().right_x, 0);
                qInfo("batch=%d us render=%d us integral=%lld stick*ms peak=%d idle=0", batchUs, renderUs, static_cast<long long>(integral), peak);
            }
        }
    }
    void persistenceAndInvalidValues()
    {
        Settings store("response");
        store.SetKeyboardEnabled(false);
        QmlSettings bridge(&store);
        QVERIFY(bridge.setProperty("mouseHorizontalSensitivity", 2.5));
        QVERIFY(bridge.setProperty("mouseVerticalSensitivity", 0.75));
        QVERIFY(bridge.setProperty("mouseDeadZoneCompensation", 0.2));
        QVERIFY(bridge.setProperty("mouseInvertVertical", true));
        Settings restarted("response");
        QCOMPARE(restarted.GetMouseResponse().horizontalSensitivity, 2.5);
        QCOMPARE(restarted.GetMouseResponse().verticalSensitivity, 0.75);
        QCOMPARE(restarted.GetMouseResponse().deadZoneCompensation, 0.2);
        QVERIFY(restarted.GetMouseResponse().invertVertical);
        QVERIFY(!restarted.GetKeyboardEnabled());
        for (double bad : {-1.0, 0.0, 100.0, std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::quiet_NaN()}) {
            bridge.setProperty("mouseHorizontalSensitivity", bad);
            QCOMPARE(bridge.property("mouseHorizontalSensitivity").toDouble(), 2.5);
        }
        bridge.setProperty("mouseDeadZoneCompensation", 0.51);
        QCOMPARE(bridge.property("mouseDeadZoneCompensation").toDouble(), 0.2);
        QSettings damaged(QCoreApplication::organizationName(), QCoreApplication::applicationName() + "-response");
        damaged.setValue("settings/keyboard_mouse/horizontal_sensitivity", "nan");
        damaged.setValue("settings/keyboard_mouse/vertical_sensitivity", 500);
        damaged.setValue("settings/keyboard_mouse/dead_zone_compensation", -1);
        damaged.setValue("settings/keyboard_mouse/invert_vertical", "invalid");
        damaged.sync();
        Settings recovered("response");
        const auto response = recovered.GetMouseResponse();
        QCOMPARE(response.horizontalSensitivity, 1.0);
        QCOMPARE(response.verticalSensitivity, 1.0);
        QCOMPARE(response.deadZoneCompensation, 0.0);
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
        mapper.advanceMouse(0);
        QCOMPARE(mapper.state().right_x, 4000);
        QCOMPARE(mapper.state().right_y, -2000);
        response.horizontalSensitivity = std::numeric_limits<double>::quiet_NaN();
        QVERIFY(!mapper.setMouseResponse(response));
        mapper.reset();
        mapper.mouseMove(1, 0, 0);
        mapper.advanceMouse(0);
        QCOMPARE(mapper.state().right_x, 2000);
        response = {};
        response.deadZoneCompensation = 0.25;
        QVERIFY(mapper.setMouseResponse(response));
        mapper.reset();
        mapper.advanceMouse(0);
        QCOMPARE(mapper.state().right_x, 0);
        mapper.mouseMove(1, 0, 0);
        mapper.advanceMouse(0);
        QCOMPARE(mapper.state().right_x, 8942);
        mapper.advanceMouse(16000);
        QCOMPARE(mapper.state().right_x, 0);
        mapper.mouseMove(1e-320, 0, 17000);
        mapper.advanceMouse(17000);
        QCOMPARE(mapper.state().right_x, 8192);
        mapper.mouseMove(10000, 10000, 20000);
        mapper.advanceMouse(20000);
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
