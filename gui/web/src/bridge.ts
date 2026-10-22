export interface Console {
  key: string; name: string; available: boolean; registered: boolean;
  state: string; route: 'local' | 'manual' | 'psn'; ps5?: boolean;
}
export interface State {
  revision: string; profileGeneration: string; consoles: Console[];
  selectedKey: string; discoveryEnabled: boolean; streamerMode: boolean;
  connectionBusy: boolean;
  streamError?: { id: string; title: string; text: string };
  mediaSettings: Record<string, boolean | number | string>;
  mediaChoices: Record<string, { label: string; value: string }[]>;
  generalSettings: Record<string, boolean | number>;
  capabilities: { discovery: boolean }; strings: Record<string, string>;
  psn: { status: 'signedOut' | 'ready' | 'expired'; id?: string; phase?: string; message?: string; profileValid?: boolean };
}
interface Reply { accepted: boolean; message?: string }
interface NativeBridge {
  snapshot(callback: (state: State) => void): void;
  setDiscoveryEnabled(value: boolean, callback: (reply: Reply) => void): void;
  selectConsole(key: string, callback: (reply: Reply) => void): void;
  connectConsole(key: string, callback: (reply: Reply) => void): void;
  dismissStreamError(id: string, callback: (reply: Reply) => void): void;
  openPsnSignIn(profile: string, callback: (reply: Reply) => void): void;
  openPsnExternal(profile: string, callback: (reply: Reply) => void): void;
  copyPsnSignInLink(profile: string, callback: (reply: Reply) => void): void;
  submitPsnRedirect(profile: string, url: string, callback: (reply: Reply) => void): void;
  setGeneralSetting(name: string, value: boolean | number, profileGeneration: string, callback: (reply: Reply) => void): void;
  setMediaSetting(name: string, value: boolean | number | string, profileGeneration: string, callback: (reply: Reply) => void): void;
  refreshAudioDevices(callback: (reply: Reply) => void): void;
  stateChanged: { connect(callback: (state: State) => void): void; disconnect(callback: (state: State) => void): void };
}
declare global {
  interface Window {
    qt?: { webChannelTransport: unknown };
    QWebChannel: new (transport: unknown, callback: (channel: { objects: { setup: NativeBridge } }) => void) => unknown;
  }
}

export function connectSetup(receive: (state: State) => void): Promise<{
  discovery(enabled: boolean): Promise<Reply>;
  select(key: string): Promise<Reply>;
  connect(key: string): Promise<Reply>;
  dismissStreamError(id: string): Promise<Reply>;
  psnEmbedded(profile: string): Promise<Reply>;
  psnExternal(profile: string): Promise<Reply>;
  psnCopyLink(profile: string): Promise<Reply>;
  psnSubmit(profile: string, url: string): Promise<Reply>;
  generalSetting(name: string, value: boolean | number, profileGeneration: string): Promise<Reply>;
  mediaSetting(name: string, value: boolean | number | string, profileGeneration: string): Promise<Reply>;
  refreshAudioDevices(): Promise<Reply>;
  dispose(): void;
}> {
  return new Promise((resolve, reject) => {
    if (!window.qt?.webChannelTransport || !window.QWebChannel) {
      reject(new Error('Setup unavailable'));
      return;
    }
    new window.QWebChannel(window.qt.webChannelTransport, ({ objects: { setup } }) => {
      let revision = -1n;
      const update = (state: State) => {
        if (BigInt(state.revision) <= revision) return;
        revision = BigInt(state.revision);
        receive(state);
      };
      let savedFocus: HTMLElement | null = null;
      const refresh = () => setup.snapshot(update);
      const resume = () => {
        if (document.hidden) { savedFocus = document.activeElement as HTMLElement; return; }
        setup.snapshot(state => {
          update(state);
          requestAnimationFrame(() => {
            const target = savedFocus?.isConnected && savedFocus.tabIndex >= 0 && !savedFocus.matches(':disabled') ? savedFocus
              : document.querySelector<HTMLElement>(`.console-card[data-console-key="${CSS.escape(state.selectedKey)}"]:enabled`)
                ?? document.querySelector<HTMLElement>('.discovery:enabled');
            target?.focus();
          });
        });
      };
      setup.stateChanged.connect(update);
      refresh();
      window.addEventListener('pageshow', refresh);
      document.addEventListener('visibilitychange', resume);
      resolve({
        discovery: value => new Promise(done => setup.setDiscoveryEnabled(value, done)),
        select: key => new Promise(done => setup.selectConsole(key, done)),
        connect: key => new Promise(done => setup.connectConsole(key, done)),
        dismissStreamError: id => new Promise(done => setup.dismissStreamError(id, done)),
        psnEmbedded: profile => new Promise(done => setup.openPsnSignIn(profile, done)),
        psnExternal: profile => new Promise(done => setup.openPsnExternal(profile, done)),
        psnCopyLink: profile => new Promise(done => setup.copyPsnSignInLink(profile, done)),
        psnSubmit: (profile, url) => new Promise(done => setup.submitPsnRedirect(profile, url, done)),
        generalSetting: (name, value, generation) => new Promise(done => setup.setGeneralSetting(name, value, generation, done)),
        mediaSetting: (name, value, generation) => new Promise(done => setup.setMediaSetting(name, value, generation, done)),
        refreshAudioDevices: () => new Promise(done => setup.refreshAudioDevices(done)),
        dispose() {
          setup.stateChanged.disconnect(update);
          window.removeEventListener('pageshow', refresh);
          document.removeEventListener('visibilitychange', resume);
        }
      });
    });
  });
}
