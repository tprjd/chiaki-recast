<script lang="ts">
  import { tick, onDestroy } from 'svelte';
  import PsnAccount from './PsnAccount.svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>> } = $props();
  let open = $state(false);
  let address = $state('255.255.255.255');
  let target = $state('1000100');
  let account = $state('');
  let onlineId = $state('');
  let pin = $state('');
  let consolePin = $state('');
  const t = (text: string) => current.strings[text] ?? text;
  const legacy = $derived(target === '800');
  const valid = $derived(!!address.trim() && (legacy ? !!onlineId.trim() : /^[A-Za-z0-9+/]{10}[AEIMQUYcgkosw048]=$/.test(account.trim()))
    && /^[0-9]{8}$/.test(pin) && /^$|^[0-9]{4}$/.test(consolePin));
  let context = $state('');
  let profile = $state('');
  let message = $state('');
  let submitting = $state(false);
  let editing = $state(false);
  let diagnostics = $state(false);
  let opener: HTMLElement | null = null;
  let accountRoute = $state('');
  let accountSource = $state('');
  let accountRead = $state('');
  let accountBaseline = $state('');
  let username = $state('');
  let lookupSubmitting = $state(false);
  const lookup = $derived(current.accountLookup.route === accountRoute ? current.accountLookup : undefined);
  const lookupBusy = $derived(lookupSubmitting || lookup?.phase === 'pending');
  $effect(() => {
    if (!accountRoute || invalidProfile) return;
    if (accountSource === 'lookup' && lookup?.phase === 'success' && lookup.id) applyAccount(lookup.id);
    else if (accountSource === 'signIn' && current.psn.phase === 'success' && current.psn.id && current.psn.id !== accountBaseline) applyAccount(current.psn.id);
  });
  onDestroy(() => { if (accountRoute) connection.accountEnd(context, accountRoute); });
  async function beginAccount(source: string) {
    const ownedContext = context;
    const reply = await connection.accountBegin(ownedContext, source);
    if (!open || context !== ownedContext || invalidProfile) { if (reply.route) connection.accountEnd(ownedContext, reply.route); return; }
    if (!reply.accepted || !reply.route) { message = reply.message ?? t('The registration context changed. Open registration again.'); return; }
    accountRoute = reply.route; accountSource = source; accountRead = ''; message = ''; username = '';
    accountBaseline = current.psn.id ?? '';
    if (source === 'saved') { await applyAccount('saved'); return; }
    await tick();
    document.querySelector<HTMLElement>(source === 'lookup' ? '[data-lookup-username]' : '.pairing .psn-account h2')?.focus();
  }
  async function backToForm() {
    const route = accountRoute;
    accountRoute = ''; username = '';
    await connection.accountEnd(context, route);
    await tick();
    document.querySelector<HTMLElement>('[data-registration-account]')?.focus();
  }
  async function applyAccount(request: string) {
    if (accountRead === request) return;
    accountRead = request;
    const ownedContext = context, ownedRoute = accountRoute;
    const reply = await connection.accountRead(ownedContext, ownedRoute, request);
    if (context !== ownedContext || accountRoute !== ownedRoute || invalidProfile) return;
    if (reply.accepted && reply.accountId) { account = reply.accountId; await backToForm(); }
    else if (accountSource === 'saved') { message = reply.message ?? t('No valid saved Account ID is available.'); await backToForm(); }
  }
  async function submitLookup(event: SubmitEvent) {
    event.preventDefault();
    if (lookupBusy || !username.trim() || invalidProfile) return;
    lookupSubmitting = true;
    try { await connection.accountLookup(context, accountRoute, username.trim()); }
    finally { lookupSubmitting = false; }
  }
  const result = $derived(current.registration.context === context ? current.registration : undefined);
  const phase = $derived(editing ? 'form' : result?.phase ?? 'form');
  const invalidProfile = $derived(!!profile && profile !== current.profileGeneration);
  const selected = $derived(current.consoles.find(console => console.key === current.selectedKey));
  $effect(() => {
    if (invalidProfile || phase === 'success' || phase === 'failure') { pin = ''; consolePin = ''; }
  });
  async function start(key = '', trigger?: HTMLButtonElement) {
    if (submitting) return;
    opener = trigger ?? document.activeElement as HTMLElement;
    const ownedProfile = current.profileGeneration;
    const reply = await connection.registrationOpen(key, ownedProfile);
    if (current.profileGeneration !== ownedProfile) return;
    if (!reply.accepted || !reply.context) { message = reply.message ?? t('Console unavailable'); return; }
    if (context !== reply.context) { account = ''; onlineId = ''; pin = ''; consolePin = ''; }
    context = reply.context;
    profile = current.profileGeneration;
    address = reply.address ?? '';
    target = reply.ps5 === false ? '1000' : '1000100';
    message = ''; editing = false; diagnostics = false; open = true;
    await tick();
    document.querySelector<HTMLElement>('[data-registration-heading]')?.focus();
  }
  async function close() {
    if (accountRoute) { connection.accountEnd(context, accountRoute); accountRoute = ''; username = ''; }
    open = false; pin = ''; consolePin = '';
    await tick();
    if (opener?.isConnected) opener.focus();
  }
  async function submit(event: SubmitEvent) {
    event.preventDefault();
    if (!valid || submitting || current.registration.busy || invalidProfile) return;
    submitting = true; editing = false; message = ''; diagnostics = false;
    const fields = { address, target: Number(target), account: legacy ? onlineId : account, pin, consolePin };
    pin = ''; consolePin = '';
    try {
      const reply = await connection.registrationSubmit(context, fields);
      if (!reply.accepted && !reply.id) message = reply.message ?? t('Registration did not start. Check your details and try again.');
    } finally { submitting = false; }
    await tick();
    document.querySelector<HTMLElement>('[data-registration-heading]')?.focus();
  }
