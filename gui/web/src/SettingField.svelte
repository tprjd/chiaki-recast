<script lang="ts">
  import type { Setting } from './settings';
  let { field, value, disabled, change, t }: {
    field: Setting; value: boolean | number | string; disabled: boolean;
    change: (value: boolean | number | string) => void; t: (text: string) => string;
  } = $props();
</script>

<div class="setting-row">
  <div class="setting-copy">
    <label for={field.key}>{t(field.label)}</label>
    <p id={`${field.key}-description`}>{t(field.description)}</p>
  </div>
  <div class="setting-control">
    {#if field.options}
      <select id={field.key} {value} {disabled} aria-describedby={`${field.key}-description ${field.key}-default`}
        onchange={event => { const next = field.optionValues ? field.optionValues[event.currentTarget.selectedIndex] : Number(event.currentTarget.value); event.currentTarget.value = String(value); change(next); }}>
        {#each field.options as option, index}<option value={field.optionValues?.[index] ?? index}>{t(option)}</option>{/each}
      </select>
    {:else if field.min !== undefined}
      <input id={field.key} type="number" value={Number(value) / (field.factor ?? 1)} min={field.min} max={field.max} step={field.step ?? 1} {disabled}
        aria-describedby={`${field.key}-description ${field.key}-default`}
        onchange={event => { const next = event.currentTarget.valueAsNumber * (field.factor ?? 1); event.currentTarget.value = String(Number(value) / (field.factor ?? 1)); change(next); }} />
      {#if field.unit}<span>{t(field.unit)}</span>{/if}
    {:else}
      <input id={field.key} type="checkbox" checked={value === true} {disabled}
        aria-describedby={`${field.key}-description ${field.key}-default`}
        onchange={event => { const next = event.currentTarget.checked; event.currentTarget.checked = value === true; change(next); }} />
    {/if}
    <small id={`${field.key}-default`}>{t('Default')}: {t(field.defaultLabel)}</small>
  </div>
</div>
