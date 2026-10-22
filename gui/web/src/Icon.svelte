<script lang="ts">
  let { name, size = 19 }: { name: string; size?: number } = $props();
  const paths: Record<string, string> = {
    'console-device': 'M7 2h9l2 20H5L7 2Z M10 4v16 M13 18h.01 M13 6h2',
    consoles: 'M6 8h12l3 10-3 1-4-4h-4l-4 4-3-1 3-10Z M7 10v5 M4.5 12.5h5 M16 11h.01 M18 14h.01',
    settings: 'M12 8a4 4 0 1 0 0 8 4 4 0 0 0 0-8Z M9 3l-.5 3-3 1L3 6l-2 4 2.5 2v2L1 16l2 4 2.5-1 3 1 .5 3h5l.5-3 3-1 2.5 1 2-4-2.5-2v-2l2.5-2-2-4-2.5 1-3-1-.5-3Z',
    search: 'M10.5 3a7.5 7.5 0 1 0 0 15 7.5 7.5 0 0 0 0-15Z M16 16l5 5',
    refresh: 'M20 7a9 9 0 1 0 1 9 M20 2v6h-6',
    plus: 'M12 5v14 M5 12h14',
    network: 'M12 3v6 M4 20v-5h16v5 M12 15v5 M8 9h8v6H8Z',
    remote: 'M3 10a14 14 0 0 1 18 0 M6 14a9 9 0 0 1 12 0 M9 18a4 4 0 0 1 6 0 M12 21h.01',
    profile: 'M12 3a4 4 0 1 0 0 8 4 4 0 0 0 0-8Z M4 21v-2a8 8 0 0 1 16 0v2',
    back: 'M15 5l-7 7 7 7',
    more: 'M5 12h.01 M12 12h.01 M19 12h.01',
    video: 'M3 5h18v14H3Z M9 9l6 3-6 3Z',
    audio: 'M11 4L5 9H2v6h3l6 5Z M15 8a6 6 0 0 1 0 8 M18 4a11 11 0 0 1 0 16',
    display: 'M2 3h20v14H2Z M12 17v5 M7 22h10',
    keyboard: 'M2 6h20v13H2Z M5 10h.01 M9 10h.01 M13 10h.01 M17 10h.01 M5 14h.01 M9 14h8'
  };
</script>
<svg width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d={paths[name] ?? paths.settings} /></svg>
