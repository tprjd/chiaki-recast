#include "backend.h"
#include "steamtools.h"
#include <QGuiApplication>
#include <QJSEngine>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <iostream>
#include <stdexcept>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
static void require(bool ok, const char *message) {
    if (!ok) throw std::runtime_error(message);
    std::cout << "PASS " << message << std::endl;
}
static void write(const QString &path, const QByteArray &data) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    require(file.open(QIODevice::WriteOnly) && file.write(data) == data.size(), "temporary fixture written");
}
static QByteArray read(const QString &path) { QFile f(path); f.open(QIODevice::ReadOnly); return f.readAll(); }
int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    try {
        QTemporaryDir temp;
        require(temp.isValid(), "isolated Steam directory exists");
        const QString base = temp.path();
        const QString shortcuts = base + "/userdata/1/config/shortcuts.vdf";
        const QString controller = base + "/steamapps/common/Steam Controller Configs/1/config/configset_controller_neptune.vdf";
        write(base + "/config/loginusers.vdf", "\"users\"\n{\n\"76561197960265729\"\n{\n\"MostRecent\" \"1\"\n}\n}\n");
        write(controller, "\"controller_mappings\"\n{\n}\n");
        QJSEngine engine;
        QmlBackend backend;
        auto run = [&](QString name, QString options) {
            engine.evaluate("var events = []; function callback(msg, ok, done) { events.push({msg:msg,ok:ok,done:done}); }");
            backend.createSteamShortcut(name, options, engine.globalObject().property("callback"), base);
            return engine.globalObject().property("events");
        };
        auto events = run("Synthetic shortcut", "--profile=synthetic");
        int terminalCount = 0;
        for (int i = 0; i < events.property("length").toInt(); ++i)
            if (events.property(i).property("done").toBool()) ++terminalCount;
        require(terminalCount == 1, "new shortcut has exactly one terminal result");
        require(events.property(events.property("length").toInt()-1).property("ok").toBool(), "new shortcut terminal result succeeds");
        bool falseBackup = false;
        for (int i = 0; i < events.property("length").toInt(); ++i)
            falseBackup |= events.property(i).property("msg").toString().contains("shortcuts.vdf backed up");
        require(!falseBackup, "new shortcut does not claim a nonexistent backup");
        QStringList errors;
        SteamTools tools([](const QString &) {}, [&](const QString &e) { errors.append(e); }, base);
        auto entries = tools.parseShortcuts();
        require(entries.size() == 1 && entries[0].getAppName() == "Synthetic shortcut", "actual shortcuts.vdf contains the new shortcut");
        require(entries[0].getLaunchOptions() == "--profile=synthetic", "actual shortcut retains launch options");
        require(QDir(base + "/userdata/1/config/grid").entryList({"*.png"}, QDir::Files).size() == 5, "all five artwork files are written");
        require(read(controller).contains("3049833406") && read(controller).contains("synthetic shortcut"), "actual controller configuration is updated");
        events = run("Renamed shortcut", "--profile=synthetic");
        entries = tools.parseShortcuts();
        require(entries.size() == 1 && entries[0].getAppName() == "Renamed shortcut", "matching executable and launch options update the existing entry");
        require(QDir(QFileInfo(shortcuts).absolutePath()).entryList({"shortcuts.vdf.*.bak"}, QDir::Files).size() > 0, "shortcut backup uses a valid Windows filename");
        QFile::remove(controller);
        events = run("Partial write", "--profile=other");
        bool progress = false; terminalCount = 0;
        for (int i = 0; i < events.property("length").toInt(); ++i) {
            auto event = events.property(i);
            if (!event.property("done").toBool() && event.property("ok").toBool()) progress = true;
            if (event.property("done").toBool()) { ++terminalCount; require(!event.property("ok").toBool(), "failure after partial progress cannot report success"); }
        }
        require(progress && terminalCount == 1, "partial write emits progress then exactly one terminal failure");
        require(tools.parseShortcuts().size() == 2, "partial failure reports existing writes without claiming rollback");
        events = run("  ", "");
        require(!events.property(0).property("ok").toBool() && events.property(0).property("done").toBool(), "native boundary rejects an empty trimmed name");
        require(tools.parseShortcuts().size() == 2, "invalid name does not change Steam files");
        const auto before = read(shortcuts);
        const QString grid = base + "/userdata/1/config/grid";
        require(QDir().rename(grid, grid + "-saved"), "temporary artwork directory moved aside");
        write(grid, "block artwork directory");
        events = run("Blocked artwork", "--profile=artwork");
        const auto artworkResult = events.property(events.property("length").toInt()-1);
        require(artworkResult.property("done").toBool() && !artworkResult.property("ok").toBool(), "actual artwork write failure cannot report terminal success");
        require(read(shortcuts) == before, "artwork failure stops shortcut updates");
        require(QFile::remove(grid) && QDir().rename(grid + "-saved", grid), "artwork directory restored after the controlled failure");
#ifdef Q_OS_WIN
        HANDLE locked = CreateFileW(reinterpret_cast<LPCWSTR>(shortcuts.utf16()), GENERIC_READ, FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
        require(locked != INVALID_HANDLE_VALUE, "temporary shortcut source denies backup reads");
        errors.clear();
        tools.updateShortcuts({});
        CloseHandle(locked);
        require(!errors.isEmpty() && read(shortcuts) == before, "failed backup prevents overwrite of an existing shortcut file");
        locked = CreateFileW(reinterpret_cast<LPCWSTR>(shortcuts.utf16()), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        require(locked != INVALID_HANDLE_VALUE, "temporary shortcut source denies write access");
        events = run("Blocked write", "--profile=blocked");
        CloseHandle(locked);
        const auto last = events.property(events.property("length").toInt()-1);
        require(last.property("done").toBool() && !last.property("ok").toBool() && last.property("msg").toString().contains("for writing"), "actual shortcut write failure cannot report terminal success");
        require(read(shortcuts) == before, "denied write retains existing shortcut bytes");
        write(controller, "\"controller_mappings\"\n{\n}\n");
        const auto beforeController = read(controller);
        locked = CreateFileW(reinterpret_cast<LPCWSTR>(controller.utf16()), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
        require(locked != INVALID_HANDLE_VALUE, "temporary controller config denies write access");
        events = run("Blocked controller write", "--profile=controller");
        CloseHandle(locked);
        const auto configResult = events.property(events.property("length").toInt()-1);
        require(configResult.property("done").toBool() && !configResult.property("ok").toBool() && configResult.property("msg").toString().contains("for writing"), "actual controller write failure cannot report terminal success");
        require(read(controller) == beforeController, "denied controller write retains existing config bytes");
#endif
        return 0;
    } catch (const std::exception &error) { std::cerr << "FAIL " << error.what() << std::endl; return 1; }
}
