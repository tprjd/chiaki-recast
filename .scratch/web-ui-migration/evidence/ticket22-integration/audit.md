# Ticket 22 integration audit

Merged worker `adc599478f97db8949a3754233c1e1ad0ea8260d` into integration
`41c6134c0c42227b7776f8df95a3fbbc821bd452` with merge commit
`270f39850c9924151bfd59fbc9081f091debffff`.
The merge had no conflicts. The merged GUI, ticket, and map equal the worker.
`git diff --check HEAD^1 HEAD` passed after the merge.

The audit read the actual changed callers and saved test artifacts.
Web file actions use the page-generation guard after the modal dialog returns.
Native QML calls retain the no-argument methods and their existing lifetime.
Page recovery keeps the restricted profile and native bridge, replaces the
channel, and defers hidden-page replacement until setup returns.
Local resume keeps the opaque target. Profile changes, new attempts, and
accepted cancellation clear obsolete resume intent.

The saved evidence is under
`C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket22-recorded-evidence`.
The audit read the commands and actual QtTest logs for native recovery,
file lifetime, affected callers, final focus, and the final channel helper.
Results are 4/0, 4/0, eight affected cases passed, 5/0, and 3/0 respectively.
The affected group has two superseded recovery failures. Final focus fixes
those failures. The final helper check removes the earlier callback error.

An independent hash comparison used each compressed `source.json.gz`.
All production C++ and native QML equal the native-green artifact.
The final-focus artifact differs only in the test helper source.
The final client artifact equals every current GUI source.
All 17 source files in `source-preservation.json` independently equal the
integration base after line-ending normalization.
Actual application and harness files match the recorded SHA256 values:

- Application: `659e86b11ae508c247ea0c01f8fa1356abf984d1ad5627bc882b3af098a08ed4`.
- Harness: `dafc8c3c20d28d8943de06a9d6de288f9fcd2307c2bb61542ffae3d10efd6479`.

The native fixture uses the production decoder, frame worker, native window,
and stream input owner. It terminates only the owned current setup renderer.
Its joined producers release every tracked decoded buffer.
The short resource probe supports keeping healthy setup hidden and frozen.

No runtime or build was repeated. No vendor change was merged.
Physical input and focus, OS power restore, live consoles, GPU acceleration,
the independent HighQuality failure, other presets, long resource trends,
performance, packages, and platform acceptance remain in ticket 23.
