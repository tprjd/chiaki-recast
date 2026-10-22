# Ticket 17 integration audit

Merged worker `1fda0725cb688f438fb0741da4cf5e1ae288807b` into integration
`82b9c65f3df81eb29418e3cf7d79a74fe233de55` with `--no-ff`.
Merge `7858fcd8f5989e9b4e90dc7a0a10107802b1ac04` had no conflicts.
Both complete trees are `88416772896f85e48e2231c96f530686b8aa9a48`.
The GUI tree is `9aa3eb50e9f9a0f18ac6b069fea3f34e6b7b2094`.

## Independent checks

- Read the actual diff, shared file/profile callers, test bodies, native
  replacement path, ticket, specification, and [worker evidence](../ticket17/verification.md).
  Imports validate arrays, MACs, required authentication fields, and legacy
  formats before store or map replacement. Default imports restore their catalog.
  Named imports retain the local catalog. Exports use a temporary child INI and
  QSaveFile. Native dialogs return actual outcomes and reject obsolete generations.
- All 425 final source SHA256 values match the worker files. Integration has
  420 exact matches and four differences in line endings only. The remaining
  file is the ignored, generated `gui/chiaking.rc`, absent from integration.
  Its tracked template is unchanged. All five retained final artifact hashes
  match the identity record, including the application, both tests, JS, and CSS.
- Independently repeated all 23 comparisons in `source-preservation.json`
  against the saved green and cleanup sources and the specified Git revisions.
  All pass with UTF-8 and normalized line endings. The corrected Profiles
  component and profileSearch body equal the search-fix sources. The four
  affected SettingsDialog, SettingsSearch, and QML test sources equal cleanup.
- The incoming backend changes only to disconnect old Settings signals.
  Wake, startup target, PS4 lookup, late-list, cancellation, registration busy,
  DUID/MAC, and hidden-resource guards remain. Controller mapping, native input,
  StreamRoot, and live QML remain. Discovery fails closed before profile fixtures.
  Constructor request and launcher argument order remains correct.
- Caller inspection confirms removal of ProfileDialog, its resource and routes,
  and the unused QmlSettings catalog/sentinel and deletion adapters.
  Shared native Settings, currentProfile, and profilesChanged remain.
  Profile drafts depend on generation and stable catalog keys, with untracked
  initialization. Unrelated snapshots do not reset the draft.

## Reused results and limits

Read the actual logs and retained pre-run source records. Reused profile/files
6 passed and 0 failed, child restart 3/0, default-catalog pass, four named QML
cases 8/0, corrected profileSearch 3/0, frontend 0 errors and 0 warnings, and
application/test builds. The earlier draft-reset failure remains recorded.
The source checks justify reuse after the additive merges and unused API removal.

No build, runtime check, capture, or matrix was repeated. Native mid-write store
rollback was reviewed but was not forced at runtime. Offscreen DOM checks at
200 percent zoom prove the documented width and overflow assertions only.
Live services, physical focus/input, GPU, performance, packages, and platform
acceptance remain in ticket 23. The gf-complete vendor patch is excluded.
