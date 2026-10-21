Spec review of final code candidate 62f9676e against production baseline 746218afec502872c4efe57aa02e9666c4858920.

All code findings are resolved:

- Linked settings open after menu closure. The pending target and Main.streamSettingsOpen prevent notices or menu focus restoration from taking settings focus.
- Ctrl+O closes the whole menu from either the native toolbar or native flyout. Their shortcut handlers are mutually exclusive. Escape retains popup-first dismissal.
- Flyout buttons have explicit focus borders and selected-state backgrounds.
- Native Tool input waits for Qt to report window focus. The cache retains popup controls, not focus scopes. Cloned events survive reentrant closure, and pending callbacks cancel when their window hides or the session changes. No additional confirmed lifetime or ordering defect remains.
- Disconnect now cancels pending settings navigation. An already-queued callback checks the current target and both pending and open disconnect states before navigation. This resolves the priority race found in 1246ab6d.

No remaining missing requirement, scope expansion, or incorrect implementation identified within the reviewed changes. The parent agent retains responsibility for final acceptance evidence, actual Main navigation checks, and the normal production build. Earlier passing checks remain valid under the user's restricted rerun scope.

Read-only review. No tests, builds, native cache changes, or desktop actions performed. This code review does not establish live media, real console commands, physical-controller operation, or final desktop composition.
