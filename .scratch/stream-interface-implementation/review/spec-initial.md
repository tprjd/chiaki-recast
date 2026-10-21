Spec review of 73b14ded against 746218afec502872c4efe57aa02e9666c4858920.

1. [P1] Settings navigation loses focus after menu closure. Spec line 103 says, "Close the menu through its normal input-release path before navigation." In `gui/src/qml/StreamView.qml:317-329`, disabled video or blocked content makes the action notice return when `closing` clears. It grabs input and calls `focusNotice()` even after Display or Renderer settings opens. Reproduce with disabled video, Open stream menu, Picture, Display settings. The notice takes focus from settings after 250 ms. Without a notice, lines 500-507 also activate the real stream window and restore game focus after settings opens. Baseline activated only the hidden Quick host and did not restore an Item. The settings-action fixture records a string, so it cannot detect either problem.

2. [P2] Ctrl+O dismisses only the popup in the separate host. Spec line 98 says, "The existing stream-menu shortcut still toggles the menu." `gui/src/qml/StreamMenuWindow.qml:45-50` emits `closeRequested`, which `StreamView.qml:498` maps to `dismiss()`. With Picture or Quality open, this clears the popup and leaves the toolbar and input capture active. The controller menu event instead closes the whole menu. Current tests call `menuRequested()` and bypass the keyboard shortcut.

3. [P2] Popup controls lack visible focus. Spec line 64 requires "visible focus on each control." `gui/src/qml/StreamMenuContent.qml:91-103` uses the default Material ToolButton background, unlike the toolbar's explicit focus border. The supplied Qt 6.7 picture capture shows no focus mark on Zoom, although the test observes active focus there. Quality also shows no selected-state mark. A player cannot see which popup action Enter will activate.

4. [P1] Native popup focus is lost after an outside click. Spec line 98 requires "Preserve controller key mappings." The acceptance agent has already reproduced this in the new Tool popup path at `gui/src/qml/StreamMenuContent.qml:55-58`: the cached focus item can become a focus scope instead of the control. The next controller key fails. This is the known active fix, not a second discovery.

No additional missing feature or scope expansion found. These are implemented requirements with incorrect behavior.

Read-only review. No tests, builds, native cache changes, or desktop actions. Final fixes and acceptance records remain outside this snapshot.
