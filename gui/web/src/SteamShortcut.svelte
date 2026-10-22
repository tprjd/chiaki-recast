<script lang="ts">
  import { tick } from 'svelte';
  import Dialog from './Dialog.svelte';
  import { moveFocus } from './navigation';
  import type { State, connectSetup } from './bridge';
  let { current, connection }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>> } = $props();
  let name = $state(''), options = $state(''), form = $state(''), profile = $state('');
  let privateMode = $state(false);
  let nameEdited = $state(false), optionsEdited = $state(false), pending = $state(false), message = $state('');
  const t = (text: string) => current.strings[text] ?? text;
  const busy = $derived(pending || ['progress', 'choosing'].includes(current.steam.phase));
  const success = $derived(current.steam.phase === 'success');
  const validName = $derived(current.streamerMode && !nameEdited || !!name.trim());
  $effect(() => {
    if (form !== current.steam.form) {
      form = current.steam.form; profile = current.profileGeneration;
      name = current.steam.name ?? ''; options = current.steam.options ?? '';
      nameEdited = false; optionsEdited = false; message = ''; pending = false;
    }
  });
  $effect(() => {
    if (privateMode !== current.streamerMode) {
      privateMode = current.streamerMode;
      name = current.steam.name ?? ''; options = current.steam.options ?? '';
      nameEdited = false; optionsEdited = false;
    }
  });
  $effect(() => {
    const phase = current.steam.phase;
    const activeForm = current.steam.form;
    if (phase === 'success' || phase === 'failure') tick().then(() => {
      if (current.steam.form === activeForm && current.steam.phase === phase)
        document.querySelector<HTMLElement>(phase === 'success' ? '[data-steam-close]' : '[data-steam-name]')?.focus();
    });
  });
  async function submit(event: SubmitEvent) {
    event.preventDefault();
    if (busy || success || !validName) return;
    const activeForm = form, activeProfile = profile;
    pending = true; message = '';
    try {
      const fields: Record<string, string> = {};
      if (nameEdited) fields.name = name;
      if (optionsEdited) fields.options = options;
      const reply = await connection.submitSteamShortcut(activeForm, fields, activeProfile);
      if (form !== activeForm || profile !== activeProfile) return;
      if (!reply.accepted) message = reply.message ?? t('The shortcut could not be created. Check the Steam folder and try again.');
    } finally { if (form === activeForm && profile === activeProfile) pending = false; }
  }
  async function chooseFolder() {
    if (busy || success) return;
    const activeForm = form, activeProfile = profile;
    pending = true;
    try { await connection.chooseSteamFolder(activeForm, activeProfile); }
    finally {
      if (form === activeForm && profile === activeProfile) {
        pending = false; await tick();
        if (form === activeForm) document.querySelector<HTMLElement>('[data-steam-folder]')?.focus();
      }
    }
  }
  function close() { if (!busy) connection.endSteamShortcut(form, profile); }
</script>
<svelte:window onkeydown={event => {
  if (event.defaultPrevented) return;
  const panel = document.querySelector<HTMLElement>('[data-steam-form]');
  if (!panel) return;
  if (event.key === 'ContextMenu') {
    event.preventDefault();
    if (!event.repeat && !busy && !success && validName) panel.querySelector<HTMLFormElement>('form')?.requestSubmit();
  } else if ((event.target as HTMLElement).matches('input') && ['ArrowUp', 'ArrowDown'].includes(event.key)) {
    const fields = [...panel.querySelectorAll<HTMLElement>('input:enabled, button:enabled')].filter(field => field.getClientRects().length);
    const next = fields[fields.indexOf(event.target as HTMLElement) + (event.key === 'ArrowDown' ? 1 : -1)];
    if (next) { event.preventDefault(); next.focus(); next.scrollIntoView({ block: 'nearest' }); }
  } else moveFocus(event, panel);
}} />
<button data-steam-open disabled={!current.steam.supported || current.connectionBusy || !!current.steam.form}
  onclick={event => { event.currentTarget.focus(); connection.beginSteamShortcut(current.profileGeneration); }}>{t('Steam shortcut')}</button>
{#if current.steam.form}
  <Dialog label={t('Add to Steam')} {close}>
    <section data-steam-form>
      <h2>{t('Add to Steam')}</h2>
      <p>{t('Create or update a non-Steam game for the selected profile.')}</p>
      {#if !success}
        <form onsubmit={submit}>
          <label>{t('Steam game name')}<input data-initial-focus data-steam-name type={current.streamerMode ? 'password' : 'text'} autocomplete="off" maxlength="256"
            bind:value={name} oninput={() => { nameEdited = true; }} placeholder={current.streamerMode ? t('Current profile default') : ''} disabled={busy} /></label>
          {#if !validName}<p>{t('Enter a name for the Steam game.')}</p>{/if}
          <label>{t('Launch options (optional)')}<input data-steam-options type={current.streamerMode ? 'password' : 'text'} autocomplete="off" maxlength="4096"
            bind:value={options} oninput={() => { optionsEdited = true; }} placeholder={current.streamerMode ? t('Current profile default') : ''} disabled={busy} /></label>
          <p>{t('The default launch options select the current profile. You can edit them before you create the shortcut.')}</p>
          <p data-steam-path>{current.steam.path}</p>
          <div class="actions"><button type="button" data-steam-folder disabled={busy} onclick={chooseFolder}>{t('Choose Steam folder')}</button>
            <button type="submit" data-steam-create disabled={busy || !validName}>{t(current.steam.phase === 'failure' ? 'Retry' : 'Create shortcut')}</button></div>
        </form>
      {/if}
      {#if current.steam.phase === 'progress'}<p role="status">{t('Wait for Steam files to finish writing. This operation cannot be cancelled.')}</p>{/if}
      {#if ['success', 'failure'].includes(current.steam.phase)}
        <div data-steam-result={current.steam.phase} role={success ? 'status' : 'alert'}>
          <h3>{t(success ? 'Steam shortcut ready' : 'Steam shortcut was not completed')}</h3>
          <p>{t(success ? 'Restart Steam yourself to see the shortcut in your library.' : 'Check the details and Steam folder, then try again. Some files may already have changed.')}</p>
          <details><summary>{t('Technical details')}</summary><pre>{current.steam.diagnostics}</pre></details>
        </div>
      {/if}
      {#if message}<p role="alert">{message}</p>{/if}
      <p>{t('A shortcut with the same executable and launch options is updated. Artwork and the Steam Deck controller configuration are also applied.')}</p>
      <div class="actions"><button data-steam-close disabled={busy} onclick={close}>{t(success ? 'Done' : 'Cancel')}</button>
        <button disabled={busy} onclick={() => connection.openSetupHelp('steam')}>{t('Steam setup help')}</button></div>
    </section>
  </Dialog>
{/if}
<style>
  button { min-height: 44px; padding: 9px 14px; border: 1px solid var(--line); border-radius: 8px; background: var(--panel); }
  section, form, label { display: grid; gap: 12px; }
  section { gap: 20px; }
  input { width: 100%; }
  .actions { display: flex; flex-wrap: wrap; gap: 10px; }
  pre { white-space: pre-wrap; overflow-wrap: anywhere; }
</style>
