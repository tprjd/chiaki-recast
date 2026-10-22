<script lang="ts">
  import Computer from '@lucide/svelte/icons/computer';
  import Gamepad2 from '@lucide/svelte/icons/gamepad-2';
  import Settings from '@lucide/svelte/icons/settings';
  import Search from '@lucide/svelte/icons/search';
  import RefreshCw from '@lucide/svelte/icons/refresh-cw';
  import Plus from '@lucide/svelte/icons/plus';
  import Network from '@lucide/svelte/icons/network';
  import Wifi from '@lucide/svelte/icons/wifi';
  import UserRound from '@lucide/svelte/icons/user-round';
  import ChevronLeft from '@lucide/svelte/icons/chevron-left';
  import Ellipsis from '@lucide/svelte/icons/ellipsis';
  import TvMinimalPlay from '@lucide/svelte/icons/tv-minimal-play';
  import Volume2 from '@lucide/svelte/icons/volume-2';
  import Monitor from '@lucide/svelte/icons/monitor';
  import Keyboard from '@lucide/svelte/icons/keyboard';
  import Check from '@lucide/svelte/icons/check';
  import Palette from '@lucide/svelte/icons/palette';
  import Info from '@lucide/svelte/icons/info';

  let { name, size = 19 }: { name: string; size?: number } = $props();
  const icons: Record<string, typeof Settings> = {
    'console-device': Computer, consoles: Gamepad2, settings: Settings,
    search: Search, refresh: RefreshCw, plus: Plus, network: Network,
    remote: Wifi, profile: UserRound, back: ChevronLeft, more: Ellipsis,
    video: TvMinimalPlay, audio: Volume2, display: Monitor, keyboard: Keyboard,
    check: Check, color: Palette, info: Info
  };
  const Icon = $derived(icons[name] ?? Settings);
</script>
<Icon {size} stroke-width={1.8} aria-hidden="true" focusable="false" />
