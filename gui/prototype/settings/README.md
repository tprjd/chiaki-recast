# Settings layout prototype

The user selected layout A, the category sidebar, on 2026-10-20. This disposable
browser prototype now contains the full Settings inventory for review. It is
not part of the Qt build. No native files or user settings are changed.

Run from the repository root:

```powershell
python -m http.server 8765 --bind 127.0.0.1 --directory gui
```

Open <http://127.0.0.1:8765/prototype/settings/?variant=A>.

Use the sidebar to open the nine native categories. Video opens Display
settings and Custom renderer settings. Renderer opens Color Mapping.
Config opens Profiles. Controllers opens the controller mapping page.

Values remain in memory when you change categories. Reload or Reset demo
clears changes. The native app and account are not connected.

Coverage includes all nine categories in `SettingsDialog.qml`, all 26 keyboard
mappings, and all 23 controller mappings with primary and alternate input.
Display has six controls. Renderer has 39 visible controls. Color Mapping has
28 controls. Profile creation, switching and deletion fields are shown.

Conditional controls stay visible for this inventory review. Their notes
describe when the native app shows them. This also exposes Steam Deck and
speech-processing controls on a Windows preview. It is not a proposal to
expose unsupported settings in the native application.

Native choices, ranges and displayed defaults are retained. Audio volume,
audio buffer size, touch increment and haptic intensity use their displayed
units. Device choices, console identities and profiles use sample data.
Search includes linked settings pages. Local and Remote stream values remain
separate for PS4 and PS5.

Action buttons for login, registration, file access, deletion and applying
controller mappings open a preview notice. They do not run those flows.
Key and controller mapping inputs accept sample names instead of capturing
real device input. The permanently hidden renderer-documentation URL field
is omitted. Its documentation and copy actions are present.

Open <http://127.0.0.1:8765/prototype/settings/check.html> for browser checks
at 1280, 800 and 390 pixels wide. The inventory check in
`.scratch/settings-prototype/check-inventory.cjs` compares native property
writes, advanced control counts, options and ranges, and keyboard mapping
count against the prototype data.

The layout choice is recorded. Native implementation still requires the
user's review of this expanded prototype.
