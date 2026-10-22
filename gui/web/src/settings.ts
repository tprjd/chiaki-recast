export interface Setting {
  key: string;
  label: string;
  group: string;
  description: string;
  defaultLabel: string;
  options?: string[];
  requires?: string;
}

const buttons = ['Not Used', 'Cross', 'Moon', 'Box', 'Pyramid', 'Dpad Left', 'Dpad Right', 'Dpad Up', 'Dpad Down',
  'L1', 'R1', 'L3', 'R3', 'Options', 'Share', 'Touchpad', 'PS'];

// Search uses this public metadata only. Never add saved values or private identifiers.
export const generalSettings: Setting[] = [
  { key: 'disconnectAction', label: 'Action on disconnect', group: 'Session behavior', description: 'Choose what the console does when the session ends.',
    defaultLabel: 'Ask', options: ['Do Nothing', 'Enter Sleep Mode', 'Ask'] },
  { key: 'suspendAction', label: 'Action on suspend', group: 'Session behavior', description: 'Choose what the console does when the computer sleeps.',
    defaultLabel: 'Do Nothing', options: ['Do Nothing', 'Enter Sleep Mode'] },
  { key: 'audioVideoDisabled', label: 'Audio/Video', group: 'Session behavior', description: 'Enable or disable audio and video for the next session.',
    defaultLabel: 'Audio and Video Enabled', options: ['Audio and Video Enabled', 'Audio Disabled', 'Video Disabled', 'Audio and Video Disabled'] },
  { key: 'steamDeckHaptics', label: 'True haptics for Steam Deck, better quality but noisier', group: 'Steam Deck',
    description: 'Enable feedback on supported Steam Deck devices.', defaultLabel: 'Unchecked' },
  { key: 'verticalDeck', label: 'Use Steam Deck in vertical orientation for motion controls', group: 'Steam Deck',
    description: 'Use the vertical orientation on supported devices.', defaultLabel: 'Unchecked' },
  { key: 'streamerMode', label: 'Streamer mode, hide private information', group: 'Privacy',
    description: 'Mask private console and profile information.', defaultLabel: 'Unchecked' },
  { key: 'streamMenuEnabled', label: 'Enable stream menu shortcut', group: 'Stream menu shortcut',
    description: 'Enable the controller shortcut for the stream menu.', defaultLabel: 'Enabled' },
  ...[1, 2, 3, 4].map((number, index) => ({ key: `streamMenuShortcut${number}`, label: `Stream menu shortcut button ${number}`,
    group: 'Stream menu shortcut', description: 'Choose a shortcut button. Enable the shortcut first.',
    defaultLabel: ['L1', 'R1', 'L3', 'R3'][index], options: buttons, requires: 'streamMenuEnabled' }))
];
