# 07: Register PS4 and PS5 consoles with a pairing code

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can register a discovered or manually entered PS4 or PS5 console, see the real registration result, and connect after success.

**Blocked by:** 06: Save manual console addresses and connect.

**Status:** needs-info

**Spec stories:** 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27.

- [ ] The guided form retains supported console and firmware choices, broadcast registration, account identity input, saved Account ID use, and an optional console login PIN.
- [ ] The Remote Play pairing code and console login PIN remain distinct. Leading zeroes survive, and invalid fields prevent native submission.
- [ ] Registration uses a current native context for the original discovered or manual target. Profile or address changes invalidate stale work.
- [ ] Native completion signals replace callback-only integration where required. One activation starts one registration, and completion is delivered once.
- [ ] Progress, success, failure, diagnostics, corrected input, and retry reflect native outcomes. Dismissal does not claim an unsupported cancellation.
- [ ] Successful registration updates the correct native store and manual entry association. The console can then use the existing native connection path.
- [ ] Streamer mode masks account and address fields, results, and diagnostics. Temporary codes are cleared when no longer needed.
- [ ] Embedded tests cover validation, leading zeroes, native results, duplicate activation, stale profiles, stale manual targets, and connection after registration.
