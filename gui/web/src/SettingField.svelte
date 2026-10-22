<script lang="ts">
  import type { Setting } from './settings';
  let { field, value, disabled, generation, change, t }: {
    field: Setting; value: boolean | number | string; disabled: boolean; generation: string;
    change: (value: boolean | number | string, generation?: string) => void; t: (text: string) => string;
  } = $props();
  let editGeneration: string | undefined;
  const displayedNumber = $derived(Number((Number(value) / (field.factor ?? 1)).toFixed((String(field.step ?? 1).split('.')[1] ?? '').length)) || field.automatic || 0);
</script>

<div class="setting-row">
  <div class="setting-copy">
    <label for={field.key}>{t(field.label)}</label>
    <p id={`${field.key}-description`}>{t(field.description)}</p>
  </div>
  <div class="setting-control">
    {#if field.action}
      <button id={field.key} {disabled} onclick={() => change(true)}>{t(field.label)}</button>
    {:else if field.options}
      <select id={field.key} {value} {disabled} aria-describedby={`${field.key}-description ${field.key}-default`}
        onchange={event => { const next = field.optionValues ? field.optionValues[event.currentTarget.selectedIndex] : Number(event.currentTarget.value); event.currentTarget.value = String(value); change(next); }}>
        {#each field.options as option, index}<option value={field.optionValues?.[index] ?? index}>{t(option)}</option>{/each}
      </select>
    {:else if field.min !== undefined}
      <input id={field.key} type="number" value={displayedNumber} min={field.min} max={field.max} step={field.step ?? 1} {disabled}
        aria-describedby={`${field.key}-description ${field.key}-default`}
        oninput={() => { editGeneration ??= generation; }}
        onchange={event => { const next = event.currentTarget.valueAsNumber * (field.factor ?? 1); event.currentTarget.value = String(displayedNumber); change(next, editGeneration ?? generation); editGeneration = undefined; }} />
      {#if field.unit}<span>{t(field.unit)}</span>{/if}
      {#if field.automatic !== undefined}<button type="button" disabled={disabled || value === 0} onclick={() => change(0)}>{t('Use automatic bitrate')}</button><small>{value === 0 ? `${t('Automatic')}: ${field.automatic} Mbps` : ''}</small>{/if}
      {#if field.key === 'audioVolume'}<output for={field.key}>{Math.round(Number(value) / 128 * 100)}{t('% volume')}</output>{/if}
    {:else}
      <input id={field.key} type="checkbox" checked={value === true} {disabled}
        aria-describedby={`${field.key}-description ${field.key}-default`}
        onchange={event => { const next = event.currentTarget.checked; event.currentTarget.checked = value === true; change(next); }} />
    {/if}
    {#if field.defaultLabel}<small id={`${field.key}-default`}>{t('Default')}: {t(field.defaultLabel)}</small>{/if}
  </div>
</div>
