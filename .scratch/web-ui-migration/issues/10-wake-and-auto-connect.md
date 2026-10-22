# 10: Wake consoles and auto-connect from web startup

**What to build:** Users can wake consoles and use or stop auto-connect for the selected local or PSN target.

**Blocked by:** 08: Stream from a registered console through PSN.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 28, 29, 30, 35, 36, 46.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] Console wake-up retains native credentials, target resolution, progress, timeout, and failure behavior.
- [ ] Auto-connect uses the selected registered console and current profile and works before the setup page finishes loading.
- [ ] The application prepares native resources and presents current progress without depending on a browser event being delivered first.
- [ ] Stopping auto-connect cancels only the current native attempt and leaves setup usable.
- [ ] Missing, hidden, ambiguous, removed, or stale-profile targets do not connect to another console.
- [ ] Embedded tests cover startup timing, wake wait, wake failure, explicit cancellation, local and PSN routes, and late native replies.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Comments

- Claimed after native PSN streaming. Prepare an isolated worktree for native wake and startup auto-connect. Follow the current 1024 CSS pixel design scope and reuse current target/profile/attempt guards.
