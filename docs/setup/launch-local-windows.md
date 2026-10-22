# Launch the local Windows app

Double-click `Launch-Recast.cmd` in the project folder. You can also use the
`Chiaki Recast` desktop shortcut created on this computer.

The launcher prepares the app from `release/Chiaki-Recast-Windows.zip`, then
opens it. You do not need to build the app or install a development runtime.
No download is required.

Keep the ZIP, `Launch-Recast.cmd`, and `scripts/launch-local-windows.ps1` in
their project folders. The ZIP includes the executable, DLLs, and Qt plugins.
The first launch extracts these files to `build/recast-launch`. Later launches
restore missing or incomplete files automatically. A changed ZIP updates the
installed copy. Close Recast before you replace the ZIP.

To launch from PowerShell, run this command in the project folder:

```powershell
.\Launch-Recast.cmd
```

The launcher does not change the system PATH. The app package is a saved build;
source edits require a new package before they appear in the launched app.

The app contains the mouse mapping changes from this session. Mouse movement
still maps to the right stick. See the [mouse aim tuning guide](../mouse-aim.md).
Mouse-to-gyro aiming is not implemented.

The desktop shortcut uses this project's absolute path. If you move the project,
use the launcher in its new folder and update the shortcut target.
