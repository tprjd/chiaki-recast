<script lang="ts">
  import { onMount } from 'svelte';
  import type { Snippet } from 'svelte';
  import { moveFocus } from './navigation';
  let { label, close, children }: { label: string; close: () => void; children: Snippet } = $props();
  let dialog: HTMLDialogElement;
  onMount(() => {
    const opener = document.activeElement as HTMLElement;
    dialog.showModal();
    (dialog.querySelector<HTMLElement>('[data-initial-focus]') ?? dialog.querySelector<HTMLElement>('button'))?.focus();
    return () => queueMicrotask(() => {
      const target = opener?.isConnected && opener.tabIndex >= 0 && !opener.matches(':disabled') ? opener
        : document.querySelector<HTMLElement>('.console-card.selected:enabled') ?? document.querySelector<HTMLElement>('[data-add-manual]');
      target?.focus();
    });
  });
</script>

<dialog bind:this={dialog} data-setup-panel aria-label={label} onkeydown={event => moveFocus(event, dialog)} oncancel={event => { event.preventDefault(); close(); }}>
  {@render children()}
</dialog>

<style>
  dialog { width: min(640px, calc(100% - 32px)); max-height: calc(100% - 32px); margin: auto; padding: 24px; color: var(--text); background: var(--panel); border: 1px solid #3b4655; border-radius: 16px; overflow: auto; overflow-wrap: anywhere; }
  dialog::backdrop { background: #080c14c9; }
</style>
