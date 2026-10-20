# Settings: sidebar layout

Status: resolved
Date: 2026-10-20

## Problem Statement

The console library has a new card layout. Settings still uses its earlier
tabbed layout and dense forms. The user wants Settings to use a consistent,
readable layout without losing existing controls or changing their behavior.

The user selected browser prototype A, the sidebar layout. They then asked to
see all existing settings in that prototype before any native application changes.
The expanded prototype is available for review. This specification records the
selected layout, complete settings scope, and approved test approach.

## Solution

Use the selected sidebar layout for native Settings. Keep the nine existing
categories visible beside the selected page at desktop widths. Group related
controls, show their units and defaults, and provide settings search.

Include all existing settings and the linked Display, Renderer, Color Mapping,
Profile, and Controller Mapping pages. Keep their native conditions, validation,
storage, and actions. Use the existing login, registration, and file operations.

Use the browser prototype as the layout reference. Use the native implementation
as the authority for behavior, available choices, defaults, and supported features.
The prototype displays conditional controls for review; the native UI must retain
the actual visibility and availability rules.

## User Stories

1. As a desktop user, I want Settings to match the console library, so that the application has a consistent appearance.
2. As a user, I want a visible category sidebar, so that I can change settings pages without searching through a long form.
3. As a user, I want the selected category to be clear, so that I know which settings I am editing.
4. As a user, I want related controls in named groups, so that I can scan each page.
5. As a user, I want all existing settings to remain available, so that the redesign does not remove functionality.
6. As a user, I want each value to show its unit and default where applicable, so that I can understand the setting.
7. As a user, I want search across categories and linked settings pages, so that I can find a setting by name.
8. As a user, I want useful empty search results and a clear search action, so that I can return to browsing.
9. As a user, I want search to respect feature availability, so that it does not expose unsupported controls.
10. As a user, I want conditional settings to appear under their existing conditions, so that I only edit applicable values.
11. As a user, I want settings changes to use the existing save behavior, so that navigation does not discard or unexpectedly apply values.
12. As a user, I want General to retain disconnect and suspend actions, so that my console responds as configured.
13. As a user, I want the existing audio/video enable modes, so that I can control which media the session uses.
14. As a Steam Deck user, I want the existing haptics and vertical-orientation controls, so that supported device behavior remains available.
15. As a user, I want streamer mode and log-folder access, so that I can manage privacy and inspect logs.
16. As a controller user, I want the stream-menu shortcut and all four button choices, so that I can retain my button combination.
17. As a user, I want decoder selection and zero-copy control, so that I can use the options supported by my device.
18. As a user, I want every window mode and custom width and height, so that I can control the stream window.
19. As a user, I want fullscreen, cursor, and vertical-sync controls, so that display behavior remains under my control.
20. As a user, I want every render preset, renderer backend, and queue-depth setting, so that the new layout preserves rendering choices.
21. As a user, I want renderer restart behavior to remain correct, so that settings take effect as expected.
22. As a user, I want separate PS4 and PS5 stream settings, so that changes for one console type do not change the other.
23. As a user, I want separate Local and Remote values, so that I can choose suitable quality for each connection route.
24. As a user, I want resolution, frame rate, bitrate, and eligible codec controls, so that I retain all stream-quality settings.
25. As a user, I want bitrate defaults and displayed units to remain correct, so that I do not apply an unintended data rate.
26. As a user, I want audio input and output devices, buffer size, and volume, so that I can configure session audio.
27. As a user, I want microphone startup behavior and supported speech processing, so that microphone behavior remains predictable.
28. As a user, I want noise and echo suppression levels when applicable, so that I can adjust speech processing.
29. As a user, I want Wi-Fi notification and packet-loss thresholds, so that connection feedback remains useful.
30. As a user, I want frame-recovery and stream-statistics controls, so that I retain the existing connection tools.
31. As a user, I want to register a console through Settings, so that the existing setup flow remains accessible.
32. As a user, I want to inspect registered consoles and select the auto-connect console, so that startup connects to the intended console.
33. As a user, I want console deletion to retain its confirmation, so that I do not remove a registration by mistake.
34. As a user, I want to see and unhide hidden consoles, so that I can restore them to the library.
35. As a keyboard user, I want all 26 key mappings and their reset action, so that my full control layout remains available.
36. As a user, I want keyboard-controller and mouse-touchpad toggles, so that I can choose how these devices control a session.
37. As a controller user, I want controller selection, mapping, and reset, so that the intended controller receives the change.
38. As a controller user, I want all mapping targets and alternate inputs, so that the redesign preserves complete controller mapping.
39. As a controller user, I want the existing analog-stick and L2/R2 mapping option, so that advanced mapping remains available.
40. As a controller user, I want background-input and button-position controls, so that input interpretation remains configurable.
41. As a controller user, I want D-pad touchpad emulation, its increment, and four shortcut choices, so that I can retain touchpad control.
42. As a controller user, I want rumble and true-haptics intensity controls, so that feedback remains configurable.
43. As a PSN user, I want login and token clearing under their existing conditions, so that I can manage the remote connection.
44. As a user, I want port-guessing controls and limits, so that remote connection options remain available.
45. As a user, I want profile selection, creation, and deletion, so that I can manage separate settings profiles.
46. As a user, I want profile changes to refresh the displayed values, so that I do not edit a stale profile.
47. As a user, I want settings import and export, so that I can retain the existing configuration transfer workflow.
48. As a user, I want sanitized logging, verbose logging, and About access, so that diagnostics and application information remain available.
49. As a user, I want all Display controls, including numeric peak and contrast values, so that I can configure the display target.
50. As a user, I want all Renderer groups and their presets, so that I can adjust scaling, debanding, sigmoidization, color, and peak detection.
51. As a user, I want renderer documentation and import/export actions, so that the existing advanced configuration workflow remains accessible.
52. As a user, I want all gamut and tone-mapping controls, so that Color Mapping retains its complete feature set.
53. As a user, I want Back to return to the correct parent page, so that nested settings remain easy to navigate.
54. As a user, I want Settings to return to the console library correctly, so that I can continue with the selected console.
55. As a keyboard user, I want visible focus and predictable navigation, so that I can operate every control without a mouse.
56. As a controller user, I want the existing navigation and button hints, so that Settings remains usable with a controller.
57. As a user with a narrow window or scaled display, I want readable controls and reachable actions, so that I can edit every setting.
58. As a user of a translated interface, I want long labels and values to remain usable, so that text does not hide controls.
59. As a user in streamer mode, I want private information masked in settings and search, so that the redesign does not expose it.
60. As a user in a streaming session, I want linked display and renderer controls to retain their existing entry points and behavior, so that the redesign does not interrupt session control.

