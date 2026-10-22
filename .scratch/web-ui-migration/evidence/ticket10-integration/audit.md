# Ticket 10 integration audit

Merge `773e08e6071b5d49c69d75740aa06ad57725568a` includes worker
`5db47a2de850abf44fa1f629f58795a8aee06316` without conflicts.
Integration was clean at `5b026017683c7f1fe0af74e21deebe098d75d07b`.
The complete merge tree equals the worker tree. The vendor patch is excluded.

## Direct checks

All 257 entries in `../ticket10/final-source-sha256.csv` match the actual
merged files without line-ending normalization. Both retained final executables
match `../ticket10/final-executables-sha256.txt`:

| Artifact | SHA256 |
| --- | --- |
| `ticket10-build/gui/chiaki.exe` | `94FEB4D85D260CCB8F28D061741933CDA2BC98BA3F9FDE003A94047A29C51D5A` |
| `ticket10-build/gui/tests/chiaki-embedded-setup-tests.exe` | `618F5C35E8D06C7397034E7698825DB696D9903F0663854FA95D05EB75E4E6A1` |

Executable paths are under `C:/Users/david/.cache/chiaki-msvc-native/`.
Earlier executable identities are saved per run. This audit checks the two
retained final executables. It does not claim that earlier binaries remain.

The last passing runtime source was reconstructed in a temporary Git index.
`git read-tree` used the saved `ticket10-late-list-green02/source.txt` commit.
`git apply --cached --ignore-whitespace` applied its saved `full.diff`.
The backend equals the merged backend after removal of exactly the two unused
`wakeupStartInitiated` and `wakeupStartFailed` emissions. The embedded test source
equals the merged test source after line-ending normalization.

Seven bounded comparisons reconstruct the earlier passing groups in the same
way. The selector test and `setAutoConnectTarget` match target-green. The wake
test and `wakeUpHost` match wake-green. The cancellation test and
`stopAutoConnect` match cancel-green. `wakeFailures` matches hidden-green.

Nine protected files match integrated source `13b594df`: the source and header
for ApplicationController, StreamSession, and QmlNativeServices, plus StreamRoot,
StreamView, and ConnectionView. Five backend functions also match that source:
`controllerMappingApply`, `controllerMappingQuit`, `startPsnConnection`,
`clearWakeWait`, and `displayServerAt`. `autoRegisterConsole` is unchanged.
Its `connectionBusy` state still includes `registrationBusy`. Native retry
still checks registration activity and the automatic registration window.

The production diff preserves controller connect/retry guards, hidden native
resource preparation, and the stream pipeline. Constructor arguments remain
PsnRequests, ProcessLauncher, RegistrationRequests, and DiscoveryRequests.
The manual PS4 nickname lookup is unchanged. Only startup selection substitutes
the saved native registration, after the single eligible PS4 check.

## Reused results

Read the saved QtTest logs, commands, source commits, and full diffs. Selector,
wake, cancellation, and corrected hidden-target groups each pass 3/0. The
failure group contributes its five passing rows. Corrected native startup
contributes local ready/wake and PSN ready/removed/replaced passes. The earlier
startup group contributes PSN cancellation/profile passes. PS4 selection and
ambiguity pass 4/0. The final late-list and affected ticket 09 success group
passes 4/0. Counts include initialization and cleanup.

The intermediate fixture failures remain recorded. The corrected checks read
the resolved native address, reconstruct startup Settings, and wait for the
public cancelable state. No production cancellation rule was weakened.

Source inspection confirms startup work with JavaScript disabled, local
priority, exact attempt/profile/registered-target checks, wake-only completion,
standby connection, current-attempt stop, retained preference, timeout/retry,
late PSN worker rejection, and late-list re-evaluation. Controlled discovery
rejects unexpected wake transport before construction. Successful native starts
create compatible threads that native cleanup joins.

Caller search confirms no remaining AutoConnectView or removed signal caller.
The retained ConnectionView fixture component is unchanged. Its recorded compile
is reused. `git diff --check 5b026017 HEAD -- gui` passes. Saved raw logs and
patches retain their original whitespace.

## Limits

No build, runtime check, capture, or hardware matrix was repeated. The source
comparisons justify reuse within the recorded limits. Real-console Windows
acceptance, live PSN/playback, physical focus/input, GPU, HighQuality, latency,
performance, packages, and platform hardware remain in ticket 23. The minimum
design width is 1024 CSS pixels, including at 200 percent zoom. This audit adds
no layout or capture claim.
