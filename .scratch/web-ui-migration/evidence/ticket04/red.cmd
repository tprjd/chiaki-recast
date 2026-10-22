@echo off
call C:\Users\david\.cache\chiaki-msvc-native\environment.cmd
set "PATH=C:\Users\david\.cache\chiaki-msvc-native\ticket04-build\third-party\steam-tools;%PATH%"
set "CHIAKI_EMBEDDED_EVIDENCE=C:/Users/david/.codex/worktrees/web-ui-ticket-04/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket04"
C:\Users\david\.cache\chiaki-msvc-native\ticket04-build\gui\tests\chiaki-embedded-setup-tests.exe localHandoff -o C:/Users/david/.codex/worktrees/web-ui-ticket-04/chiaki-ng-ui/.scratch/web-ui-migration/evidence/ticket04/red-handoff.txt,txt
