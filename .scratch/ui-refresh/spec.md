# Main console screen: card library

Status: resolved
Date: 2026-10-20

## Problem statement

The current main console screen uses full-width rows and a toolbar with large,
fixed-width buttons. Console selection, technical details, and management
actions compete for space. The user wants to improve this screen in small
steps, using a browser design before changing the native application.

The user selected prototype B, Card library, after removal of slogans and
decorative text. Desktop mouse and keyboard use is the first priority.

## Solution

Build B as a native Qt/QML screen. The screen has a compact navigation sidebar,
a Consoles heading, search, Add console, and a responsive grid of console cards.
Each card shows the console artwork, name, status, connection route, primary
action, and a details control. Discovery and console counts appear below the
library. Settings opens the existing settings screen.

Use the prototype's layout, colors, spacing, and functional text as the visual
baseline. Use the existing backend as the authority for available actions and
connection behavior. Keep the browser prototype's comparison controls and
sample data outside the application.

## User stories

1. As a desktop user, I want to identify consoles by name and artwork so that
   I can select the correct console.
2. As a desktop user, I want a visible connection action on each card so that
   I can start a session without opening its details.
3. As a user with several consoles, I want to search by name or console type
   so that I can find a console quickly.
4. As a user, I want accurate status text so that an unknown state is not
   presented as an offline console.
5. As a user of manual connections, I want to connect when discovery cannot
   confirm the console's status.
6. As a PSN user, I want to identify and connect to remote consoles without
   requiring a local address or local discovery status.
7. As a user with a console in standby, I want the existing wake-and-connect
   flow to remain available.
8. As a new user, I want an unregistered console to open the existing
   registration flow, including automatic registration when available.
9. As a user, I want Add console to open the existing manual-host dialog.
10. As a user, I want the details control to show available console information
    and management actions without starting a connection.
11. As a user, I want to delete manual entries or hide eligible discovered
    consoles with the existing confirmation and recovery behavior.
12. As a user, I want to update the console PIN through the existing dialog.
13. As a user, I want to control local discovery and refresh PSN consoles
    through clearly named controls.
14. As a user, I want Settings and supported Steam shortcut actions to remain
    accessible from the main screen.
15. As a user with a narrow window or a scaled display, I want cards and
    controls to remain readable and reachable.
16. As a keyboard user, I want visible focus, predictable navigation, and
    editable search text without accidental console actions.
17. As a controller user, I want the existing main-screen actions to remain
    available after the layout changes.
18. As a user, I want discovery updates and search changes to preserve the
    intended console target, including while a menu or dialog is open.
19. As a user with no visible consoles, I want a clear explanation and an
    appropriate next action.
20. As a user in streamer mode, I want private console identifiers to remain
    hidden throughout the new screen.
21. As a user, I want current settings, startup reminders, automatic connection,
    and return-from-stream behavior to keep working.
22. As a user, I want functional labels without slogans or repeated tips.

## Implementation decisions

### Design and scope

- Implement the card-library design in Qt Quick and Qt Quick Controls.
- Reuse existing console artwork, the SVG image provider, controls, dialogs,
  theme conventions, and connection handlers.
- Keep screen-specific styling local to this screen. Do not introduce a
  global design system, web view, new frontend framework, or persistent UI model.
- Use a responsive native grid. Filtered or hidden entries must leave no blank
  slots. Use vertical scrolling when the cards exceed the available height.
- Collapse the sidebar to accessible icon controls when width is limited.
  Keep Settings and Add console reachable at the narrowest supported size.
- Allow names to wrap within consistent cards. If a name must be elided, expose
  its full value through an accessible name and a tooltip or details view.
- Translate new user-facing strings through the existing Qt translation
  mechanism. Status must be conveyed by text as well as color.
- Show neutral console artwork or a neutral type label when the model lacks
  the console type. Do not infer PS4 from a missing PS5 flag.

### Host data and action targets

The backend currently returns a combined list of discovered, manual, and PSN
hosts. Some entries intentionally have `display` set to false. Some manual
entries duplicate a discovered console and must remain suppressed.

- Derive the displayed cards from the existing host list. Respect `display`
  before applying search.
- Search console names and known console types. Keep backend ordering and
  duplicate suppression. Do not add sorting or favorites.
- Keep display positions separate from backend host indices. A filtered card's
  position must never become the target index for an action.
- Preserve selection by console identity when possible. If the selected entry
  disappears, select a remaining visible entry or clear the selection.
- Resolve the current target when an action executes, including after a
  confirmation dialog. If the target disappeared or became ambiguous, cancel
  the action safely. Never act on another entry that reused its old position.
