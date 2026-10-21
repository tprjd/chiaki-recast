#include <QtTest>
#include <QQuickItem>
#include <QQuickWindow>
#include "streaminputownership.h"

class StreamInputTest : public QObject
{
    Q_OBJECT
private slots:
    void presentations()
    {
        QQuickWindow window;
        QQuickItem opener(window.contentItem()), first(window.contentItem()), second(window.contentItem());
        opener.forceActiveFocus();
        StreamInputOwnership input(&window, &window);
        const auto a = input.present(&first, "display", &first);
        QVERIFY(input.blocked());
        QCOMPARE(input.present(&first, "display", &second), a);
        QCOMPARE(window.activeFocusItem(), &first);
        const auto b = input.present(&second, "renderer", &second);
        input.dismiss(a);
        QVERIFY(input.blocked());
        QCOMPARE(window.activeFocusItem(), &second);
        input.settled(a);
        input.dismiss(b);
        input.settled(b);
        QVERIFY(!input.blocked());
    }
    void obsoleteCloseAndDestruction()
    {
        QQuickWindow window;
        QQuickItem opener(window.contentItem()), other(window.contentItem());
        auto *owner = new QQuickItem(window.contentItem());
        StreamInputOwnership input(&window, &window);
        opener.forceActiveFocus();
        const auto old = input.present(owner, "pin", owner);
        input.dismiss(old);
        input.settled(old);
        const auto reopened = input.present(owner, "pin", owner);
        QVERIFY(old != reopened);
        input.dismiss(old);
        input.settled(old);
        QCoreApplication::processEvents();
        QVERIFY(input.blocked());
        QCOMPARE(window.activeFocusItem(), owner);
        const auto overlapping = input.present(&other, "display", &other);
        delete owner;
        QVERIFY(input.blocked());
        QCOMPARE(window.activeFocusItem(), &other);
        input.dismiss(reopened);
        input.settled(reopened);
        input.dismiss(overlapping);
        input.settled(overlapping);
        QVERIFY(!input.blocked());
    }
    void sessionCapture()
    {
        QQuickWindow window;
        QObject menu, connection, pin;
        StreamInputOwnership input(&window, &window);
        const auto menuId = input.present(&menu, "display", nullptr);
        input.sessionChanged(true, false, false);
        QVERIFY(!input.blocked());
        QCOMPARE(input.present(&menu, "display", nullptr), menuId);
        const auto connectionId = input.present(&connection, "connection", nullptr);
        input.sessionChanged(false, false, false);
        QVERIFY(input.blocked());
        QCOMPARE(input.present(&connection, "connection", nullptr), connectionId);
        input.dismiss(connectionId);
        input.settled(connectionId);
        const auto pinId = input.present(&pin, "pin", nullptr);
        input.sessionChanged(true, true, false);
        QVERIFY(input.blocked());
        input.sessionChanged(true, false, false);
        QCOMPARE(input.present(&pin, "pin", nullptr), pinId);
        QVERIFY(input.blocked());
        input.dismiss(pinId);
        input.settled(pinId);
        input.sessionChanged(false, false, false);
        const auto psnId = input.present(&pin, "psn", nullptr);
        input.sessionChanged(true, false, true);
        QVERIFY(input.blocked());
        input.dismiss(psnId);
        input.settled(psnId);
        QVERIFY(!input.blocked());
    }
    void reentrantDismissal()
    {
        QQuickWindow window;
        QQuickItem owner(window.contentItem());
        StreamInputOwnership input(&window, &window);
        const auto old = input.present(&owner, "display", &owner);
        QString reopened;
        connect(&input, &StreamInputOwnership::blockedChanged, &input, [&] {
            if (!input.blocked()) reopened = input.present(&owner, "display", &owner);
        });
        input.dismiss(old);
        QVERIFY(!reopened.isEmpty());
        input.settled(old);
        QVERIFY(input.blocked());
        QCOMPARE(input.present(&owner, "display", nullptr), reopened);
    }
};
QTEST_MAIN(StreamInputTest)
#include "streaminput.moc"
