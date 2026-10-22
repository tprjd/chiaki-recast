export interface Setting {
  key: string;
  label: string;
  group: string;
  description: string;
  defaultLabel: string;
  options?: string[];
  requires?: string;
  category?: string;
  optionValues?: (string | number)[];
  min?: number; max?: number; step?: number; factor?: number; unit?: string; automatic?: number;
  requiresValue?: { key: string; value: number };
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

export const mediaSettings: Setting[] = [
  { key: 'decoder', label: 'Hardware decoder', category: 'Video', group: 'Decoding', description: 'Choose a native decoder. Auto chooses a supported decoder. None uses software decoding.', defaultLabel: 'Auto' },
  { key: 'useZeroCopy', label: 'Zero-copy', category: 'Video', group: 'Decoding', description: 'Use the native zero-copy video path when available.', defaultLabel: 'On' },
  { key: 'windowType', label: 'Window type', category: 'Video', group: 'Stream window', description: 'Choose the stream window size and display mode.', defaultLabel: 'Fullscreen', options: ['Stream Resolution', 'Custom Resolution', 'Adjust Resolution Manually', 'Fullscreen', 'Zoom [adjust zoom using slider in stream menu]', 'Stretch'] },
  ...['Width', 'Height'].map((dimension, index) => ({ key: `customResolution${dimension}`, label: `Custom resolution ${dimension.toLowerCase()}`, category: 'Video', group: 'Stream window', description: 'Set the custom stream window size in pixels.', defaultLabel: index ? '1080 pixels' : '1920 pixels', min: 0, max: 9999, unit: 'pixels', requiresValue: { key: 'windowType', value: 1 } })),
  { key: 'fullscreenDoubleClick', label: 'Toggle fullscreen on double-click', category: 'Video', group: 'Stream window', description: 'Double-click the native stream window to toggle fullscreen.', defaultLabel: 'Off' },
  { key: 'hideCursor', label: 'Hide cursor during stream', category: 'Video', group: 'Stream window', description: 'Hide the pointer during native playback.', defaultLabel: 'On' },
  ...['LocalPS4', 'RemotePS4', 'LocalPS5', 'RemotePS5'].flatMap(suffix => {
    const group = `${suffix.slice(-3)} ${suffix.startsWith('Local') ? 'Local' : 'Remote'}`;
    return [
      { key: `resolution${suffix}`, label: `${group} Resolution`, category: 'Stream', group, description: 'Changing resolution restores automatic bitrate. 1080p on PS4 requires PS4 Pro.', defaultLabel: suffix === 'LocalPS5' ? '1080p' : '720p', options: ['360p', '540p', '720p', '1080p'], optionValues: [1, 2, 3, 4] },
      { key: `fps${suffix}`, label: `${group} Frame rate`, category: 'Stream', group, description: 'Choose video frames per second.', defaultLabel: '60 fps', options: ['30 fps', '60 fps'], optionValues: [30, 60] },
      { key: `bitrate${suffix}`, label: `${group} Bitrate`, category: 'Stream', group, description: 'Set video data rate. Automatic bitrate uses: 2, 6, 10 or 15 Mbps for 360p, 540p, 720p or 1080p.', defaultLabel: 'Automatic', min: 2, max: 100, factor: 1000, unit: 'Mbps' },
      ...(suffix.endsWith('PS5') ? [{ key: `codec${suffix}`, label: `${group} Codec`, category: 'Stream', group, description: 'Choose the supported PS5 video codec. HDR needs a compatible native display and decoder.', defaultLabel: 'H265', options: ['H264', 'H265', 'H265 HDR'] }] : [])
    ];
  }),
  ...['Out', 'In'].map(direction => ({ key: `audio${direction}Device`, label: direction === 'Out' ? 'Output device' : 'Input device', category: 'Audio/Wifi', group: 'Audio', description: 'Choose an available native audio device. A missing saved device stays selected until you choose another device.', defaultLabel: 'Auto' })),
  { key: 'audioBufferSize', label: 'Audio buffer size', category: 'Audio/Wifi', group: 'Audio', description: 'Set the native audio buffer in milliseconds.', defaultLabel: '50 ms', min: 10, max: 100, step: 10, factor: 192, unit: 'ms' },
  { key: 'audioVolume', label: 'Audio volume', category: 'Audio/Wifi', group: 'Audio', description: 'Set playback volume. 128 is 100 percent volume.', defaultLabel: '128 (100%)', min: 0, max: 128 },
  { key: 'startMicUnmuted', label: 'Start mic unmuted', category: 'Audio/Wifi', group: 'Microphone', description: 'Start the next session with microphone input enabled.', defaultLabel: 'Unchecked' },
  { key: 'speechProcessing', label: 'Speech processing', category: 'Audio/Wifi', group: 'Microphone', description: 'Enable native noise suppression and echo cancellation on supported builds.', defaultLabel: 'Unchecked' },
  ...['noise', 'echo'].map((kind, index) => ({ key: `${kind}SuppressLevel`, label: `${kind === 'noise' ? 'Noise' : 'Echo'} to suppress`, category: 'Audio/Wifi', group: 'Microphone', description: 'Set microphone suppression. Requires Speech processing.', defaultLabel: index ? '30 dB' : '6 dB', min: 0, max: 60, unit: 'dB', requires: 'speechProcessing' })),
  { key: 'wifiDroppedNotif', label: 'Weak Wifi notification', category: 'Audio/Wifi', group: 'Connection feedback', description: 'Show the warning at this percentage of dropped packets.', defaultLabel: '3%', min: 0, max: 100, unit: '%' },
  { key: 'packetLossReportedMax', label: 'Packet loss reported max', category: 'Audio/Wifi', group: 'Connection feedback', description: 'Set the maximum packet loss reported to the console.', defaultLabel: '5%', min: 0, max: 100, unit: '%' },
  { key: 'iDROnFECFailureEnabled', label: 'Request IDR frame on FEC failure', category: 'Audio/Wifi', group: 'Connection feedback', description: 'Request a complete video frame when forward error correction fails.', defaultLabel: 'Unchecked' },
  { key: 'showStreamStats', label: 'Show stream stats during gameplay', category: 'Audio/Wifi', group: 'Connection feedback', description: 'Show statistics in the native stream window.', defaultLabel: 'Unchecked' }
];

export const keyboardMouseSettings: Setting[] = [
  { key: 'mouseCameraEnabled', label: 'Enable mouse camera mode', group: 'Keyboard and mouse', description: 'During a stream, press F8 to capture or release your controls.', defaultLabel: 'Off' },
  { key: 'mouseHorizontalSensitivity', label: 'Horizontal sensitivity', group: 'Mouse response', description: 'Set the horizontal mouse sensitivity multiplier.', defaultLabel: '1.00 ×', min: 0.05, max: 8, step: 0.05, unit: '×', requires: 'mouseCameraEnabled' },
  { key: 'mouseVerticalSensitivity', label: 'Vertical sensitivity', group: 'Mouse response', description: 'Set the vertical mouse sensitivity multiplier.', defaultLabel: '1.00 ×', min: 0.05, max: 8, step: 0.05, unit: '×', requires: 'mouseCameraEnabled' },
  { key: 'mouseDeadZoneCompensation', label: 'Game dead-zone compensation', group: 'Mouse response', description: 'Compensate for small movements that do not move the camera.', defaultLabel: '0%', min: 0, max: 50, step: 1, factor: 0.01, unit: '%', requires: 'mouseCameraEnabled' },
  { key: 'mouseResponseExponent', label: 'Game response correction', group: 'Mouse response', description: 'Lower values increase small movements to compensate for a game\'s slow stick response.', defaultLabel: '1.00', min: 0.1, max: 3, step: 0.05, requires: 'mouseCameraEnabled' },
  { key: 'mouseInvertVertical', label: 'Invert vertical camera movement', group: 'Mouse response', description: 'Invert the vertical mouse camera direction.', defaultLabel: 'Off', requires: 'mouseCameraEnabled' },
  { key: 'keyboardEnabled', label: 'Keyboard as controller', group: 'Key assignments', description: 'Enable keyboard controller input when mouse camera mode is off.', defaultLabel: 'On' },
  { key: 'mouseTouchEnabled', label: 'Enable Mouse Touchpad', group: 'Key assignments', description: 'Enable mouse touchpad input when mouse camera mode is off.', defaultLabel: 'On' }
].map(field => ({ ...field, category: 'Keyboard and mouse' }));
