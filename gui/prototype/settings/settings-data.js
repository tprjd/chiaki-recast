// Browser-only inventory of SettingsDialog.qml. Metadata names the native property.
const comboButtons = ["Not Used","Cross","Moon","Box","Pyramid","Dpad Left","Dpad Right","Dpad Up","Dpad Down","L1","R1","L3","R3","Options","Share","Touchpad","PS"];
const keyMappings = [
  ["Cross","Return"],["Moon","Backspace"],["Box","Backslash"],["Pyramid","C"],
  ["D-Pad Left","Left"],["D-Pad Right","Right"],["D-Pad Up","Up"],["D-Pad Down","Down"],
  ["L1","2"],["R1","3"],["L3","5"],["R3","6"],["Options","O"],["Share","F"],
  ["Touchpad","T"],["PS","Escape"],["L2","1"],["R2","4"],
  ["Left Stick Right","]"],["Left Stick Left","["],["Left Stick Up","Insert"],["Left Stick Down","Delete"],
  ["Right Stick Right","="],["Right Stick Left","-"],["Right Stick Up","PageUp"],["Right Stick Down","PageDown"]
];
// [label, control type or options, sample/default value, help, metadata]
const sections = {
  General: [
    ["Session", [
      ["Action on disconnect", ["Do Nothing","Enter Sleep Mode","Ask"], "Ask", "", {ref:"disconnectAction"}],
      ["Action on suspend", ["Do Nothing","Enter Sleep Mode"], "Do Nothing", "", {ref:"suspendAction"}],
      ["Audio/video", ["Audio and Video Enabled","Audio Disabled","Video Disabled","Audio and Video Disabled"], "Audio and Video Enabled", "", {ref:"audioVideoDisabled"}]
    ]],
    ["Steam Deck", [
      ["Steam Deck haptics", "toggle", false, "True haptics. Better quality, but noisier.", {ref:"steamDeckHaptics", condition:"Steam Deck builds only"}],
      ["Steam Deck vertical", "toggle", false, "Use vertical orientation for motion controls.", {ref:"verticalDeck", condition:"Steam Deck builds only"}]
    ]],
    ["Privacy and logs", [
      ["Streamer mode", "toggle", false, "Hide private console information.", {ref:"streamerMode"}],
      ["Log directory", "action", "Open folder", "Sample path: C:/Users/Demo/AppData/Local/chiaki/logs", {ref:"logDirectory"}]
    ]],
    ["Stream menu", [
      ["Stream menu shortcut", "toggle", true, "", {ref:"streamMenuEnabled"}],
      ["Stream menu combination", "combo", ["L1","R1","L3","R3"], "Choose up to four buttons.", {refs:["streamMenuShortcut1","streamMenuShortcut2","streamMenuShortcut3","streamMenuShortcut4"], condition:"When the stream menu shortcut is enabled"}]
    ]]
  ],
  Video: [
    ["Decoding", [
      ["Hardware decoder", ["Auto","D3D11VA","DXVA2","Vulkan","VAAPI","VDPAU","VideoToolbox","CUDA","Software"], "Auto", "Available decoders depend on the build and device. These are sample choices.", {ref:"decoder", sample:true}],
      ["Zero-copy", "toggle", true, "Avoid copying decoded frames when supported.", {ref:"useZeroCopy"}]
    ]],
    ["Window", [
      ["Window type", ["Stream Resolution","Custom Resolution","Adjust Resolution Manually","Fullscreen","Zoom","Stretch"], "Fullscreen", "Adjust Zoom with the slider in the stream menu.", {ref:"windowType"}],
      ["Custom resolution width", "number", 1920, "Pixels", {ref:"customResolutionWidth", min:0,max:9999,step:1,condition:"When Window type is Custom Resolution"}],
      ["Custom resolution height", "number", 1080, "Pixels", {ref:"customResolutionHeight", min:0,max:9999,step:1,condition:"When Window type is Custom Resolution"}],
      ["Fullscreen on double-click", "toggle", false, "", {ref:"fullscreenDoubleClick"}],
      ["Hide cursor during stream", "toggle", true, "", {ref:"hideCursor"}]
    ]],
    ["Rendering", [
      ["Vertical sync", "toggle", true, "The native app restarts for this change when using OpenGL.", {ref:"vSyncEnabled"}],
      ["Render preset", ["Fast","Default","High Quality","High Quality + Spatial Upscaling","High Quality + Advanced Spatial Upscaling","Custom"], "High Quality", "", {ref:"videoPreset"}],
      ["Placebo queue depth limit", "number", 2, "Frames", {ref:"queueDepthLimit",min:2,max:5,step:1}],
      ["Renderer backend", ["Vulkan","OpenGL"], "Vulkan", "Changing the backend restarts the native app.", {ref:"rendererBackend"}],
      ["Custom renderer settings", "action", "Open", "", {page:"Renderer",condition:"When Render preset is Custom"}],
      ["Display settings", "action", "Open", "", {page:"Display"}]
    ]]
  ],
  "Audio/Wifi": [
    ["Audio", [
      ["Output device", ["Auto","Speakers","Headphones"], "Auto", "Sample devices", {ref:"audioOutDevice",sample:true}],
      ["Input device", ["Auto","Microphone"], "Auto", "Sample devices", {ref:"audioInDevice",sample:true}],
      ["Audio buffer size", "number", 50, "Milliseconds", {ref:"audioBufferSize",min:10,max:100,step:10}],
      ["Audio volume", "number", 100, "Percent", {ref:"audioVolume",min:0,max:100,step:0.78125}]
    ]],
    ["Microphone", [
      ["Start microphone unmuted", "toggle", false, "", {ref:"startMicUnmuted"}],
      ["Speech processing", "toggle", false, "Noise suppression and echo cancellation.", {ref:"speechProcessing",condition:"Builds with speech processing support"}],
      ["Noise to suppress", "number", 6, "dB", {ref:"noiseSuppressLevel",min:0,max:60,step:1,condition:"When speech processing is enabled"}],
      ["Echo to suppress", "number", 30, "dB", {ref:"echoSuppressLevel",min:0,max:60,step:1,condition:"When speech processing is enabled"}]
    ]],
    ["Connection feedback", [
      ["Weak Wi-Fi notification", "number", 3, "Dropped packets, percent", {ref:"wifiDroppedNotif",min:0,max:100,step:1}],
      ["Packet loss reported max", "number", 5, "Packet loss, percent", {ref:"packetLossReportedMax",min:0,max:100,step:1}],
      ["Request IDR frame on FEC failure", "toggle", false, "", {ref:"iDROnFECFailureEnabled"}],
      ["Show stream statistics", "toggle", false, "Display statistics during gameplay.", {ref:"showStreamStats"}]
    ]]
  ],
  Consoles: [
    ["Registration", [["Register a console","action","Register new","Opens the existing registration flow.",{ref:"registeredHosts"}]]],
    ["Living room PS5", [
      ["Console details","info","PS5 · 02:00:00:00:00:01","Sample registered console",{sample:true}],
      ["Auto-connect to Living room PS5","toggle",false,"Only one console can auto-connect.",{ref:"autoConnectMac",auto:true}],
      ["Delete Living room PS5","action","Delete","Remove this registration.",{}]
    ]],
    ["Office PS4", [
      ["Console details","info","PS4 · 02:00:00:00:00:02","Sample registered console",{sample:true}],
      ["Auto-connect to Office PS4","toggle",false,"Only one console can auto-connect.",{ref:"autoConnectMac",auto:true}],
      ["Delete Office PS4","action","Delete","Remove this registration.",{}]
    ]],
    ["Hidden consoles", [
      ["Guest PS5","info","02:00:00:00:00:03","Sample hidden console",{}],
      ["Unhide Guest PS5","action","Unhide","Show this console in the library.",{}]
    ]]
  ],
  Keys: [
    ["Keyboard and mouse", [
      ["Reset all keys","action","Reset","Restore the native default key mapping.",{ref:"clearKeyMapping"}],
      ["Keyboard as controller","toggle",true,"",{ref:"keyboardEnabled"}],
      ["Enable mouse touchpad","toggle",true,"",{ref:"mouseTouchEnabled"}]
    ]],
    ["Button mapping", keyMappings.map(([label,key]) => [label,"text",key,"Key name",{ref:"controllerMapping",key:true}])]
  ],
  Controllers: [
    ["Mapping", [
      ["Change controller mapping","action","Configure","Sample controller mapping.",{page:"Controller mapping"}],
      ["Reset controller mapping","action","Reset","Choose a controller, then restore its default mapping.",{}],
      ["Background controller events","toggle",true,"Process input when the app is in the background.",{ref:"allowJoystickBackgroundEvents"}],
      ["Buttons by position","toggle",false,"Use button positions instead of labels.",{ref:"buttonsByPosition"}]
    ]],
    ["Touchpad", [
      ["D-pad touchpad emulation","toggle",true,"",{ref:"dpadTouchEnabled"}],
      ["D-pad touch increment","number",0.3,"Millimeters",{ref:"dpadTouchIncrement",min:0.01,max:10.79,step:0.01,condition:"When D-pad touchpad emulation is enabled"}],
      ["D-pad mode combination","combo",["L1","R1","Dpad Up","Not Used"],"Switch between regular D-pad and touchpad.",{refs:["dpadTouchShortcut1","dpadTouchShortcut2","dpadTouchShortcut3","dpadTouchShortcut4"],condition:"When D-pad touchpad emulation is enabled"}]
    ]],
    ["Haptics", [
      ["Rumble haptics",["Off","Very Weak","Weak","Normal","Strong","Very Strong"],"Normal","",{ref:"rumbleHapticsIntensity"}],
      ["True haptics intensity","number",100,"Percent of console setting. 100% uses the console setting.",{ref:"hapticOverride",min:0,max:200,step:10}]
    ]]
  ],
  Remote: [
    ["PlayStation Network", [
      ["PSN account","info","Not connected","Demo account state",{}],
      ["Login to PSN","action","Log in","Opens the existing PSN login flow.",{condition:"When PSN credentials are missing"}],
      ["Clear PSN token","action","Clear token","Clear the account tokens.",{refs:["psnRefreshToken","psnAuthToken","psnAuthTokenExpiry","psnAccountId"],condition:"When PSN credentials are present"}]
    ]],
    ["Advanced connection options", [
      ["Force STUN port guessing","toggle",false,"Hole punching port guessing",{ref:"portGuessingEnabled"}],
      ["Port guess count","number",75,"Ports",{ref:"portGuessCount",min:0,max:75,step:1}],
      ["Port guess socket count","number",250,"Sockets",{ref:"portGuessSocketCount",min:0,max:500,step:1}]
    ]]
  ],
  Config: [
    ["Profiles and settings", [
      ["Current profile","info","Default","Sample profile",{ref:"currentProfile"}],
      ["Manage profiles","action","Open","Switch, create or delete a profile.",{page:"Profiles"}],
      ["Export settings","action","Export","Export settings to a file.",{ref:"exportSettings"}],
      ["Import settings","action","Import","Import settings from a file.",{ref:"importSettings"}]
    ]],
    ["Diagnostics", [
      ["Sanitize logs","toggle",true,"",{ref:"logSanitize"}],
      ["Verbose logging","toggle",false,"",{ref:"logVerbose"}],
      ["About chiaki-ng","action","About","Version, project links and license.",{}]
    ]]
  ],
  Profiles: [["User profiles", [
    ["User profile",["Default","Travel","Create new profile"],"Default","Sample profiles",{ref:"profiles",sample:true}],
    ["New profile name","text","","",{condition:"When Create new profile is selected"}],
    ["Delete selected profile","toggle",false,"",{condition:"For a profile other than Default or the active profile"}],
    ["Switch profile","action","Switch","",{}],["Create profile","action","Create","",{}],["Delete profile","action","Delete","",{}]
  ]]],
  "Controller mapping": [
    ["Controller", [
      ["Selected controller","info","DualSense","Sample controller",{}],
      ["Allow mapping analog sticks and L2/R2","toggle",false,"",{ref:"enableAnalogStickMapping"}]
    ]],
    ["Physical button mapping", [...keyMappings.slice(0,18).map(([name])=>name), "Left Stick X","Left Stick Y","Right Stick X","Right Stick Y","MIC"].map(name =>
      [name,"mapping",[name,""],"Primary and alternate physical input",{condition:/Stick|L2|R2/.test(name) ? "When analog mapping is enabled" : ""}])],
    ["Apply", [["Update controller mapping","action","Update mapping","Apply this sample mapping.",{}]]]
  ]
};
