# 06: Sign in to PSN independently from web setup

**What to build:** Users can sign in through an embedded or external browser, complete sign-in with a redirect URL, and see native account status in web setup.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** claimed

**Source:** [Web setup UI specification](../spec.md). Stories 40, 41, 42, 43, 46, 47.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [ ] The web UI offers the existing embedded sign-in route and external browser fallback, with a native-validated sign-in URL and redirect submission.
- [ ] Remote sign-in content runs without the setup bridge. Cookie and sign-in profile behavior retain the current rules.
- [ ] Native services own token exchange, refresh, expiry, Account ID retrieval, and credential storage. Tokens and keys never enter setup snapshots or browser storage.
- [ ] Native attempt and profile identities prevent stale sign-in completion, duplicate submission, and updates to another profile.
- [ ] The UI shows actual progress, success, expiry, failure, and retry. Temporary redirect URLs and secret input clear on completion or dismissal.
- [ ] The standalone account route reports native account status and makes the obtained Account ID available to native services. Returning to a retained registration form belongs to the pairing ticket and is not a prerequisite here.
- [ ] Embedded tests verify real transport, native state changes, expired credentials, stale replies, and absence of a bridge on remote sign-in pages.
- [ ] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.
