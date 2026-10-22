export interface Console {
  key: string; name: string; available: boolean; registered: boolean;
  state: string; route: 'local' | 'manual' | 'psn'; ps5?: boolean;
}
export interface State {
  revision: string; profileGeneration: string; consoles: Console[];
  selectedKey: string; discoveryEnabled: boolean; streamerMode: boolean;
  generalSettings: Record<string, boolean | number>;
  capabilities: { discovery: boolean }; strings: Record<string, string>;
}
interface Reply { accepted: boolean; message?: string }
interface NativeBridge {
  snapshot(callback: (state: State) => void): void;
  setDiscoveryEnabled(value: boolean, callback: (reply: Reply) => void): void;
  selectConsole(key: string, callback: (reply: Reply) => void): void;
  setGeneralSetting(name: string, value: boolean | number, profileGeneration: string, callback: (reply: Reply) => void): void;
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
  generalSetting(name: string, value: boolean | number, profileGeneration: string): Promise<Reply>;
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
        generalSetting: (name, value, generation) => new Promise(done => setup.setGeneralSetting(name, value, generation, done)),
        dispose() {
          setup.stateChanged.disconnect(update);
          window.removeEventListener('pageshow', refresh);
          document.removeEventListener('visibilitychange', resume);
        }
      });
    });
  });
}
