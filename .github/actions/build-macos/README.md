# macOS package builds

The manual Intel and Apple silicon workflows, combined workflow, PR workflow,
release workflow, and weekly workflow call this action. Each job builds one
native architecture. The official Qt 6.9.3 SDK and WebEngine extension contain
both architecture slices. The action checks the selected architecture in every
deployed Mach-O file. Node 24.16.0 and npm 11.13.0 build the locked frontend.
The executable contains the frontend. The package does not install Node.

The action retains native Qt Quick deployment, SDL fixups, MoltenVK 1.2.9,
the helper Frameworks link, and microphone permission. It checks the bundle
layout, matching SDK locale files, V8 snapshot, library paths, native architecture,
and the shared read-only `--check-package` command before signing.

`macos-notices.py` copies the matching SDK SPDX records with Chromium license
texts, Qt license files, project and source dependency notices, and notices from
the Homebrew kegs whose libraries ship in the bundle. It records each selected
formula and version. A missing required notice stops packaging. The actual Mac
SDK and Homebrew output must pass this check on a Mac runner.

## Release signing inputs

Only release and weekly jobs receive these repository secrets:

- `MACOS_SIGNING_CERTIFICATE_P12`: Base64 Developer ID Application PKCS#12 certificate.
- `MACOS_SIGNING_CERTIFICATE_PASSWORD`: PKCS#12 password.
- `MACOS_SIGNING_IDENTITY`: Full `Developer ID Application: ...` identity.
- `MACOS_NOTARY_APPLE_ID`: Apple ID for notarization.
- `MACOS_NOTARY_TEAM_ID`: Developer team ID.
- `MACOS_NOTARY_PASSWORD`: App-specific notarization password.

With all inputs absent, the package uses ad-hoc signatures. A partial input set
fails. The configured path imports the certificate into a temporary keychain,
signs contained code before enclosing bundles with hardened runtime and secure
timestamps, and uses the matching installed SDK helper entitlements. The app
retains its microphone entitlement. The script verifies the signed entitlements,
creates and signs the DMG, submits it with `notarytool --wait`, and requires an
Accepted result before stapling and Gatekeeper assessment. Cleanup restores the
keychain search list and removes the temporary certificate and keychain on failure.

The JSON file beside each DMG records the source revision, frontend lock hash,
architecture, toolchain, final DMG hash, signing mode, and `runtime_checked: false`.
A mode of `developer-id-notarized` requires successful notary and staple commands.
PR jobs receive no signing secrets. No workflow uses `pull_request_target`.

These scripts do not establish actual Mac startup, Chromium GPU acceleration,
sandbox behavior, native playback, focus, input, scaling, or microphone behavior.
Those checks require Mac hardware and remain in ticket 23.
