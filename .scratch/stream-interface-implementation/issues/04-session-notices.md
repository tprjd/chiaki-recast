# 04: Build the approved A session notices

Status: resolved
Type: task
Blocked by: 01

**What to build:** Explain network problems, blocked Remote Play content, and
disabled media through the approved small A notices. Keep recovery actions,
current session state, input ownership, and notice cleanup correct in both
renderer presentations.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [x] Replace the large network indicator with a compact top-right Unstable connection notice and the measured packet-loss percentage. Preserve the existing configured threshold and warning timing.
- [x] Show Audio disabled as a small top-right notice when audio is disabled. Identify Settings → General → Audio/Video as the recovery location. Do not confuse a settings choice with a network or connection failure.
- [x] Stack simultaneous audio and network notices compactly without overlap. These routine notices remain non-modal and leave focus and input with the game.
- [x] Show Content blocked by the console in a short bottom-center notice. Explain that the screen cannot be shown through Remote Play. Preserve the existing condition, Go to console home command, and input ownership.
- [x] Distinguish Video disabled from Audio and video disabled using the existing settings flags. Explain that the session remains connected and identify the existing settings location.
- [x] Disabled-video notices offer Open stream menu through the existing menu route. Do not invent a direct restore command. Preserve the existing black video area when video is disabled.
- [x] Use A's compact placement and dark translucent background with opaque text and visible focus for actions. Do not add full-screen dimming or blur.
- [x] Notices update and clear when the condition changes, a session is replaced, or the session ends. No command can target an old or absent session.
- [x] Preserve connection progress, console login PIN, and disconnect-dialog priority. Actionable notices and their focus restoration do not leave input captured or compete with a higher-priority dialog.
- [x] Both renderer routes show the same notice content and actions. Movement, resizing, and teardown keep notices attached to the stream and remove stale windows.
- [x] Production QML tests use the boundary from 01. Cover warning values below, at, and above the existing threshold; warning clearance; blocked content; audio, video, and combined disabled states; combinations; and session replacement/destruction.
- [x] Observe Go to console home and Open stream menu through real production bindings with controlled commands. Test keyboard/controller key handling, enabled actions, input ownership, and focus return without a real console command.
- [x] Check the three agreed window sizes, long translated messages, and dark, bright, and detailed backgrounds. Keep notices readable and clear of existing menu/dialog regions. Combined geometry with ticket 02's final pop-ups is also checked in 05.
- [x] Build the affected native target and record feature verification. Do not add a new health classifier, warning policy, settings schema, or redesigned recovery page.

## Comments

Ticket 01 is the only blocker. Use the existing menu and session action contracts.
The final combined interaction with the redesigned toolbar and statistics is a
required part of 05, not a reason to delay this complete notice slice.

Claimed after ticket 01 merged into the integration branch in `fe4850b8`.


## Answer

Implemented the approved compact A notices in production `StreamView`.
`StreamNotice`, `StreamPassiveNotices`, and `StreamActionNotice` share the content
between the inline and native window hosts. Passive windows are transparent to
input. Action windows retain the actual native stream window as their owner.

The warning retains the configured greater-than threshold, 400 ms timer, and
fade. It displays the current packet-loss percentage. Session replacement clears
the previous warning. Audio and network notices form a compact top-right stack.
Video-disabled and blocked-content notices use the short bottom-center action
panel. The original black disabled-video area remains in place.

Notice state gives priority to connection progress, console login PIN requests,
and disconnect dialogs. All notices hide while the menu is open or closing.
Action notices own a separate input count and saved game focus. This prevents a
menu or dialog handoff from releasing another owner's input. The native notice
close route uses the existing disconnect policy. Commands require the current
connected session and the current visible condition.

Verification is recorded in [ticket 04 evidence](../evidence/ticket04/README.md).
The normal native build passes. Its 341 source inputs and new notice resources
match the worktree, and test resources and QA hooks are excluded. The native cache
is restored, all 341 saved path hashes and 36 options match, and both original
targets rebuild.

The composed notice suite passes 47 Windows checks on Qt 6.9.3. The normal-build
Qt 6.11.2 runner passes 46 offscreen checks with one passive-activation skip and
nine selected Windows checks. Related menu, connection, PIN, disconnect, and
settings suites pass 171 checks. Layout checks cover the three agreed sizes,
long text, and dark, bright, and detailed backgrounds. The offscreen images show
normal text, long action text, long passive stacks, and the focus outline.

Windows behavior and logical geometry are verified at the observed DPR of 1.25.
QtTest native captures are cropped at that scale, so native pixel claims are
excluded. No real backend session, console command, or user settings write is
used. Physical controller input, live media, actual OpenGL and Vulkan composition,
fullscreen, and further display scales remain in ticket 05.
