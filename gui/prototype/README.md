# Console screen design lab

This is a disposable browser prototype of the chiaki-ng main console screen.
It uses HTML, CSS, and JavaScript, with no packages or build step.
It is not included in the Qt resource build.

## Open the prototype

Double-click `index.html`, or run this command from the repository root:

```powershell
python -m http.server 8765 --bind 127.0.0.1 --directory gui
```

Open `http://127.0.0.1:8765/prototype/?variant=A`.

## Compare the designs

- A, Console list: scan status and connect from each row.
- B, Card library: identify consoles by artwork and name.
- C, Focus view: select a console to see its details and connection action.

Use the bottom switcher or the Left and Right arrow keys to change designs.
The URL retains the selected design. Arrow keys retain their normal behavior
in form fields and dialogs. Use Tab, Enter, and Escape for controls and dialogs.

Use Scenario to compare the same design with no consoles, an unregistered
console, or long names. Reset restores that scenario's sample data.

Connect, wake-up, registration, PIN entry, add, and remove actions are simulations.
No account, network, storage, or real console is accessed. Reload clears changes.
Settings is an entry-point preview. Its contents are outside this design step.

## Change the design

- `styles.css`: shared color tokens and separate layout rules for A, B, and C.
- `app.js`: sample data, `VariantA`, `VariantB`, `VariantC`, and simulated actions.
- `index.html`: the preview shell and comparison controls.

Console artwork comes from the existing `gui/res` files.

## Check the prototype

With the local server running, open
`http://127.0.0.1:8765/prototype/self-check.html`.
The browser check exercises the layouts at 1280, 800, and 390 pixels wide,
variant switching, dialogs, focus restoration, search, escaped names,
registration, wake-up, removal, and discovery.

## Next step

The user selected B after removal of decorative text.
The implementation specification is `.scratch/ui-refresh/spec.md`.
Review the native implementation separately for rendering, keyboard focus,
controller navigation, and actual connection behavior.

Keep the prototype out of the application build. Once the design is approved,
preserve it on a prototype branch and link that revision from implementation
tickets. Do not ship the prototype JavaScript as application code.
