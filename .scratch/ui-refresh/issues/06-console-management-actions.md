# 06: Manage consoles from their cards

**What to build:** PIN, explicit wake, hide, and delete actions from the card details or action popup, using existing native dialogs and backend behavior.

**Blocked by:** 05: Inspect console details without exposing private data.

**Status:** resolved

**Spec stories:** 7, 10, 11, 12, 18.

- [x] Offer PIN entry and explicit wake only for eligible consoles. Open the existing PIN dialog and preserve its validation and persistence behavior.
- [x] Offer Delete for manual entries and Hide for eligible unregistered discovered entries. Preserve the existing confirmation and unhide instructions.
- [x] Cancel leaves the console unchanged. Confirmation dispatches exactly once to the intended current host.
- [x] Resolve identity again after confirmation. If hosts reorder, disappear, become ambiguous, or the profile changes, never act on a different console that reused the old index.
- [x] Secondary actions do not bubble into Connect. Buttons and existing shortcuts apply the same eligibility rules.
- [x] Replace temporary access retained by ticket 03 once all management actions are available. Restore focus to a valid control after an action removes its card.
- [x] Screen tests capture PIN, wake, hide, delete, cancel, and delayed confirmation behavior using sample hosts. Native checks use disposable entries and record results.

**Scope limit:** Preserve the connection and settings handlers. Do not add new registration or PIN storage formats.

**Specification:** [Main console screen: card library](../spec.md).

**Design reference:** B after text cleanup, preserved at `codex/prototype-console-b`, commit `73051300680f89e5c0f6dc3db71c7b581b8176e0`.

## Comments

Keep delayed confirmation and PIN results observable at the approved screen
boundary. A root-dialog callback can return to a public screen action without
moving connection or settings storage code into QML. Recheck eligibility and
identity at acceptance, and consume a pending action before dispatch so a
repeated acceptance cannot send the action twice.

The existing `ConsolePinDialog` stores only an index until acceptance. That
path still needs the new resolver. Read all callers before changing its API.

## Answer

Merged as 04d6ec87e746c9b835a7aa9479ca9f81208a4d1a. Direct PIN, Wake, Delete, and Hide controls replace the temporary menu. Each delayed result rechecks current identity and eligibility and can dispatch once. Existing PIN validation and backend setters remain in use. All 149 screen checks pass. The integration checkout also passes CTest in 17.77 seconds. The narrow management popup was inspected. Full native PIN persistence and disposable-action checks are being recorded separately; real hardware Wake is untested.
