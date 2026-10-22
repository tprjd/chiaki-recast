export interface Console {
  key: string; name: string; available: boolean; registered: boolean;
  state: string; route: 'local' | 'manual' | 'psn'; ps5?: boolean;
}
export interface State {
  revision: string; profileGeneration: string; consoles: Console[];
  selectedKey: string; discoveryEnabled: boolean; streamerMode: boolean;
  capabilities: { discovery: boolean }; strings: Record<string, string>;
  psn: { status: 'signedOut' | 'ready' | 'expired'; id?: string; phase?: string; message?: string; profileValid?: boolean };
}
interface Reply { accepted: boolean; message?: string }
interface NativeBridge {
  snapshot(callback: (state: State) => void): void;
  setDiscoveryEnabled(value: boolean, callback: (reply: Reply) => void): void;
  selectConsole(key: string, callback: (reply: Reply) => void): void;
  openPsnSignIn(profile: string, callback: (reply: Reply) => void): void;
  openPsnExternal(profile: string, callback: (reply: Reply) => void): void;
  copyPsnSignInLink(profile: string, callback: (reply: Reply) => void): void;
  submitPsnRedirect(profile: string, url: string, callback: (reply: Reply) => void): void;
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
  psnEmbedded(profile: string): Promise<Reply>;
  psnExternal(profile: string): Promise<Reply>;
  psnCopyLink(profile: string): Promise<Reply>;
  psnSubmit(profile: string, url: string): Promise<Reply>;
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
      const refresh = () => setup.snapshot(update);
      const resume = () => { if (!document.hidden) refresh(); };
      setup.stateChanged.connect(update);
      refresh();
      window.addEventListener('pageshow', refresh);
      document.addEventListener('visibilitychange', resume);
      resolve({
        discovery: value => new Promise(done => setup.setDiscoveryEnabled(value, done)),
        select: key => new Promise(done => setup.selectConsole(key, done)),
        psnEmbedded: profile => new Promise(done => setup.openPsnSignIn(profile, done)),
        psnExternal: profile => new Promise(done => setup.openPsnExternal(profile, done)),
        psnCopyLink: profile => new Promise(done => setup.copyPsnSignInLink(profile, done)),
        psnSubmit: (profile, url) => new Promise(done => setup.submitPsnRedirect(profile, url, done)),
        dispose() {
          setup.stateChanged.disconnect(update);
          window.removeEventListener('pageshow', refresh);
          document.removeEventListener('visibilitychange', resume);
        }
      });
    });
  });
}
