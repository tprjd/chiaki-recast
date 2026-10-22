<script lang="ts">
  import { tick, onMount } from 'svelte';
  import Icon from './Icon.svelte';
  import KeyboardMouse from './KeyboardMouse.svelte';
  import Controllers from './Controllers.svelte';
  import Profiles from './Profiles.svelte';
  import { moveFocus } from './navigation';
  import { generalSettings, mediaSettings, rendererSettings, colorMappingSettings, profileSettings, settingConditionMet, keyboardMouseSettings, controllerSettings } from './settings';
  import SettingField from './SettingField.svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection, close, t, query = $bindable(''), category = $bindable('General') }: {
    current: State; connection: Awaited<ReturnType<typeof connectSetup>>;
    close: () => void; t: (text: string) => string;
    query?: string; category?: string;
  } = $props();
  let message = $state('');
  let pending = $state(false);
  let keyboardMouseTarget = $state('');
  let controllerTarget = $state('');
  let search: HTMLInputElement;
  const values = $derived({ ...current.generalSettings, ...current.mediaSettings, ...current.rendererSettings, ...current.keyboardMouse.values, ...current.controllerSetup.values });
  const fields = $derived([...generalSettings, ...profileSettings, ...mediaSettings, ...rendererSettings, ...colorMappingSettings, ...controllerSettings, ...keyboardMouseSettings.filter(field => current.keyboardMouse.supported || ['keyboardEnabled', 'mouseTouchEnabled'].includes(field.key))].filter(field => field.action || Object.hasOwn(values, field.key)).map(field => {
    const choices = current.mediaChoices[field.key];
    if (field.key.startsWith('bitrate')) return { ...field, automatic: [0, 2, 6, 10, 15][Number(values[`resolution${field.key.slice(7)}`])] };
    return choices ? { ...field, options: choices.map(choice => choice.label), optionValues: choices.map(choice => choice.value) } : field;
  }));
  const categoryIcons: Record<string, string> = { General: 'settings', Video: 'video', Stream: 'remote', 'Audio/Wifi': 'audio', Display: 'display', Renderer: 'settings', 'Color mapping': 'video', 'Keyboard and mouse': 'keyboard', Controllers: 'consoles', Profiles: 'profile' };
  const categoryDescriptions: Record<string, string> = {
    General: 'Session behavior, privacy and shortcuts',
    Video: 'Decoder, stream window and rendering',
    Stream: 'Local and remote quality for each console type',
    'Audio/Wifi': 'Devices, microphone and connection feedback',
    Display: 'Display resolution, brightness and color',
    Renderer: 'Native rendering and video processing',
    'Color mapping': 'Gamut and tone mapping',
    'Keyboard and mouse': 'Keyboard controls, mouse response and bindings',
    Controllers: 'Controller input, mapping, touchpad and haptics',
    Profiles: 'Profiles and settings files'
  };
  const categories = ['General', 'Video', 'Stream', 'Audio/Wifi', 'Display', 'Renderer', 'Color mapping', 'Keyboard and mouse', 'Controllers', 'Profiles'];
  const visibleFields = $derived(fields.filter(field => (field.category ?? 'General') === category
    && (!field.requires || values[field.requires])
    && (!field.requiresValue || values[field.requiresValue.key] === field.requiresValue.value)
    && (!field.when || field.when.every(condition => settingConditionMet(condition, values)))));
  const groups = $derived([...new Set(visibleFields.map(field => field.group))]);
  const searchFields = $derived([...fields,
    { key: 'profileSelection', label: 'Create or switch profile', category: 'Profiles', group: 'Profiles', description: 'Select a profile, then use the explicit action.' },
    { key: 'newProfileName', label: 'New profile name', category: 'Profiles', group: 'Profiles', description: 'Create a named profile.' },
    { key: 'deleteProfile', label: 'Delete selected profile', category: 'Profiles', group: 'Profiles', description: 'The default and current profiles cannot be deleted.' },
    ...['controllerMappingChange', 'controllerMappingReset'].map((key, index) => ({ key, label: index ? 'Reset controller mapping' : 'Change controller mapping', category: 'Controllers', group: 'Controller mapping', description: 'Select a controller by pressing a button.' })),
    ...['enableAnalogStickMapping', 'controllerMappingApply', 'controllerMappingCancel'].map((key, index) => ({ key, label: ['Analog stick and L2/R2 mapping', 'Apply controller mapping', 'Cancel controller mapping'][index], category: 'Controllers', group: 'Controller mapping', description: 'Select a controller before editing its mappings.' })),
    ...current.controllerSetup.targets.flatMap(mapping => [0, 1].map(slot => ({ key: `controllerMap${mapping.buttonValue}Slot${slot}`, label: `${mapping.buttonName} ${t(slot ? 'alternate input' : 'primary input')}`, category: 'Controllers', group: 'Controller mapping', description: 'Assign a controller input.' }))),
    ...(current.keyboardMouse.supported ? [{ key: 'keyboardMouseReset', label: 'Reset defaults', category: 'Keyboard and mouse', group: 'Keyboard and mouse', description: 'Restore mouse response and bindings and turn mouse camera mode off. Confirmation is required.' },
      ...current.keyboardMouse.bindings.flatMap(binding => [0, 1].map(slot => ({ key: `keyboardMouseBinding${binding.action}_${slot}`, label: `${binding.name} ${t(slot ? 'Alternative' : 'Primary')}`, category: 'Keyboard and mouse', group: 'Bindings', description: 'Assign or clear one key or mouse button.', requires: 'mouseCameraEnabled' })))] : []),
    { key: 'resetAllKeys', label: 'Reset all keys', category: 'Keyboard and mouse', group: 'Key assignments', description: 'Restore all keyboard assignments to their native defaults.' },
    ...current.keyboardMouse.keys.map(mapping => ({ key: `keyMapping${mapping.buttonValue}`, label: `${t('Key for')} ${mapping.buttonName}`, category: 'Keyboard and mouse', group: 'Key assignments', description: 'Assign a keyboard key.' }))]);
  const results = $derived(searchFields.filter(field => query.trim().toLocaleLowerCase().split(/\s+/)
    .every(term => `${t(field.category ?? 'General')} ${t(field.group)} ${t(field.label)} ${t(field.description)}`.toLocaleLowerCase().includes(term))));
  onMount(() => {
    (query ? search : document.querySelector<HTMLElement>('.settings-content button:enabled, .settings-content input:enabled, .settings-content select:enabled'))?.focus();
  });

  async function reveal(key: string, requires?: string) {
    query = '';
    const field = fields.find(field => field.key === key);
    const unmet = field?.when?.find(condition => !settingConditionMet(condition, values));
    const focusKey = unmet?.key ?? key;
    category = searchFields.find(field => field.key === focusKey)?.category ?? 'General';
    controllerTarget = key;
    keyboardMouseTarget = requires && !values[requires] ? requires
      : category === 'Keyboard and mouse' && values.mouseCameraEnabled && (key.startsWith('keyMapping') || ['keyboardEnabled', 'mouseTouchEnabled', 'resetAllKeys'].includes(key)) ? 'mouseCameraEnabled' : key;
    if (category === 'Audio/Wifi') connection.refreshAudioDevices();
    await tick();
    const target = document.getElementById(unmet ? unmet.key : requires && !values[requires] ? requires : field?.requiresValue && values[field.requiresValue.key] !== field.requiresValue.value ? field.requiresValue.key : key);
    const available = target ?? (category === 'Profiles' ? document.getElementById('profileSelection') : null);
    available?.focus();
    available?.scrollIntoView({ block: 'nearest' });
  }
  async function change(key: string, value: boolean | number | string, generation = current.profileGeneration) {
    if (key === 'openPlaceboColorMapping') { await reveal('placeboGamutMappingFunction'); return; }
    if (pending) return;
    pending = true;
    try {
      const action = key === 'restartRenderer' || rendererSettings.find(field => field.key === key)?.action;
      const reply = await (profileSettings.some(field => field.key === key && field.action) ? connection.settingsFileAction(key, generation) : action ? connection.rendererAction(key, generation) : Object.hasOwn(current.controllerSetup.values, key) ? connection.controllerSetting(key, value, generation) : Object.hasOwn(current.rendererSettings, key) ? connection.rendererSetting(key, value, generation) : Object.hasOwn(current.generalSettings, key) ? connection.generalSetting(key, value as boolean | number, generation) : connection.mediaSetting(key, value, generation));
      if (generation !== current.profileGeneration && !(key === 'importSettings' && reply.accepted && reply.profileGeneration === current.profileGeneration)) return;
      message = reply.message ?? (reply.accepted ? t('Saved') : t('The setting could not be saved.'));
    } catch {
      message = t('The setting could not be saved.');
    } finally { pending = false; }
  }
  function keyboard(event: KeyboardEvent) {
    if (document.querySelector('dialog[open]')) return;
    moveFocus(event, document.querySelector('[data-setup-panel]')!);
    if (event.key === 'Escape') {
      event.preventDefault();
      if (query) { query = ''; search.focus(); } else if (category === 'Color mapping') reveal('openPlaceboColorMapping'); else close();
    } else if (event.key === 'PageUp') {
      event.preventDefault(); search.focus();
    } else if (event.key === 'PageDown') {
      event.preventDefault();
      if (query) document.querySelector<HTMLButtonElement>('.settings-results button')?.focus();
      else {
        const controls = [...document.querySelectorAll<HTMLElement>('.settings-content button:enabled, .settings-content input:enabled, .settings-content select:enabled')];
        (controls[controls.indexOf(document.activeElement as HTMLElement) + 1] ?? controls[0])?.focus();
      }
    }
  }
