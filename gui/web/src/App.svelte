<script lang="ts">
  import { onMount, tick } from 'svelte';
  import { connectSetup, type State, type Console } from './bridge';
  import Settings from './Settings.svelte';

  let current = $state<State>();
  let connection = $state<Awaited<ReturnType<typeof connectSetup>>>();
  let query = $state('');
  let message = $state('');
  let changingDiscovery = $state(false);
  let settingsOpen = $state(false);
  const t = (text: string) => current?.strings[text] ?? text;
  const type = (console: Console) => console.ps5 === undefined ? t('Console') : console.ps5 ? 'PlayStation 5' : 'PlayStation 4';
  const status = (console: Console) => !console.available ? t('Console unavailable') : !console.registered ? t('Not registered')
    : console.state === 'ready' ? t('Online') : console.state === 'standby' ? t('In standby') : t('Status unknown');
  const route = (console: Console) => t(console.route === 'local' ? 'Local network' : console.route === 'psn' ? 'Remote connection via PSN' : 'Manual address');
  let consoles = $derived(current?.consoles.filter(console => query.trim().toLocaleLowerCase().split(/\s+/)
    .every(term => `${console.name} ${type(console)} ${console.ps5 === undefined ? '' : console.ps5 ? 'PS5' : 'PS4'}`.toLocaleLowerCase().includes(term))) ?? []);

  onMount(() => {
    let disposed = false;
    connectSetup(value => { current = value; }).then(value => {
      if (disposed) value.dispose(); else connection = value;
    }).catch(() => { message = t('Setup unavailable'); });
    return () => { disposed = true; connection?.dispose(); };
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
    if (settingsOpen || target.closest('[data-setup-panel]')) return;
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
  async function closeSettings() {
    settingsOpen = false;
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
    <Settings {current} {connection} close={closeSettings} {t} />
  {:else}
  <div class="library-heading">
    <h1>{t('Consoles')}</h1>
    <label class="search">{t('Search consoles')}
      <input type="search" bind:value={query} aria-label={t('Search consoles')} />
    </label>
  </div>
  <div class="library-tools">
    <button class="open-settings" disabled={!current || !connection} onclick={() => { settingsOpen = true; }}>{t('Settings')}</button>
    <button class="discovery" aria-pressed={current?.discoveryEnabled ?? false}
      disabled={!connection || changingDiscovery || !current?.capabilities.discovery} onclick={discovery}>
      {t('Discovery')}<span>{t(current?.discoveryEnabled ? 'Discovery is on' : 'Discovery is off')}</span>
    </button>
    <span role="status" class="message">{message || (!current ? t('Loading consoles') : '')}</span>
  </div>
  {#if current && !consoles.length}
    <section class="empty" aria-live="polite">
      <h2>{t(query ? 'No matching consoles' : 'No consoles found')}</h2>
      {#if !query}<p>{t('Turn on discovery to find consoles on your local network.')}</p>{/if}
    </section>
  {/if}
  <div class="console-grid">
    {#each consoles as console, index (console.key || `unavailable-${index}`)}
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
    {/each}
  </div>
  {/if}
</main>
