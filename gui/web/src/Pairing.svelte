<script lang="ts">
  import { tick, onDestroy } from 'svelte';
  import Icon from './Icon.svelte';
  import PsnAccount from './PsnAccount.svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection, open = $bindable(false) }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>>; open?: boolean } = $props();
  let choosing = $state(false);
  let presentation = 0;
  let choiceKey = $state('');
  const automaticTargets = $derived(current.consoles.filter(console => console.available && console.actions.includes('autoRegister') && (!choiceKey || console.key === choiceKey)));
  export async function choose(key = '', trigger?: HTMLButtonElement) {
    ++presentation;
    if (current.registration.busy) { await start(key, trigger); return; }
    opener = trigger ?? document.activeElement as HTMLElement;
    choiceKey = key; choosing = true; open = true; message = '';
    await tick();
    document.querySelector<HTMLElement>('[data-registration-heading]')?.focus();
  }
  async function automatic(key: string) {
    const ownedPresentation = presentation;
    const reply = await connection.autoRegister(key, current.profileGeneration);
    if (ownedPresentation !== presentation) return;
    if (reply.accepted) await close(); else message = reply.message ?? t('Console unavailable');
  }
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
  onDestroy(() => { ++presentation; if (accountRoute) connection.accountEnd(context, accountRoute); });
  $effect(() => {
    if (open) return;
    if (accountRoute) { connection.accountEnd(context, accountRoute); accountRoute = ''; username = ''; }
    pin = ''; consolePin = '';
  });
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
  $effect(() => {
    if (invalidProfile || phase === 'success' || phase === 'failure') { pin = ''; consolePin = ''; }
  });
  async function start(key = '', trigger?: HTMLButtonElement) {
    if (submitting) return;
    if (trigger || !choosing) opener = trigger ?? document.activeElement as HTMLElement;
    const ownedProfile = current.profileGeneration;
    const ownedPresentation = presentation;
    const reply = await connection.registrationOpen(key, ownedProfile);
    if (current.profileGeneration !== ownedProfile || ownedPresentation !== presentation) return;
    if (!reply.accepted || !reply.context) { message = reply.message ?? t('Console unavailable'); return; }
    if (context !== reply.context) { account = ''; onlineId = ''; pin = ''; consolePin = ''; }
    context = reply.context;
    profile = current.profileGeneration;
    address = reply.address ?? '';
    target = reply.ps5 === false ? '1000' : '1000100';
    message = ''; editing = false; diagnostics = false; choosing = false; open = true;
    await tick();
    document.querySelector<HTMLElement>('[data-registration-heading]')?.focus();
  }
  export async function close() {
    ++presentation;
    if (accountRoute) { connection.accountEnd(context, accountRoute); accountRoute = ''; username = ''; }
    open = false; pin = ''; consolePin = '';
    await tick();
    (opener?.isConnected ? opener : document.querySelector<HTMLElement>('[data-register-new]'))?.focus();
  }
  async function submit(event: SubmitEvent) {
    event.preventDefault();
    if (!valid || submitting || current.registration.busy || current.connectionBusy || invalidProfile) return;
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
  const target = event.target as Element;
  if (!event.defaultPrevented && open && event.key === 'Escape' && target.closest('.pairing') && !target.closest('.psn-account')) { event.preventDefault(); close(); }
}} />
<section data-setup-panel class="pairing">
  {#if open}
    <div class="breadcrumb"><button data-registration-close onclick={close}><Icon name="back" size={12} />{t('Consoles')}</button><span>/</span><span>{t(choosing ? 'Add console' : 'Register console')}</span></div>
    <div class="pairing-panel">
      <header class="setup-heading">
        <div><h1 data-registration-heading tabindex="-1">{t(choosing ? 'Add console' : 'Register console')}</h1><p>{t(choosing ? 'Choose how to register your console for Remote Play.' : 'Pair your console with Chiaki Recast for Remote Play.')}</p></div>
        <span class="setup-chip">{t('Remote Play setup')}</span>
      </header>
      {#if !choosing && invalidProfile}<p role="alert">{t('The profile changed. Close this screen and open registration again.')}</p>{/if}
      {#if message}<p role="alert">{message}</p>{/if}
      {#if choosing}
        <div class="registration-choices">
          <section><Icon name="network" size={28} /><h2>{t('Manual pairing')}</h2><p>{t('Use the Remote Play pairing code displayed on your console.')}</p><button data-registration-manual class="primary" onclick={() => start(choiceKey)}>{t('Register console')}</button></section>
          <section><Icon name="remote" size={28} /><h2>{t('Automatic registration')}</h2><p>{t('Use your PlayStation Network account to register a console.')}</p>
            {#each automaticTargets as console (console.key)}<button data-automatic-register disabled={current.connectionBusy} onclick={() => automatic(console.key)}>{console.name}</button>{/each}
            {#if !automaticTargets.length}<p>{t('Sign in to PSN and refresh remote consoles to find available consoles.')}</p><PsnAccount {current} {connection} />{/if}
            <small>{t('Automatic registration requires your main PS4 console registered to your PSN account.')}</small>
          </section>
        </div>
      {:else if accountRoute && accountSource === 'lookup'}
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
          <div class="form-heading"><p class="eyebrow">{t('Console details')}</p><h2>{t('Set up Remote Play')}</h2></div>
          <div class="field">
            <label>{t('Console and firmware')}
              <select data-registration-target bind:value={target} aria-describedby="registration-target-help">
                <option value="1000100">PS5</option>
                <option value="1000">{t('PS4 firmware 8.0 or later')}</option>
                <option value="900">{t('PS4 firmware 7.0 to before 8.0')}</option>
                <option value="800">{t('PS4 firmware before 7.0')}</option>
              </select>
            </label>
            <small id="registration-target-help">{t('Choose the console and its firmware version.')}</small>
          </div>
          <div class="field">
            <label>{t('Console address')}<input data-registration-address type={current.streamerMode ? 'password' : 'text'} bind:value={address} autocomplete="off" spellcheck="false" aria-describedby="registration-address-help" /></label>
            <small id="registration-address-help">{t('Use an IP address, hostname, or 255.255.255.255 for local broadcast.')}</small>
          </div>
          <hr />
          <div class="field">
            {#if legacy}
              <label>{t('PSN Online ID')}<input data-registration-online-id type={current.streamerMode ? 'password' : 'text'} bind:value={onlineId} autocomplete="off" spellcheck="false" /></label>
            {:else}
              <label>{t('PSN Account ID')}<input data-registration-account type={current.streamerMode ? 'password' : 'text'} bind:value={account} placeholder={t('Paste your Account ID')} autocomplete="off" spellcheck="false" aria-describedby="registration-account-help" /></label>
              <small id="registration-account-help">{t('Use your Account ID encoded as Base64. This is different from your PSN username.')}</small>
              <div class="account-actions">
                {#if current.registration.savedAccount}<button type="button" data-registration-saved disabled={invalidProfile} onclick={() => beginAccount('saved')}>{t('Use saved Account ID')}</button>{/if}
                <button type="button" data-registration-sign-in disabled={invalidProfile} onclick={() => beginAccount('signIn')}>{t('PSN sign-in')} <span aria-hidden="true">&#8599;</span></button>
                <button type="button" data-registration-lookup disabled={invalidProfile} onclick={() => beginAccount('lookup')}>{t('Public lookup')} <span aria-hidden="true">&#8599;</span></button>
              </div>
            {/if}
          </div>
          <div class="pairing-codes">
            <div class="field">
              <label>{t('Remote Play pairing code')}<input data-registration-pin type="password" inputmode="numeric" bind:value={pin} autocomplete="off" placeholder="00000000" aria-describedby="registration-pin-help" /></label>
              <small id="registration-pin-help">{t('Enter the eight-digit code shown by your console.')}</small>
            </div>
            <div class="field">
              <label><span>{t('Console login PIN')} <span class="optional">{t('Optional')}</span></span><input data-registration-console-pin type="password" inputmode="numeric" bind:value={consolePin} autocomplete="off" placeholder="0000" aria-describedby="registration-console-pin-help" /></label>
              <small id="registration-console-pin-help">{t('Your four-digit console login PIN, if you use one.')}</small>
            </div>
          </div>
          <div class="setup-actions"><small>{t('Keep your console on during registration.')}</small><div><button type="button" onclick={close}>{t('Cancel')}</button><button class="primary" data-registration-submit type="submit" disabled={!valid || submitting || current.registration.busy || current.connectionBusy || invalidProfile}>{t('Register console')} <span aria-hidden="true">&#8594;</span></button></div></div>
        </form>
        <aside aria-label={t('Setup help')}>
          <span class="help-symbol"><Icon name="console-device" size={34} /></span>
          <div class="help-heading"><p class="eyebrow">{t(target === '1000100' ? 'On your PlayStation 5' : 'On your PlayStation 4')}</p><h2>{t('Get your pairing code')}</h2></div>
          <p>{t('Use the same PlayStation Network account as the user signed in on your console.')}</p>
          <ol>
            <li><span class="help-number" aria-hidden="true">1</span><div><strong>{t('Open Settings')}</strong><p>{t(target === '1000100' ? 'Go to System, then Remote Play.' : 'Go to Remote Play Connection Settings.')}</p></div></li>
            <li><span class="help-number" aria-hidden="true">2</span><div><strong>{t('Enable Remote Play')}</strong><p>{t('Keep your console on and connected to your network.')}</p></div></li>
            <li><span class="help-number" aria-hidden="true">3</span><div><strong>{t(target === '1000100' ? 'Select Link Device' : 'Select Add Device')}</strong><p>{t('Enter the eight-digit code before it expires.')}</p></div></li>
          </ol>
          <div class="help-footer"><h3>{t('Pairing code and console login PIN')}</h3><p>{t('The Remote Play pairing code pairs the console. The optional console login PIN signs in to your console user.')}</p></div>
        </aside>
      </div>
      {:else}
        <div class="registration-result" data-registration-result role="status">
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
    </div>
  {/if}
</section>
<style>
  .pairing { margin-block: 0; }
  .pairing-panel { display: grid; gap: 25px; }
  .breadcrumb { display: flex; align-items: center; gap: 9px; color: var(--muted); font-size: 12px; margin-bottom: 22px; }
  .breadcrumb button { display: inline-flex; align-items: center; gap: 5px; color: inherit; border: 0; padding: 0; min-height: 0; background: transparent; }
  .setup-heading { display: flex; align-items: center; justify-content: space-between; gap: 20px; }
  .setup-heading p { margin-top: 8px; font-size: 13px; }
  .setup-chip { display: inline-flex; align-items: center; gap: 7px; padding: 5px 10px; border: 1px solid var(--line); border-radius: 20px; color: var(--muted); font-size: 11px; white-space: nowrap; }
  .setup-chip::before { content: ''; width: 5px; height: 5px; border-radius: 50%; background: var(--accent); }
  .registration-choices { display: grid; grid-template-columns: 1fr 1fr; gap: 24px; }
  .registration-choices section, .pairing-layout form { padding: 25px; border: 1px solid var(--line); border-radius: 12px; background: var(--panel); }
  .registration-choices section { display: grid; align-content: start; gap: 18px; }
  .registration-choices :global(svg), .help-symbol { color: var(--accent); }
  .registration-choices small { color: var(--muted); }
  .pairing-layout { display: grid; grid-template-columns: minmax(0, 1.7fr) minmax(230px, 1fr); align-items: start; gap: 24px; }
  .pairing-layout form { gap: 0; }
  .form-heading { margin-bottom: 22px; }
  .pairing-layout h2 { font-size: 21px; }
  .field { min-width: 0; margin-bottom: 17px; }
  .field label { gap: 7px; font-size: 13px; font-weight: 550; }
  .field small { display: block; margin-top: 6px; color: var(--muted); font-size: 11px; line-height: 1.5; }
  .optional { color: var(--muted); font-size: 11px; font-weight: 400; margin-left: 4px; }
  hr { width: 100%; border: 0; border-top: 1px solid var(--line); margin: 6px 0 20px; }
  .pairing-codes { display: grid; grid-template-columns: minmax(0, 1fr) minmax(0, 1fr); align-items: start; gap: 18px; }
  .pairing-codes input { letter-spacing: 4px; font-variant-numeric: tabular-nums; }
  .setup-actions { display: flex; align-items: center; flex-wrap: wrap; gap: 16px; border-top: 1px solid var(--line); padding-top: 20px; margin-top: 6px; }
  .setup-actions small { color: var(--muted); font-size: 11px; max-width: 260px; }
  .setup-actions > div { display: flex; gap: 10px; margin-left: auto; }
  .setup-actions .primary { display: inline-flex; align-items: center; gap: 9px; background: var(--accent); color: #101c29; border-color: transparent; }
  .setup-actions button:first-child { background: transparent; }
  .help-symbol { display: grid; place-items: center; width: 60px; height: 60px; border: 1px solid #3b5875; border-radius: 14px; background: #1b2b3d; margin-bottom: 6px; }
  .pairing-layout aside { padding: 23px 5px 20px 6px; }
  .pairing-layout aside p { font-size: 12px; line-height: 1.7; }
  .help-heading .eyebrow { font-size: 10px; }
  .pairing-layout ol { list-style: none; margin: 0; padding: 0; display: grid; gap: 17px; }
  .pairing-layout li { display: flex; align-items: flex-start; gap: 10px; font-size: 12px; }
  .help-number { display: inline-grid; place-items: center; width: 23px; height: 23px; border: 1px solid #435369; color: #c3d4e8; border-radius: 50%; font-size: 11px; flex-shrink: 0; }
  .pairing-layout li strong { font-weight: 500; }
  .pairing-layout li p { margin-top: 3px; }
  .help-footer { border-top: 1px solid var(--line); padding-top: 17px; margin-top: 5px; }
  .help-footer p { margin-top: 9px; }
  .account-actions { display: flex; flex-wrap: wrap; gap: 8px 17px; margin-top: 9px; }
  .account-actions button { min-height: 24px; border: 0; padding: 0; color: var(--accent); font-size: 12px; background: transparent; text-align: left; }
  .registration-result { display: grid; justify-items: start; gap: 18px; padding: 25px; border: 1px solid var(--line); border-radius: 12px; background: var(--panel); }
  .account-lookup { display: grid; gap: 16px; max-width: 720px; }
  form, label, aside { display: grid; gap: 10px; min-width: 0; }
  aside { align-content: start; gap: 18px; }
  button { min-height: 40px; padding: 9px 14px; background: var(--raised); border: 1px solid var(--line); border-radius: 7px; }
  input, select { width: 100%; min-height: 42px; }
  pre { max-height: 240px; overflow: auto; white-space: pre-wrap; overflow-wrap: anywhere; }
  p { color: var(--muted); overflow-wrap: anywhere; }
  @media (max-width: 760px) { .pairing-layout { grid-template-columns: minmax(0, 1fr); } }
</style>