- Cancel or update open details when their target disappears. Clear saved
  target state when the active profile changes.
- Prefer a small view-local adapter over a new C++ model. Add a backend identity
  field only if existing host identity fields cannot meet the action-target
  checks. No storage or protocol changes are expected.

### Status and connection behavior

| Actual host case | Display | Primary action |
| --- | --- | --- |
| Registered, discovered, ready | Online; Local network | Connect through the existing handler |
| Registered, discovered, standby | In standby; Local network | Wake and connect through the existing connection handler, with the nickname |
| Unregistered, discovered | Not registered; automatic-registration indication when available | Register through the existing connection entry point |
| Registered manual host, not discovered | Status unknown; Manual address | Connect; retain the existing separate wake action |
| Unregistered manual host | Not registered; Manual address | Register through the existing connection entry point |
| PSN remote host | PSN; no invented local power status | Connect through the existing PSN flow |
| Hidden or suppressed duplicate | No card | No action |
| Missing or unrecognized discovery state | Status unknown | Preserve backend-supported actions |

The browser sample's Offline/Unavailable card is illustrative. Absence of a
discovery result is not proof that the console cannot accept a connection.

The current native connection handler already wakes discovered standby
consoles and continues the connection flow. Preserve that behavior rather than
requiring the prototype's separate wake click followed by a connect click.
Never mark a real console online solely because a wake request was sent.

### Controls and details

- Add console opens the existing manual-host dialog. Registration and PIN entry
  use the existing dialogs and validation.
- The details control exposes available address, ID, registration, connection
  route, running application, and title ID information. Omit unavailable fields.
- Preserve streamer-mode masking in the cards, details, tooltips, accessible
  descriptions, and any fallback text. A manual entry can use its address as
  its name; that fallback also needs masking.
- Offer Delete for manual entries and Hide for eligible unregistered discovered
  entries. Preserve the existing confirmation text and unhide instructions.
- Keep eligible PIN and explicit wake actions available in the card's details
  or action menu. Clicking a secondary action must not also connect.
- The prototype's generic Refresh becomes **Refresh PSN consoles**, visible
  when the existing PSN authentication condition permits it. Call the existing
  PSN refresh handler. Local discovery remains automatic and retains its toggle.
- Keep Steam shortcut creation accessible when the build supports it. Keep
  the existing application version display and quit path accessible without
  adding decorative content.
- Preserve startup reminders, automatic connection, and existing PSN and stream
  navigation. Settings contents and streaming overlays are outside this change.
- Count only visible host entries in the library total. Show a filtered count
  during search. An online count, if retained, includes only confirmed ready
  discovery results, not unknown or PSN-only entries.

### Keyboard, focus, and empty states

- Tab and Shift+Tab reach the sidebar, search, toolbar, card actions, and
  discovery control in visual order.
- Arrow keys move between cards when the card grid has focus. Search retains
  text-editing keys. Enter in a form or menu must not also connect a console.
- Preserve existing controller shortcuts and controller-button hints for the
  selected console. Apply the same eligibility rules to buttons and shortcuts.
- Closing a menu or dialog restores focus to its opener or a valid nearby
  control if the opener no longer exists.
- Escape closes an active popup first. In search it clears a nonempty query.
  Otherwise retain the existing main-screen quit confirmation.
- Distinguish no visible consoles, discovery disabled, and no search matches.
  Offer Add console, Enable discovery, or Clear search as appropriate.
- Empty lists and updates that hide every entry must not cause a navigation
  loop or an action against a stale selection.

### Step-by-step implementation

Each stage ends with a runnable native result and a review of that result.
This sequence is the implementation plan, not a claim that work has started.

1. **Establish the native baseline and a working card.**
   Locate or configure the Windows Qt build using the current build workflow.
   Record the existing main-screen and connection behavior. Introduce the
   smallest real QML card-library component with supplied host records and
   action signals. Connect a real registered console through it. Add the
   focused Qt Quick test harness here, so later stages use the same screen.
   This stage is complete when the app launches, a real connection path works,
   and a sample-card action reports the intended host.
2. **Replace the list with B's responsive library.**
   Add the sidebar, title, Add console control, card artwork, status, and primary
   action. Connect all supported host cases from the table above. Preserve
   startup behavior, Settings, discovery, and optional commands. Remove the old
   list after its actions have replacements. Check a normal and a narrow window.
3. **Add search and reliable live updates.**
   Filter the real host records without losing source identity. Add counts,
   the empty states, and stable selection. Test hidden duplicates, reordered
   host lists, profile changes, and deletion of the selected console. A filtered
   action must always target the intended host.
