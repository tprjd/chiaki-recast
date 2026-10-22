import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';

export default defineConfig({
  plugins: [svelte()],
  base: './',
  build: {
    target: 'chrome130',
    rollupOptions: { output: {
      entryFileNames: 'app.js',
      assetFileNames: 'app.[ext]',
      codeSplitting: false
    } }
  }
});