</script>
<svelte:window onkeydown={event => {
  if (open && event.key === 'Escape' && (event.target as Element).closest('.pairing')) { event.preventDefault(); close(); }
}} />
<section data-setup-panel class="pairing">
  <button data-register-new onclick={event => start('', event.currentTarget)}>{t(current.registration.busy ? 'View registration progress' : 'Register console')}</button>
  {#if selected && selected.available && selected.route !== 'psn' && !selected.registered}
    <button data-register-selected onclick={event => start(selected!.key, event.currentTarget)}>{t('Register selected console')}</button>
  {/if}
  {#if !open && message}<p role="alert">{message}</p>{/if}
  {#if open}
    <div class="pairing-panel">
      <h2 data-registration-heading tabindex="-1">{t('Register console')}</h2>
      {#if invalidProfile}<p role="alert">{t('The profile changed. Close this screen and open registration again.')}</p>{/if}
      {#if message}<p role="alert">{message}</p>{/if}
      {#if accountRoute && accountSource === 'lookup'}
        <div class="account-lookup">
          <h3 data-lookup-heading>{t('Find a public Account ID')}</h3>
          <p data-lookup-explanation>{t('This sends your PSN username to the third-party service psn.flipscreen.games. Your PSN privacy settings must allow anyone to find you in search. Public lookup retrieves your Account ID. It does not sign you in or save authentication credentials.')}</p>
          <form onsubmit={submitLookup}>
            <label>{t('PSN username')}<input data-lookup-username type={current.streamerMode ? 'password' : 'text'} bind:value={username} disabled={lookupBusy || invalidProfile} autocomplete="off" spellcheck="false" /></label>
            {#if lookupBusy}<p role="status">{t('Retrieving your Account ID')}</p>{/if}
            {#if lookup?.phase === 'failure'}<p role="alert" data-lookup-error>{lookup.message}</p>{/if}
            <button data-lookup-submit type="submit" disabled={lookupBusy || !username.trim() || invalidProfile}>{t(lookup?.phase === 'failure' ? 'Retry lookup' : 'Find Account ID')}</button>
          </form>
          <button data-lookup-cancel onclick={backToForm}>{t('Cancel')}</button>
        </div>
      {:else if accountRoute && accountSource === 'signIn'}
        <PsnAccount {current} {connection} route close={backToForm} />
      {:else if phase === 'form'}
      <div class="pairing-layout">
        <form data-registration-form onsubmit={submit}>
          <label>{t('Console and firmware')}
            <select data-registration-target bind:value={target}>
              <option value="1000100">PS5</option>
              <option value="800">{t('PS4 firmware before 7.0')}</option>
              <option value="900">{t('PS4 firmware 7.0 to before 8.0')}</option>
              <option value="1000">{t('PS4 firmware 8.0 or later')}</option>
            </select>
          </label>
          <label>{t('Console address')}<input data-registration-address type={current.streamerMode ? 'password' : 'text'} bind:value={address} autocomplete="off" spellcheck="false" /></label>
          <p>{t('Use an IP address, hostname, or 255.255.255.255 for local broadcast.')}</p>
          {#if legacy}
            <label>{t('PSN Online ID')}<input data-registration-online-id type={current.streamerMode ? 'password' : 'text'} bind:value={onlineId} autocomplete="off" spellcheck="false" /></label>
          {:else}
            <label>{t('PSN Account ID in Base64')}<input data-registration-account type={current.streamerMode ? 'password' : 'text'} bind:value={account} autocomplete="off" spellcheck="false" /></label>
            <p>{t('Use your Account ID encoded as Base64. This is different from your PSN username.')}</p>
            <div class="account-actions">
              {#if current.registration.savedAccount}<button type="button" data-registration-saved disabled={invalidProfile} onclick={() => beginAccount('saved')}>{t('Use saved Account ID')}</button>{/if}
              <button type="button" data-registration-lookup disabled={invalidProfile} onclick={() => beginAccount('lookup')}>{t('Public lookup')}</button>
              <button type="button" data-registration-sign-in disabled={invalidProfile} onclick={() => beginAccount('signIn')}>{t('PSN sign-in')}</button>
            </div>
          {/if}
          <label>{t('Remote Play pairing code')}<input data-registration-pin type="password" inputmode="numeric" bind:value={pin} autocomplete="off" /></label>
          <label>{t('Console login PIN, optional')}<input data-registration-console-pin type="password" inputmode="numeric" bind:value={consolePin} autocomplete="off" /></label>
          <button data-registration-submit type="submit" disabled={!valid || submitting || current.registration.busy || invalidProfile}>{t('Register console')}</button>
        </form>
        <aside>
          <h3>{t('Get your pairing code')}</h3>
          <p>{t('Use the same PlayStation Network account as the user signed in on your console.')}</p>
          <p>{t(target === '1000100' ? 'Open Settings, System, then Remote Play. Enable Remote Play and select Link Device.'
            : 'Open Settings, then Remote Play Connection Settings. Enable Remote Play and select Add Device.')}</p>
          <p>{t('Keep your console on and connected to your network. Enter the eight-digit code before it expires.')}</p>
          <p>{t('The Remote Play pairing code pairs the console. The optional console login PIN signs in to your console user.')}</p>
        </aside>
      </div>
      {:else}
        <div data-registration-result role="status">
          <h3>{t(phase === 'progress' ? 'Registering console' : phase === 'success' ? 'Console registered' : 'Registration failed')}</h3>
          <p>{current.streamerMode ? t('Address hidden') : address}</p>
          {#if phase === 'progress'}
            <p>{t('Waiting for the console. Keep it on and connected to your network.')}</p>
            <p>{t('Closing this screen does not cancel registration. The operation can continue in the background.')}</p>
          {:else if phase === 'failure'}
            <p role="alert">{result?.message}</p>
            <button data-registration-edit disabled={invalidProfile || current.registration.busy} onclick={() => { editing = true; }}>{t('Correct details and retry')}</button>
          {:else if result?.connectKey}
            <button data-registration-connect disabled={current.connectionBusy} onclick={() => connection.registrationConnect(result!.id!)}>{t('Connect')}</button>
          {/if}
          {#if current.streamerMode}<p>{t('Diagnostics are hidden in streamer mode.')}</p>
          {:else}
            <button data-registration-diagnostics onclick={() => { diagnostics = !diagnostics; }}>{t(diagnostics ? 'Hide diagnostics' : 'Show diagnostics')}</button>
            {#if diagnostics}<pre data-registration-log>{result?.diagnostics}</pre>{/if}
          {/if}
        </div>
      {/if}
      <button data-registration-close onclick={close}>{t('Close')}</button>
    </div>
  {/if}
</section>
<style>
  .pairing { margin-block: 24px; }
  .pairing-panel { display: grid; gap: 20px; padding: 24px; margin-block: 16px; border: 1px solid var(--line); border-radius: 12px; background: var(--panel); }
  .pairing-layout { display: grid; grid-template-columns: minmax(0, 3fr) minmax(0, 2fr); gap: 32px; }
  .account-actions { display: flex; flex-wrap: wrap; gap: 10px; }
  .account-lookup { display: grid; gap: 16px; }
  form, label, aside { display: grid; gap: 10px; min-width: 0; }
  aside { align-content: start; gap: 18px; }
  button { min-height: 44px; padding: 9px 14px; background: var(--panel); border: 1px solid var(--line); border-radius: 8px; }
  input, select { width: 100%; min-height: 44px; }
  select { padding: 8px; background: var(--panel); }
  pre { max-height: 240px; overflow: auto; white-space: pre-wrap; overflow-wrap: anywhere; }
  p { color: var(--muted); overflow-wrap: anywhere; }
  @media (max-width: 760px) { .pairing-layout { grid-template-columns: minmax(0, 1fr); } }
</style>
