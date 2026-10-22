# Ticket 23 prototype fidelity

The web frontend follows the selected B console library, A Settings sidebar,
and A guided setup prototypes. This is the source and visual milestone under
the user's 2026-10-22 priority update. Remaining release builds and performance
qualification were skipped at the user's request.

## Changes

- The persistent app rail contains Consoles, Settings, and the current profile.
- Each library card contains artwork, native status, a primary action, and its
  details button. Registered console actions use the existing connect method
  with the card's opaque target key. Local standby keeps Wake and connect.
- The library toolbar contains search and PSN refresh. Discovery and actual
  filtered, total, and online counts appear below the cards.
- Connection progress, errors, cancel, retry, and dismiss appear once. The
  saved auto-connect target, including Off, and selected console extras remain
  in Connection options below the library.
- Add console and an unregistered card open one manual/automatic registration
  chooser. Manual pairing retains the native context and Account ID methods.
  Automatic registration uses the existing opaque native target key.
- Guided pairing uses a separate form and console instruction column. Closing
  the page ends its account route and clears both PIN drafts. Rail navigation
  waits for the same close method before changing the page.
- Settings uses the selected category sidebar, current profile chip, grouped
  rows, and compact controls. Checkbox changes still restore the native value
  before sending the requested value. Conditional fields leave no empty groups.
- Profiles, controllers, and keyboard and mouse controls use the same row and
  group styles. Their handlers and native guards remain.
- Maintained embedded fixture selectors now scope connect to the selected
  card and select Manual pairing after opening the chooser. Their runtime
  suites were not run.

## Frontend check and compile

`npm.cmd --prefix gui/web run check` passes with zero errors and zero warnings.
`npm.cmd --prefix gui/web run build` compiles the visual changes. The final
pairing navigation guard was added after this build and frontend check. The
direct Svelte compiler reports zero warnings for the updated pairing source.
The [bounded async check](pairing-navigation-check.cjs) runs the actual
production start and automatic methods. A current manual reply opens the
form. Late manual replies leave the page closed. Late automatic success and
failure cannot close or overwrite a newer page. This check uses controlled
replies and does not exercise native services.
The existing Qt WebChannel script notice remains in the build output because
Qt provides that script at runtime. See [check output](frontend-check.txt) and
[build output](frontend-build.txt). [SHA-256 identities](identities.json) record
source, lock, and compiled frontend assets.

The frontend build serves the visual work. It is not a native application,
platform package, or release build qualification result. Dependencies were
installed from the unchanged lock file with npm ci. No dependency was added.

## Visual review

The review uses the real Svelte components through a local Vite preview.
The adapter and its sample state are outside the repository at
`C:/Users/david/.codex/tmp/ticket23-preview/`. The adapter supplies Qt WebChannel
fixture replies and rewrites artwork paths only in the preview config.
It is outside the production entry point and RCC resource list.

The parent agent inspected the library, General Settings, Profiles, and
guided manual pairing at 1280 CSS pixels. It also inspected the library,
Audio/Wifi, and guided pairing in an actual 1024 CSS pixel iframe. Cards,
columns, and control labels fit the supported width. The ticket lead inspected
the three saved minimum-width captures. The screens match the selected
reference layouts. Saved captures are [library](library-1024.png),
[Audio/Wifi](settings-audio-1024.png), [pairing at 1024](registration-1024.png),
and [pairing at 1280](registration-1280.png). The iframe preview is
`http://127.0.0.1:8877/review-1024.html`. This preview contains representative
sample consoles and settings inventory. It proves layout only. It does not
prove native services, device availability, or persisted settings. The audio
fixture has a value/option mismatch and shows blank device selections. It
does not supply device behavior evidence.

## Preserved limits

No native media, input, stream overlays, lifetime, authentication, profiles,
packaging, or bridge implementation changed. Shared native QML components
remain. Completed ticket evidence remains valid within its stated source and
hardware limits; this work does not extend that evidence.

No native/platform build, package check, embedded runtime suite, live console,
physical input, GPU, HDR, audio, HighQuality diagnosis, or performance check
was run. No work below 1024 CSS pixels was added. The independent native
HighQuality failure remains documented. Production presets and defaults stay
unchanged. Unchecked acceptance items are unverified, not passed.
