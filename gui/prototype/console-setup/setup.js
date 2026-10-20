// PROTOTYPE ONLY. Sample values stay in this tab. No console or account requests.
const names = { A: 'Guided workspace', B: 'Compact sheet', C: 'Settings rows' };
const notes = {
  A: 'A focused form with console instructions alongside it.',
  B: 'A compact sheet keeps the console library in view.',
  C: 'Numbered sections use the same rhythm as Settings.'
};
const params = new URLSearchParams(location.search);
let variant = names[params.get('variant')] ? params.get('variant') : 'A';
let screen = params.get('screen') === 'manual' ? 'manual' : 'register';
let phase = ['form', 'progress', 'success', 'failure'].includes(params.get('state')) ? params.get('state') : 'form';
if (screen === 'manual' && phase === 'progress') phase = 'form';
let hidden = false;
const defaults = {
  register: { console: 'ps5', address: '192.0.2.42', account: '', online: '', pin: '', consolePin: '' },
  manual: { address: '', savedConsole: 'choose' }
};
let values = structuredClone(defaults);
const content = document.querySelector('#content');
const escape = value => String(value).replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const isLegacy = () => values.register.console === 'ps4old';
const consoleName = () => values.register.console === 'ps5' ? 'PlayStation 5' : 'PlayStation 4';
const title = () => screen === 'register' ? 'Register console' : 'Add manual console';
const subtitle = () => screen === 'register'
  ? 'Pair your console with chiaki-ng for Remote Play.'
  : 'Add an address for a console that discovery cannot find.';
const consoleIcon = `<div class="console-symbol" aria-hidden="true"><svg viewBox="0 0 40 40"><path d="M11 4 8 35h9V7ZM24 7v28h8L29 4M17 10h7M17 33h7M13 32h1"/></svg></div>`;

