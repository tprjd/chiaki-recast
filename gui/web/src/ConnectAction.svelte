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
      ? t('Connection cancelled') : t('Connecting through PSN')}</h2>
    <p>{current.connection.errorText || (current.connection.cleanupPending && ['failure', 'cancelled'].includes(current.connection.phase ?? '')
      ? t('Waiting for connection cleanup') : '')}</p>
    {#if current.connection.phase === 'failure'}
      <button data-connection-retry disabled={current.connection.cleanupPending || !current.connection.targetValid}
        onclick={() => connection?.retryConnection(current!.connection.id!)}>{t('Retry')}</button>
    {:else if current.connection.phase !== 'cancelled'}
      <button data-connection-cancel disabled={!current.connection.cancelable || current.connection.phase === 'cancelling'}
        onclick={() => connection?.cancelConnection(current!.connection.id!)}>{t('Cancel')}</button>
    {/if}
    {#if ['failure', 'cancelled'].includes(current.connection.phase ?? '')}
      <button data-connection-back disabled={current.connection.cleanupPending}
        onclick={() => connection?.dismissConnection(current!.connection.id!)}>{t('Back to consoles')}</button>
    {/if}
  </section>
{/if}
<div class="connect-action">
  <button class="connect-console" disabled={!connection || pending || current?.connectionBusy || !selected?.available
    || !selected.registered || (selected.route !== 'psn' && (selected.route !== 'local' || selected.state !== 'ready'))} onclick={connect}>{t('Connect')}</button>
  <span role="status">{message}</span>
</div>

<style>
  .connect-action { display: flex; align-items: center; gap: 16px; flex-wrap: wrap; margin: 0 0 26px; }
  button { min-height: 44px; padding: 9px 24px; background: var(--accent); color: #102033; border: 1px solid var(--accent); border-radius: 8px; }
  span { color: var(--muted); }
  .stream-error { border: 1px solid var(--accent); border-radius: 8px; padding: 16px; margin-bottom: 26px; }
  .stream-error p { margin: 12px 0; white-space: pre-wrap; }
</style>
