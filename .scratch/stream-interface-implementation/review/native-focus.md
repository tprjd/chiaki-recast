# Native popup focus diagnosis

The initial review identified the outside-click failure. Further source inspection
showed that the cached control was correct. Qt's native activation state was the
cause.

In Qt 6.11.2, `QQuickDeliveryAgentPrivate::setFocusInScope` permits root focus only
when `QGuiApplication::focusWindow()` matches the window. `QWindow::isActive()`
does not establish this condition. An owned Tool window can report active while
the main stream window still has Qt's keyboard focus. A synthetic FocusIn event
and `forceActiveFocus()` cannot override the condition.

The fix retains each captured key until `focusWindowChanged` identifies the
target. It then uses normal window event delivery. Target closure, destruction,
session change, or a different focus destination must cancel the retained event.
Event dispatch can close the target immediately, so cleanup must not access a
deleted pending-event object after dispatch.

Primary sources:

- [Qt delivery-agent focus rules](https://github.com/qt/qtdeclarative/blob/v6.11.2/src/quick/util/qquickdeliveryagent.cpp)
- [Qt native focus processing](https://github.com/qt/qtbase/blob/v6.11.2/src/gui/kernel/qguiapplication.cpp)

The first physical check passed 26 assertions at 900×600 logical pixels and DPR 1.
The final acceptance record owns the source identity and the results after all
review fixes. This diagnosis does not replace that record.
