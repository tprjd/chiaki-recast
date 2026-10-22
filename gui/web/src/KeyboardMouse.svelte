<script lang="ts">
  import { tick } from 'svelte';
  import { moveFocus } from './navigation';
  import SettingField from './SettingField.svelte';
  import type { State, connectSetup } from './bridge';
  import { keyboardMouseSettings } from './settings';
  let { current, connection, target = '', t }: {
    current: State; connection: Awaited<ReturnType<typeof connectSetup>>;
    target?: string; t: (text: string) => string;
  } = $props();
  let form = $state('');
  let pending = $state(false);
  let message = $state('');
  let tab = $state('Mouse response');
  let confirmation: HTMLDialogElement;
  let resetIdentity: { form: string; generation: string } | undefined;
  const generation = $derived(current.profileGeneration);
  const windowGeneration = $derived(current.keyboardMouse.windowGeneration);
  const values = $derived(current.keyboardMouse.values);
  const enabled = $derived(values.mouseCameraEnabled === true);
  const fields = $derived(keyboardMouseSettings.map(field => {
    const limits = current.keyboardMouse.limits;
    if (field.key.endsWith('Sensitivity')) return { ...field, min: limits.sensitivityMinimum, max: limits.sensitivityMaximum };
    if (field.key === 'mouseDeadZoneCompensation') return { ...field, max: limits.compensationMaximum * 100 };
    if (field.key === 'mouseResponseExponent') return { ...field, min: limits.exponentMinimum, max: limits.exponentMaximum };
    return field;
  }));

  $effect(() => {
    windowGeneration;
    const profile = generation;
    let disposed = false;
    let identity = '';
    form = '';
    resetIdentity = undefined;
    confirmation?.close();
    connection.beginKeyboardMouse(profile).then(reply => {
      if (disposed) { if (reply.form) connection.endKeyboardMouse(reply.form); return; }
      identity = reply.form ?? '';
      form = identity;
    });
    return () => { disposed = true; if (identity) connection.endKeyboardMouse(identity); };
  });
  $effect(() => {
    if (target.startsWith('keyboardMouseBinding')) tab = 'Bindings';
    if (form && target) tick().then(() => document.getElementById(target)?.focus());
  });
  async function change(key: string, value: boolean | number | string, editGeneration = generation) {
    if (!form || pending) return;
    pending = true;
    try {
      const reply = await connection.keyboardMouseSetting(key, value as boolean | number, form, editGeneration);
      message = reply.accepted ? t('Saved · Applies the next time you press F8.') : reply.message ?? t('The setting could not be saved.');
    } catch { message = t('The setting could not be saved.'); }
    finally { pending = false; }
  }
  async function capture(action: number, slot: number, legacy = false) {
    const reply = await connection.captureKeyboardMouse(action, slot, legacy, form, generation);
    if (!reply.accepted) message = t('The binding could not be opened.');
  }
  function openReset() {
    resetIdentity = { form, generation };
    confirmation.showModal();
    confirmation.querySelector<HTMLButtonElement>('[data-keyboard-reset-cancel]')?.focus();
  }
  function cancelReset() {
    resetIdentity = undefined;
    confirmation.close();
    document.getElementById('keyboardMouseReset')?.focus();
  }
  async function reset() {
    const identity = resetIdentity;
    cancelReset();
    if (!identity) return;
    const reply = await connection.resetKeyboardMouse(identity.form, identity.generation);
    message = reply.accepted ? t('Saved · Applies the next time you press F8.') : t('The setting could not be saved.');
  }
  async function resetKeys() {
    const reply = await connection.resetKeyboardKeys(form, generation);
    message = reply.accepted ? t('Saved') : t('The setting could not be saved.');
  }
  function tabKey(event: KeyboardEvent) {
    if (!['ArrowLeft', 'ArrowRight', 'Home', 'End'].includes(event.key)) return;
    event.preventDefault();
    tab = event.key === 'Home' ? 'Mouse response' : event.key === 'End' ? 'Bindings' : tab === 'Bindings' ? 'Mouse response' : 'Bindings';
    tick().then(() => document.getElementById(tab === 'Bindings' ? 'keyboard-mouse-bindings' : 'keyboard-mouse-response')?.focus());
  }
</script>

