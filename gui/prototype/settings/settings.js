// PROTOTYPE ONLY. Selected sidebar layout; values stay in memory.
const categories = [
  ["General", "◫", "Session behavior, privacy and shortcuts"],
  ["Video", "▣", "Decoder, window and rendering"],
  ["Stream", "⇄", "Local and remote quality for each console type"],
  ["Audio/Wifi", "≋", "Devices, microphone and connection feedback"],
  ["Consoles", "▦", "Registered and hidden consoles"],
  ["Keys", "⌨", "Keyboard controls and mouse touchpad"],
  ["Controllers", "⊕", "Mapping, touchpad and haptics"],
  ["Remote", "◎", "PlayStation Network and remote connection"],
  ["Config", "⚙", "Profiles, settings files and logging"]
];
const params = new URLSearchParams(location.search);
let category = categories.some(c => c[0] === params.get("section")) ? params.get("section") : "General";
let subpage = Object.hasOwn(sections, params.get("page")) && !categories.some(c => c[0] === params.get("page")) ? params.get("page") : "";
let consoleType = "PS5";
let values = {};
let query = "";
let noticeTimer;
const layout = document.querySelector("#layout");
const esc = s => String(s).replace(/[&<>"']/g, c => ({ "&":"&amp;", "<":"&lt;", ">":"&gt;", '"':"&quot;", "'":"&#39;" }[c]));
const fieldId = key => "field-" + Array.from(key).map(c => c.codePointAt(0).toString(16)).join("-");
function field(row, prefix) {
  const [label, type, initial, description, meta = {}] = row;
  const key = prefix + "/" + label;
  const value = Object.hasOwn(values, key) ? values[key] : initial;
  const id = fieldId(key);
  const help = ` aria-describedby="${id}-help"`;
  const attrs = `id="${id}" data-key="${esc(key)}"${help}`;
  let control;
  if (Array.isArray(type)) control = `<div class="field"><select ${attrs}>${type.map(v => `<option${v === value ? " selected" : ""}>${esc(v)}</option>`).join("")}</select></div>`;
  else if (type === "toggle") control = `<input class="toggle" type="checkbox" role="switch" ${attrs}${meta.auto ? ' data-auto="true"' : ""}${value ? " checked" : ""}>`;
  else if (type === "action") control = `<button class="button" id="${id}" ${meta.page ? `data-page="${esc(meta.page)}"` : `data-demo="${esc(label)}"`} aria-label="${esc(label)}: ${esc(initial)}">${esc(initial)}</button>`;
  else if (type === "info") control = `<span class="info-value" id="${id}">${esc(initial)}</span>`;
  else if (type === "combo" || type === "mapping") control = `<div class="multi-field" role="group" aria-label="${esc(label)}">${value.map((v,i) => type === "combo" ?
    `<select data-key="${esc(key)}" data-part="${i}" aria-label="${esc(label)} button ${i+1}">${comboButtons.map(option=>`<option${option===v?" selected":""}>${option}</option>`).join("")}</select>` :
    `<input type="text" data-key="${esc(key)}" data-part="${i}" aria-label="${esc(label)} ${i ? "alternate" : "primary"} input" value="${esc(v)}" placeholder="Not mapped">`).join("")}</div>`;
  else {
    const step = meta.step && Math.abs((Number(initial) - (meta.min ?? 0)) / meta.step - Math.round((Number(initial) - (meta.min ?? 0)) / meta.step)) < .00001 ? meta.step : "any";
    control = `<div class="field"><input type="${type}" ${attrs} value="${esc(value)}"${type === "number" ? ` min="${meta.min ?? 0}"${meta.max !== undefined ? ` max="${meta.max}"` : ""} step="${step}"` : ""}></div>`;
  }
  const defaultText = meta.sample ? "Sample value" : "Default: " + (meta.defaultText || (type === "toggle" ? (initial ? "On" : "Off") : Array.isArray(initial) ? initial.join(" + ") : initial));
  const defaultVisible = !["action","info","mapping"].includes(type);
  return `<div class="row" data-source="${esc(meta.source || "SettingsDialog.qml")}"><div class="row-copy"><${["combo","mapping","info"].includes(type) ? "span class=\"label\"" : `label for="${id}"`}>${esc(label)}</${["combo","mapping","info"].includes(type) ? "span" : "label"}><p id="${id}-help">${esc(description)}${meta.condition ? `<span class="condition">${esc(meta.condition)}</span>` : ""}</p></div><div class="control">${control}${defaultVisible ? `<small class="default">${esc(defaultText)}</small>` : ""}</div></div>`;
}
function streamSections() {
  return ["Local", "Remote"].map(route => [route, [
    ["Resolution", consoleType === "PS5" ? ["360p", "540p", "720p", "1080p"] : ["360p", "540p", "720p", "1080p · PS4 Pro"], consoleType === "PS5" && route === "Local" ? "1080p" : "720p", "",{ref:`resolution${route}${consoleType}`}],
    ["Frame rate", ["30 fps", "60 fps"], "60 fps", "",{ref:`fps${route}${consoleType}`}],
    ["Bitrate", "number", consoleType === "PS5" && route === "Local" ? 15 : 10, "Mbps. Native automatic defaults: 360p = 2, 540p = 6, 720p = 10, 1080p = 15.",{ref:`bitrate${route}${consoleType}`,min:2,max:100,step:1,defaultText:"Based on resolution"}],
    ...(consoleType === "PS5" ? [["Codec", ["H264", "H265", "H265 HDR"], "H265", "",{ref:`codec${route}${consoleType}`}]] : [])
  ]]);
}
function renderSections(name, filter = "") {
  const list = name === "Stream" ? streamSections() : sections[name];
  return list.map(([group, rows]) => {
    const filtered = rows.filter(r => !filter || (name + " " + group + " " + r[0] + " " + r[3] + " " + (r[4]?.condition || "")).toLowerCase().includes(filter));
    if (!filtered.length) return "";
    const prefix = name === "Stream" ? `${name}/${consoleType}/${group}` : `${name}/${group}`;
    return `<section class="section"><h3>${esc(group)}${name === "Stream" ? `<span class="route-tag">${group === "Local" ? "Home network" : "Away from home"}</span>` : ""}</h3><div class="rows">${filtered.map(r => field(r, prefix)).join("")}</div></section>`;
  }).join("");
}
function body(name) {
  if (name === "Stream") return `<div class="stream-top"><div class="segmented" role="group" aria-label="Console type">${["PS5","PS4"].map(c => `<button data-console="${c}" aria-pressed="${consoleType === c}">${c}</button>`).join("")}</div><span>Separate values for each connection</span></div><div class="stream-grid">${renderSections(name)}</div>`;
  return `<div class="sections">${renderSections(name)}</div>`;
}
function heading(name) {
  return `<div class="category-head"><div><h2>${esc(name)}</h2><p>${esc(categories.find(c => c[0] === name)?.[2] || "All controls, including conditional options")}</p></div></div>`;
}
function nav() {
  return `<nav class="category-nav" aria-label="Settings categories"><div class="nav-label">PREFERENCES</div>${categories.map(([name, icon]) => `<button data-category="${esc(name)}" aria-pressed="${name === category}"><span class="nav-icon" aria-hidden="true">${icon}</span>${esc(name)}</button>`).join("")}</nav>`;
}
function VariantA() {
  const name = subpage || category;
  return `<div class="layout-A">${nav()}<div id="content" tabindex="-1">${subpage ? `<div class="back-row"><button class="button" data-back="true">← ${subpage === "Color mapping" ? "Renderer" : category}</button></div>` : ""}${heading(name)}${body(name)}</div></div>`;
}
function render() {
  if (query) {
    const matches = [...categories.map(([name]) => name), ...Object.keys(sections).filter(name => !categories.some(c => c[0] === name))].map(name => {
      const content = renderSections(name, query.toLowerCase());
      return content ? `<section>${heading(name)}${name === "Stream" ? `<small>Values shown for ${consoleType}</small>` : ""}${content}</section>` : "";
    }).join("");
    layout.innerHTML = `<div class="layout-A">${nav()}<div class="search-results" id="content" tabindex="-1"><h2>Search results</h2>${matches || '<p class="empty">No matching settings. Try another search.</p>'}</div></div>`;
  } else layout.innerHTML = VariantA();
  const url = new URL(location.href);
  url.searchParams.set("variant", "A");
  url.searchParams.set("section", category);
  if (subpage) url.searchParams.set("page", subpage); else url.searchParams.delete("page");
  history.replaceState(null, "", url);
}
function notify(message) {
  const notice = document.querySelector("#notice");
  notice.textContent = message;
  notice.classList.add("show");
  clearTimeout(noticeTimer);
  noticeTimer = setTimeout(() => notice.classList.remove("show"), 2000);
}
document.addEventListener("click", e => {
  const button = e.target.closest("button");
  if (!button) return;
  if (button.dataset.category) {
    category = button.dataset.category;
    subpage = "";
    query = "";
    document.querySelector("#search").value = "";
    render();
    document.querySelector(`[data-category="${category}"]`).focus();
    window.scrollTo(0,0);
  } else if (button.dataset.page || button.dataset.back) {
    subpage = button.dataset.page || (subpage === "Color mapping" ? "Renderer" : "");
    if (button.dataset.page && query) {
      category = ["Display","Renderer","Color mapping"].includes(subpage) ? "Video" : subpage === "Profiles" ? "Config" : "Controllers";
      query = "";
      document.querySelector("#search").value = "";
    }
    render();
    document.querySelector("#content").focus();
    window.scrollTo(0,0);
  } else if (button.dataset.console) {
    consoleType = button.dataset.console;
    render();
    document.querySelector(`[data-console="${consoleType}"]`).focus();
  } else if (button.dataset.demo) {
    document.querySelector("#dialog-title").textContent = button.dataset.demo;
    document.querySelector("#dialog-message").textContent = "Preview only. This action uses the existing native flow. No console, account, file or mapping is changed.";
    if (button.dataset.demo === "Reset all keys") {
      Object.keys(values).filter(key=>key.startsWith("Keys/Button mapping/")).forEach(key=>delete values[key]);
      render();
      notify("Sample key mappings reset");
      return;
    }
    document.querySelector("dialog").showModal();
  } else if (button.id === "reset") {
    values = {}; query = ""; consoleType = "PS5";
    document.querySelector("#search").value = "";
    render();
    notify("Demo values reset");
  }
});
layout.addEventListener("change", e => {
  const key = e.target.dataset.key;
  if (!key) return;
  if (e.target.dataset.part !== undefined) {
    values[key] = [...layout.querySelectorAll("[data-part]")].filter(input=>input.dataset.key === key).map(input=>input.value);
  } else values[key] = e.target.type === "checkbox" ? e.target.checked : e.target.value;
  if (e.target.dataset.auto && e.target.checked) {
    layout.querySelectorAll("[data-auto]").forEach(input => { if (input !== e.target) { input.checked=false; values[input.dataset.key]=false; } });
  }
  notify("Changed in this preview");
});
document.querySelector("#search").addEventListener("input", e => { query = e.target.value.trim(); render(); });
console.assert(categories.length === 9 && categories.every(([name]) => name === "Stream" || sections[name]), "All nine native categories must have content");
render();
