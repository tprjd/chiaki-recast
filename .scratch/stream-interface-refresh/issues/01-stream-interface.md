# Implement the approved stream interface

Status: resolved
Type: task

Specification: [Stream menu, statistics, and session notices](../spec.md)
Design and testing scope: confirmed on 2026-10-21

## Request

Redesign the stream menu, statistics overlay, and session notices to match the
new Library, Settings, connection, and dialog design. The user requested the
prototype workflow on 2026-10-21.

## Design question

Which control arrangement gives clear access to stream settings while leaving
enough of the game visible?

The revised browser prototype compares A, a slim toolbar; B, small corner
controls; and C, an edge rail. Each layout has compact statistics and notices.
The menu keeps the existing volume, microphone, picture modes, zoom range, five
presets, Display settings, and conditional Custom renderer settings entry point.
Statistics include bitrate, packet loss, dropped frames, lost frames, average
queue depth, and pending frame age. All five notice cases have sample states.

## Primary source

- Branch: `codex/stream-interface-prototype`, based on `746218af`.
- Prototype: `gui/prototype/stream/`.
- URL: <http://127.0.0.1:8766/prototype/stream/?variant=A>.
- Current menu evidence: `.scratch/stream-interface-prototype/evidence/descriptive-v3/`.
- Selected statistics and notice evidence: `.scratch/stream-interface-prototype/evidence/compact-v2/`.
- Initial version: commit `2ee3cf81` and the evidence directory's root files.
- Source context: `gui/src/qml/StreamView.qml`, `StreamMenuWindow.qml`,
  `ConsoleButton.qml`, and `ConnectionView.qml`.
- Prior work: `.scratch/psn-dialog-refresh/spec.md` and `design.md`.

## Current verdict

The user rejected the first version as too obtrusive. They requested smaller,
more transparent overlays and said that the overlays need not follow the app's
main design language. This replaces the first version's design direction.

The user selected A for statistics and session notices. Keep those designs.
The user requested a slightly larger, more descriptive stream menu.
The third revision gives A larger controls and separate labels for Volume,
Microphone, Picture mode, and Quality preset. It keeps the translucent toolbar.
The user approved the third revision with "looks good" and requested the specification.
The final menu is A. Statistics and notices also use A. No design choice remains open.
A uses a slim row with picture and quality pop-ups. B uses a small corner panel.
C uses an edge rail with pop-ups. Selected statistics use A's single row.
Selected session notices use A's short text and one action when
needed. No notice dims the whole stream. The preview allows 45%, 65%, or 85%
background opacity while text remains opaque. Top tabs expose all three previews.

The [specification](../spec.md) is the implementation contract. Preserve the existing 900-pixel
minimum width, keyboard and controller support, and both renderer presentations.

## Prototype checks and limits

The browser self-check passed at 900×600, 1280×720, and 1920×1080, with 675
assertions and no failures. It checked bounds, sample controls, notice states,
privacy, initial Cancel focus, pop-up Escape behavior, opacity, and variant state.
It also checked the longest quality label and the fixed A statistics and notices.
The resting menus cover less than 20% of the preview stream at all checked sizes.
The small toolbar, corner controls, statistics, network notice, and blocked-content
notice were inspected visually. A bright backdrop was checked at 45% opacity.

Only prototype files and this record changed. No real settings or session
commands were used. The browser host toggle does not establish native renderer
behavior. Physical controllers, actual statistics, native focus and window
composition, and live session behavior remain unchecked.

## Comments

The native QML is the reference for behavior. The prototype is a layout study.
Do not promote its JavaScript or simplified sample state into production code.

On 2026-10-21, the user confirmed production QML tests with supplied session state
plus native checks for both renderer modes, transparency, window placement,
keyboard/controller key handling, and focus. Live console commands and
physical-controller testing remain separate. Implementation had not started when
the specification was published.

## Answer

Implemented on `codex/stream-interface-integration`. All five
[implementation tickets](../../stream-interface-implementation/map.md) are resolved.
The native application uses the approved larger A toolbar and compact A statistics
and notices. Existing session actions and settings contracts remain in place.

The final native checks passed for OpenGL (270) and Vulkan (251) at 900x600,
measured DPR 1. The normal production build passed. Both code review axes have no
remaining findings. Passing tests were retained under the user's reduced test scope.

The [acceptance record](../../stream-interface-implementation/evidence/ticket05/acceptance.md)
contains source identities, captures, test results, and build restoration evidence.
Live console/media/network effects, physical controllers, multiple displays, and
live display-scale changes remain unverified. Final Tool flyout resize/fullscreen
checks remain unverified; earlier host results are recorded separately.
No prototype code, release package, push, or pull request is part of this delivery.
