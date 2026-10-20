# Integrated screen check before Settings search

Source: codex/settings-refresh-integration, ec5d19ba866f029fdbd4fdbc7745b223bc95249d.
Qt 6.9.3 MSVC; production QML loaded from the integration checkout.
Result: 511 passed, 0 failed, 0 skipped; no QML warnings.
Log: C:/Users/david/AppData/Local/Temp/settings-refresh-pre-search-suite.txt.

This check covers all screen suites in the integration branch through ticket 11.
It does not include the uncommitted console-library corrections in the main
checkout. Those corrections passed their own 206-check baseline. The final
combined run must include them after Settings integration is applied.