## Implementation Decisions

### Layout and component boundaries

- Use layout A as the visual baseline: application navigation, Settings heading,
  current profile, search, category sidebar, and grouped forms.
- Retain General, Video, Stream, Audio/Wifi, Consoles, Keys, Controllers, Remote,
  and Config. Do not rename or merge categories as part of implementation.
- Use Qt Quick and the existing native controls and theme conventions. Do not
  ship a web view or browser prototype code.
- Keep styling changes within Settings and the included settings pages. Do not
  change a shared dialog wrapper in ways that alter unrelated screens.
- Keep the category sidebar available at desktop widths. At narrow widths,
  keep every category reachable without horizontal overflow of the page.
- Allow long pages to scroll. Keep controls, labels, defaults, and actions
  readable at supported window sizes and display scales.
- Retain the current profile display. Read it from native settings.
- Do not include the prototype comparison bar, reset-demo command, sample
  labels, synthetic data, or the discarded B and C layouts.

### Full settings coverage

- Treat the expanded browser inventory as a checklist, not a replacement for
  the native settings definitions.
- Preserve all controls in the nine categories, including platform-specific
  controls and controls hidden by another setting.
- Preserve all choices in static option lists. Populate device, decoder,
  console, and profile lists from their existing native sources.
- Keep all four stream-menu shortcut choices and all four D-pad mode shortcut
  choices. Do not reduce them to one display-only combination.
- Preserve all 26 keyboard mappings. Keep the native key-capture workflow,
  keyboard-controller toggle, mouse-touchpad toggle, and Reset All Keys action.
- Preserve all 23 controller mapping targets and primary and alternate inputs.
  Keep controller selection, analog mapping, apply, cancel, and reset behavior.
  Retain the existing handling of controllers managed by Steam.
- Preserve registered-console information, auto-connect selection, deletion,
  and hidden-console recovery. Keep confirmation and action-target behavior.
- Preserve profile creation, switching, and deletion restrictions. Do not allow
  actions against stale profile values after a profile change.

### Linked settings pages

- Include Display, Renderer, Color Mapping, Profiles, and Controller Mapping
  in the settings layout and coverage work.
- Preserve the relationships between parent and child pages. Color Mapping
  returns to Renderer; Renderer and Display return to their correct caller.
- Retain all six Display controls, 39 visible Renderer controls and actions,
  and 28 Color Mapping controls in the current inventory.
- Keep existing conditional groups, presets, numeric limits, and defaults.
- Preserve renderer documentation and renderer-settings import/export.
- Keep the permanently hidden documentation URL implementation detail hidden.
  Preserve the visible documentation and copy actions.
- Maintain linked display and renderer entry points used during streaming.
  Do not replace their window or overlay behavior merely to match the browser.
