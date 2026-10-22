<script lang="ts">
  import { tick, untrack } from 'svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection, t }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>>; t: (text: string) => string } = $props();
  let selected = $state('');
  let name = $state('');
  let deleting = $state(false);
  let pending = $state(false);
  let message = $state('');
  let generation = '';
  const item = $derived(current.profiles.items.find(item => item.key === selected));
  const creating = $derived(selected === 'create');
  const profileGeneration = $derived(current.profileGeneration);
  const catalog = $derived(current.profiles.items.map(item => item.key).join(','));
  $effect(() => {
    generation = profileGeneration;
    void catalog;
    untrack(() => { selected = current.profiles.items.find(item => item.active)?.key ?? ''; name = ''; deleting = false; message = ''; });
  });
  async function apply() {
    if (pending) return;
    pending = true;
    const owner = generation;
    try {
      const result = await connection.profileAction(creating ? 'create' : deleting ? 'delete' : 'switch', selected, name, deleting, owner);
      if (owner !== current.profileGeneration && result.profileGeneration !== current.profileGeneration) return;
      message = result.message ?? '';
      if (result.accepted) {
        pending = false;
        await tick();
        document.getElementById('profileSelection')?.focus();
      }
    } finally { pending = false; }
  }
</script>
<section class="settings-group" aria-label={t('Profiles')}>
  <h3>{t('Profiles')}</h3>
  <div class="setting-rows">
  <div class="setting-row">
    <div class="setting-copy"><label for="profileSelection">{t('User profile')}</label><p data-active-profile>{t('Current profile')}: {current.profiles.current}</p></div>
    <div class="setting-control">
  <select id="profileSelection" bind:value={selected} disabled={pending || current.connectionBusy} onchange={() => { deleting = false; name = ''; }}>
    {#each current.profiles.items as profile}<option value={profile.key}>{profile.label}</option>{/each}
    <option value="create">{t('Create new profile')}</option>
  </select>
    </div>
  </div>
  {#if creating}
    <div class="setting-row">
      <div class="setting-copy"><label for="newProfileName">{t('New profile name')}</label></div>
      <div class="setting-control"><input id="newProfileName" type={current.streamerMode ? 'password' : 'text'} bind:value={name} disabled={pending} autocomplete="off" /></div>
    </div>
  {/if}
  {#if item?.deletable}
    <div class="setting-row">
      <div class="setting-copy"><label for="deleteProfile">{t('Delete selected profile')}</label></div>
      <div class="setting-control"><input id="deleteProfile" type="checkbox" bind:checked={deleting} disabled={pending} /></div>
    </div>
  {/if}
  <div class="setting-row profile-action">
  <p>{t('The default and current profiles cannot be deleted.')}</p>
  <button id="profileAction" disabled={pending || current.connectionBusy || (creating ? !name.trim() : !item || item.active)} onclick={apply}>
    {t(deleting ? 'Delete Profile' : creating ? 'Create Profile' : 'Switch Profile')}
  </button>
  </div>
  </div>
  <p class="profile-message" data-profile-message role="status">{message}</p>
</section>

<style>
  .setting-copy label { display:block; }
  .setting-control input:not([type=checkbox]) { width:100%; min-height:38px; padding:8px 11px; }
  .setting-control input[type=checkbox] { appearance:auto; width:16px; height:16px; accent-color:var(--accent); }
  .setting-control input[type=checkbox]::after { display:none; }
  .profile-action p,.profile-message { color:var(--muted); font-size:12px; }
  .profile-action button { min-height:36px; font-size:12px; color:var(--accent); flex-shrink:0; }
  .profile-message:not(:empty) { margin-top:12px; }
</style>
