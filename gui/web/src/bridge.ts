export interface Console {
  key: string; name: string; available: boolean; registered: boolean;
  address: string; identifier: string; app: string; titleId: string; actions: string[];
  state: string; route: 'local' | 'manual' | 'psn'; ps5?: boolean;
}
export interface State {
  revision: string; profileGeneration: string; consoles: Console[];
  registrations: { key: string; name: string; ps5: boolean; identifier: string }[];
  hiddenConsoles: { key: string; name: string; identifier: string }[];
  selectedKey: string; discoveryEnabled: boolean; streamerMode: boolean;
  connectionBusy: boolean;
  connection: { id?: string; phase?: string; cancelable?: boolean; cleanupPending?: boolean; targetValid?: boolean; errorTitle?: string; errorText?: string };
  streamError?: { id: string; title: string; text: string };
  mediaSettings: Record<string, boolean | number | string>;
  mediaChoices: Record<string, { label: string; value: string }[]>;
  generalSettings: Record<string, boolean | number>;
  controllerSetup: {
    values: Record<string, boolean | number>; devices: { name: string; type: string; playStation: boolean; steamVirtual: boolean }[];
    operation: string; phase: string; message: string; type: string; altered: boolean; analog: boolean; button: number; slot: number;
    mappings: { buttonName: string; buttonValue: number; physicalButton: string[] }[];
    targets: { buttonName: string; buttonValue: number }[];
  };
  keyboardMouse: {
    supported: boolean; windowGeneration: string; values: Record<string, boolean | number>; limits: Record<string, number>;
    bindings: { action: number; name: string; group: number; primaryName: string; alternativeName: string }[];
    keys: { buttonName: string; buttonValue: number; keyName: string }[];
  };
  rendererSettings: Record<string, boolean | number>;
  rendererStatus: { restartRequired: boolean; runtimeBackend?: number };
  capabilities: { discovery: boolean }; strings: Record<string, string>;
  psn: { status: 'signedOut' | 'ready' | 'expired'; id?: string; phase?: string; message?: string; profileValid?: boolean;
    refresh: { phase?: string; message?: string } };
}
interface Reply { accepted: boolean; message?: string; form?: string }
interface NativeBridge {
  setControllerSetting(name: string, value: boolean | number | string, profile: string, callback: (reply: Reply) => void): void;
  beginControllerMapping(reset: boolean, profile: string, callback: (reply: Reply) => void): void;
  controllerMappingAction(action: string, operation: string, profile: string, callback: (reply: Reply) => void): void;
  captureControllerMapping(button: number, slot: number, operation: string, profile: string, callback: (reply: Reply) => void): void;
  setRendererSetting(name: string, value: boolean | number | string, profileGeneration: string, callback: (reply: Reply) => void): void;
  rendererAction(action: string, profileGeneration: string, callback: (reply: Reply) => void): void;
  snapshot(callback: (state: State) => void): void;
  setDiscoveryEnabled(value: boolean, callback: (reply: Reply) => void): void;
  selectConsole(key: string, callback: (reply: Reply) => void): void;
  connectConsole(key: string, callback: (reply: Reply) => void): void;
  refreshPsnConsoles(profile: string, callback: (reply: Reply) => void): void;
  cancelConnection(attempt: string, callback: (reply: Reply) => void): void;
  retryConnection(attempt: string, callback: (reply: Reply) => void): void;
  dismissConnection(attempt: string, callback: (reply: Reply) => void): void;
  saveManualAddress(address: string, registrationKey: string, profile: string, callback: (reply: Reply) => void): void;
  manageConsole(action: string, key: string, pin: string, callback: (reply: Reply) => void): void;
  dismissStreamError(id: string, callback: (reply: Reply) => void): void;
  openPsnSignIn(profile: string, callback: (reply: Reply) => void): void;
  openPsnExternal(profile: string, callback: (reply: Reply) => void): void;
  copyPsnSignInLink(profile: string, callback: (reply: Reply) => void): void;
  submitPsnRedirect(profile: string, url: string, callback: (reply: Reply) => void): void;
  setGeneralSetting(name: string, value: boolean | number, profileGeneration: string, callback: (reply: Reply) => void): void;
  setMediaSetting(name: string, value: boolean | number | string, profileGeneration: string, callback: (reply: Reply) => void): void;
  refreshAudioDevices(callback: (reply: Reply) => void): void;
  beginKeyboardMouse(profile: string, callback: (reply: Reply) => void): void;
  endKeyboardMouse(form: string, callback: (reply: Reply) => void): void;
  setKeyboardMouseSetting(name: string, value: boolean | number, form: string, profile: string, callback: (reply: Reply) => void): void;
  captureKeyboardMouse(action: number, slot: number, legacy: boolean, form: string, profile: string, callback: (reply: Reply) => void): void;
  resetKeyboardMouse(form: string, profile: string, callback: (reply: Reply) => void): void;
  resetKeyboardKeys(form: string, profile: string, callback: (reply: Reply) => void): void;
  stateChanged: { connect(callback: (state: State) => void): void; disconnect(callback: (state: State) => void): void };
}
declare global {
  interface Window {
    qt?: { webChannelTransport: unknown };
    QWebChannel: new (transport: unknown, callback: (channel: { objects: { setup: NativeBridge } }) => void) => unknown;
  }
}

