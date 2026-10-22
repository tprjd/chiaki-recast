# 10: Wake consoles and auto-connect from web startup

**What to build:** Users can wake consoles and use or stop auto-connect for the selected local or PSN target.

**Blocked by:** 08: Stream from a registered console through PSN.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 28, 29, 30, 35, 36, 46.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] Console wake-up retains native credentials, target resolution, progress, timeout, and failure behavior.
- [x] Auto-connect uses the selected registered console and current profile and works before the setup page finishes loading.
- [x] The application prepares native resources and presents current progress without depending on a browser event being delivered first.
- [x] Stopping auto-connect cancels only the current native attempt and leaves setup usable.
- [x] Missing, hidden, ambiguous, removed, or stale-profile targets do not connect to another console.
- [x] Embedded tests cover startup timing, wake wait, wake failure, explicit cancellation, local and PSN routes, and late native replies.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after native PSN streaming. Prepare an isolated worktree for native wake and startup auto-connect. Follow the current 1024 CSS pixel design scope and reuse current target/profile/attempt guards.

## Answer

The compiled web setup now saves an opaque registered-console auto-connect
choice, sends native wake requests, and presents current progress and recovery.
Native startup timers, local priority, session preparation, and profile ownership
remain in QmlBackend. Native resources and connection work pass with page
JavaScript disabled. A stop applies only to the current automatic attempt.

The native target guard rejects removed or replaced registrations before late
PSN success starts a session. A late remote list can complete the current
startup wait. Automatic PS4 selection requires one eligible saved registration.
Multiple eligible PS4 targets do not cause a guess. Existing manual PS4 lookup
and registered-console unhide behavior are preserved.

[Verification](../evidence/ticket10/verification.md) records focused red and green
results, compiled native transport checks, source comparisons, and limits.
The unused setup wrapper and fake wake tests are removed. Native stream QML,
media, and input remain. Real registered-console Windows acceptance stays with
ticket 23. No hardware, focus, GPU, performance, or package pass is claimed.