</script>

<svelte:window onkeydown={keyboard} />
<section data-setup-panel aria-label={t('Settings')}>
<div class="settings-heading">
  <div><p class="breadcrumb">{t('Preferences')}</p><h1>{t('Settings')}</h1></div>
  <div class="profile-chip"><span class="profile-avatar"><Icon name="profile" size={17} /></span><span><small>{t('Current profile')}</small><strong>{current.profiles.current}</strong></span></div>
</div>
<div class="settings-tools">
  <label class="search settings-search"><Icon name="search" size={17} /><input bind:this={search} type="search" maxlength="256" bind:value={query} placeholder={t('Find a setting')} aria-label={t('Find a setting')} /></label>
  <span>{t('Changes are saved automatically.')}</span>
</div>
<div class="settings-layout">
  <nav class="category-nav" aria-label={t('Settings categories')}>
    <span class="nav-label">{t('Preferences')}</span>
    {#each categories as item}
      <button aria-current={category === item ? 'page' : undefined} onclick={() => { category = item; query = ''; controllerTarget = ''; if (item === 'Audio/Wifi') connection.refreshAudioDevices(); }}><Icon name={categoryIcons[item]} size={17} />{t(item)}</button>
    {/each}
  </nav>
  <div class="settings-content">
    {#if query.trim()}
      <section class="settings-results" aria-label={t('Search results')}>
        <h2>{t('Search results')}</h2>
        <p role="status">{results.length ? `${results.length} ${t('settings found')}` : t('No matching settings')}</p>
        {#each results as field}
          <button onclick={() => reveal(field.key, 'requires' in field ? field.requires : undefined)}><strong>{t(field.label)}</strong><span>{t(field.category ?? 'General')} / {t(field.group)}</span></button>
        {/each}
      </section>
    {:else}
      <div class="category-heading">
        <div><h2>{t(category)}</h2><p class="category-description">{t(categoryDescriptions[category] ?? 'Configure your Remote Play experience.')}</p></div>
        {#if category === 'Audio/Wifi'}<button class="category-action" disabled={pending} onclick={() => connection.refreshAudioDevices()}><Icon name="refresh" size={15} />{t('Refresh audio devices')}</button>{/if}
      </div>
      {#if category === 'Color mapping'}
        <button class="back-consoles" id="backRenderer" onclick={() => reveal('openPlaceboColorMapping')}>{t('Back to Renderer')}</button>
        {#if values.videoPreset !== 5 || !values.placeboColorMappingEnabled || values.placeboColorMappingPreset !== 0}
          <p>{t('Requires the Custom render preset, Color Mapping Enabled, and the Custom color mapping preset in Renderer.')}</p>
          <button class="back-consoles" onclick={() => reveal('openPlaceboColorMapping')}>{t('Configure color mapping in Renderer')}</button>
        {/if}
      {/if}
      {#if ['Video', 'Stream', 'Audio/Wifi'].includes(category)}<p class="settings-note">{t('Stream settings apply to the next session. Audio volume also changes the current session.')}</p>{/if}
      {#if category === 'Renderer'}
        <p data-renderer-runtime>{current.rendererStatus.runtimeBackend === undefined ? t('No native stream window is active.') : `${t('Active native renderer')}: ${current.rendererStatus.runtimeBackend === 0 ? 'Vulkan' : 'OpenGL'}`}</p>
        {#if current.rendererStatus.restartRequired}
          <p data-renderer-restart role="status">{t('Restart the application to apply the saved renderer backend or OpenGL vertical sync change.')}</p>
          <button disabled={pending} onclick={() => change('restartRenderer', true)}>{t('Restart application')}</button>
        {/if}
      {/if}
      {#if category === 'Profiles'}<Profiles {current} {connection} {t} />{/if}
      {#key current.profileGeneration}
      {#if category === 'Keyboard and mouse'}
        <KeyboardMouse {current} {connection} {t} target={keyboardMouseTarget} />
      {:else}
      {#if category === 'Controllers'}<Controllers {current} {connection} {t} target={controllerTarget} />{/if}
      {#each groups as group}
        <section class="settings-group" aria-label={t(group)}>
          <h3>{t(group)}</h3>
          <div class="setting-rows">
            {#each visibleFields.filter(field => field.group === group) as field (field.key)}
                <SettingField {field} value={values[field.key] ?? false} disabled={pending}
                  generation={current.profileGeneration} change={(value, generation) => change(field.key, value, generation)} {t} />
            {/each}
          </div>
        </section>
      {/each}
      {/if}
      {/key}
      {#if category === 'General'}<details class="log-location"><summary>{t('Log directory')}</summary><p data-log-directory>{current.profiles.logDirectory}</p></details>{/if}
    {/if}
    <p class="settings-message" role="status">{message}</p>
  </div>
</div>
</section>

<style>
  .settings-heading h1 { font-size:34px; }
  .settings-search { padding:10px 13px; }
  .category-heading { display:flex; align-items:center; justify-content:space-between; gap:16px; margin-bottom:22px; }
  .category-description { margin-top:4px; color:var(--muted); font-size:13px; }
  .category-action { display:inline-flex; align-items:center; justify-content:center; gap:7px; min-height:36px; padding:8px 11px; color:var(--accent); font-size:12px; }
  .log-location p { margin-top:8px; }
</style>
