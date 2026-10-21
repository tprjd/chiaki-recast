# 04: Build the approved A session notices

Status: ready-for-agent
Type: task
Blocked by: 01

**What to build:** Explain network problems, blocked Remote Play content, and
disabled media through the approved small A notices. Keep recovery actions,
current session state, input ownership, and notice cleanup correct in both
renderer presentations.

Parent and specification: [implementation map](../map.md).

## Acceptance criteria

- [ ] Replace the large network indicator with a compact top-right Unstable connection notice and the measured packet-loss percentage. Preserve the existing configured threshold and warning timing.
- [ ] Show Audio disabled as a small top-right notice when audio is disabled. Identify Settings → General → Audio/Video as the recovery location. Do not confuse a settings choice with a network or connection failure.
- [ ] Stack simultaneous audio and network notices compactly without overlap. These routine notices remain non-modal and leave focus and input with the game.
- [ ] Show Content blocked by the console in a short bottom-center notice. Explain that the screen cannot be shown through Remote Play. Preserve the existing condition, Go to console home command, and input ownership.
- [ ] Distinguish Video disabled from Audio and video disabled using the existing settings flags. Explain that the session remains connected and identify the existing settings location.
- [ ] Disabled-video notices offer Open stream menu through the existing menu route. Do not invent a direct restore command. Preserve the existing black video area when video is disabled.
- [ ] Use A's compact placement and dark translucent background with opaque text and visible focus for actions. Do not add full-screen dimming or blur.
- [ ] Notices update and clear when the condition changes, a session is replaced, or the session ends. No command can target an old or absent session.
- [ ] Preserve connection progress, console login PIN, and disconnect-dialog priority. Actionable notices and their focus restoration do not leave input captured or compete with a higher-priority dialog.
- [ ] Both renderer routes show the same notice content and actions. Movement, resizing, and teardown keep notices attached to the stream and remove stale windows.
- [ ] Production QML tests use the boundary from 01. Cover warning values below, at, and above the existing threshold; warning clearance; blocked content; audio, video, and combined disabled states; combinations; and session replacement/destruction.
- [ ] Observe Go to console home and Open stream menu through real production bindings with controlled commands. Test keyboard/controller key handling, enabled actions, input ownership, and focus return without a real console command.
- [ ] Check the three agreed window sizes, long translated messages, and dark, bright, and detailed backgrounds. Keep notices readable and clear of existing menu/dialog regions. Combined geometry with ticket 02's final pop-ups is also checked in 05.
- [ ] Build the affected native target and record feature verification. Do not add a new health classifier, warning policy, settings schema, or redesigned recovery page.

## Comments

Ticket 01 is the only blocker. Use the existing menu and session action contracts.
The final combined interaction with the redesigned toolbar and statistics is a
required part of 05, not a reason to delay this complete notice slice.