function inputField(key, label, hint, placeholder = '', extra = '') {
  const numeric = key === 'pin' || key === 'consolePin';
  const max = key === 'pin' ? 8 : 4;
  return `<div class="field ${extra}"><label for="${key}">${label}</label>
    <input id="${key}" name="${key}" value="${escape(values[screen][key])}" type="${hidden ? 'password' : 'text'}" placeholder="${escape(placeholder)}"
      ${numeric ? `inputmode="numeric" maxlength="${max}" class="pin"` : ''} autocomplete="off" spellcheck="false" aria-describedby="${key}-hint">
    <small id="${key}-hint">${hint}</small>${numeric ? `<small class="input-note" id="${key}-error" hidden></small>` : ''}</div>`;
}
function consoleField() {
  const options = [['ps5', 'PS5'], ['ps4', 'PS4 · Firmware 8.0 or later'], ['ps4v7', 'PS4 · Firmware 7.0 to before 8.0'], ['ps4old', 'PS4 · Firmware before 7.0']];
  return `<div class="field"><label for="console">Console type</label><select id="console" name="console">${options.map(([v, text]) => `<option value="${v}" ${values.register.console === v ? 'selected' : ''}>${text}</option>`).join('')}</select><small>Choose the console and its firmware version.</small></div>`;
}
function accountField(extra = '') {
  if (isLegacy()) return inputField('online', 'PSN Online ID', 'Use the exact spelling and letter case of your username.', 'Your PSN username', extra);
  return inputField('account', 'PSN Account ID', 'Your account ID in Base64 format. This is not your username.', 'Paste your account ID', extra)
    .replace('</div>', `<div class="field-actions"><button type="button" class="text-button" data-action="login">PSN sign-in ↗</button><button type="button" class="text-button" data-action="lookup">Public lookup ↗</button></div></div>`);
}
function addressField() {
  return inputField('address', 'Console address', screen === 'register' ? 'An IP address or hostname. Use 255.255.255.255 for local broadcast.' : 'An IP address or hostname, such as 192.0.2.42 or ps5.local.', '192.0.2.42');
}
function pinField() { return inputField('pin', 'Remote Play PIN', 'Enter the 8-digit code shown by your console.', '00000000'); }
function consolePinField() { return inputField('consolePin', 'Console PIN <span class="optional">Optional</span>', 'Your 4-digit console login PIN, if you use one.', '0000'); }
function savedConsoleField() {
  const options = [['choose', 'Select a console'], ['living', 'Living room · PS5'], ['desk', 'Desk · PS4'], ['later', 'Register on first connection']];
  return `<div class="field"><label for="savedConsole">Registered console</label><select id="savedConsole" name="savedConsole">${options.map(([v, text]) => `<option value="${v}" ${values.manual.savedConsole === v ? 'selected' : ''}>${text}</option>`).join('')}</select><small id="savedConsole-hint">${values.manual.savedConsole === 'later' ? 'The address is saved now. Registration opens when you first connect.' : 'Use an existing registration, or register when you first connect.'}</small></div>`;
}
function valid() {
  const v = values[screen];
  return screen === 'manual' ? Boolean(v.address.trim() && v.savedConsole !== 'choose')
    : Boolean(v.address.trim() && (isLegacy() ? v.online : v.account).trim() && /^\d{8}$/.test(v.pin) && /^(\d{4})?$/.test(v.consolePin));
}
function actions() {
  return `<footer class="actions"><small>${screen === 'register' ? 'Keep your console on during registration.' : 'Adding an address does not start a connection.'}</small><div class="button-row"><button type="button" class="button ghost" data-action="cancel">Cancel</button><button type="submit" class="button primary" id="submit" ${valid() ? '' : 'disabled'}>${screen === 'register' ? 'Register console' : 'Add console'} <span aria-hidden="true">→</span></button></div></footer>`;
}
function help() {
  return `<aside class="help-panel" aria-label="Setup help">${consoleIcon}${screen === 'register' ? `
    <div class="section-kicker">On your ${consoleName()}</div><h2>Get your pairing code</h2>
    <p>Use the account that you want to play with.</p>
    <ol><li><span class="help-number">1</span><div><strong>Open Settings</strong><p>${values.register.console === 'ps5' ? 'Go to System → Remote Play.' : 'Go to Remote Play Connection Settings.'}</p></div></li>
    <li><span class="help-number">2</span><div><strong>Enable Remote Play</strong><p>Keep your console on and connected to your network.</p></div></li>
    <li><span class="help-number">3</span><div><strong>${values.register.console === 'ps5' ? 'Select Link Device' : 'Select Add Device'}</strong><p>Enter the 8-digit code before it expires.</p></div></li></ol>
    <div class="help-footer"><h3>Two PINs, two purposes</h3><p>The Remote Play PIN pairs the console. The optional console PIN signs in to your console user.</p></div>` : `
    <div class="section-kicker">Connect by address</div><h2>Same console.<br>Another way to find it.</h2><p>Use a manual address when automatic discovery does not show your console.</p>
    <ol><li><span class="help-number">1</span><div><strong>Enter its address</strong><p>Find the console’s IP address in its network settings.</p></div></li><li><span class="help-number">2</span><div><strong>Choose its registration</strong><p>Select the console that belongs to this address.</p></div></li></ol>
    <div class="help-footer"><h3>Not registered yet?</h3><p>Choose “Register on first connection”. You can pair it when you connect from the library.</p></div>`}</aside>`;
}
function shortHelp() {
  return screen === 'register' ? `<strong>Find the Remote Play PIN on your console</strong><br>${values.register.console === 'ps5' ? 'Settings → System → Remote Play → Link Device' : 'Settings → Remote Play Connection Settings → Add Device'}`
    : '<strong>Use an existing console registration</strong><br>Or choose “Register on first connection” to pair the console later.';
}
function formA() {
  return `<form novalidate><div class="section-kicker">${screen === 'register' ? 'Console details' : 'Manual address'}</div><h2>${screen === 'register' ? 'Set up Remote Play' : 'Find your console'}</h2>${screen === 'register' ? `${consoleField()}${addressField()}<hr class="rule">${accountField()}<div class="pair">${pinField()}${consolePinField()}</div>` : `${addressField()}${savedConsoleField()}`}${actions()}</form>`;
}
function formB() {
  return `<form novalidate><div class="inline-help"><span aria-hidden="true">ⓘ</span><p>${shortHelp()}</p></div><div class="sheet-fields">${screen === 'register' ? `${consoleField()}${addressField()}${accountField('wide')}${pinField()}${consolePinField()}` : `${addressField()}${savedConsoleField()}`}</div>${actions()}</form>`;
}
function section(number, label, description, fields) {
  return `<section class="settings-section"><div class="section-side"><div class="section-kicker">${number}</div><h2>${label}</h2><p>${description}</p></div><div class="settings-fields">${fields}</div></section>`;
}
function formC() {
  return `<div class="settings-banner"><p>${shortHelp()}</p></div><form class="settings-form" novalidate>${screen === 'register' ?
    section('01', 'Console', 'Choose what to pair and where to find it.', consoleField() + addressField()) +
    section('02', 'Your account', 'Use the same PSN account as your console user.', accountField()) +
    section('03', 'Pairing', 'Enter the code from your console.', pinField() + consolePinField()) :
    section('01', 'Address', 'Find this console without discovery.', addressField()) +
    section('02', 'Registration', 'Choose which console uses this address.', savedConsoleField())}${actions()}</form>`;
}
function result() {
  const registration = screen === 'register';
  const address = hidden ? '••••••••' : escape(values[screen].address || '192.0.2.42');
  const labels = registration ? { progress: 'Registering console', success: 'Console registered', failure: 'Registration failed' } : { success: 'Manual console added', failure: 'Console was not added' };
  const description = phase === 'progress' ? 'Waiting for the console to complete pairing.' : phase === 'success' ? (registration ? 'Your console is ready for Remote Play.' : 'The address is now available in your console library.') : (registration ? 'The console did not accept the Remote Play PIN.' : 'The console registration is no longer available.');
  const diagnostic = phase === 'failure' ? (registration ? '[sample] Registration request rejected.\n[sample] The Remote Play PIN is incorrect or expired.' : '[sample] The selected registration could not be found.\n[sample] No manual address was saved.') : phase === 'progress' ? '[sample] Connecting to console…\n[sample] Registration request sent.\n[sample] Waiting for response…' : (registration ? '[sample] Registration completed.\n[sample] Console added to registered consoles.' : '[sample] Manual address saved.');
  return `<section class="result" data-state="${phase}" aria-label="${labels[phase]}"><div class="result-heading"><div class="result-icon" aria-hidden="true">${phase === 'success' ? '✓' : phase === 'failure' ? '!' : '↻'}</div><div><h2>${labels[phase]}</h2><p>${description}</p></div></div>
    ${phase === 'progress' ? `<div class="progress-track" aria-hidden="true"><span></span></div><ul class="operation-steps"><li class="done"><span>✓</span> Console reached</li><li class="current"><span>◌</span> Pairing your account</li><li><span>○</span> Save console registration</li></ul>` : ''}
    <dl class="result-data"><dt>Console address</dt><dd>${address}</dd><dt>${registration ? 'Console type' : 'Registration'}</dt><dd>${registration ? consoleName() : values.manual.savedConsole === 'later' ? 'Register on first connection' : values.manual.savedConsole === 'desk' ? 'Desk · PS4' : 'Living room · PS5'}</dd></dl>
    ${phase === 'failure' ? `<p class="result-note"><strong>${registration ? 'Get a new code and try again.' : 'Choose the registration again.'}</strong><br>${registration ? 'Open the Remote Play settings on your console and generate a new 8-digit PIN. Your other entries are kept.' : 'The console may have been removed from Settings. Select another registration, or register on first connection.'}</p>` : phase === 'success' && !registration && values.manual.savedConsole === 'later' ? '<p class="result-note">Registration will open when you first connect to this address.</p>' : ''}
    <details><summary>Diagnostic log</summary><pre>${diagnostic}</pre></details>
    <div class="button-row">${phase === 'progress' ? '<button class="button ghost" data-action="dismiss">Close progress</button>' : phase === 'failure' ? '<button class="button ghost" data-action="cancel">Cancel</button><button class="button primary" data-action="edit">Back to details</button>' : '<button class="button primary" data-action="library">Back to consoles →</button>'}</div></section>`;
}
function header() { return `<div class="breadcrumb"><a href="../?variant=B">← Consoles</a><span>/</span><span>${title()}</span></div><header class="page-head"><div><h1>${title()}</h1><p>${subtitle()}</p></div><span class="tag">${screen === 'register' ? 'Remote Play setup' : 'Manual connection'}</span></header>`; }
function render() {
  document.body.dataset.variant = variant;
  const url = new URL(location.href);
  url.searchParams.set('variant', variant); url.searchParams.set('screen', screen); url.searchParams.set('state', phase);
  history.replaceState(null, '', url);
  if (variant === 'A') content.innerHTML = `${header()}<div class="workspace">${phase === 'form' ? formA() : result()}${help()}</div>`;
  if (variant === 'B') content.innerHTML = `<div class="library-context" aria-hidden="true"><h1>Consoles</h1><div class="mini-card">${consoleIcon}<h3>Living room</h3><small>PlayStation 5</small></div></div><section class="sheet" aria-label="${title()}"><header class="sheet-head"><div><div class="section-kicker">Consoles / Setup</div><h1>${title()}</h1><p>${subtitle()}</p></div><button class="close-button" data-action="cancel" aria-label="Close setup">×</button></header>${phase === 'form' ? formB() : result()}</section>`;
  if (variant === 'C') content.innerHTML = `${header()}${phase === 'form' ? formC() : result()}`;
  document.querySelector('#variant-label').textContent = `${variant} · ${names[variant]}`;
  document.querySelector('#layout-note').textContent = notes[variant];
  document.querySelectorAll('[data-screen]').forEach(button => button.setAttribute('aria-pressed', button.dataset.screen === screen));
  document.querySelector('#preview-state').innerHTML = (screen === 'register' ? [['form', 'Form'], ['progress', 'Progress'], ['success', 'Success'], ['failure', 'Failure']] : [['form', 'Form'], ['success', 'Saved'], ['failure', 'Failure']]).map(([value, name]) => `<option value="${value}" ${phase === value ? 'selected' : ''}>${name}</option>`).join('');
  updateState();
}
function updateState() {
  const v = values[screen];
  const status = screen === 'register' ? `${consoleName()} · ${isLegacy() ? 'Online ID' : 'Account ID'} ${(isLegacy() ? v.online : v.account).trim() ? 'entered' : 'empty'} · PIN ${v.pin.length}/8 · Console PIN ${v.consolePin.length ? v.consolePin.length + '/4' : 'off'}` : `${v.address.trim() ? 'Address entered' : 'Address empty'} · ${v.savedConsole === 'choose' ? 'No console selected' : v.savedConsole === 'later' ? 'Register later' : 'Existing registration'}`;
  document.querySelector('#lab-status').textContent = `${status} · ${phase} · ${valid() ? 'Ready to submit' : 'Details incomplete'} · Sample data only. Nothing is saved.`;
  const submit = document.querySelector('#submit');
  if (submit) submit.disabled = !valid();
}
let noticeTimer;
function notice(message) {
  clearTimeout(noticeTimer);
  const element = document.querySelector('#notice');
  element.textContent = message; element.classList.add('visible');
  noticeTimer = setTimeout(() => element.classList.remove('visible'), 5000);
}
function switchVariant(offset) {
  variant = ['A', 'B', 'C'][(['A', 'B', 'C'].indexOf(variant) + offset + 3) % 3];
  render(); window.scrollTo(0, 0);
}
content.addEventListener('input', event => {
  if (!event.target.name) return;
  values[screen][event.target.name] = event.target.value;
  if (event.target.name === 'pin' || event.target.name === 'consolePin') {
    const optional = event.target.name === 'consolePin';
    const bad = event.target.value !== '' && !(optional ? /^\d{4}$/ : /^\d{8}$/).test(event.target.value);
    event.target.setAttribute('aria-invalid', bad);
    const error = document.querySelector(`#${event.target.name}-error`);
    error.hidden = !bad; error.textContent = optional ? 'Use exactly 4 digits, or leave this empty.' : 'Use exactly 8 digits.';
  }
  updateState();
});
content.addEventListener('change', event => {
  if (!['console', 'savedConsole'].includes(event.target.name)) return;
  const id = event.target.id;
  values[screen][id] = event.target.value; render(); document.getElementById(id).focus();
});
content.addEventListener('submit', event => {
  event.preventDefault();
  if (!valid()) return;
  phase = screen === 'register' ? 'progress' : 'success'; render(); content.focus();
});
content.addEventListener('click', event => {
  const action = event.target.closest('[data-action]')?.dataset.action;
  if (!action) return;
  if (action === 'login' || action === 'lookup') {
    values.register.account = 'EjRWeJCrze8='; render(); document.querySelector('#account')?.focus();
    notice(`Preview only: ${action === 'login' ? 'PSN sign-in' : 'public lookup'} supplied a sample account ID.`);
  } else if (action === 'edit') { phase = 'form'; render(); content.focus(); }
  else if (action === 'dismiss') { phase = 'form'; render(); notice('Preview: progress closed. This does not claim to cancel registration.'); }
  else if (action === 'library') location.href = '../?variant=B';
  else if (action === 'cancel') location.href = '../?variant=B';
});
document.querySelector('#previous').onclick = () => switchVariant(-1);
document.querySelector('#next').onclick = () => switchVariant(1);
document.querySelectorAll('[data-screen]').forEach(button => button.onclick = () => { screen = button.dataset.screen; phase = 'form'; render(); window.scrollTo(0, 0); });
document.querySelector('#preview-state').onchange = event => { phase = event.target.value; render(); window.scrollTo(0, 0); };
document.querySelector('#sample').onclick = () => {
  values[screen] = screen === 'register' ? { ...values.register, address: '192.0.2.42', account: 'EjRWeJCrze8=', online: 'SamplePlayer', pin: '12345678', consolePin: '' } : { address: '192.0.2.42', savedConsole: 'living' };
  phase = 'form'; render();
};
document.querySelector('#reset').onclick = () => { values = structuredClone(defaults); phase = 'form'; hidden = false; document.querySelector('#privacy').checked = false; render(); window.scrollTo(0, 0); };
document.querySelector('#privacy').onchange = event => { hidden = event.target.checked; render(); };
document.addEventListener('keydown', event => {
  if (event.defaultPrevented || event.altKey || event.ctrlKey || event.metaKey || event.target.closest('input,textarea,select,[contenteditable]')) return;
  if (event.key === 'ArrowLeft' || event.key === 'ArrowRight') { event.preventDefault(); switchVariant(event.key === 'ArrowLeft' ? -1 : 1); }
});
// Small self-check for the submit gate; this prototype has no production backend.
const savedValues = structuredClone(values), savedScreen = screen;
screen = 'register'; values.register = { ...defaults.register, account: 'sample', pin: '12345678' };
console.assert(valid(), 'Complete registration must enable submit');
values.register.pin = '1234567x'; console.assert(!valid(), 'PIN must contain eight digits');
values.register.pin = '12345678'; values.register.consolePin = '12'; console.assert(!valid(), 'Optional PIN must be empty or four digits');
screen = 'manual'; values.manual = { address: '192.0.2.42', savedConsole: 'choose' }; console.assert(!valid(), 'Manual entry needs a console choice');
values.manual.savedConsole = 'later'; console.assert(valid(), 'Deferred registration is supported');
values = savedValues; screen = savedScreen;
render();
