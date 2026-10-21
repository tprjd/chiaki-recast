# 01: PSN setup and credential renewal

Status: ready-for-agent
Type: task
Blocked by: None (can start immediately)

**What to build:** Deliver PSN remote-access setup and expired-credential renewal through the current embedded and external browser routes, including input, actual operation progress, results, retry, and screen closure.

Source specification: PSN sign-in and remaining dialogs.

## Acceptance criteria

- [ ] Use the new design and existing production controls. Verify reachable actions at 900 by 600 and 1280 by 720 logical pixels, long text, keyboard/controller navigation, visible focus, return focus, and applicable privacy rules. Keep test fixtures and synthetic data out of production resources.
- [ ] Use the new design, preserve platform browser defaults, retain external-browser fallback and browser recovery actions, and preserve Settings, reminder, and expiry entry routes.
- [ ] Explain the complete Sign-in redirect URL and saved credentials. Provide explicit copy/paste actions, masked input, reveal controls, and streamer-mode protection.
- [ ] Reject invalid redirect input at the native boundary and provide a route to fresh sign-in for expired or used codes.
- [ ] Show progress, terminal success, or persistent failure with retry and collapsed sanitized details. Correct expiry routing and embedded/external reminder-completion parity.
- [ ] Before submission, Cancel exits. After submission, Close explains that sign-in can finish and save credentials. Prevent duplicate requests across closure and re-entry.
- [ ] Associate requests with their original profile and presentation. Late results cannot save into another profile or change a replacement screen. Preserve compatibility for existing registration callers while the next ticket refreshes that route.
- [ ] Verify production QML behavior and actual native persistence with isolated settings and controlled responses. Cover failure, replacement, duplicate actions, late results, and sensitive-value exclusion.

## Coverage and dependencies

Primary user stories: 9, 14-35, 37. Shared user stories: 1-7, 63.

## Comments

Published on 2026-10-21 after the user approved the eight-ticket breakdown. No implementation work has started.

