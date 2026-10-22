export function moveFocus(event: KeyboardEvent, root: HTMLElement) {
  if (!['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(event.key)) return;
  const active = document.activeElement as HTMLElement;
  if (active.matches('select, textarea, input:not([type=checkbox])')) return;
  const rect = active.getBoundingClientRect();
  const horizontal = event.key === 'ArrowLeft' || event.key === 'ArrowRight';
  const reverse = event.key === 'ArrowLeft' || event.key === 'ArrowUp';
  const x = rect.x + rect.width / 2, y = rect.y + rect.height / 2;
  const candidates = [...root.querySelectorAll<HTMLElement>('button:enabled, input:enabled, select:enabled, a[href]')]
    .filter(field => field !== active && field.getClientRects().length)
    .map(field => {
      const next = field.getBoundingClientRect();
      const dx = next.x + next.width / 2 - x, dy = next.y + next.height / 2 - y;
      return { field, forward: (horizontal ? dx : dy) * (reverse ? -1 : 1), side: Math.abs(horizontal ? dy : dx) };
    }).filter(item => item.forward > 1)
    .sort((a, b) => (a.forward + a.side * 3) - (b.forward + b.side * 3));
  if (candidates[0]) {
    event.preventDefault();
    candidates[0].field.focus();
    candidates[0].field.scrollIntoView({ block: 'nearest' });
  }
}
