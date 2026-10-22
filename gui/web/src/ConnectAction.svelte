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
  async function register() {
    if (!connection || !selected || pending || !current || current.connectionBusy) return;
    pending = true;
    try {
      const reply = await connection.autoRegister(selected.key, current.profileGeneration);
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
      ? t('Connection cancelled') : current.connection.phase === 'registered' ? t('Console registered')
      : current.connection.registrationOnly ? t('Registering through PSN') : t('Connecting through PSN')}</h2>
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
    {:else if !['cancelled', 'registered'].includes(current.connection.phase ?? '')}
      <button data-connection-cancel disabled={!current.connection.cancelable || current.connection.phase === 'cancelling'}
        onclick={() => connection?.cancelConnection(current!.connection.id!)}>{t('Cancel')}</button>
    {/if}
    {#if ['failure', 'cancelled', 'registered'].includes(current.connection.phase ?? '')}
      <button data-connection-back disabled={current.connection.cleanupPending}
        onclick={() => connection?.dismissConnection(current!.connection.id!)}>{t('Back to consoles')}</button>
    {/if}
  </section>
{/if}
<div class="connect-action">
  <button class="connect-console" disabled={!connection || pending || current?.connectionBusy || !selected?.available
    || !selected.registered || (selected.route === 'local' && selected.state !== 'ready')} onclick={connect}>{t('Connect')}</button>
  {#if selected?.actions.includes('autoRegister')}
    <button data-automatic-register disabled={!connection || pending || current?.connectionBusy || !selected.available} onclick={register}>{t('Automatic registration')}</button>
    {#if selected.ps5 === false}<span>{t('Automatic registration requires your main PS4 console registered to your PSN account.')}</span>{/if}
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
