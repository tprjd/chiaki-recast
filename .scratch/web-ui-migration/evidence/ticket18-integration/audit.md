# Ticket 18 integration audit

Merged worker `4e66aec70cf8e7c516d20f5ab93565ce94d389fd` into integration
`478a116d7700dda3762958d5806d6843c60ce75a` with `--no-ff`.
Merge `5e9de8922c5b2e04cb7b61a6fbd8995ce8893ba2` had no conflicts.
Both complete trees are `1c0f4c354013caaa6ab4f86740a24bfe17977351`.
The merged GUI tree equals tested source `80503e5773f811b38c0df7aa7e3efe219313b6e1`.

## Independent checks

- Read the ticket, specification, actual diff, callers, test bodies, and
  [worker evidence](../ticket18/verification.md). Native code keeps the Steam
  path, profile defaults, operation identity, progress, completion, validation,
  backup, and partial-failure result. The frontend has no path input command.
  Deferred work rechecks the form, profile, presentation, and native attachment.
- All 248 final source hashes and four retained artifact hashes match exactly.
  The application SHA256 is
  `52c7ffd6bb97ebd4e840249b6c65833377cce863c3829c62b62d271b1bd71ee3`.
  The embedded test SHA256 is
  `c948fa308b8fe074a45e795c5f8418d9f3072418e4d60a426fcd1abc9fbc4e6e`.
  The compiled JS and CSS also match. [Identities](identities.json) retain the
  actual values and the bounded comparisons. The retained QML test executable
  also matches its eligibility identity. The existing merged build log records
  its successful link. Its path and hash are retained with the identities.
- Repeated the existing 21 native file and 14 bridge method comparisons against
  `478a116d`. Backend code outside the Steam operation is unchanged. Native Steam
  and five saved passing test bodies equal `8aa79312`. Reconstructed the actual
  Steam, ownership, and eligibility pre-run test sources in temporary Git indexes.
  Their recorded hashes match with CRLF restoration. Six relevant test bodies
  and the actual pre-run native Steam operation equal the merged source.
- Native `initialAsk` retains Steam-before-PSN order and once-per-setup behavior.
  Later keeps its flag. Never persists. Complete credentials clear the PSN flag.
  Auto-connect and direct native stream state suppress startup reminders.
  Page reload does not reset `initialAsk`. Ticket 22 must preserve that rule when
  it combines page invalidation. Steam invalidation remains separate.
- Shared Dialog focus handling, permitted native help links, privacy masking,
  and account routing remain. Native `Key_Yes` accepts one press. Autorepeat and
  paired release cannot dismiss the next reminder. Library F1/L3 checks the
  current route and open dialogs. Menu still submits the active Steam form.
  Settings and capture retain their input guards. Native controller and live
  stream input files are unchanged.
- Native profile replacement, old Settings signal disconnection, file validation,
  wake and `woken`, `autoConnectKey`, current target and worker guards,
  `registrationBusy`, mapping, keyboard capture, and stream lifetime remain.
  Constructor arguments retain PsnRequests, ProcessLauncher,
  RegistrationRequests, and DiscoveryRequests order. Fixtures install the
  controlled discovery and fail-closed external boundaries before services.
- Caller inspection found no remaining removed reminder/Steam QML leaf,
  resource, fixture, QJS-only callback, or standalone callback test caller.
  Shared MainView and SettingsDialog cleanup stays with ticket 23. StreamRoot
  and shared native stream components remain. The vendor patch is excluded.

## Reused results and limits

Read the actual commands, pre-run identities, logs, and final build output.
Reused final guards 3 passed and 0 failed, four eligibility rows plus affected
wake 7/0, Steam/reminder 4/0, ownership/earlier guards 4/0, frontend zero errors
and zero warnings, and application, embedded, and QML test builds.
Temporary Steam files prove create, update, five artwork files, controller
workshop configuration, backup, partial failure, and retry without duplicates.
The tests do not write the user's Steam files or restart Steam.

No build, runtime check, capture, or matrix was repeated. The 200 percent check
proves DOM bounds at a width of at least 1024 CSS pixels. It is not visual
acceptance. Actual foreground focus, physical input, GPU, live PSN, real-console
playback, Steam library visibility, and platform acceptance remain in ticket 23.
Native file-lock failure branches and the Steam-disabled build were not repeated.
Steam writes can leave partial changes. The UI reports that result.
