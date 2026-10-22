# Linux and macOS source integration

Date: 2026-10-22.

Linux worker `0c246052` merged into integration `06b38253` as `983a9ff6` without conflicts.
macOS worker `7f5c4786` then merged as `35b78e9a`.
The only conflict was appended map text. The resolution keeps both the macOS milestone notes and the user priority update.

Read-only comparison of PR, release, and weekly workflow blocks confirms the Windows jobs equal integration `06b38253`.
Linux jobs equal worker `0c246052`. Mac jobs equal worker `7f5c4786`.
The removed ARM AppImage jobs remain absent. ARM Flatpak remains configured and untested.
The comparison used Git source text, not workflow execution.

The GUI diff contains only the macOS automatic graphics switching property.
The native backend, frontend, stream renderer, input code, defaults, and bridge contracts are unchanged.
No prototype or fixture code entered the shipping application.
The Mac preflight runs once after final application signing and entitlement checks, before DMG creation, with an empty environment and system PATH.

The [Linux verification](../ticket20/verification.md) and [checkpoint](../ticket20/checkpoint.md) retain the completed SDK builder, dependency preparation, fixture, and syntax results.
Those results do not establish an application build or package pass.
The [macOS verification](../ticket21/verification.md) retains the earlier fixture and source results.
The final signing-order correction has no new runtime evidence.

No build, application export, package, fixture run, runtime, signing, notarization, performance test, or broad validation matrix ran during integration.
The user waived the remaining checks to prioritize the selected prototypes.
Linux and Mac application packages, normal startup, hardware behavior, and release acceptance remain unverified.
Tickets 20 and 21 resolve their source milestones with those limits. Ticket 23 is unblocked for the selected UI work at a minimum width of 1024 CSS pixels.
