// PROTOTYPE ONLY: All actions affect in-memory samples. No network or account access.
const icons = {
  gamepad: '<path d="M6 8h12l3 9c.5 2-2 3-3 1l-2-2H8l-2 2c-1 2-3.5 1-3-1Z"/><path d="M8 10v4m-2-2h4m6-1h.01M18 13h.01"/>',
  plus: '<path d="M12 5v14M5 12h14"/>',
  settings: '<path d="m9 3-.8 3-2.9.8L3 9l2 3-2 3 2.3 2.2 2.9.8.8 3h6l.8-3 2.9-.8L21 15l-2-3 2-3-2.3-2.2-2.9-.8L15 3Z"/><circle cx="12" cy="12" r="3"/>',
  network: '<rect x="8" y="3" width="8" height="5" rx="1"/><path d="M12 8v5M5 16v-3h14v3"/><rect x="2" y="16" width="6" height="5" rx="1"/><rect x="16" y="16" width="6" height="5" rx="1"/>',
  globe: '<circle cx="12" cy="12" r="9"/><ellipse cx="12" cy="12" rx="4" ry="9"/><path d="M3 12h18"/>',
  refresh: '<path d="M20 7v5h-5M4 17v-5h5M5 8a8 8 0 0 1 13-3l2 3M4 16l2 3a8 8 0 0 0 13-3"/>',
  search: '<circle cx="10.5" cy="10.5" r="6.5"/><path d="m16 16 5 5"/>',
  arrow: '<path d="M5 12h14m-6-6 6 6-6 6"/>',
  power: '<path d="M12 3v9m-5-7a9 9 0 1 0 10 0"/>',
  more: '<circle cx="5" cy="12" r="1"/><circle cx="12" cy="12" r="1"/><circle cx="19" cy="12" r="1"/>',
  close: '<path d="m6 6 12 12M6 18 18 6"/>',
  screen: '<rect x="3" y="4" width="18" height="13" rx="2"/><path d="M8 21h8m-4-4v4"/>',
  grid: '<rect x="3" y="3" width="7" height="7" rx="1"/><rect x="14" y="3" width="7" height="7" rx="1"/><rect x="3" y="14" width="7" height="7" rx="1"/><rect x="14" y="14" width="7" height="7" rx="1"/>',
  info: '<circle cx="12" cy="12" r="9"/><path d="M12 11v6m0-10h.01"/>',
  check: '<path d="m5 12 4 4L19 6"/>',
};
const icon = name => `<svg class="icon" viewBox="0 0 24 24" aria-hidden="true">${icons[name] || icons.info}</svg>`;
const escapeHtml = value => String(value).replace(/[&<>"']/g, c => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" })[c]);
const variants = {
  A: { name: "Console list", description: "List with per-console actions." },
  B: { name: "Card library", description: "Cards with per-console actions." },
  C: { name: "Focus view", description: "Console selector with a details panel." },
};
const sampleConsoles = [
  { id: "living", name: "Living room", model: "PS5", status: "online", route: "Local network", address: "192.168.1.42", registered: true, manual: false, game: "ASTRO BOT" },
  { id: "bedroom", name: "Bedroom", model: "PS5", status: "standby", route: "Local network", address: "192.168.1.64", registered: true, manual: true, game: "" },
  { id: "studio", name: "Studio", model: "PS4", status: "offline", route: "Local network", address: "192.168.1.86", registered: true, manual: true, game: "" },
];
const url = new URL(location.href);
const state = {
  variant: Object.hasOwn(variants, url.searchParams.get("variant")) ? url.searchParams.get("variant") : "A",
  consoles: structuredClone(sampleConsoles),
  selected: "living",
  query: "",
  scenario: "everyday",
  discovery: true,
};
const app = document.querySelector("#app");
const dialog = document.querySelector("#dialog");
let toastTimeout;
let dialogOpener;
let nextManualId = 1;