- Use existing registration, PSN login, About, confirmation, and operating-system
  file dialogs. Their entry points are included; redesigning those flows is not.

### Value handling and side effects

- Use existing settings and backend handlers as the authority. Preserve setting
  keys, profile storage, import/export formats, defaults, and validation.
- Keep the current update timing of each control. Immediate controls remain
  immediate; validated text and mapping operations retain their existing
  commit or apply behavior. Do not add a global Save or Apply transaction.
- Preserve numeric precision, bounds, and step behavior. Retain the existing
  conversions between displayed units and stored values.
- Keep Local and Remote values separate for each console type. PS5 codec
  controls must not create unsupported PS4 codec settings.
- Preserve automatic bitrate behavior and resolution-based defaults. A displayed
  default must not become an explicit stored override merely by opening a page.
- Keep reset behavior specific to its existing action. Do not add a global
  settings reset or copy the prototype's Reset demo command.
- Retain native renderer restart behavior. Changing pages, constructing a
  control, or searching must not write settings or restart the application.
- Do not change discovery, authentication, codecs, streaming, or controller
  processing to support the layout.

### Conditions, search, and privacy

- Retain native build, platform, device, and dependent-setting conditions.
  The prototype's always-visible conditional controls are for inventory review.
- Search names and relevant setting descriptions across the included settings
  pages. Match without case sensitivity and ignore surrounding whitespace.
- Show category and group context in search results. Preserve native validation
  and action routing when a result exposes a control or opens its settings page.
- Unsupported controls must not become editable through search. For a supported
  conditional setting, either retain its native visibility or explain its
  prerequisite without bypassing that prerequisite.
- Clearing search restores category browsing. Selecting a category opens that
  category and clears the query. An empty query shows the normal settings page.
- Keep search transient. Do not create a persistent search database or index
  account credentials, tokens, log contents, or arbitrary settings values.
- Preserve streamer-mode masking in visible text, search results, accessible
  names, tooltips, and console or profile fallback descriptions.

### Navigation and accessibility

- Keep mouse and keyboard use as the desktop priority. Preserve existing
  controller navigation and shortcuts.
- Give each control an accessible name. Explain units and conditions in text;
  do not communicate state through color alone.
- Keep focus visible. Tab, Shift+Tab, and existing directional navigation must
  reach the appropriate controls without trapping the user.
- Text inputs, numeric inputs, and option lists retain their editing keys.
  Navigation shortcuts must not also change a value or dispatch an action.
- Restore focus after closing a child page, popup, or capture dialog. If the
  original item no longer exists, focus a valid nearby control.
- Escape closes the active popup or capture flow first. Preserve the existing
  back behavior for the containing page. Do not let one event close multiple
  layers or trigger an action in the console library.
- Keep the selected console and appropriate library focus when returning
  from Settings.
- Translate user-facing text with the existing Qt translation mechanism.

### Delivery

- Keep the browser prototype available as the design reference during native
  implementation.
- Build and review runnable native results in bounded steps: sidebar and
  navigation, full form coverage, linked pages, then final native verification.
- Preserve the existing uncommitted console-library corrections. They are
  separate work and must not be removed, restaged wholesale, or replaced.
- No storage migration, new backend API, dependency, global theme system,
  or generic schema-driven form engine is required by this specification.

## Testing Decisions

### Approved primary test boundary

The user approved this approach during specification preparation: test the
production QML Settings screen and its linked settings pages through the existing
Qt Quick harness, using fake settings and action handlers, then check the result
in the native Windows application.

- Use one primary boundary: the production Settings UI with its settings and
  backend collaborators supplied by the test harness.
- Reuse the existing Qt Quick Test setup, production-QML loading pattern,
  synthetic fixtures, input simulation, and observable action assertions.
  The console-library screen tests provide this prior art.
- The current harness tests the console library; it does not already verify
  Settings. Extend it with Settings fixtures and cases.
- Supply the minimum QML context, types, and fake collaborators needed to load
  the production settings pages. Do not build a second settings implementation
  for tests or test only a copied form.
- If a direct global dependency prevents isolation, add the smallest UI-level
  injection point needed. Keep production defaults connected to the existing
  native backend. Do not introduce a new domain or storage layer.

### Observable behavior

- A good test performs a user action and checks the displayed result, the
  intended settings write, or the requested native action.
- Do not assert private helper structure, exact child order, or a particular
  component decomposition.
- Check every setting's destination and value conversion with data-driven
  cases at this screen boundary. Opening pages and searches must cause no writes.
- Check all categories, child-page navigation, Back, return to the library,
  focus restoration, and popup or capture cancellation.
- Check platform and feature conditions, dependent controls, preset changes,
  live device-list updates, empty lists, and long translated labels.
- Check PS4 and PS5 Local and Remote isolation, eligible codecs, automatic
  bitrate defaults, numeric boundaries, invalid text, and displayed units.
