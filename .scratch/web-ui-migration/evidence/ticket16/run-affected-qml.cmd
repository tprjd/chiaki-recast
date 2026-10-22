@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket16-build\third-party\steam-tools;%PATH%
set QT_QPA_PLATFORM=offscreen
set QT_QUICK_BACKEND=software
set QT_QPA_FONTDIR=C:/Windows/Fonts
set QT_QUICK_CONTROLS_CONF=C:/Users/david/.codex/worktrees/web-ui-ticket-16/chiaki-ng-ui/gui/src/qml/qtquickcontrols2.conf
C:\Users\david\.cache\chiaki-msvc-native\ticket16-build\gui\tests\chiaki-console-screen-tests.exe -input C:/Users/david/.codex/worktrees/web-ui-ticket-16/chiaki-ng-ui/gui/tests/qml/tst_SettingsSearch.qml SettingsSearch::test_everyCategoryAndStreamRoute SettingsSearch::test_liveConditionsAndPrivacy SettingsSearch::test_searchFindsAndRevealsAcrossCategoriesWithoutWrites -o C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket16-green03/qml.txt,txt