function button(label, action, consoleId = "", style = "", symbol = "") {
  return `<button class="button ${style}" data-action="${action}"${consoleId ? ` data-id="${escapeHtml(consoleId)}"` : ""}>${symbol ? icon(symbol) : ""}${label}</button>`;
}
function brand() {
  return `<div class="brand"><span class="brand-mark">${icon("gamepad")}</span><span class="brand-word">chiaki<em>-ng</em></span></div>`;
}
function search() {
  return `<label class="search">${icon("search")}<input id="search" type="search" aria-label="Find a console" placeholder="Find a console" value="${escapeHtml(state.query)}" autocomplete="off"></label>`;
}
function picture(console, className = "") {
  return `<img class="console-picture ${console.status} ${className}" src="../res/console-${console.model.toLowerCase()}.svg" alt="" draggable="false">`;
}
function status(console, pill = false) {
  const labels = { online: "Online", standby: "In standby", offline: "Offline", unregistered: "Not registered" };
  const value = console.registered ? console.status : "unregistered";
  return `<span class="status ${value} ${pill ? "pill" : ""}">${labels[value]}</span>`;
}
function primaryAction(console) {
  if (!console.registered) return button("Register", "register", console.id, "primary", "plus");
  if (console.status === "standby") return button("Wake up", "wake", console.id, "", "power");
  if (console.status === "offline") return '<button class="button" disabled title="The console must be available before you can connect">Unavailable</button>';
  return button("Connect", "connect", console.id, "primary", "arrow");
}
function more(console) {
  return `<button class="icon-button" data-action="details" data-id="${escapeHtml(console.id)}" aria-label="Details for ${escapeHtml(console.name)}">${icon("more")}</button>`;
}
function discovery() {
  return `<button class="discovery-control" data-action="discovery" aria-pressed="${state.discovery}">${icon("network")}Discovery ${state.discovery ? "on" : "off"}</button>`;
}
function topbar() {
  return `<header class="topbar">${brand()}<div class="header-actions"><span class="connection-label">Local network</span>${button("Add console", "add", "", "ghost", "plus")}<button class="icon-button" data-action="settings" aria-label="Settings">${icon("settings")}</button></div></header>`;
}
function empty() {
  const filtered = Boolean(state.query);
  return `<div class="empty">${icon(filtered ? "search" : "gamepad")}<h2>${filtered ? "No matching consoles" : "No consoles found"}</h2><p>${filtered ? "Try another name, or clear your search to see all consoles." : "Turn on your PlayStation and connect to the same network, or add a console manually."}</p>${button(filtered ? "Clear search" : "Add console", filtered ? "clear-search" : "add", "", "primary", filtered ? "close" : "plus")}</div>`;
}
function footer(consoles) {
  return `<footer class="footer">${discovery()}<span>${consoles.length} of ${state.consoles.length} consoles · ${state.consoles.filter(c => c.status === "online").length} online</span></footer>`;
}
function VariantA(consoles) {
  return `<div class="app-shell variant-a">${topbar()}<main id="main" class="workspace" tabindex="-1">
    <div class="page-heading"><h1>Consoles</h1>${search()}</div>
    <div class="section-heading"><h2>Your consoles <span class="count">${state.consoles.length}</span></h2>${button("Refresh", "refresh", "", "subtle", "refresh")}</div>
    ${consoles.length ? `<div class="table-labels" aria-hidden="true"><span>CONSOLE</span><span>STATUS</span><span class="network-column">CONNECTION</span><span></span></div><div class="console-list">${consoles.map(c => `
      <article class="console-row" aria-label="${escapeHtml(c.name)}">
        <div class="row-identity">${picture(c)}<div><h3 class="console-name">${escapeHtml(c.name)}</h3><p>${c.model}${c.game ? ` · ${escapeHtml(c.game)}` : ""}</p></div></div>
        ${status(c)}<span class="network-column secondary">${escapeHtml(c.route)}</span>
        <div class="row-actions">${primaryAction(c)}${more(c)}</div>
      </article>`).join("")}</div>` : empty()}
    ${footer(consoles)}
  </main></div>`;
}
function VariantB(consoles) {
  return `<div class="app-shell variant-b">
    <aside class="rail" aria-label="Application sidebar">${brand()}
      <nav class="rail-section" aria-label="Main navigation"><button class="rail-link active" data-action="clear-search" aria-current="page" aria-label="Consoles">${icon("grid")}<span>Consoles</span></button><button class="rail-link" data-action="settings" aria-label="Settings">${icon("settings")}<span>Settings</span></button></nav>
    </aside>
    <main id="main" class="workspace" tabindex="-1">
      <div class="page-heading"><h1>Consoles <span class="count">${state.consoles.length}</span></h1>${button("Add console", "add", "", "ghost", "plus")}</div>
      <div class="library-toolbar">${search()}${button("Refresh", "refresh", "", "subtle", "refresh")}</div>
      ${consoles.length ? `<div class="card-grid">${consoles.map(c => `
        <article class="console-card" aria-label="${escapeHtml(c.name)}">
          <div class="card-art ${c.status}"><div class="card-art-top"><span>PLAYSTATION ${c.model.slice(-1)}</span>${icon("network")}</div>${picture(c)}</div>
          <div class="card-body"><h2 class="console-name">${escapeHtml(c.name)}</h2>${status(c)}
            <div class="card-meta">${icon(c.game ? "gamepad" : "network")}<span>${escapeHtml(c.game || c.route)}</span></div>
            <div class="button-row">${primaryAction(c)}${more(c)}</div>
          </div>
        </article>`).join("")}</div>` : empty()}
      ${footer(consoles)}
    </main>
  </div>`;
}
function VariantC(consoles) {
  const selected = consoles.find(c => c.id === state.selected) || consoles[0];
  if (selected) state.selected = selected.id;
  return `<div class="app-shell variant-c">${topbar()}<main id="main" class="focus-workspace" tabindex="-1">
    <section class="console-sidebar" aria-label="Choose a console"><div class="section-heading"><h1>Consoles <span class="count">${state.consoles.length}</span></h1><button class="icon-button" data-action="refresh" aria-label="Refresh consoles">${icon("refresh")}</button></div>${search()}
      <div class="console-picker">${consoles.map(c => `<button class="pick-console" data-action="select" data-id="${escapeHtml(c.id)}" aria-pressed="${c.id === selected?.id}">${picture(c)}<span><strong class="console-name">${escapeHtml(c.name)}</strong>${status(c)}</span></button>`).join("")}</div>${discovery()}
    </section>
    <section class="focus-content" aria-label="Selected console">${selected ? `
      <div class="focus-top"><span class="eyebrow">PlayStation ${selected.model.slice(-1)}</span>${more(selected)}</div>
      <div class="focus-art">${picture(selected)}</div>
      <div class="focus-title"><div><h1 class="console-name">${escapeHtml(selected.name)}</h1>${selected.game ? `<p>Running: ${escapeHtml(selected.game)}</p>` : ""}</div>${status(selected, true)}</div>
      <dl class="focus-details"><div><dt>Connection</dt><dd>${escapeHtml(selected.route)}</dd></div><div><dt>Address</dt><dd>${escapeHtml(selected.address)}</dd></div><div><dt>Console</dt><dd>PlayStation ${selected.model.slice(-1)}</dd></div><div><dt>Registration</dt><dd>${selected.registered ? "Registered" : "Required before your first session"}</dd></div></dl>
      <div class="focus-action">${primaryAction(selected)}${!selected.registered ? "<p>Register this console before connecting.</p>" : selected.status === "standby" ? "<p>Wake your console, then connect when it is ready.</p>" : selected.status === "offline" ? "<p>Turn on your console and check its network connection.</p>" : ""}</div>
    ` : empty()}</section>
  </main></div>`;
}
function render() {
  // Keep keyboard focus stable when a simulated action replaces the main view.
  const focused = document.activeElement;
  const focusId = focused?.id;
  const selection = focusId === "search" ? [focused.selectionStart, focused.selectionEnd] : null;
  const action = focused?.dataset.action;
  const consoleId = focused?.dataset.id;
  const consoles = state.consoles.filter(c => `${c.name} ${c.model}`.toLowerCase().includes(state.query.toLowerCase()));
  app.innerHTML = ({ A: VariantA, B: VariantB, C: VariantC })[state.variant](consoles);
  document.title = `chiaki-ng · ${state.variant} · ${variants[state.variant].name}`;
  document.querySelectorAll("[data-variant]").forEach(b => b.setAttribute("aria-pressed", String(b.dataset.variant === state.variant)));
  document.querySelector("#design-description").textContent = variants[state.variant].description;
  if (focusId === "search") {
    const searchField = document.querySelector("#search");
    searchField.focus();
    searchField.setSelectionRange(...selection);
  }
  else if (focused && !focused.isConnected && action) {
    const replacement = [...app.querySelectorAll("[data-action]")].find(b => b.dataset.action === action && b.dataset.id === consoleId)
      || [...app.querySelectorAll("[data-id]")].find(b => b.dataset.id === consoleId)
      || document.querySelector("#main");
    replacement.focus();
  }
}
function setVariant(variant, updateUrl = true) {
  state.variant = Object.hasOwn(variants, variant) ? variant : "A";
  if (updateUrl) {
    const nextUrl = new URL(location.href);
    nextUrl.searchParams.set("variant", state.variant);
    history.replaceState(null, "", nextUrl);
  }
  render();
}
function resetScenario(scenario = state.scenario) {
  state.scenario = scenario;
  state.consoles = scenario === "empty" ? [] : structuredClone(sampleConsoles);
  if (scenario === "registration") state.consoles.push({ id: "new", name: "New PlayStation", model: "PS5", status: "online", route: "Local network", address: "192.168.1.97", registered: false, manual: false, game: "" });
  if (scenario === "long") {
    state.consoles[0].name = "Living room PlayStation — shared with the whole family";
    state.consoles[1].name = "Upstairs bedroom & weekend gaming corner";
    state.consoles[2].name = "Studio PlayStation 4 — the original";
  }
  state.query = "";
  state.selected = state.consoles[0]?.id || "";
  state.discovery = true;
  document.querySelector("#scenario").value = scenario;
  render();
}
function notify(message) {
  clearTimeout(toastTimeout);
  document.querySelector("#toast").textContent = message;
  toastTimeout = setTimeout(() => { document.querySelector("#toast").textContent = ""; }, 4200);
}
function showDialog(title, body, subtitle = "DESIGN PREVIEW") {
  if (!dialog.open) dialogOpener = document.activeElement;
  dialog.innerHTML = `<div class="dialog-header"><div><p class="dialog-tag">${subtitle}</p><h2 id="dialog-title">${escapeHtml(title)}</h2></div><button class="icon-button" data-action="close" aria-label="Close dialog">${icon("close")}</button></div>${body}`;
  if (!dialog.open) dialog.showModal();
}
function details(console) {
  showDialog(console.name, `${status(console)}<dl class="details-grid"><div><dt>Console</dt><dd>${console.model}</dd></div><div><dt>Registration</dt><dd>${console.registered ? "Registered" : "Not registered"}</dd></div><div><dt>Address</dt><dd>${escapeHtml(console.address)}</dd></div><div><dt>Connection</dt><dd>${escapeHtml(console.route)}</dd></div></dl><p>Sample console details. All actions in this design lab are simulated.</p>
    <div class="button-row">${console.registered ? button("Console PIN", "pin", console.id, "ghost") : ""}${console.manual || !console.registered ? button(console.manual ? "Delete console" : "Hide console", "remove", console.id, "danger") : ""}${primaryAction(console)}</div>`);
}
document.addEventListener("click", event => {
  const target = event.target.closest("button");
  if (!target || target.disabled) return;
  if (target.dataset.variant) { setVariant(target.dataset.variant); return; }
  const action = target.dataset.action;
  const console = state.consoles.find(c => c.id === target.dataset.id);
  if (action === "previous" || action === "next") {
    const keys = Object.keys(variants);
    setVariant(keys[(keys.indexOf(state.variant) + (action === "next" ? 1 : 2)) % keys.length]);
  } else if (action === "reset") { resetScenario(); notify("Sample data reset."); }
  else if (action === "close") dialog.close();
  else if (action === "select" && console) { state.selected = console.id; render(); }
  else if (action === "clear-search") { state.query = ""; render(); }
  else if (action === "discovery") { state.discovery = !state.discovery; render(); notify(`Discovery ${state.discovery ? "enabled" : "paused"} in this preview.`); }
  else if (action === "refresh") notify("Sample consoles refreshed. This preview does not scan your network.");
  else if (action === "details" && console) details(console);
  else if (action === "wake" && console) {
    console.status = "online";
    if (dialog.open) dialog.close();
    render();
    notify(`${console.name} is online. Wake-up simulated.`);
  } else if (action === "connect" && console) {
    showDialog("Connection preview", `<div class="preview-screen">${icon("gamepad")}<span>Remote Play preview</span></div><p><strong>${escapeHtml(console.name)}</strong> is selected. In the app, Connect would start streaming from this console.</p><p>No connection is made in this prototype.</p><div class="button-row">${button("Back to consoles", "close", "", "primary")}</div>`, "SIMULATED CONNECTION");
  } else if (action === "settings") {
    showDialog("Settings", `<p>This prototype covers the main console screen. This button shows where app settings would open.</p><p>Settings controls will be designed in a later step.</p><div class="button-row">${button("Back to consoles", "close", "", "primary")}</div>`);
  } else if (action === "add") {
    showDialog("Add a console", `<p>Add a sample console to compare the layouts. Nothing is saved when you reload.</p><form id="add-form"><label>Console name<input name="name" placeholder="e.g. Office" required maxlength="80"></label><label>Address<input name="address" placeholder="e.g. 192.168.1.100" required maxlength="100"></label><label>Console type<select name="model"><option>PS5</option><option>PS4</option></select></label><div class="button-row"><button type="button" class="button ghost" data-action="close">Cancel</button><button class="button primary" type="submit">Add sample console</button></div></form>`);
  } else if (action === "register" && console) {
    showDialog("Register your console", `<p>In the app, you would use the registration code from your PlayStation. Enter any eight digits to preview this step.</p><form id="register-form" data-id="${escapeHtml(console.id)}"><label>Registration code<input name="code" inputmode="numeric" pattern="[0-9]{8}" maxlength="8" placeholder="12345678" required autocomplete="off"></label><div class="button-row"><button type="button" class="button ghost" data-action="close">Cancel</button><button class="button primary" type="submit">Register sample console</button></div></form>`);
  } else if (action === "pin" && console) {
    showDialog("Console PIN", `<p>Preview the entry point for the console PIN. This form does not save or send a PIN.</p><form id="pin-form"><label>Console PIN<input name="pin" type="password" inputmode="numeric" pattern="[0-9]{4}" maxlength="4" placeholder="4 digits" required autocomplete="off"></label><div class="button-row"><button type="button" class="button ghost" data-action="close">Cancel</button><button class="button primary" type="submit">Finish preview</button></div></form>`);
  } else if (action === "remove" && console) {
    showDialog(`Remove ${console.name}?`, `<p>This removes only the sample console from this preview. Use Reset to restore the sample data.</p><div class="button-row">${button("Cancel", "close", "", "ghost")}${button("Remove sample", "confirm-remove", console.id, "danger")}</div>`);
  } else if (action === "confirm-remove" && console) {
    state.consoles = state.consoles.filter(c => c.id !== console.id);
    dialog.close();
    render();
    notify("Sample console removed.");
  }
});
document.addEventListener("input", event => {
  if (event.target.id === "search") { state.query = event.target.value; render(); }
});
document.querySelector("#scenario").addEventListener("change", event => resetScenario(event.target.value));
document.addEventListener("keydown", event => {
  if (dialog.open || event.altKey || event.ctrlKey || event.metaKey || event.shiftKey || event.target.closest('input, textarea, select, [contenteditable]')) return;
  if (event.key === "ArrowLeft" || event.key === "ArrowRight") {
    event.preventDefault();
    document.querySelector(`[data-action="${event.key === "ArrowLeft" ? "previous" : "next"}"]`).click();
  }
});
dialog.addEventListener("submit", event => {
  event.preventDefault();
  const form = event.target;
  const data = new FormData(form);
  if (form.id === "add-form") {
    const name = data.get("name").trim();
    const address = data.get("address").trim();
    if (!name || !address) {
      const field = form.elements[!name ? "name" : "address"];
      field.setCustomValidity("Enter a value with at least one non-space character.");
      field.reportValidity();
      field.addEventListener("input", () => field.setCustomValidity(""), { once: true });
      return;
    }
    const id = `manual-${nextManualId++}`;
    state.consoles.push({ id, name, address, model: data.get("model"), status: "online", route: "Local network", registered: false, manual: true, game: "" });
    state.selected = id;
    state.query = "";
    notify("Sample console added. Register it to preview Connect.");
  } else if (form.id === "register-form") {
    const console = state.consoles.find(c => c.id === form.dataset.id);
    if (console) console.registered = true;
    notify("Registration simulated. Your sample console is ready.");
  } else if (form.id === "pin-form") notify("PIN entry preview complete. Nothing was saved.");
  dialog.close();
  render();
});
dialog.addEventListener("close", () => {
  // Rendering can replace the element that opened the dialog.
  const candidates = [...app.querySelectorAll("[data-action]")];
  const replacement = dialogOpener?.isConnected ? dialogOpener
    : candidates.find(b => b.dataset.action === dialogOpener?.dataset.action && b.dataset.id === dialogOpener?.dataset.id)
      || candidates.find(b => b.dataset.id && b.dataset.id === dialogOpener?.dataset.id)
      || document.querySelector("#main");
  replacement?.focus();
});
window.addEventListener("popstate", () => setVariant(new URL(location.href).searchParams.get("variant"), false));
render();
