<script lang="ts">
  import { tick } from 'svelte';
  import type { State, connectSetup } from './bridge';
  import Dialog from './Dialog.svelte';
  let { current, connection, detailsKey = $bindable('') }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>>; detailsKey: string } = $props();
  let mode = $state('');
  let profile = $state('');
  let target = $state('');
  let action = $state('');
  let address = $state('');
  let registration = $state('');
  let pin = $state('');
  let reveal = $state(false);
  let busy = $state(false);
  let message = $state('');
  let saved = $state(false);
  const t = (text: string) => current.strings[text] ?? text;
  const actions: Record<string, string> = { pin: 'Save console login PIN', deleteManual: 'Delete manual address', hide: 'Hide console', unhide: 'Unhide console', deleteRegistration: 'Delete registration' };
  const explanations: Record<string, string> = {
    deleteManual: 'Delete this saved address? The console registration will be kept.',
    hide: 'Hide this console? You can restore it from Manage consoles.',
    unhide: 'Show this console in the library again?',
    deleteRegistration: 'Delete this console registration? You must pair the console again to connect.'
  };
  function validAddress(value: string) {
    const host = value.trim();
    if (!host || host.length > 253) return false;
    if (host.includes(':')) {
      const [ip, scope, ...extra] = host.split('%');
      if (extra.length || (scope !== undefined && !/^[A-Za-z0-9_.-]+$/.test(scope))) return false;
      try { return new URL(`http://[${ip}]`).hostname.startsWith('['); } catch { return false; }
    }
    if (!/^(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\.)*[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\.?$/.test(host)) return false;
    try { return !!new URL(`http://${host}`).hostname; } catch { return false; }
  }
  let addressValid = $derived(validAddress(address));
  let console = $derived(current.consoles.find(console => console.key === target));
  let registrationTarget = $derived(current.registrations.find(host => host.key === target));
  let hiddenTarget = $derived(current.hiddenConsoles.find(host => host.key === target));
  let targetName = $derived(console?.name ?? registrationTarget?.name ?? hiddenTarget?.name ?? t('Console unavailable'));
  let validTarget = $derived(profile === current.profileGeneration && !!(console ?? registrationTarget ?? hiddenTarget));
  $effect(() => {
    if (detailsKey && mode !== 'details') {
      target = detailsKey; profile = current.profileGeneration; mode = 'details'; message = ''; saved = false;
    }
    if (current.streamerMode) reveal = false;
    if (mode && profile !== current.profileGeneration || mode === 'pin' && !validTarget) { pin = ''; reveal = false; }
    if (mode === 'manual' && registration && registration !== 'later' && !current.registrations.some(host => host.key === registration)) {
      registration = ''; message = t('The registration is unavailable. Choose a console again.');
    }
  });
  async function open(next: string) {
    detailsKey = ''; mode = next; profile = current.profileGeneration;
    message = ''; saved = false; pin = ''; reveal = false;
    if (next === 'manual') { address = ''; registration = current.registrations.length ? '' : 'later'; }
    await tick();
    document.querySelector<HTMLElement>('dialog [data-initial-focus]')?.focus();
  }
  function close() { detailsKey = ''; mode = ''; pin = ''; reveal = false; address = ''; message = ''; }
  async function request(next: string, key: string) {
    detailsKey = ''; target = key; action = next; mode = next === 'pin' ? 'pin' : 'confirm';
    message = ''; pin = ''; saved = false; reveal = false; profile = current.profileGeneration;
    await tick();
    document.querySelector<HTMLElement>('dialog [data-initial-focus]')?.focus();
  }
  async function saveManual(event: SubmitEvent) {
    event.preventDefault();
    if (busy || saved || !addressValid || !registration) return;
    busy = true; message = '';
    try {
      const reply = await connection.saveManual(address, registration, profile);
      saved = reply.accepted;
      message = reply.accepted ? t('Address saved') : reply.message ?? t('The address could not be saved. Try again.');
      if (saved) { address = ''; await tick(); document.querySelector<HTMLElement>('[data-close-management]')?.focus(); }
    } finally { busy = false; }
  }
  async function apply(event?: SubmitEvent) {
    event?.preventDefault();
    if (busy || !validTarget || current.connectionBusy || (mode === 'pin' && !/^[0-9]{4}$/.test(pin))) return;
    busy = true;
    const value = pin; pin = ''; reveal = false;
    try {
      const reply = await connection.manage(action, target, value);
      if (reply.accepted) {
        if (action === 'pin') { saved = true; message = t('Console login PIN saved'); }
        else { close(); return; }
      } else message = reply.message ?? t('The change could not be saved. Try again.');
      await tick();
      document.querySelector<HTMLElement>(!reply.accepted && mode === 'pin' && validTarget
        ? '[data-saved-pin]' : '[data-close-management]')?.focus();
    } finally { busy = false; }
  }
</script>

<div class="management-tools" data-setup-panel>
  <button data-add-manual disabled={current.connectionBusy} onclick={event => { event.currentTarget.focus(); open('manual'); }}>{t('Add manual address')}</button>
  <button data-manage-consoles onclick={event => { event.currentTarget.focus(); open('manage'); }}>{t('Manage consoles')}</button>
</div>
{#if mode}
  <Dialog label={t(mode === 'manual' ? 'Add console' : mode === 'manage' ? 'Manage consoles' : mode === 'details' ? 'Console details' : actions[action])} {close}>
    <div class="dialog-heading"><h2>{t(mode === 'manual' ? 'Add console' : mode === 'manage' ? 'Manage consoles' : mode === 'details' ? 'Console details' : actions[action])}</h2>
      <button data-close-management onclick={close}>{t('Close')}</button></div>
    {#if mode === 'manual'}
      {#if !saved}
        <div class="manual-workspace"><form onsubmit={saveManual}>
          <p class="kicker">{t('Manual address')}</p><h3>{t('Find your console')}</h3>
          <label>{t('Console address')}<input data-manual-address data-initial-focus type={current.streamerMode ? 'password' : 'text'} autocomplete="off" spellcheck="false" bind:value={address} disabled={busy} /></label>
          <p>{t('Enter an IP address or hostname without a port or URL.')}</p>
          <label>{t('Registered console')}<select data-manual-registration bind:value={registration} disabled={busy}>
            <option value="">{t('Select a console')}</option>
            {#each current.registrations as host (host.key)}<option value={host.key}>{host.name} · {host.ps5 ? 'PS5' : 'PS4'} · {host.identifier}</option>{/each}
            <option value="later">{t('Register on first connection')}</option>
          </select></label>
          <button data-save-manual disabled={busy || !addressValid || !registration || profile !== current.profileGeneration || current.connectionBusy}>{t('Save address')}</button>
        </form><aside><h3>{t('Use a manual address')}</h3><p>{t('Save an address when discovery does not show your console.')}</p><h3>{t('Choose its registration')}</h3><p>{t('Use an existing registration, or register when you first connect. Saving an address does not pair the console or start a connection.')}</p></aside></div>
      {/if}
      <p data-manual-result role={saved ? 'status' : 'alert'}>{message}</p>
    {:else if mode === 'manage'}
      <h3>{t('Registered consoles')}</h3>
      {#each current.registrations as host (host.key)}<div class="management-row"><span>{host.name} · {host.ps5 ? 'PS5' : 'PS4'}<small>{host.identifier}</small></span><button data-delete-registration={host.key} disabled={current.connectionBusy} onclick={() => request('deleteRegistration', host.key)}>{t('Delete registration')}</button></div>{:else}<p>{t('No registered consoles')}</p>{/each}
      <h3>{t('Hidden consoles')}</h3>
      {#each current.hiddenConsoles as host (host.key)}<div class="management-row"><span>{host.name}<small>{host.identifier}</small></span><button data-unhide-console={host.key} disabled={current.connectionBusy} onclick={() => request('unhide', host.key)}>{t('Unhide console')}</button></div>{:else}<p>{t('No hidden consoles')}</p>{/each}
    {:else if mode === 'details'}
      {#if console && validTarget}
        <h3>{console.name}</h3>
        <dl><dt>{t('Console')}</dt><dd>{console.ps5 === undefined ? t('Console') : console.ps5 ? 'PlayStation 5' : 'PlayStation 4'}</dd>
          <dt>{t('Status')}</dt><dd>{t(console.state === 'ready' ? 'Online' : console.state === 'standby' ? 'In standby' : 'Status unknown')}</dd>
          <dt>{t('Registration')}</dt><dd>{t(console.registered ? 'Registered' : 'Not registered')}</dd>
          <dt>{t('Address')}</dt><dd>{console.address || '—'}</dd><dt>{t('ID')}</dt><dd>{console.identifier || '—'}</dd>
          {#if console.app}<dt>{t('App')}</dt><dd>{console.app}</dd>{/if}{#if console.titleId}<dt>{t('Title ID')}</dt><dd>{console.titleId}</dd>{/if}
        </dl><div class="actions">{#each console.actions as item}<button data-console-action={item} disabled={current.connectionBusy} onclick={() => request(item, target)}>{t(actions[item])}</button>{/each}</div>
      {:else}<p role="alert">{t('Console unavailable. Close this screen and select the console again.')}</p>{/if}
    {:else if mode === 'pin'}
      <h3>{targetName}</h3><p>{t('This is the four-digit PIN for your console user. It is different from the eight-digit Remote Play pairing code.')}</p>
      {#if !saved}<form onsubmit={apply}><label>{t('Console login PIN')}<input data-saved-pin data-initial-focus type={reveal && !current.streamerMode ? 'text' : 'password'} inputmode="numeric" maxlength="4" autocomplete="off" bind:value={pin} disabled={busy || !validTarget} /></label>
        <label class="reveal"><input type="checkbox" bind:checked={reveal} disabled={current.streamerMode} />{t('Show PIN')}</label>
        <button data-save-pin disabled={busy || !validTarget || current.connectionBusy || !/^[0-9]{4}$/.test(pin)}>{t('Save PIN')}</button></form>{/if}
      <p role="status">{message}</p>
    {:else if mode === 'confirm'}
      <h3>{targetName}</h3><p>{t(explanations[action])}</p>
      <div class="actions"><button data-cancel-management data-initial-focus onclick={close}>{t('Cancel')}</button><button data-confirm-management disabled={busy || !validTarget || current.connectionBusy} onclick={() => apply()}>{t(actions[action])}</button></div>
      <p role="alert">{message}</p>
    {/if}
    {#if mode !== 'manage' && profile !== current.profileGeneration}<p role="alert">{t('The profile changed. Open this form again.')}</p>{:else if (mode === 'pin' || mode === 'confirm') && !validTarget}<p role="alert">{t('Console unavailable. Close this screen and select the console again.')}</p>{/if}
  </Dialog>
{/if}

<style>
  button, select { min-height: 44px; padding: 9px 14px; border: 1px solid var(--line); border-radius: 8px; background: var(--panel); }
  .management-tools, .actions { display: flex; gap: 12px; flex-wrap: wrap; margin-bottom: 24px; }
  .dialog-heading, .management-row { display: flex; align-items: center; justify-content: space-between; gap: 16px; }
  .dialog-heading { margin-bottom: 24px; } h2 { font-size: 23px; } h3 { margin: 16px 0 8px; }
  form, label { display: grid; gap: 10px; } form { gap: 16px; } input, select { width: 100%; }
  p, dt, small, aside { color: var(--muted); } p { margin-block: 12px; } small { display: block; }
  .manual-workspace { display: grid; grid-template-columns: minmax(0, 1.6fr) minmax(0, 1fr); gap: 24px; }
  aside { border-left: 1px solid var(--line); padding-left: 24px; } .kicker { text-transform: uppercase; font-size: 11px; }
  .management-row { padding: 16px 0; border-bottom: 1px solid var(--line); flex-wrap: wrap; } .management-row span { min-width: 0; }
  dl { display: grid; grid-template-columns: 110px minmax(0, 1fr); gap: 12px; } dd { margin: 0; }
  .reveal { display: flex; align-items: center; } .reveal input { width: 20px; height: 20px; }
  @media (max-width: 600px) { .manual-workspace { grid-template-columns: 1fr; } aside { border-left: 0; padding-left: 0; } dl { grid-template-columns: 1fr; } }
</style>
