<script lang="ts">
  import { onMount, tick } from 'svelte';
  import { connectSetup, type State, type Console } from './bridge';
  import SteamShortcut from './SteamShortcut.svelte';
  import Reminders from './Reminders.svelte';
  import ConnectAction from './ConnectAction.svelte';
  import PsnAccount from './PsnAccount.svelte';
  import Pairing from './Pairing.svelte';
  import Settings from './Settings.svelte';
  import ConsoleManagement from './ConsoleManagement.svelte';
  import Icon from './Icon.svelte';

  let current = $state<State>();
  let profileGeneration = $derived(current?.profileGeneration);
  let connection = $state<Awaited<ReturnType<typeof connectSetup>>>();
  let query = $state('');
  let message = $state('');
  let changingDiscovery = $state(false);
  let settingsOpen = $state(false);
  let settingsCategory = $state('General');
  let settingsSearch = $state('');
  let restoredProfile = '';
  let accountRoute = $state(false);
  let detailsKey = $state('');
  let pairing = $state<Pairing>();
  let pairingOpen = $state(false);
  let connectingKey = $state('');
  const t = (text: string) => current?.strings[text] ?? text;
  const type = (console: Console) => console.ps5 === undefined ? t('Console') : console.ps5 ? 'PlayStation 5' : 'PlayStation 4';
  const status = (console: Console) => !console.available ? t('Console unavailable') : !console.registered ? t('Not registered')
    : console.state === 'ready' ? t('Online') : console.state === 'standby' ? t('In standby') : t('Status unknown');
  const route = (console: Console) => t(console.route === 'local' ? 'Local network' : console.route === 'psn' ? 'Remote connection via PSN' : 'Manual address');
  let consoles = $derived(current?.consoles.filter(console => query.trim().toLocaleLowerCase().split(/\s+/)
    .every(term => `${console.name} ${type(console)} ${console.ps5 === undefined ? '' : console.ps5 ? 'PS5' : 'PS4'}`.toLocaleLowerCase().includes(term))) ?? []);

  onMount(() => {
    let disposed = false;
    connectSetup(value => {
      const restore = restoredProfile !== value.profileGeneration;
      if (restore) {
        restoredProfile = value.profileGeneration;
        query = value.viewState.search;
        settingsOpen = value.viewState.route === 'settings';
        accountRoute = value.viewState.route === 'account';
        settingsCategory = value.viewState.category;
        settingsSearch = value.viewState.settingsSearch;
      }
      current = value;
      if (restore) tick().then(() => {
        if (!disposed && !settingsOpen && !document.querySelector('dialog[open]'))
          (document.querySelector<HTMLElement>(accountRoute ? '[data-account-route] .psn-account h2' : '.console-card.selected:enabled')
            ?? document.querySelector<HTMLElement>('.discovery:enabled'))?.focus();
      });
    }).then(value => {
      if (disposed) value.dispose(); else connection = value;
    }).catch(() => { message = t('Setup unavailable'); });
    return () => { disposed = true; connection?.dispose(); };
  });

  $effect(() => {
    if (connection && profileGeneration) connection.saveViewState(settingsOpen ? 'settings' : accountRoute ? 'account' : 'consoles', query, settingsCategory, settingsSearch, profileGeneration);
  });

  async function discovery() {
    if (!current || !connection || changingDiscovery) return;
    changingDiscovery = true;
    try {
      const reply = await connection.discovery(!current.discoveryEnabled);
      message = reply.accepted ? '' : reply.message ?? t('Setup unavailable');
    } finally { changingDiscovery = false; }
  }
  async function select(key: string) {
    const reply = await connection?.select(key);
    message = reply?.accepted ? '' : reply?.message ?? t('Console unavailable');
  }
  async function navigate(event: KeyboardEvent) {
    const target = event.target as HTMLElement;
    if (settingsOpen || pairingOpen || target.closest('[data-setup-panel]') || document.querySelector('dialog[open]')) return;
    if (event.key === 'F1') {
      event.preventDefault();
      if (!event.repeat) document.querySelector<HTMLButtonElement>('[data-steam-open]:enabled')?.click();
      return;
    }
    if (event.key === 'ContextMenu') { event.preventDefault(); settingsOpen = true; return; }
    if (event.key === 'Escape') {
      event.preventDefault();
      query = '';
      await tick();
    }
    const cards = [...document.querySelectorAll<HTMLButtonElement>('.console-card:enabled')];
    const selected = cards.find(card => card.dataset.consoleKey === current?.selectedKey) ?? cards[0]
      ?? document.querySelector<HTMLButtonElement>('.discovery:enabled');
    if (event.key === 'PageUp') {
      document.querySelector<HTMLInputElement>('input')?.focus();
      event.preventDefault();
      return;
    }
    if (event.key === 'PageDown' || event.key === 'Escape' ||
      (!target.matches('.console-card') && target.tagName !== 'INPUT' && event.key.startsWith('Arrow')) ||
      (target.tagName === 'INPUT' && event.key === 'ArrowDown')) {
      selected?.focus();
      event.preventDefault();
      return;
    }
    if (!target.matches('.console-card') || !['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown'].includes(event.key)) return;
    const columns = Math.max(1, Math.round(document.querySelector('.console-grid')!.clientWidth / (target.clientWidth + 18)));
    const offset = event.key === 'ArrowLeft' ? -1 : event.key === 'ArrowRight' ? 1 : event.key === 'ArrowUp' ? -columns : columns;
    if (cards.indexOf(target as HTMLButtonElement) + offset < 0 && event.key === 'ArrowUp') {
      document.querySelector<HTMLButtonElement>('.discovery')?.focus();
      event.preventDefault();
      return;
    }
    const next = cards[Math.max(0, Math.min(cards.length - 1, cards.indexOf(target as HTMLButtonElement) + offset))];
    next?.focus();
    event.preventDefault();
  }
  async function showLibrary() {
    if (pairingOpen) await pairing?.close();
    settingsOpen = false; accountRoute = false;
  }
  async function showSettings() {
    if (pairingOpen) await pairing?.close();
    settingsOpen = true;
  }
  async function openReminderRoute(route: string) {
    if (route !== 'psn') return;
    if (pairingOpen) await pairing?.close();
    accountRoute = true;
    await tick();
    document.querySelector<HTMLElement>('[data-account-route] .psn-account h2')?.focus();
  }
  async function closeAccountRoute() {
    accountRoute = false;
    await tick();
    document.querySelector<HTMLElement>('[data-psn-account]')?.focus();
  }
  async function closeSettings() {
    settingsOpen = false;
    settingsCategory = 'General';
    settingsSearch = '';
    await tick();
    document.querySelector<HTMLButtonElement>('.open-settings')?.focus();
  }
  async function connectConsole(console: Console) {
    if (!connection || !current || connectingKey || current.connectionBusy) return;
    connectingKey = console.key;
    try {
      const reply = await connection.connect(console.key);
      message = reply.accepted ? '' : reply.message ?? t('Console unavailable');
    } finally { connectingKey = ''; }
  }
