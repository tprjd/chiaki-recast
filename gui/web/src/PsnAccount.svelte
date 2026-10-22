<script lang="ts">
  import { tick } from 'svelte';
  import type { State, connectSetup } from './bridge';
  let { current, connection, route = false, close }: { current: State; connection: Awaited<ReturnType<typeof connectSetup>>; route?: boolean; close?: () => void } = $props();
  let open = $state(false);
  $effect(() => { if (route) open = true; });
  let redirect = $state('');
  let submitting = $state(false);
  let profile = $state('');
  let terminal = $state('');
  let heading = $state<HTMLHeadingElement>();
  let opener = $state<HTMLButtonElement>();
  const busy = $derived(submitting || current.psn.phase === 'exchanging' || current.psn.phase === 'account');
  const t = (text: string) => current.strings[text] ?? text;
  $effect(() => {
    if (profile !== current.profileGeneration) { redirect = ''; profile = current.profileGeneration; }
    const result = `${current.psn.id}:${current.psn.phase}`;
    if (result !== terminal && (current.psn.phase === 'success' || current.psn.phase === 'failure')) {
      redirect = ''; terminal = result;
    }
  });
  async function toggle() {
    redirect = '';
    if (route) { close?.(); return; }
    open = !open;
    await tick();
    if (open) heading?.focus(); else opener?.focus();
  }
  async function submit(event: SubmitEvent) {
    event.preventDefault();
    if (busy) return;
    const value = redirect.trim();
    redirect = '';
    submitting = true;
    try { await connection.psnSubmit(current.profileGeneration, value); }
    finally { submitting = false; }
  }
  function start(embedded: boolean) {
    redirect = '';
    if (embedded) connection.psnEmbedded(current.profileGeneration);
    else connection.psnExternal(current.profileGeneration);
  }
</script>

<svelte:window onkeydown={event => {
  if (!event.defaultPrevented && open && event.key === 'Escape' && (event.target as Element).closest('.psn-account')) {
    event.preventDefault(); toggle();
  }
}} />
<section class="psn-account" data-setup-panel aria-label={t('PSN account')}>
  {#if !route}<button bind:this={opener} data-psn-account aria-expanded={open} aria-controls="psn-account-panel" onclick={toggle}>{t('PSN account')}</button>{/if}
  {#if open}
    <div class="account-panel" id={route ? "registration-psn-account-panel" : "psn-account-panel"}>
      <h2 bind:this={heading} tabindex="-1">{t('PSN remote access')}</h2>
      <p data-psn-status role="status">{t(current.psn.phase === 'account' ? 'Retrieving your Account ID'
        : current.psn.phase === 'exchanging' ? 'Completing PSN sign-in' : current.psn.status === 'ready' ? 'PSN remote access is ready'
        : current.psn.status === 'expired' ? 'PSN credentials expired' : 'Not signed in')}</p>
      <p>{t('Sign-in saves PSN credentials and your Account ID in the current profile. These credentials allow remote access to your consoles.')}</p>
      {#if busy}<p>{t('You can close this screen. Sign-in can still finish and save credentials in this profile while the app is open.')}</p>{/if}
      {#if current.psn.phase === 'failure'}<p role="alert">{current.psn.message}</p>{/if}
      <div class="actions">
        <button data-psn-embedded disabled={busy} onclick={() => start(true)}>{t('Open embedded sign-in')}</button>
        <button data-psn-external disabled={busy} onclick={() => start(false)}>{t('Use external browser')}</button>
        <button disabled={busy} onclick={() => connection.psnCopyLink(current.profileGeneration)}>{t('Copy sign-in link')}</button>
      </div>
      <p>{t('After sign-in in an external browser, copy the complete final URL from its address bar. Paste the full URL, including its authorization code, below.')}</p>
      <form onsubmit={submit}>
        <label>{t('Sign-in redirect URL')}
          <input data-psn-redirect type="password" autocomplete="off" spellcheck="false" bind:value={redirect} disabled={busy} />
        </label>
        <button data-psn-submit type="submit" disabled={busy || !redirect.trim()}>{t(current.psn.phase === 'failure' ? 'Retry' : 'Complete sign-in')}</button>
      </form>
      <p>{t('Use the same PlayStation account as your console. The Sony sign-in page and your external browser can show account details.')}</p>
      <div><button data-psn-close onclick={toggle}>{t('Close')}</button></div>
    </div>
  {/if}
</section>

<style>
  .psn-account { margin-block: 24px; }
  button { min-height: 44px; }
  .account-panel { display: grid; gap: 16px; padding: 24px; margin-top: 16px; border: 1px solid var(--line); border-radius: 12px; background: var(--panel); }
  .actions { display: flex; flex-wrap: wrap; gap: 10px; }
  form, label { display: grid; gap: 10px; }
  form { justify-items: start; }
  label, input { width: 100%; }
  p { overflow-wrap: anywhere; }
</style>
