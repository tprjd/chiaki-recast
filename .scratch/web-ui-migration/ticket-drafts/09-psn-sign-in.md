# 09: Sign in to PSN from web setup

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** Users can sign in through an embedded or external browser, complete sign-in with a redirect URL, and see native account status in web setup.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** needs-info

**Spec stories:** 21, 22, 40, 41, 42, 43, 46, 47.

- [ ] The web UI offers the existing embedded sign-in route and external browser fallback, with a native-validated sign-in URL and redirect submission.
- [ ] Remote sign-in content runs without the setup bridge. Cookie and sign-in profile behavior retain the current rules.
- [ ] Native services own token exchange, refresh, expiry, Account ID retrieval, and credential storage. Tokens and keys never enter setup snapshots or browser storage.
- [ ] Native attempt and profile identities prevent stale sign-in completion, duplicate submission, and updates to another profile.
- [ ] The UI shows actual progress, success, expiry, failure, and retry. Temporary redirect URLs and secret input clear on completion or dismissal.
- [ ] An Account ID obtained through sign-in is available to native registration without losing a still-valid form context.
- [ ] Embedded tests verify real transport, native state changes, expired credentials, stale replies, and absence of a bridge on remote sign-in pages.