4. **Complete details and input behavior.**
   Add the compact details or action popup and connect existing PIN, wake,
   hide, and delete flows. Complete privacy masking, keyboard navigation,
   focus restoration, accessible names, and controller shortcuts. Test target
   changes while a confirmation is open. Basic usable focus is required in
   earlier stages; this stage completes the full input checks.
5. **Verify and finish the native screen.**
   Compare the native screen with the approved B reference. Check Windows
   display scaling, long names, missing fields, and large host lists. Complete
   native discovery, registration, PSN, standby, and streaming checks with
   available hardware. Record results and any untested hardware cases. Remove
   temporary preview fixtures from production resources and review the final diff.

Stages run in order. Do not expand a later stage into a settings redesign or a
connection-engine rewrite. Implementation tickets can be derived from this
sequence in a separate planning step.

## Testing decisions

The user approved testing the real QML console screen with sample host data
and action-target assertions, followed by native discovery and streaming checks.

### Automated screen checks

Use one primary test boundary: the production QML card-library screen with
supplied host data and observable action signals. A minimal Qt Quick Test
harness supplies sample hosts and captures actions without opening network
sessions. Test the screen's public behavior rather than private layout details.

The repository's existing native test target uses munit for core C behavior.
The inspected test setup does not provide a QML screen harness. Add only the
Qt Quick Test setup needed for this screen, under the existing test option.
Keep test dependencies out of normal application packaging.

Required checks:

- Ready, standby, manual unknown, unregistered, PSN, missing-type, and
  missing-state records render appropriate labels and enabled actions.
- Hidden and duplicate-suppressed records never produce cards or grid holes.
- Filtering to the second or later source entry still dispatches the correct
  source target. Reordering or removing hosts before confirmation cannot
  redirect an action to another console.
- Connect, Register, Wake, PIN, Hide, Delete, Settings, Add console, and
  conditional PSN/Steam commands dispatch only their intended action.
- Search handles spaces, mixed case, clearing, no matches, and live updates.
- Focus is visible and restored after dialogs. Keyboard events do not cause
  two actions. Navigation terminates with an empty or fully hidden list.
- Streamer mode conceals private values in all visible and accessible text,
  including an address used as a console name.
- Count labels agree with visible and filtered records.
- Long text and many consoles remain usable at 1280 × 720 and 800 × 600 logical
  pixels. Add a narrower check if the supported main-window minimum allows it.

### Native checks

- Build and launch the changed application. A successful build alone is not
  proof of correct screen behavior.
- Compare against B at matching viewport sizes. Check 100%, 150%, and 200%
  Windows scaling without clipped actions or unreadable text.
- Check a real local ready console, a real standby console, manual connection,
  registration, PSN connection, and return from a stream where hardware and
  accounts permit. Record unavailable cases explicitly.
- Check mouse, keyboard, and an available controller, including input after
  returning from Settings, PIN entry, and connection errors.
- Run the existing relevant native tests when backend code changes. Browser
  prototype checks remain design evidence, not native acceptance tests.

## Out of scope

- A settings-page redesign, Android UI changes, or streaming-overlay changes.
- A web-based application shell or shipping the prototype JavaScript.
- Changes to discovery protocols, PSN authentication, codecs, or streaming.
- Favorites, sorting, game-cover downloads, new account features, or UI telemetry.
- A new persistent host database, broad model refactor, or global theme system.
- Shipping the A/C alternatives, design switcher, or sample-state controls.

## Further notes

### Approved references

- Prototype: `gui/prototype/`, variant B after the copy cleanup.
- Screenshot: `.scratch/ui-refresh/previews/B-card-library.png`.
- Decision record: `.scratch/ui-refresh/design-notes.md`.
- Prototype preservation branch: `codex/prototype-console-b`.

These are repository-relative artifact references, not implementation file
assignments. The implementation should use the current code organization.

### Initial verification status

The prototype passed 86 browser checks before this specification. That result
does not verify any native behavior. No Qt build, native UI test, or hardware
connection test was run while preparing this specification.

No CMake, qmllint, or qmltestrunner command was found on the current shell's
PATH. No root build directory was found. These checks do not prove that the
tools are absent from the computer. The first stage must locate or configure
the toolchain. The current Windows build workflow uses Qt 6 and is a more
relevant starting point than the older platform-build prose.

## Answer

Status corrected on 2026-10-21. All eight implementation tickets are resolved.
The [final ticket](issues/08-native-acceptance-and-polish.md) records integration
at `43a839d8c528c574f6e2e2e7ede961c253cf3c3b`.
The [acceptance record](acceptance.md) contains the native screen checks and
hardware limits. The [final review](code-review.md) reports no findings.
The initial verification notes above describe the specification stage.
