# 08: Retrieve an Account ID during registration

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** A user can choose public Account ID lookup, retrieve a validated result, and continue the same registration form.

**Blocked by:** 07: Register PS4 and PS5 consoles with a pairing code.

**Status:** needs-info

**Spec stories:** 21, 22, 44, 45, 46.

- [ ] The lookup route retains its service explanation and privacy requirement before sending the username.
- [ ] A shared native operation owns the request, timeout, response validation, and completion. No authentication credentials are created by public lookup.
- [ ] Only a valid Account ID fills the registration field. Unreadable responses, missing results, service failures, and timeouts show an actionable error.
- [ ] Registration details survive lookup, cancellation, and retry. Stale results after navigation, replacement requests, or profile changes are ignored.
- [ ] Embedded tests exercise successful lookup, invalid replies, timeouts, late replies, and retention of the original registration context.
