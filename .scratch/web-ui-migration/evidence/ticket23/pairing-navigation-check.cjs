// Bounded check of the actual production async methods, with no native runtime.
const assert = require('node:assert/strict');
const { readFileSync } = require('node:fs');
const path = require('node:path');
const { createRequire } = require('node:module');
const root = path.resolve(__dirname, '../../../..');
const webRequire = createRequire(path.join(root, 'gui/web/package.json'));
const ts = webRequire('typescript');
const { compile } = webRequire('svelte/compiler');
const source = readFileSync(path.join(root, 'gui/web/src/Pairing.svelte'), 'utf8');
const compiled = compile(source, { filename: 'Pairing.svelte', generate: 'client' });
assert.equal(compiled.warnings.length, 0);
const start = source.slice(source.indexOf('  async function start('), source.indexOf('  export async function close()'));
const automatic = source.slice(source.indexOf('  async function automatic('), source.indexOf("  let address ="));
const methods = ts.transpileModule(start + automatic, { compilerOptions: { target: ts.ScriptTarget.ES2022 } }).outputText;
const createFixture = new Function(`
  let finish;
  const reply = new Promise(resolve => { finish = resolve; });
  let presentation = 1, open = false, context = '', message = '', closeCalls = 0;
  let choosing = true, submitting = false, opener = null;
  let account = '', onlineId = '', pin = '', consolePin = '', profile = '', address = '', target = '', editing = false, diagnostics = false;
  const current = {profileGeneration: 'profile'};
  const connection = {registrationOpen: () => reply, autoRegister: () => reply};
  const tick = async () => {};
  const document = {activeElement: null, querySelector: () => null};
  const t = text => text;
  const close = async () => { ++closeCalls; ++presentation; open = false; };
  ${methods}
  return {start, automatic, finish, leave: () => { ++presentation; }, value: () => ({open, context, message, closeCalls})};
`);
(async () => {
  const staleManual = createFixture();
  const pendingManual = staleManual.start('opaque-console');
  staleManual.leave();
  staleManual.finish({accepted: true, context: 'stale-context'});
  await pendingManual;
  assert.deepEqual(staleManual.value(), {open: false, context: '', message: '', closeCalls: 0});
  const currentManual = createFixture();
  const validManual = currentManual.start('opaque-console');
  currentManual.finish({accepted: true, context: 'current-context'});
  await validManual;
  assert.equal(currentManual.value().open, true);
  assert.equal(currentManual.value().context, 'current-context');
  for (const accepted of [true, false]) {
    const staleAutomatic = createFixture();
    const pendingAutomatic = staleAutomatic.automatic('opaque-console');
    staleAutomatic.leave();
    staleAutomatic.finish({accepted, message: 'stale-error'});
    await pendingAutomatic;
    assert.equal(staleAutomatic.value().closeCalls, 0);
    assert.equal(staleAutomatic.value().message, '');
  }
  console.log('Pairing compile: 0 warnings. Current manual route opens; late manual and automatic replies are ignored.');
})().catch(error => { console.error(error); process.exitCode = 1; });
