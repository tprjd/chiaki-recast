<script lang="ts">
  import { tick, onMount } from 'svelte';
  import { generalSettings } from './settings';
  import SettingField from './SettingField.svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection, close, t }: {
    current: State; connection: Awaited<ReturnType<typeof connectSetup>>;
    close: () => void; t: (text: string) => string;
  } = $props();
  let query = $state('');
  let message = $state('');
  let pending = $state(false);
  let search: HTMLInputElement;
  const fields = $derived(generalSettings.filter(field => Object.hasOwn(current.generalSettings, field.key)));
  const groups = $derived([...new Set(fields.map(field => field.group))]);
  const results = $derived(fields.filter(field => query.trim().toLocaleLowerCase().split(/\s+/)
    .every(term => `${t('General')} ${t(field.group)} ${t(field.label)} ${t(field.description)}`.toLocaleLowerCase().includes(term))));
  onMount(() => { document.getElementById('disconnectAction')?.focus(); });

  async function reveal(key: string, requires?: string) {
    query = '';
    await tick();
    const target = document.getElementById(requires && !current.generalSettings[requires] ? requires : key);
    target?.focus();
    target?.scrollIntoView({ block: 'nearest' });
  }
  async function change(key: string, value: boolean | number) {
    if (pending) return;
    const generation = current.profileGeneration;
    pending = true;
    try {
      const reply = await connection.generalSetting(key, value, generation);
      message = reply.accepted ? t('Saved') : reply.message ?? t('The setting could not be saved.');
    } catch {
      message = t('The setting could not be saved.');
    } finally { pending = false; }
  }
  function keyboard(event: KeyboardEvent) {
    if (event.key === 'Escape') {
      event.preventDefault();
      if (query) { query = ''; search.focus(); } else close();
    } else if (event.key === 'PageUp') {
      event.preventDefault(); search.focus();
    } else if (event.key === 'PageDown' && event.target === search) {
      event.preventDefault();
      if (query) document.querySelector<HTMLButtonElement>('.settings-results button')?.focus();
      else document.getElementById('disconnectAction')?.focus();
    }
  }
</script>

<svelte:window onkeydown={keyboard} />
<section data-setup-panel aria-label={t('Settings')}>
<div class="settings-heading">
  <h1>{t('Settings')}</h1>
  <button class="back-consoles" onclick={close}>{t('Back to consoles')}</button>
</div>
<label class="search settings-search">{t('Find a setting')}
  <input bind:this={search} type="search" bind:value={query} aria-label={t('Find a setting')} />
</label>
<div class="settings-layout">
  <nav class="category-nav" aria-label={t('Settings categories')}>
    <button aria-current="page" onclick={() => reveal('disconnectAction')}>{t('General')}</button>
  </nav>
  <div class="settings-content">
    {#if query.trim()}
      <section class="settings-results" aria-label={t('Search results')}>
        <h2>{t('Search results')}</h2>
        <p role="status">{results.length ? `${results.length} ${t('settings found')}` : t('No matching settings')}</p>
        {#each results as field}
          <button onclick={() => reveal(field.key, field.requires)}><strong>{t(field.label)}</strong><span>{t('General')} / {t(field.group)}</span></button>
        {/each}
      </section>
    {:else}
      <h2>{t('General')}</h2>
      <p class="settings-note">{t('Changes are saved automatically.')}</p>
      {#each groups as group}
        <section class="settings-group" aria-label={t(group)}>
          <h3>{t(group)}</h3>
          <div class="setting-rows">
            {#each fields.filter(field => field.group === group) as field (field.key)}
              {#if !field.requires || current.generalSettings[field.requires]}
                <SettingField {field} value={current.generalSettings[field.key]} disabled={pending}
                  change={value => change(field.key, value)} {t} />
              {/if}
            {/each}
          </div>
        </section>
      {/each}
    {/if}
    <p class="settings-message" role="status">{message}</p>
  </div>
</div>
</section>