- Check all shortcut slots, complete keyboard and controller mapping lists,
  capture routing, reset behavior, apply, and cancel.
- Check console actions against the intended identity after list changes.
  Check exclusive auto-connect selection and confirmation cancellation.
- Check active-profile changes, protected profile deletion cases, token-state
  changes, import/export dispatch, and renderer restart requests.
- Check settings search across linked pages, clear and empty results, native
  conditions, and privacy masking.
- Keep test settings separate from user settings. Fake handlers must not
  authenticate, connect, delete registrations, overwrite files, or restart
  the user's application.

### Coverage and native verification

- Compare the final control inventory with the current native implementation.
  Include conditional controls, repeated mapping entries, actions, options,
  limits, defaults, and conversions.
- Do not treat a property-write count alone as proof of full coverage. Inspect
  read-only information, dynamic lists, and action handlers as well.
- Use the prototype inventory checks as supporting evidence. They are not
  native acceptance tests.
- Build and launch the native Windows application. Inspect the real Settings
  UI at 1280 by 720 and 900 by 600 logical pixels. The supported minimum width is 900 pixels.
- Check 100%, 150%, and 200% display scaling, long labels, scrolling, and
  controls at the edges of the viewport.
- Compare the native forms with the selected sidebar reference at matching
  sizes. Verify readable text, group spacing, focus, and reachable actions.
- Check actual settings persistence using an isolated test profile. Check
  navigation from the console library and the linked settings entry points.
- Check physical controller input and stream-time display or renderer controls
  where hardware is available. Record unavailable cases as untested.
- Run the existing relevant screen tests to detect regressions in the console
  library and shared controls.
- Report actual results and limitations. A successful build or browser test
  does not prove correct native behavior.

## Out of Scope

- Android UI changes.
- Further console-library redesign or unrelated dialogs and screens.
- A new streaming menu or overlay layout. Existing linked settings entry points
  must continue to work.
- Redesign of registration, PSN authentication, About, confirmation, or native
  file-picker flows.
- Changes to settings storage, profiles' data format, import/export formats,
  protocol behavior, codecs, authentication, or controller processing.
- New settings, presets, account features, telemetry, or performance tuning.
- Global theme replacement or a new frontend framework.
- Shipping sample values, fake action handlers, the prototype review bar,
  or an always-visible mode for unsupported controls.
- Implementing the discarded tabbed and expandable layouts.
- Starting native implementation as part of this specification-writing task.

## Further Notes

- The user explicitly selected A, the sidebar layout.
- The user requested the full settings inventory in the browser before native
  changes. The expanded inventory is a review artifact; no native Settings
  change has been made in this conversation.
- The requested native follow-up is described here. Publishing this specification
  does not itself start implementation.
- The user explicitly approved the proposed Qt Quick screen test boundary and
  native Windows checks during this specification task.
- Prototype: [Settings sidebar](http://127.0.0.1:8765/prototype/settings/?variant=A).
- Preserved prototype branch: `codex/prototype-settings-a`.
- Preserved prototype commit: `b188338`.
- The recorded browser run passed 297 checks at widths of 1280, 800, and 390
  pixels. It covered all categories and linked settings pages.
- The source inventory check covered 70 setting writes in the main Settings
  screen, four in Display, 34 in Renderer, and 28 in Color Mapping. These are
  write counts per screen, not a unique-settings total.
- The inventory also checked the 26 keyboard mappings and the advanced control
  counts, option lists, and numeric limits. It excluded the permanently hidden
  renderer-documentation URL field.
- Browser values are synthetic. Real login, registration, file operations,
  controller capture, and native persistence were not exercised by those checks.
- Existing console-library build and test results belong to that earlier work.
  They are not acceptance evidence for this Settings redesign.

## Implementation scope update, 2026-10-20

The user set 900 pixels as the minimum supported width during implementation. Verify native Settings at 1280 by 720 and 900 by 600, including long labels, scrolling, and focus. Do not optimize or require acceptance below 900 pixels. Existing smaller-width evidence remains historical and does not define the delivery scope.


## Answer

Implemented on `codex/settings-refresh-integration` at `e0e33b830d0f5f7f8eb5101657b16845e1f4d9ba`. All 13 tickets are resolved. The main checkout contains the final Settings implementation and preserves the original uncommitted console-library work.

The combined screen suite passes 562 checks, final focused corrections pass, and native acceptance passes both supported sizes at measured 100%, 150%, and 200% scale, including expanded translated text. The normal executable builds after removal of test instrumentation. See [native acceptance and launch links](native-acceptance.md), [review](final-review.md), and [ticket map](map.md).

Hardware and live-stream checks were unavailable. The acceptance report records the remaining native-operation and OS-scaling limits.
