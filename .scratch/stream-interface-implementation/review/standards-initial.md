Reviewed candidate `73b14ded` against production baseline `746218afec502872c4efe57aa02e9666c4858920`.

No hard documented-standard violations found. The changes follow the local issue layout, domain vocabulary, supplied-state test boundary, and production-resource separation described in `AGENTS.md`, `docs/agents/issue-tracker.md`, `docs/agents/domain.md`, `GLOSSARY.md`, and `gui/tests/README.md`. No relevant ADR exists.

One low-priority judgment call:

- Possible **Duplicated Code**, `gui/src/qml/StreamMenuContent.qml:188,315–320`. The toolbar has `text: [qsTr("Default"), qsTr("High quality"), ...][streamWindow.videoPreset - 1]`. The popup separately defines `model: [{... label: qsTr("Default"), preset: ChiakiWindow.VideoPreset.Default}, ...]`. The same five names and their order must stay aligned. Reuse the existing popup model for the toolbar label and initial preset selection. This is a maintenance suggestion, not a current failure or a hard rule.

I read all changed production and test logic, plus connected window input, enum, metric, and test contracts. The shared menu and notice boundaries have a defined current purpose. I found no other actionable baseline smell.

No builds or tests run, as requested. Generated evidence logs were excluded from style review. The known outside-click popup-focus defect remains with the acceptance agent.

