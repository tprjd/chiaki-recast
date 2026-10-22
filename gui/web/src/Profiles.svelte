<script lang="ts">
  import { untrack } from 'svelte';
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
  $effect(() => {
    generation = current.profileGeneration;
    const active = current.profiles.items.find(item => item.active)?.key ?? '';
    untrack(() => { selected = active; name = ''; deleting = false; message = ''; });
  });
  async function apply() {
    if (pending) return;
    pending = true;
    const owner = generation;
    try {
      const result = await connection.profileAction(creating ? 'create' : deleting ? 'delete' : 'switch', selected, name, deleting, owner);
      if (owner === current.profileGeneration || result.accepted) message = result.message ?? '';
    } finally { pending = false; }
  }
</script>
<section class="settings-group" aria-label={t('Profiles')}>
  <h3>{t('Profiles')}</h3>
  <p data-active-profile>{t('Current profile')}: {current.profiles.current}</p>
  <label for="profileSelection">{t('User profile')}</label>
  <select id="profileSelection" bind:value={selected} disabled={pending || current.connectionBusy} onchange={() => { deleting = false; name = ''; }}>
    {#each current.profiles.items as profile}<option value={profile.key}>{profile.label}</option>{/each}
    <option value="create">{t('Create new profile')}</option>
  </select>
  {#if creating}
    <label for="newProfileName">{t('New profile name')}</label>
    <input id="newProfileName" type={current.streamerMode ? 'password' : 'text'} bind:value={name} disabled={pending} autocomplete="off" />
  {/if}
  {#if item?.deletable}
    <label><input id="deleteProfile" type="checkbox" bind:checked={deleting} disabled={pending} />{t('Delete selected profile')}</label>
  {/if}
  <p>{t('The default and current profiles cannot be deleted.')}</p>
  <button id="profileAction" disabled={pending || current.connectionBusy || (creating ? !name.trim() : !item || item.active)} onclick={apply}>
    {t(deleting ? 'Delete Profile' : creating ? 'Create Profile' : 'Switch Profile')}
  </button>
  <p data-profile-message role="status">{message}</p>
</section>