export function connectSetup(receive: (state: State) => void): Promise<{
  controllerSetting(name: string, value: boolean | number | string, profile: string): Promise<Reply>;
  beginControllerMapping(reset: boolean, profile: string): Promise<Reply>;
  controllerMappingAction(action: string, operation: string, profile: string): Promise<Reply>;
  captureControllerMapping(button: number, slot: number, operation: string, profile: string): Promise<Reply>;
  discovery(enabled: boolean): Promise<Reply>;
  select(key: string): Promise<Reply>;
  connect(key: string): Promise<Reply>;
  refreshPsnConsoles(profile: string): Promise<Reply>;
  cancelConnection(attempt: string): Promise<Reply>;
  retryConnection(attempt: string): Promise<Reply>;
  dismissConnection(attempt: string): Promise<Reply>;
  saveManual(address: string, registrationKey: string, profile: string): Promise<Reply>;
  manage(action: string, key: string, pin?: string): Promise<Reply>;
  dismissStreamError(id: string): Promise<Reply>;
  psnEmbedded(profile: string): Promise<Reply>;
  psnExternal(profile: string): Promise<Reply>;
  psnCopyLink(profile: string): Promise<Reply>;
  psnSubmit(profile: string, url: string): Promise<Reply>;
  generalSetting(name: string, value: boolean | number, profileGeneration: string): Promise<Reply>;
  mediaSetting(name: string, value: boolean | number | string, profileGeneration: string): Promise<Reply>;
  refreshAudioDevices(): Promise<Reply>;
  beginKeyboardMouse(profile: string): Promise<Reply>;
  endKeyboardMouse(form: string): Promise<Reply>;
  keyboardMouseSetting(name: string, value: boolean | number, form: string, profile: string): Promise<Reply>;
  captureKeyboardMouse(action: number, slot: number, legacy: boolean, form: string, profile: string): Promise<Reply>;
  resetKeyboardMouse(form: string, profile: string): Promise<Reply>;
  resetKeyboardKeys(form: string, profile: string): Promise<Reply>;
  rendererSetting(name: string, value: boolean | number | string, profileGeneration: string): Promise<Reply>;
  rendererAction(action: string, profileGeneration: string): Promise<Reply>;
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
        refreshPsnConsoles: profile => new Promise(done => setup.refreshPsnConsoles(profile, done)),
        cancelConnection: attempt => new Promise(done => setup.cancelConnection(attempt, done)),
        retryConnection: attempt => new Promise(done => setup.retryConnection(attempt, done)),
        dismissConnection: attempt => new Promise(done => setup.dismissConnection(attempt, done)),
        saveManual: (address, registrationKey, profile) => new Promise(done => setup.saveManualAddress(address, registrationKey, profile, done)),
        manage: (action, key, pin = '') => new Promise(done => setup.manageConsole(action, key, pin, done)),
        dismissStreamError: id => new Promise(done => setup.dismissStreamError(id, done)),
        psnEmbedded: profile => new Promise(done => setup.openPsnSignIn(profile, done)),
        psnExternal: profile => new Promise(done => setup.openPsnExternal(profile, done)),
        psnCopyLink: profile => new Promise(done => setup.copyPsnSignInLink(profile, done)),
        psnSubmit: (profile, url) => new Promise(done => setup.submitPsnRedirect(profile, url, done)),
        controllerSetting: (name, value, profile) => new Promise(done => setup.setControllerSetting(name, value, profile, done)),
        beginControllerMapping: (reset, profile) => new Promise(done => setup.beginControllerMapping(reset, profile, done)),
        controllerMappingAction: (action, operation, profile) => new Promise(done => setup.controllerMappingAction(action, operation, profile, done)),
        captureControllerMapping: (button, slot, operation, profile) => new Promise(done => setup.captureControllerMapping(button, slot, operation, profile, done)),
        generalSetting: (name, value, generation) => new Promise(done => setup.setGeneralSetting(name, value, generation, done)),
        mediaSetting: (name, value, generation) => new Promise(done => setup.setMediaSetting(name, value, generation, done)),
        refreshAudioDevices: () => new Promise(done => setup.refreshAudioDevices(done)),
        beginKeyboardMouse: profile => new Promise(done => setup.beginKeyboardMouse(profile, done)),
        endKeyboardMouse: form => new Promise(done => setup.endKeyboardMouse(form, done)),
        keyboardMouseSetting: (name, value, form, profile) => new Promise(done => setup.setKeyboardMouseSetting(name, value, form, profile, done)),
        captureKeyboardMouse: (action, slot, legacy, form, profile) => new Promise(done => setup.captureKeyboardMouse(action, slot, legacy, form, profile, done)),
        resetKeyboardMouse: (form, profile) => new Promise(done => setup.resetKeyboardMouse(form, profile, done)),
        resetKeyboardKeys: (form, profile) => new Promise(done => setup.resetKeyboardKeys(form, profile, done)),
        rendererSetting: (name, value, generation) => new Promise(done => setup.setRendererSetting(name, value, generation, done)),
        rendererAction: (action, generation) => new Promise(done => setup.rendererAction(action, generation, done)),
        dispose() {
          setup.stateChanged.disconnect(update);
          window.removeEventListener('pageshow', refresh);
          document.removeEventListener('visibilitychange', resume);
        }
      });
    });
  });
}
