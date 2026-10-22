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
          (document.querySelector<HTMLElement>(accountRoute ? '.psn-account h2' : '.console-card.selected:enabled')
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
    if (settingsOpen || target.closest('[data-setup-panel]') || document.querySelector('dialog[open]')) return;
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
    const columns = Math.max(1, Math.round(document.querySelector('.console-grid')!.clientWidth / (target.clientWidth + 24)));
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
  async function openReminderRoute(route: string) {
    if (route !== 'psn') return;
    accountRoute = true;
    await tick();
    document.querySelector<HTMLElement>('.psn-account h2')?.focus();
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
</script>

<svelte:window onkeydown={navigate} />
<header>
  <img class="logo" src="qrc:/icons/chiaking-logo-white.svg" alt="Chiaki Recast" />
</header>
<main>
  {#if settingsOpen && current && connection}
    <Settings {current} {connection} close={closeSettings} {t} bind:category={settingsCategory} bind:query={settingsSearch} />
  {:else}
  <div class="library-heading">
    <h1>{t('Consoles')}</h1>
    <label class="search">{t('Search consoles')}
      <input type="search" maxlength="256" bind:value={query} aria-label={t('Search consoles')} />
    </label>
  </div>
  <div class="library-tools">
    <button class="open-settings" disabled={!current || !connection} onclick={() => { settingsOpen = true; }}>{t('Settings')}</button>
    <button class="discovery" aria-pressed={current?.discoveryEnabled ?? false}
      disabled={!connection || changingDiscovery || !current?.capabilities.discovery} onclick={discovery}>
      {t('Discovery')}<span>{t(current?.discoveryEnabled ? 'Discovery is on' : 'Discovery is off')}</span>
    </button>
    <button class="refresh-psn" data-psn-refresh disabled={!connection || !current || ['refreshing', 'listing'].includes(current.psn.refresh.phase ?? '')}
      onclick={() => current && connection?.refreshPsnConsoles(current.profileGeneration)}>{t('Refresh remote consoles')}</button>
    <span role="status" data-psn-refresh-status>{current?.psn.refresh.phase === 'refreshing' ? t('Refreshing PSN credentials')
      : current?.psn.refresh.phase === 'listing' ? t('Finding remote consoles') : current?.psn.refresh.message ?? ''}</span>
    {#if current && connection && current.steam.supported}<SteamShortcut {current} {connection} />{/if}
    <span role="status" class="message">{message || (!current ? t('Loading consoles') : '')}</span>
  </div>
  <ConnectAction {current} {connection} />
  {#if current && connection}<Pairing {current} {connection} />{/if}
  {#if current && connection}<ConsoleManagement {current} {connection} bind:detailsKey />{/if}
  {#if current && !consoles.length}
    <section class="empty" aria-live="polite">
      <h2>{t(query ? 'No matching consoles' : 'No consoles found')}</h2>
      {#if !query}<p>{t('Turn on discovery to find consoles on your local network.')}</p>{/if}
    </section>
  {/if}
  {#if current && connection}
    <Reminders {current} {connection} setup={openReminderRoute} />
    {#key accountRoute}<PsnAccount {current} {connection} route={accountRoute} close={closeAccountRoute} />{/key}
  {/if}
  <div class="console-grid">
    {#each consoles as console, index (console.key || `unavailable-${index}`)}
      <div class="console-entry">
      <button class="console-card" class:selected={current?.selectedKey === console.key && !!console.key}
        disabled={!console.available} aria-pressed={current?.selectedKey === console.key && !!console.key}
        aria-label={`${console.name}, ${type(console)}, ${status(console)}`} data-console-key={console.key}
        onclick={() => select(console.key)} onfocus={() => select(console.key)}>
        <span class="art">
          <span class="console-type">{type(console)}</span>
          {#if console.ps5 !== undefined}
            <img src={`qrc:/icons/console-ps${console.ps5 ? '5' : '4'}.svg#${console.state === 'ready' ? 'light_on' : console.state === 'standby' ? 'light_standby' : 'console'}`} alt="" />
          {/if}
        </span>
        <span class="card-content">
          <strong class="console-name" title={console.name}>{console.name}</strong>
          <span class="console-status" class:online={console.state === 'ready'}>{status(console)}</span>
          <span class="route">{route(console)}</span>
          {#if current?.selectedKey === console.key && console.key}<span class="selection">{t('Selected')}</span>{/if}
        </span>
      </button>
      <button class="console-details" disabled={!console.available} aria-label={`${t('Console details')}: ${console.name}`}
        data-console-details-key={console.key} onclick={event => { event.currentTarget.focus(); detailsKey = console.key; }}>{t('Console details')}</button>
      </div>
    {/each}
  </div>
  {/if}
</main>
