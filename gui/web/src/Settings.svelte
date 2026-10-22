<script lang="ts">
  import { tick, onMount } from 'svelte';
  import { generalSettings, mediaSettings, rendererSettings } from './settings';
  import SettingField from './SettingField.svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection, close, t }: {
    current: State; connection: Awaited<ReturnType<typeof connectSetup>>;
    close: () => void; t: (text: string) => string;
  } = $props();
  let query = $state('');
  let category = $state('General');
  let message = $state('');
  let pending = $state(false);
  let search: HTMLInputElement;
  const values = $derived({ ...current.generalSettings, ...current.mediaSettings, ...current.rendererSettings });
  const fields = $derived([...generalSettings, ...mediaSettings, ...rendererSettings].filter(field => field.action || Object.hasOwn(values, field.key)).map(field => {
    const choices = current.mediaChoices[field.key];
    if (field.key.startsWith('bitrate')) return { ...field, automatic: [0, 2, 6, 10, 15][Number(values[`resolution${field.key.slice(7)}`])] };
    return choices ? { ...field, options: choices.map(choice => choice.label), optionValues: choices.map(choice => choice.value) } : field;
  }));
  const categories = ['General', 'Video', 'Stream', 'Audio/Wifi', 'Display', 'Renderer', 'Color mapping'];
  const visibleFields = $derived(fields.filter(field => (field.category ?? 'General') === category));
  const groups = $derived([...new Set(visibleFields.map(field => field.group))]);
  const results = $derived(fields.filter(field => query.trim().toLocaleLowerCase().split(/\s+/)
    .every(term => `${t(field.category ?? 'General')} ${t(field.group)} ${t(field.label)} ${t(field.description)}`.toLocaleLowerCase().includes(term))));
  onMount(() => { document.getElementById('disconnectAction')?.focus(); });

  async function reveal(key: string, requires?: string) {
    query = '';
    category = fields.find(field => field.key === key)?.category ?? 'General';
    if (category === 'Audio/Wifi') connection.refreshAudioDevices();
    await tick();
    const field = fields.find(field => field.key === key);
    const unmet = field?.when?.find(condition => values[condition.key] !== condition.value);
    const target = document.getElementById(unmet ? unmet.key : requires && !values[requires] ? requires : field?.requiresValue && values[field.requiresValue.key] !== field.requiresValue.value ? field.requiresValue.key : key);
    target?.focus();
    target?.scrollIntoView({ block: 'nearest' });
  }
  async function change(key: string, value: boolean | number | string, generation = current.profileGeneration) {
    if (pending) return;
    pending = true;
    try {
      const action = key === 'restartRenderer' || rendererSettings.find(field => field.key === key)?.action;
      const reply = await (action ? connection.rendererAction(key, generation) : Object.hasOwn(current.rendererSettings, key) ? connection.rendererSetting(key, value, generation) : Object.hasOwn(current.generalSettings, key) ? connection.generalSetting(key, value as boolean | number, generation) : connection.mediaSetting(key, value, generation));
      if (generation !== current.profileGeneration) return;
      message = reply.message ?? (reply.accepted ? t('Saved') : t('The setting could not be saved.'));
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
      else document.getElementById(visibleFields[0]?.key)?.focus();
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
    {#each categories as item}
      <button aria-current={category === item ? 'page' : undefined} onclick={() => { category = item; query = ''; if (item === 'Audio/Wifi') connection.refreshAudioDevices(); }}>{t(item)}</button>
    {/each}
  </nav>
  <div class="settings-content">
    {#if query.trim()}
      <section class="settings-results" aria-label={t('Search results')}>
        <h2>{t('Search results')}</h2>
        <p role="status">{results.length ? `${results.length} ${t('settings found')}` : t('No matching settings')}</p>
        {#each results as field}
          <button onclick={() => reveal(field.key, field.requires)}><strong>{t(field.label)}</strong><span>{t(field.category ?? 'General')} / {t(field.group)}</span></button>
        {/each}
      </section>
    {:else}
      <h2>{t(category)}</h2>
      <p class="settings-note">{t('Changes are saved automatically.')}</p>
      {#if ['Video', 'Stream', 'Audio/Wifi'].includes(category)}<p class="settings-note">{t('Stream settings apply to the next session. Audio volume also changes the current session.')}</p>{/if}
      {#if category === 'Renderer'}
        <p data-renderer-runtime>{current.rendererStatus.runtimeBackend === undefined ? t('No native stream window is active.') : `${t('Active native renderer')}: ${current.rendererStatus.runtimeBackend === 0 ? 'Vulkan' : 'OpenGL'}`}</p>
        {#if current.rendererStatus.restartRequired}
          <p data-renderer-restart role="status">{t('Restart the application to apply the saved renderer backend or OpenGL vertical sync change.')}</p>
          <button disabled={pending} onclick={() => change('restartRenderer', true)}>{t('Restart application')}</button>
        {/if}
      {/if}
      {#if category === 'Audio/Wifi'}<button disabled={pending} onclick={() => connection.refreshAudioDevices()}>{t('Refresh audio devices')}</button>{/if}
      {#key current.profileGeneration}
      {#each groups as group}
        <section class="settings-group" aria-label={t(group)}>
          <h3>{t(group)}</h3>
          <div class="setting-rows">
            {#each visibleFields.filter(field => field.group === group) as field (field.key)}
              {#if (!field.requires || values[field.requires]) && (!field.requiresValue || values[field.requiresValue.key] === field.requiresValue.value) && (!field.when || field.when.every(condition => values[condition.key] === condition.value))}
                <SettingField {field} value={values[field.key] ?? false} disabled={pending}
                  generation={current.profileGeneration} change={(value, generation) => change(field.key, value, generation)} {t} />
              {/if}
            {/each}
          </div>
        </section>
      {/each}
      {/key}
    {/if}
    <p class="settings-message" role="status">{message}</p>
  </div>
</div>
</section>