<div data-keyboard-mouse>
  {#if current.keyboardMouse.supported}
    <div class="settings-group">
      <div class="setting-rows">
      <SettingField field={fields[0]} value={values.mouseCameraEnabled} disabled={!form || pending} {generation} change={value => change('mouseCameraEnabled', value)} {t} />
      <div class="setting-row reset-row"><button id="keyboardMouseReset" disabled={!form || pending} onclick={openReset}>{t('Reset defaults')}</button></div>
      </div>
      <div class="keyboard-mouse-tabs" role="tablist" tabindex="-1" aria-label={t('Keyboard and mouse')} onkeydown={tabKey}>
        {#each ['Mouse response', 'Bindings'] as item}
          <button role="tab" id={item === 'Bindings' ? 'keyboard-mouse-bindings' : 'keyboard-mouse-response'} tabindex={tab === item ? 0 : -1} aria-selected={tab === item} aria-controls="keyboard-mouse-tab" onclick={() => tab = item}>{t(item)}</button>
        {/each}
      </div>
      <div id="keyboard-mouse-tab" role="tabpanel" tabindex="0" aria-labelledby={tab === 'Bindings' ? 'keyboard-mouse-bindings' : 'keyboard-mouse-response'}>
        {#if tab === 'Mouse response'}
          {#each fields.slice(1, 6) as field (field.key)}
            <div class="setting-rows mouse-response-row">
            <SettingField {field} value={values[field.key]} disabled={!form || pending || !enabled} {generation} change={(value, editGeneration) => change(field.key, value, editGeneration)} {t} />
            {#if field.min !== undefined}
              <input type="range" aria-label={`${t(field.label)} ${t('slider')}`} min={field.min} max={field.max} step={field.step} value={Number(values[field.key]) / (field.factor ?? 1)} disabled={!form || pending || !enabled}
                onchange={event => change(field.key, event.currentTarget.valueAsNumber * (field.factor ?? 1))} />
            {/if}
            </div>
          {/each}
          <p class="keyboard-note">{t("Increase compensation only if small mouse movements do not move the camera. Start response correction at 1.00. Lower values increase small movements to compensate for a game's slow stick response.")}</p>
          <p class="keyboard-note">{t('Sensitivity is a multiplier. The game still controls its response curve and maximum turn speed.')}</p>
        {:else}
          <p class="keyboard-note">{t('Choose a slot to assign or clear one key or mouse button. Mouse movement controls the right stick.')}</p>
          {#each ['Movement', 'Buttons', 'Shoulders and sticks', 'D-pad', 'System'] as group, index}
            <section class="settings-group" aria-label={t(group)}>
              <h3>{t(group)}</h3>
              <div class="setting-rows">
              {#each current.keyboardMouse.bindings.filter(binding => binding.group === index) as binding (binding.action)}
                <div class="binding-row setting-row"><span>{binding.name}</span>
                  {#each [binding.primaryName, binding.alternativeName] as name, slot}
                    <button id={`keyboardMouseBinding${binding.action}_${slot}`} disabled={!form || !enabled} aria-label={`${binding.name}, ${t(slot ? 'Alternative' : 'Primary')}, ${name}`} onclick={() => capture(binding.action, slot)}><small>{t(slot ? 'Alternative' : 'Primary')}</small>{name}</button>
                  {/each}
                </div>
              {/each}
              </div>
            </section>
          {/each}
        {/if}
      </div>
    </div>
  {/if}
  <section class="settings-group" aria-label={t('Key assignments')}>
    <h3>{t('Key assignments')}</h3>
    <div class="setting-rows">
    {#each fields.slice(6) as field}
      <SettingField {field} value={values[field.key]} disabled={!form || pending || enabled} {generation} change={value => change(field.key, value)} {t} />
    {/each}
    <div class="setting-row reset-row"><button id="resetAllKeys" disabled={!form || enabled} onclick={resetKeys}>{t('Reset All Keys')}</button></div>
    {#each current.keyboardMouse.keys as mapping (mapping.buttonValue)}
      <div class="setting-row"><span>{mapping.buttonName}</span><button id={`keyMapping${mapping.buttonValue}`} disabled={!form || enabled} aria-label={`${t('Key for')} ${mapping.buttonName}`} onclick={() => capture(mapping.buttonValue, 0, true)}>{mapping.keyName || t('Unassigned')}</button></div>
    {/each}
    </div>
  </section>
  <p class="keyboard-note" role="status">{message}</p>
</div>
<dialog bind:this={confirmation} aria-labelledby="reset-keyboard-title" onkeydown={event => moveFocus(event, confirmation)} oncancel={event => { event.preventDefault(); cancelReset(); }}>
  <h2 id="reset-keyboard-title">{t('Reset keyboard and mouse defaults?')}</h2>
  <p>{t('This restores mouse response and bindings and turns mouse camera mode off. Other settings stay unchanged.')}</p>
  <button data-keyboard-reset-confirm onclick={reset}>{t('Reset defaults')}</button>
  <button data-keyboard-reset-cancel onclick={cancelReset}>{t('Cancel')}</button>
</dialog>

<style>
  .reset-row { justify-content:flex-end; }
  .keyboard-note { color:var(--muted); font-size:12px; margin-block:12px; }
  .keyboard-note:empty { display:none; }
  .binding-row { display:grid; grid-template-columns:minmax(7rem,1fr) repeat(2,minmax(0,1fr)); margin:0; gap:12px; }
  .binding-row button { font-size:12px; }
  .keyboard-mouse-tabs { gap:4px; padding:3px; border:1px solid var(--line); border-radius:8px; background:#11151c; width:fit-content; }
  .keyboard-mouse-tabs button { border:0; background:transparent; color:var(--muted); font-size:12px; }
  .keyboard-mouse-tabs button[aria-selected=true] { background:var(--raised); color:var(--text); }
  .mouse-response-row input[type=range] { display:block; width:calc(100% - 40px); margin:0 20px 17px; }
</style>
