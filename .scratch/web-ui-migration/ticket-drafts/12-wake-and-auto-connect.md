# 12: Wake consoles and auto-connect from web startup

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can wake consoles and use or stop auto-connect for the selected local or PSN target.

**Blocked by:** 10: Stream from a registered console through PSN.

**Status:** needs-info

**Spec stories:** 28, 29, 30, 35, 36, 46.

- [ ] Console wake-up retains native credentials, target resolution, progress, timeout, and failure behavior.
- [ ] Auto-connect uses the selected registered console and current profile and works before the setup page finishes loading.
- [ ] The application prepares native resources and presents current progress without depending on a browser event being delivered first.
- [ ] Stopping auto-connect cancels only the current native attempt and leaves setup usable.
- [ ] Missing, hidden, ambiguous, removed, or stale-profile targets do not connect to another console.
- [ ] Embedded tests cover startup timing, wake wait, wake failure, explicit cancellation, local and PSN routes, and late native replies.
