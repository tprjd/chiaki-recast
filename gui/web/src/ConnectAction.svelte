<script lang="ts">
  import type { connectSetup, State } from './bridge';
  let { current, connection }: { current?: State; connection?: Awaited<ReturnType<typeof connectSetup>> } = $props();
  let pending = $state(false);
  let message = $state('');
  const t = (text: string) => current?.strings[text] ?? text;
  let selected = $derived(current?.consoles.find(console => console.key === current.selectedKey));
  async function connect() {
    if (!connection || !selected || pending || current?.connectionBusy) return;
    pending = true;
    try {
      const reply = await connection.connect(selected.key);
      message = reply.accepted ? '' : reply.message ?? t('Console unavailable');
    } finally { pending = false; }
  }
  async function autoConnectTarget(key: string) {
    if (!connection || !current || pending) return;
    pending = true;
    try {
      const reply = await connection.autoConnectTarget(key, current.profileGeneration);
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
      ? t('Connection cancelled') : current.connection.context === 'startup' ? t('Auto-connect') : current.connection.context === 'wakeOnly' ? t('Wake console')
      : current.connection.route === 'psn' ? t('Connecting through PSN') : t('Connecting to console')}</h2>
    <p data-connection-phase>{current.connection.phase === 'waking' ? t('Waiting for the console to wake')
      : current.connection.phase === 'woken' ? t('Console is awake') : current.connection.phase === 'waiting' ? t('Waiting for the selected console') : ''}</p>
    <p>{current.connection.errorText || (current.connection.cleanupPending && ['failure', 'cancelled'].includes(current.connection.phase ?? '')
      ? t('Waiting for connection cleanup') : '')}</p>
    {#if current.connection.phase === 'failure'}
      <button data-connection-retry disabled={current.connection.cleanupPending || !current.connection.targetValid}
        onclick={() => connection?.retryConnection(current!.connection.id!)}>{t('Retry')}</button>
    {:else if !['cancelled', 'woken'].includes(current.connection.phase ?? '')}
      {#if current.connection.context === 'startup'}
        <button data-stop-auto-connect disabled={!current.connection.cancelable || current.connection.phase === 'cancelling'}
          onclick={() => connection?.stopAutoConnect(current!.connection.id!)}>{t('Stop auto-connect')}</button>
      {:else}
        <button data-connection-cancel disabled={!current.connection.cancelable || current.connection.phase === 'cancelling'}
          onclick={() => connection?.cancelConnection(current!.connection.id!)}>{t('Cancel')}</button>
      {/if}
    {/if}
    {#if ['failure', 'cancelled', 'woken'].includes(current.connection.phase ?? '')}
      <button data-connection-back disabled={current.connection.cleanupPending}
        onclick={() => connection?.dismissConnection(current!.connection.id!)}>{t('Back to consoles')}</button>
    {/if}
  </section>
{/if}
<div class="connect-action">
  {#if current}
    <label class="setting-control">{t('Auto-connect at startup')}
      <select data-auto-connect-target value={current.autoConnectKey} disabled={!connection || pending || current.connectionBusy}
        onchange={event => autoConnectTarget(event.currentTarget.value)}>
        <option value="">{t('Off')}</option>
        {#each current.registrations as console (console.key)}<option value={console.key}>{console.name}</option>{/each}
      </select>
    </label>
  {/if}
  <button class="connect-console" disabled={!connection || pending || current?.connectionBusy || !selected?.available
    || !selected.registered || (selected.route === 'local' && !['ready', 'standby'].includes(selected.state))} onclick={connect}>
    {t(selected?.route === 'local' && selected.state === 'standby' ? 'Wake and connect' : 'Connect')}</button>
  {#if selected?.registered && selected.route !== 'psn'}
    <button data-wake-console disabled={!connection || pending || current?.connectionBusy || !selected.available} onclick={wake}>{t('Wake console')}</button>
  {/if}
  <span role="status">{message}</span>
</div>

<style>
  .connect-action { display: flex; align-items: center; gap: 16px; flex-wrap: wrap; margin: 0 0 26px; }
  button { min-height: 44px; padding: 9px 24px; background: var(--accent); color: #102033; border: 1px solid var(--accent); border-radius: 8px; }
  span { color: var(--muted); }
  .stream-error { border: 1px solid var(--accent); border-radius: 8px; padding: 16px; margin-bottom: 26px; }
  .stream-error p { margin: 12px 0; white-space: pre-wrap; }
</style>
