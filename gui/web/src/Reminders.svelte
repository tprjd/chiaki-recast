<script lang="ts">
  import { tick } from 'svelte';
  import Dialog from './Dialog.svelte';
  import { moveFocus } from './navigation';
  import type { State, connectSetup } from './bridge';
  let { current, connection, setup }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>>; setup: (route: string) => void } = $props();
  let pending = $state(false);
  const t = (text: string) => current.strings[text] ?? text;
  $effect(() => { if (current.reminder.eligible) connection.startReminders(current.profileGeneration); });
  async function choose(action: string) {
    if (pending) return;
    pending = true;
    try {
      const reply = await connection.reminderAction(current.reminder.id, action, current.profileGeneration);
      if (reply.accepted && reply.route) { await tick(); setup(reply.route); }
    } finally { pending = false; }
  }
</script>
<svelte:window onkeydown={event => {
  const panel = document.querySelector<HTMLElement>('[data-reminder]');
  if (panel) moveFocus(event, panel);
}} />
{#if current.reminder.id}
  {#key current.reminder.id}
    <Dialog label={t(current.reminder.kind === 'steam' ? 'Steam shortcut' : 'PSN sign-in')} close={() => choose('later')}>
      <section data-reminder={current.reminder.kind}>
        <h2>{t(current.reminder.kind === 'steam' ? 'Steam shortcut' : 'PSN sign-in')}</h2>
        <p>{t(current.reminder.kind === 'steam'
          ? 'Create or update a non-Steam game for Chiaki Recast with the official artwork and controller layout. You can also set this up later from the console library.'
          : 'Sign in to PSN to register consoles automatically and use Remote Play outside your home network. Sign-in saves your account credentials in Chiaki Recast. You can also set this up later from PSN account.')}</p>
        <div class="actions">
          <button data-reminder-now disabled={pending} onclick={() => choose('now')}>{t('Set up now')}</button>
          <button data-reminder-never disabled={pending} onclick={() => choose('never')}>{t("Don't ask again")}</button>
          <button data-initial-focus data-reminder-later disabled={pending} onclick={() => choose('later')}>{t('Later')}</button>
        </div>
      </section>
    </Dialog>
  {/key}
{/if}
<style>
  p { margin-block: 20px; }
  .actions { display: flex; flex-wrap: wrap; gap: 10px; }
  button { min-height: 44px; padding: 9px 14px; border: 1px solid var(--line); border-radius: 8px; background: var(--panel); }
</style>
