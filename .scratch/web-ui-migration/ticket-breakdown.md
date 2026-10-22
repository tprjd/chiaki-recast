# Approved web UI migration tickets

Status: ready-for-agent
Approval: the user accepted the Astra High review on 2026-10-22.
Publication: 23 implementation tickets in the local Markdown issue tracker.
Source: [Web setup UI specification](spec.md).
Review: [Astra High recommendations and original-to-new mapping](ticket-review.md).

The tickets deliver GPU-accelerated web setup and separate native streaming
video, input, and overlays. There is no QML setup fallback or runtime UI selector.
The three approved merges replace the original 26-ticket proposal.

Each ticket has its own acceptance criteria, specification story references,
and blocking edges. Together the tickets cover all 95 user stories.
All published tickets have ready-for-agent status. A blocked ticket must wait
until its blockers are complete. Ticket 01 is the initial implementation
frontier. This publication does not start implementation.

## Ticket order and blockers

1. [Preserve native playback with application-owned services](issues/01-application-owned-native-services.md)

   Blocked by: none.
   Delivers: The current application can discover consoles, connect, stop, and connect again after backend and QML engine lifetime are separated from the native window.

2. [Run native playback with a stream-only interface](issues/02-native-stream-only-interface.md)

   Blocked by: 01.
   Delivers: A direct stream launch displays native video, overlays, console login PIN prompts, and live settings without loading the application setup UI.

3. [Show live consoles in the GPU-accelerated web UI](issues/03-accelerated-web-console-library.md)

   Blocked by: 01.
   Delivers: Normal startup opens a packaged Svelte setup window that uses GPU acceleration and displays real discovered consoles through Qt WebChannel.

4. [Connect locally from web setup to native playback](issues/04-local-native-stream-handoff.md)

   Blocked by: 02, 03.
   Delivers: A user selects an active registered local console in the web UI, streams in a separate native window, disconnects, and returns to setup.

5. [Manage consoles and manual addresses from the web library](issues/05-console-management-and-manual-addresses.md)

   Blocked by: 04.
   Delivers: Users can inspect and manage consoles, save manual addresses, associate existing registrations, update a saved console login PIN, and connect a registered manual host through native playback.

6. [Sign in to PSN independently from web setup](issues/06-psn-sign-in.md)

   Blocked by: 03.
   Delivers: Users can sign in through an embedded or external browser, complete sign-in with a redirect URL, and see native account status in web setup.

7. [Pair PS4 and PS5 consoles and retrieve an Account ID](issues/07-pairing-and-account-id.md)

   Blocked by: 04, 06.
   Delivers: Users can register a discovered or manually entered PS4 or PS5 console, retrieve an Account ID through public lookup or sign-in without losing valid form input, and connect after native registration succeeds.

8. [Stream from a registered console through PSN](issues/08-psn-native-streaming.md)

   Blocked by: 04, 06.
   Delivers: Users can refresh remote consoles, connect to a registered console through PSN, and return to web setup after native playback ends.

9. [Register a console automatically through PSN](issues/09-automatic-psn-registration.md)

   Blocked by: 08.
   Delivers: Users can choose the existing automatic PSN registration method and receive its native result while remaining in web setup.

10. [Wake consoles and auto-connect from web startup](issues/10-wake-and-auto-connect.md)

   Blocked by: 08.
   Delivers: Users can wake consoles and use or stop auto-connect for the selected local or PSN target.

11. [Open settings and change general session options](issues/11-general-and-stream-settings.md)

   Blocked by: 03.
   Delivers: Users can open the settings sidebar, search implemented controls, and save general and session-behavior settings through native services.

12. [Configure stream profiles and audio devices](issues/12-video-and-audio-settings.md)

   Blocked by: 11.
   Delivers: Users can set local and remote PS4 and PS5 video profiles and configure native audio devices and audio options.

13. [Configure display targets and the native renderer](issues/13-display-and-renderer-settings.md)

   Blocked by: 11.
   Delivers: Users can configure display targets, renderer selection, presets, and custom native renderer options from web setup.

