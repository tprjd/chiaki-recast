<script lang="ts">
  import type { connectSetup, State } from './bridge';
  let { current, connection, optionsOnly = false }: { current?: State; connection?: Awaited<ReturnType<typeof connectSetup>>; optionsOnly?: boolean } = $props();
  let pending = $state(false);
  let message = $state('');
  const t = (text: string) => current?.strings[text] ?? text;
  let selected = $derived(current?.consoles.find(console => console.key === current.selectedKey));
  async function autoConnectTarget(key: string) {
    if (!connection || !current || pending) return;
    pending = true;
    try {
      const reply = await connection.autoConnectTarget(key, current.profileGeneration);
      message = reply.accepted ? '' : reply.message ?? t('Console unavailable');
    } finally { pending = false; }
  }
  async function register() {
    if (!connection || !selected || pending || !current || current.connectionBusy) return;
    pending = true;
    try {
      const reply = await connection.autoRegister(selected.key, current.profileGeneration);
      message = reply.accepted ? '' : reply.message ?? t('Console unavailable');
    } finally { pending = false; }
  }
  async function wake() {
    if (!connection || !selected || pending || current?.connectionBusy) return;
    pending = true;
    try {
      const reply = await connection.wake(selected.key);
      message = reply.accepted ? '' : reply.message ?? t('Console unavailable');
    } finally { pending = false; }
  }
</script>

{#if !optionsOnly}
{#if current?.streamError?.id}
  <section class="stream-error" role="alert">
    <h2>{current.streamError.title}</h2>
    <p>{current.streamError.text}</p>
    <button class="dismiss-stream-error" onclick={() => connection?.dismissStreamError(current!.streamError!.id)}>{t('Dismiss')}</button>
  </section>
{/if}
{#if current?.connection.id}
  <section class="stream-error" data-psn-connection aria-live="polite">
    <h2>{current.connection.phase === 'failure' ? current.connection.errorTitle : current.connection.phase === 'cancelled'
      ? t('Connection cancelled') : current.connection.phase === 'registered' ? t('Console registered') : current.connection.registrationOnly ? t('Registering through PSN')
      : current.connection.context === 'startup' ? t('Auto-connect') : current.connection.context === 'wakeOnly' ? t('Wake console')
      : current.connection.route === 'psn' ? t('Connecting through PSN') : t('Connecting to console')}</h2>
    <p data-connection-phase>{current.connection.phase === 'waking' ? t('Waiting for the console to wake')
      : current.connection.phase === 'woken' ? t('Console is awake') : current.connection.phase === 'waiting' ? t('Waiting for the selected console') : ''}</p>
    {#if current.connection.registrationOnly && !['failure', 'cancelled', 'registered'].includes(current.connection.phase ?? '')}
      <p data-registration-phase>{t(current.connection.phase === 'internet' ? 'Waiting for internet'
        : current.connection.phase === 'initiating' ? 'Starting PSN connection'
        : current.connection.phase === 'registering' ? 'Registering console' : 'Waiting for connection cleanup')}</p>
    {/if}
    <p>{current.connection.errorText || (current.connection.cleanupPending && ['failure', 'cancelled'].includes(current.connection.phase ?? '')
      ? t('Waiting for connection cleanup') : '')}</p>
    {#if current.connection.phase === 'failure'}
      <button data-connection-retry disabled={current.connection.cleanupPending || !current.connection.targetValid}
        onclick={() => connection?.retryConnection(current!.connection.id!)}>{t('Retry')}</button>
    {:else if !['cancelled', 'registered', 'woken'].includes(current.connection.phase ?? '')}
      {#if current.connection.context === 'startup'}
        <button data-stop-auto-connect disabled={!current.connection.cancelable || current.connection.phase === 'cancelling'}
          onclick={() => connection?.stopAutoConnect(current!.connection.id!)}>{t('Stop auto-connect')}</button>
      {:else}
        <button data-connection-cancel disabled={!current.connection.cancelable || current.connection.phase === 'cancelling'}
          onclick={() => connection?.cancelConnection(current!.connection.id!)}>{t('Cancel')}</button>
      {/if}
    {/if}
    {#if ['failure', 'cancelled', 'registered', 'woken'].includes(current.connection.phase ?? '')}
      <button data-connection-back disabled={current.connection.cleanupPending}
        onclick={() => connection?.dismissConnection(current!.connection.id!)}>{t('Back to consoles')}</button>
    {/if}
  </section>
{/if}
{/if}
{#if optionsOnly}
<details class="connection-options"><summary>{t('Connection options')}</summary>
<div class="connect-action">
  {#if selected}<p class="selected-target">{t('Selected')}: {selected.name}</p>{/if}
  {#if current}
    <label class="setting-control">{t('Auto-connect at startup')}
      <select data-auto-connect-target value={current.autoConnectKey} disabled={!connection || pending || current.connectionBusy}
        onchange={event => autoConnectTarget(event.currentTarget.value)}>
        <option value="">{t('Off')}</option>
        {#each current.registrations as console (console.key)}<option value={console.key}>{console.name}</option>{/each}
      </select>
    </label>
  {/if}
  {#if selected?.registered && selected.route !== 'psn'}
    <button data-wake-console disabled={!connection || pending || current?.connectionBusy || !selected.available} onclick={wake}>{t('Wake console')}</button>
  {/if}
  {#if selected?.actions.includes('autoRegister')}
    <button data-automatic-register disabled={!connection || pending || current?.connectionBusy || !selected.available} onclick={register}>{t('Automatic registration')}</button>
    {#if selected.ps5 === false}<span>{t('Automatic registration requires your main PS4 console registered to your PSN account.')}</span>{/if}
  {/if}
  <span role="status">{message}</span>
</div>
</details>
{/if}

<style>
  .connection-options { margin-top: 18px; color: var(--muted); font-size: 12px; }
  summary { cursor: pointer; width: max-content; }
  .selected-target { width: 100%; }
  .connect-action { display: flex; align-items: center; gap: 16px; flex-wrap: wrap; margin: 16px 0 0; }
  button { min-height: 44px; padding: 9px 24px; background: var(--accent); color: #102033; border: 1px solid var(--accent); border-radius: 8px; }
  span { color: var(--muted); }
  .stream-error { border: 1px solid var(--accent); border-radius: 8px; padding: 16px; margin-bottom: 26px; }
  .stream-error p { margin: 12px 0; white-space: pre-wrap; }
</style>
