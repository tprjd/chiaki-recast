# 06: Sign in to PSN independently from web setup

**What to build:** Users can sign in through an embedded or external browser, complete sign-in with a redirect URL, and see native account status in web setup.

**Blocked by:** 03: Show live consoles in the GPU-accelerated web UI.

**Status:** resolved

**Source:** [Web setup UI specification](../spec.md). Stories 40, 41, 42, 43, 46, 47.

**Verification:** Run focused checks for this ticket and affected callers. Reuse applicable passing evidence. Repeat or broaden checks only when relevant changes, failures, or unresolved concerns justify them.

- [x] The web UI offers the existing embedded sign-in route and external browser fallback, with a native-validated sign-in URL and redirect submission.
- [x] Remote sign-in content runs without the setup bridge. Cookie and sign-in profile behavior retain the current rules.
- [x] Native services own token exchange, refresh, expiry, Account ID retrieval, and credential storage. Tokens and keys never enter setup snapshots or browser storage.
- [x] Native attempt and profile identities prevent stale sign-in completion, duplicate submission, and updates to another profile.
- [x] The UI shows actual progress, success, expiry, failure, and retry. Temporary redirect URLs and secret input clear on completion or dismissal.
- [x] The standalone account route reports native account status and makes the obtained Account ID available to native services. Returning to a retained registration form belongs to the pairing ticket and is not a prerequisite here.
- [x] Embedded tests verify real transport, native state changes, expired credentials, stale replies, and absence of a bridge on remote sign-in pages.
- [x] Remove setup-only routes, resources, callback adapters, and tests replaced by this slice once no caller needs them. Preserve shared native stream components and their tests.

## Answer

Implemented native PSN sign-in for the standalone web account route. The embedded
browser uses a separate persistent `psn-token` profile with no setup WebChannel.
The external browser and copied link use the native sign-in URL. Redirect submission
uses the existing native attempt, validation, profile guards, token exchange,
Account ID retrieval and credential store. The web snapshot contains only safe
account status and an allowlisted attempt summary.

The panel shows native progress, expiry, failure, success and retry. Redirect input
is masked and clears on submission, fresh sign-in, profile change or dismissal.
Native refresh rejects incomplete credentials without overwriting saved values and
returns safe errors. Its success notification updates the account status.

The instance-scoped request dependencies control native HTTP replies and device-list
responses in tests. No fixture credential reaches live PSN. Focused real-application
checks pass for status, sign-in, refresh, native page isolation and presentation.
Cookie persistence uses exact fixture metadata before success and after profile
flush. See [verification](../evidence/ticket06/verification.md) and
[source and binary identity](../evidence/ticket06/identity.json).

Unused callback adapters were removed. PSNTokenDialog.qml remains for the source
callers in registration and retained settings/reminders; the pairing slice owns
its final removal. Native stream QML remains unchanged.

Live PSN page compatibility, actual sign-in and visible desktop focus were not
checked. Software Chromium rendering applies only to these offscreen tests.
The unchanged ticket 03 GPU evidence is reused. Physical hardware acceptance remains
with tickets 04 and 23.