</script>

<svelte:window onkeydown={navigate} />
<a class="skip-link" href="#workspace">{t('Consoles')}</a>
<div class="app-shell">
  <aside class="app-rail" aria-label="Chiaki Recast">
    <div class="brand"><span class="brand-mark"><Icon name="consoles" size={23} /></span><span>Chiaki <em>Recast</em></span></div>
    <nav aria-label={t('Navigation')}>
      <button class="back-consoles" class:active={!settingsOpen} aria-current={!settingsOpen ? 'page' : undefined}
        onclick={showLibrary}><Icon name="consoles" />{t('Consoles')}</button>
      <button class="open-settings" class:active={settingsOpen} aria-current={settingsOpen ? 'page' : undefined}
        disabled={!current || !connection} onclick={showSettings}><Icon name="settings" />{t('Settings')}</button>
    </nav>
    <div class="rail-foot">
      {#if current}<span class="profile-label"><Icon name="profile" size={16} /><span>{t('Current profile')}<strong>{current.profiles.current}</strong></span></span>{/if}
      <span class="application-name">Chiaki Recast</span>
    </div>
  </aside>
  <main id="workspace">
  {#if settingsOpen && current && connection}
    <Settings {current} {connection} close={closeSettings} {t} bind:category={settingsCategory} bind:query={settingsSearch} />
  {:else}
    <div class:hidden={pairingOpen || accountRoute}>
      <div class="library-heading">
        <div><div class="library-title"><h1>{t('Consoles')}</h1><span class="heading-count">{current?.consoles.length ?? 0}</span></div></div>
        <button class="add-console" data-register-new={current && connection ? '' : undefined} disabled={!current || !connection || current.connectionBusy} onclick={event => pairing?.choose('', event.currentTarget)}><Icon name="plus" />{t(current?.registration.busy ? 'View registration progress' : 'Add console')}</button>
      </div>
      <div class="library-toolbar">
        <label class="search"><Icon name="search" size={17} /><input type="search" maxlength="256" bind:value={query} placeholder={t('Search consoles')} aria-label={t('Search consoles')} /></label>
        <button class="refresh-psn quiet-button" data-psn-refresh disabled={!connection || !current || ['refreshing', 'listing'].includes(current.psn.refresh.phase ?? '')}
          onclick={() => current && connection?.refreshPsnConsoles(current.profileGeneration)}><Icon name="refresh" size={17} />{t('Refresh remote consoles')}</button>
      </div>
    </div>
    <ConnectAction {current} {connection} />
    {#if current && connection}<Pairing bind:this={pairing} {current} {connection} bind:open={pairingOpen} />{/if}
    <div class:hidden={pairingOpen || accountRoute}>
      {#if current && !consoles.length}
        <section class="empty" aria-live="polite">
          <Icon name="consoles" size={40} /><h2>{t(query ? 'No matching consoles' : 'No consoles found')}</h2>
          {#if !query}<p>{t('Turn on discovery to find consoles on your local network.')}</p>{/if}
        </section>
      {/if}
      <div class="console-grid">
        {#each consoles as console, index (console.key || `unavailable-${index}`)}
          <article class="console-entry" class:selected={current?.selectedKey === console.key && !!console.key}>
            <button class="console-card" class:selected={current?.selectedKey === console.key && !!console.key}
              disabled={!console.available} aria-pressed={current?.selectedKey === console.key && !!console.key}
              aria-label={`${console.name}, ${type(console)}, ${status(console)}`} data-console-key={console.key}
              onclick={() => select(console.key)} onfocus={() => select(console.key)}>
              <span class="art" class:standby={console.state === 'standby'} class:offline={!['ready', 'standby'].includes(console.state)}>
                <span class="art-top"><span class="console-type">{type(console)}</span><Icon name={console.route === 'psn' ? 'remote' : 'network'} size={17} /></span>
                {#if console.ps5 !== undefined}<img src={`qrc:/icons/console-ps${console.ps5 ? '5' : '4'}.svg#${console.state === 'ready' ? 'light_on' : console.state === 'standby' ? 'light_standby' : 'console'}`} alt="" />{/if}
              </span>
              <span class="card-content">
                <strong class="console-name" title={console.name}>{console.name}</strong>
                <span class="console-status" class:online={console.state === 'ready'} class:standby={console.state === 'standby'}>{status(console)}</span>
                <span class="route"><Icon name={console.route === 'psn' ? 'remote' : 'network'} size={14} />{route(console)}</span>
              </span>
            </button>
            <div class="card-actions">
              {#if console.registered}
                <button class="primary connect-console" class:standby={console.route === 'local' && console.state === 'standby'} disabled={!connection || !!connectingKey || current?.connectionBusy || !console.available || (console.route === 'local' && !['ready', 'standby'].includes(console.state))}
                  onclick={() => connectConsole(console)}>{t(console.route === 'local' && console.state === 'standby' ? 'Wake and connect' : 'Connect')}</button>
              {:else}
                <button class="primary register-console" data-register-selected={current?.selectedKey === console.key ? '' : undefined} disabled={!connection || current?.connectionBusy || !console.available} onclick={event => pairing?.choose(console.key, event.currentTarget)}>{t('Register console')}</button>
              {/if}
              <button class="console-details" disabled={!console.available} aria-label={`${t('Console details')}: ${console.name}`}
                data-console-details-key={console.key} onclick={event => { event.currentTarget.focus(); detailsKey = console.key; }}><Icon name="more" /></button>
            </div>
          </article>
        {/each}
      </div>
      <div class="library-footer">
        <button class="discovery quiet-button" aria-pressed={current?.discoveryEnabled ?? false} disabled={!connection || changingDiscovery || !current?.capabilities.discovery} onclick={discovery}>
          <Icon name="network" size={15} />{t(current?.discoveryEnabled ? 'Discovery is on' : 'Discovery is off')}</button>
        <span class="library-count">{consoles.length} {t('of')} {current?.consoles.length ?? 0} {t('consoles')} , {consoles.filter(console => console.available && console.state === 'ready').length} {t('online')}</span>
        <span role="status" data-psn-refresh-status>{current?.psn.refresh.phase === 'refreshing' ? t('Refreshing PSN credentials') : current?.psn.refresh.phase === 'listing' ? t('Finding remote consoles') : current?.psn.refresh.message ?? ''}</span>
      </div>
      <ConnectAction {current} {connection} optionsOnly />
      <div class="library-secondary">
        {#if current && connection}<ConsoleManagement {current} {connection} bind:detailsKey />{/if}
        {#if current && connection && current.steam.supported}<SteamShortcut {current} {connection} />{/if}
        {#if current && connection}<PsnAccount {current} {connection} />{/if}
      </div>
    </div>
    {#if current && connection && accountRoute}<div data-account-route><PsnAccount {current} {connection} route close={closeAccountRoute} /></div>{/if}
    {#if current && connection}<Reminders {current} {connection} setup={openReminderRoute} />{/if}
    <p role="status" class="message">{message || (!current ? t('Loading consoles') : '')}</p>
  {/if}
  </main>
</div>
