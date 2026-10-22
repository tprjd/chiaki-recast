import { defineConfig, type Plugin } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';
import { existsSync, readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { createRequire } from 'node:module';

const require = createRequire(import.meta.url);
const runtimeNotices: Plugin = {
  name: 'runtime-notices',
  generateBundle(_options, bundle) {
    const notices = new Map<string, string>();
    for (const chunk of Object.values(bundle)) {
      if (chunk.type !== 'chunk') continue;
      for (const [id, module] of Object.entries(chunk.modules)) {
        if (!module.renderedLength) continue;
        const file = id.startsWith('\0vite/') ? require.resolve('vite/package.json') : id.split('?')[0];
        if (!file.replaceAll('\\', '/').includes('/node_modules/')) continue;
        let directory = dirname(file);
        while (!existsSync(join(directory, 'package.json'))) {
          const parent = dirname(directory);
          if (parent === directory) this.error(`No package metadata for bundled code: ${id}`);
          directory = parent;
        }
        const pkg = JSON.parse(readFileSync(join(directory, 'package.json'), 'utf8'));
        if (notices.has(pkg.name)) continue;
        const license = ['LICENSE.md', 'LICENSE', 'LICENSE.txt', 'COPYING'].find(name => existsSync(join(directory, name)));
        if (!license) this.error(`No license text for bundled package: ${pkg.name}`);
        notices.set(pkg.name, `${pkg.name} ${pkg.version}\n${readFileSync(join(directory, license), 'utf8')}`);
      }
    }
    if (!notices.has('svelte')) this.error('The setup bundle must include the Svelte runtime notice.');
    this.emitFile({ type: 'asset', fileName: 'THIRD-PARTY-NOTICES.txt',
      source: 'Third-party code in the compiled setup application\n\n' + [...notices].sort(([a], [b]) => a.localeCompare(b)).map(([, text]) => text).join('\n\n') });
  }
};

export default defineConfig({
  plugins: [svelte(), runtimeNotices],
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
