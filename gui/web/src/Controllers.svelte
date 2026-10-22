<script lang="ts">
  import { onDestroy, tick, untrack } from 'svelte';
  import Dialog from './Dialog.svelte';
  import { moveFocus } from './navigation';
  import type { State, connectSetup } from './bridge';
  let { current, connection, t, target = '' }: {
    current: State; connection: Awaited<ReturnType<typeof connectSetup>>; t: (text: string) => string; target?: string;
  } = $props();
  const mapping = $derived(current.controllerSetup);
  let pending = $state(false);
  let message = $state('');
  let focus = '';
  let operation = '';
  const profile = untrack(() => current.profileGeneration);
  $effect(() => { operation = mapping.operation; });
  onDestroy(() => { if (operation) connection.controllerMappingAction('cancel', operation, profile); });
  $effect(() => {
    const key = target;
    if (key) untrack(async () => {
      focus = key;
      if ((key.startsWith('controllerMap') && !['controllerMappingChange', 'controllerMappingReset'].includes(key)) || key === 'enableAnalogStickMapping')
        await run(() => connection.beginControllerMapping(false, current.profileGeneration));
      await tick();
      const field = document.getElementById(key); field?.focus(); field?.scrollIntoView({ block: 'nearest' });
    });
  });
  $effect(() => {
    if (mapping.phase === 'edit') tick().then(() => {
      const field = document.getElementById(focus);
      (field && !field.matches(':disabled') ? field : document.getElementById('enableAnalogStickMapping'))?.focus();
      document.activeElement?.scrollIntoView({ block: 'nearest' });
    });
  });
  async function run(action: () => ReturnType<typeof connection.beginControllerMapping>) {
    if (pending) return;
    pending = true;
    try { const reply = await action(); message = reply.accepted ? '' : reply.message ?? t('Select the controller again.'); }
    finally { pending = false; }
  }
  function command(action: string) { return run(() => connection.controllerMappingAction(action, mapping.operation, current.profileGeneration)); }
</script>

<svelte:window onkeydown={event => { const dialog = document.querySelector<HTMLElement>('dialog[open]'); if (dialog && mapping.phase === 'edit') moveFocus(event, dialog); }} />

<section class="settings-group" aria-label={t('Available controllers')}>
  <h3>{t('Available controllers')}</h3>
  <div class="setting-rows">
  <div class="setting-row controller-devices">
  {#if !mapping.devices.length}<p data-controller-empty role="status">{t('No controllers connected.')}</p>{/if}
  <ul data-controller-devices>{#each mapping.devices as device}<li>{device.name} · {device.type}{device.steamVirtual ? ` · ${t('Managed by Steam')}` : ''}</li>{/each}</ul>
  </div>
  <div class="setting-row controller-actions">
  <button id="controllerMappingChange" disabled={pending || !mapping.devices.length || current.connectionBusy || !!mapping.operation}
    onclick={() => run(() => connection.beginControllerMapping(false, current.profileGeneration))}>{t('Change controller mapping')}</button>
  <button id="controllerMappingReset" disabled={pending || !mapping.devices.length || current.connectionBusy || !!mapping.operation}
    onclick={() => run(() => connection.beginControllerMapping(true, current.profileGeneration))}>{t('Reset controller mapping')}</button>
  </div>
  </div>
  <p class="controller-status" role="status" data-controller-status>{message || mapping.message}</p>
</section>

{#if mapping.operation}
  <Dialog label={t('Controller mapping')} close={() => command('cancel')}>
    <h2>{t('Controller mapping')}</h2>
    {#if mapping.phase === 'select'}
      <p role="status">{t('Choose the controller by pressing any button on the controller.')}</p>
    {:else}
      <p>{mapping.type}</p>
      <div class="setting-rows"><div class="setting-row"><div class="setting-copy"><label for="enableAnalogStickMapping">{t('Allow mapping analog sticks and L2/R2')}</label></div><div class="setting-control"><input id="enableAnalogStickMapping" type="checkbox" role="switch" checked={mapping.analog} disabled={pending || mapping.phase === 'capture'}
        onchange={event => command(event.currentTarget.checked ? 'analogOn' : 'analogOff')} /></div></div></div>
      <p>{t('Select a primary or alternate input to capture a controller button. A conflicting input moves to the new mapping.')}</p>
      {#if mapping.phase === 'capture'}
        <p role="status" data-controller-capture>{t('Press a controller input, or cancel capture.')}</p>
        <button id="controllerCaptureCancel" onclick={() => command('cancelCapture')}>{t('Cancel capture')}</button>
      {/if}
      <div class="controller-mappings setting-rows">
        {#each mapping.mappings as row (row.buttonValue)}
          <div class="setting-row"><span>{row.buttonName}</span>
            {#each [0, 1] as slot}
              <button id={`controllerMap${row.buttonValue}Slot${slot}`} disabled={pending || mapping.phase === 'capture'} aria-label={`${row.buttonName} ${t(slot ? 'alternate input' : 'primary input')}`}
                onclick={() => { focus = `controllerMap${row.buttonValue}Slot${slot}`; run(() => connection.captureControllerMapping(row.buttonValue, slot, mapping.operation, current.profileGeneration)); }}>
                <small>{t(slot ? 'Alternate' : 'Primary')}</small>{row.physicalButton[slot] || t('Not mapped')}
              </button>
            {/each}
          </div>
        {/each}
      </div>
    {/if}
    <div class="dialog-actions">
      <button id="controllerMappingCancel" disabled={pending} onclick={() => command('cancel')}>{t('Cancel')}</button>
      {#if mapping.phase !== 'select'}<button id="controllerMappingApply" disabled={pending || mapping.phase !== 'edit' || !mapping.altered} onclick={() => command('apply')}>{t('Update controller mapping')}</button>{/if}
    </div>
  </Dialog>
{/if}

<style>
  .controller-devices { display:block; color:var(--muted); font-size:12px; }
  .controller-devices ul { list-style:none; padding:0; margin:0; }
  .controller-devices li + li { margin-top:8px; }
  .controller-actions { justify-content:flex-end; flex-wrap:wrap; gap:8px; }
  .controller-actions button { min-height:36px; color:var(--accent); font-size:12px; }
  .controller-status { color:var(--muted); font-size:12px; }
  .controller-status:not(:empty) { margin-top:12px; }
  .setting-copy label { font-size:13px; font-weight:500; }
  .controller-mappings { margin-block:16px; }
  .controller-mappings > div { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 12px; align-items: center; }
  .controller-mappings button { min-width:0; overflow-wrap:anywhere; font-size:12px; }
  small { display: block; opacity: .7; }
</style>
