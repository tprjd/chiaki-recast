export function moveFocus(event: KeyboardEvent, root: HTMLElement) {
  if (event.defaultPrevented) return;
  const active = document.activeElement as HTMLElement;
  if (event.key === 'Enter' && active.matches('input[type=checkbox]:enabled')) {
    event.preventDefault();
    if (!event.repeat) active.click();
    return;
  }
  const fields = [...root.querySelectorAll<HTMLElement>('button:enabled, input:enabled, select:enabled, textarea:enabled, a[href], summary')]
    .filter(field => field.getClientRects().length);
  if (event.key === 'PageUp' || event.key === 'PageDown') {
    event.preventDefault();
    const next = fields.indexOf(active) + (event.key === 'PageUp' ? -1 : 1);
    (fields[next] ?? fields[event.key === 'PageUp' ? fields.length - 1 : 0])?.focus();
    (document.activeElement as HTMLElement)?.scrollIntoView({ block: 'nearest' });
    return;
  }
  if (!['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(event.key)) return;
  if (active.matches('select, textarea, input:not([type=checkbox])')) return;
  const rect = active.getBoundingClientRect();
  const horizontal = event.key === 'ArrowLeft' || event.key === 'ArrowRight';
  const reverse = event.key === 'ArrowLeft' || event.key === 'ArrowUp';
  const x = rect.x + rect.width / 2, y = rect.y + rect.height / 2;
  const candidates = fields.filter(field => field !== active)
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
