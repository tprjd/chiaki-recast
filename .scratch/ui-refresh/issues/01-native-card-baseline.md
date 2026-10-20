# 01: Connect through a native console card

**What to build:** A runnable native card for a registered, ready console, connected to the existing streaming handler. Establish the focused screen test setup used by later tickets.

**Blocked by:** None (can start immediately).

**Status:** resolved

**Spec stories:** 1, 2, 16.

- [ ] Locate or configure the Windows Qt toolchain using the current build workflow. Record reproducible build, launch, and screen-test commands.
- [ ] Record existing main-screen and connection behavior before changing it. Preserve the existing screen and commands while the replacement is incomplete.
- [ ] Render the production QML card with console name, existing artwork, status text, and a focusable Connect action. Use functional, translatable text.
- [ ] Supply host data and observe action signals at one QML screen test boundary. The test captures the intended source host and one action per activation.
- [ ] Connect the card to the existing native handler and demonstrate a registered local console connection. Record a hardware blocker explicitly if this cannot be exercised; do not claim the native acceptance check passed.
- [ ] Add only the Qt Quick Test setup needed for the real screen, under the existing test option. Keep fake hosts and test dependencies out of application packaging.
- [ ] Preserve streamer-mode masking for displayed values, including any address used as a name. Support mouse activation, visible keyboard focus, and Enter without duplicate dispatch.

**Scope limit:** This ticket establishes one working vertical path. It does not add search, redesign Settings, or replace the connection engine.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Answer

Implemented in `4ebb436`, merged as `9afeb20` on
`codex/ui-refresh-integration`. The production QML card uses the existing
Connect handler. The old screen retains its other commands until ticket 03.

The C++ Qt Quick Test harness built with MSVC 19.44 and Qt 6.9.3. CTest passed
the mouse target, single Return action, and streamer-mode masking checks.
`gui/tests/README.md` records the build and test commands. The integration
agent repeated the CTest check after merging.

The full application build is in progress in a separate cache checkout.
A real console connection remains untested; ticket 08 must record this limit.