14. [Configure native color mapping from web setup](issues/14-color-mapping-settings.md)

   Blocked by: 13.
   Delivers: Users can open color mapping from renderer settings and configure the existing native tone and color mapping options.

15. [Configure keyboard and mouse bindings, response, and reset](issues/15-keyboard-and-mouse-controls.md)

   Blocked by: 11.
   Delivers: Users can capture physical input natively, edit keyboard and mouse bindings, tune the existing mouse response settings, and reset the complete feature to its defined defaults.

16. [Configure controllers and their mappings](issues/16-controller-setup-and-mapping.md)

   Blocked by: 11.
   Delivers: Users can inspect available controllers, change supported controller options, and complete the existing native mapping flow from web setup.

17. [Manage profiles, settings files, and logs](issues/17-profiles-files-and-logs.md)

   Blocked by: 11.
   Delivers: Users can create, switch, and delete profiles, import and export settings, and open native log and file actions from web setup.

18. [Complete setup reminders and Steam shortcuts](issues/18-reminders-and-steam-shortcuts.md)

   Blocked by: 06.
   Delivers: Users can complete the existing setup reminders, confirmations, and supported Steam shortcut workflow from the web UI.

19. [Ship the Windows web and native application](issues/19-windows-web-native-package.md)

   Blocked by: 04.
   Delivers: A supported Windows machine without a Qt SDK or Node installation can launch accelerated web setup and stream in the native window from an installed package.

20. [Ship the Linux web and native application](issues/20-linux-web-native-packages.md)

   Blocked by: 04.
   Delivers: Supported Linux users can run accelerated web setup and native playback from the maintained AppImage and Flatpak packages.

21. [Ship the macOS web and native application](issues/21-macos-web-native-package.md)

   Blocked by: 04.
   Delivers: Supported Intel and Apple silicon Macs can launch accelerated web setup and native playback from the signed application package.

22. [Resume interrupted sessions and recover web setup](issues/22-session-and-setup-recovery.md)

   Blocked by: 08.
   Delivers: Users can recover from setup reloads, a setup renderer crash, suspend and resume, or an interrupted connection without duplicate work or damage to native playback.

23. [Complete the web-only application and verify native performance](issues/23-complete-web-only-acceptance.md)

   Blocked by: 05, 07, 09, 10, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22.
   Delivers: The complete packaged application provides GPU-accelerated web setup and separate native playback and overlays, with the required feature and performance evidence.

## Verification policy

Run focused checks for changed behavior and affected callers. Use the
production Svelte UI, real QWebEngineView, real WebChannel transport,
production bridge, and native services. Check visible behavior, native
windows, saved native values, and real outcomes. Control external results
at native asynchronous request boundaries.

Establish the embedded harness once in ticket 03, then extend it. Retain
native input ownership and output coverage on its supported toolchains.
A run that omits GNU-only native input targets cannot claim that coverage.

Record passing evidence and reuse it while the tested code, dependencies,
configuration, and relevant environment remain applicable. Repeat or broaden
checks only when a relevant change, failure, or unresolved concern justifies
them. Unrelated setup forms do not require the complete hardware matrix.

Use matched baseline comparisons with predefined tolerances. Add independent
runs when variation is unknown, results approach a limit, or a regression
appears. Choose sustained playback duration from warm-up, resource trends,
and known failures. Exercise first and subsequent connections with recreated
resources. Extend connection loops only when cleanup or a race needs them.
There are no blanket minimums of three comparisons, 30 minutes, or 20 cycles.

Check each final package once and exercise affected migrated features.
Reuse deeper hardware evidence unless relevant native code, Qt, WebEngine,
package contents, drivers, or configuration changed. Keep real GPU, HDR,
microphone, controller, focus, and available external latency acceptance.
Report unverified cases explicitly.

## Publication notes

The original 26 [draft tickets](ticket-drafts/README.md) remain as history.
Their numbers belong to the original proposal. Use the 23 published issues
above for implementation. The review report retains its original numbering
and records the mapping.

Each migration ticket removes replaced setup routes, resources, adapters,
and tests once no caller needs them. Preserve shared native streaming
components. Ticket 23 audits remaining references and final integration.
