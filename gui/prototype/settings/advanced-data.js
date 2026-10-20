// Extracted from native QML for this disposable prototype.
Object.assign(sections, {
  "Display": [
    [
      "Stream Display Settings",
      [
        [
          "Target Primaries",
          [
            "Auto",
            "ITU-R Rec. BT.601 NTSC (Standard Gamut)",
            "ITU-R Rec. BT.601 PAL (Standard Gamut)",
            "ITU-R Rec. BT.709 (Standard Gamut)",
            "ITU-R Rec. BT.470 M (Standard Gamut)",
            "EBU Tech. 3213-E (Standard Gamut)",
            "ITU-R Rec. BT.2020 (Wide Gamut)",
            "Apple RGB (Wide Gamut)",
            "Adobe RGB (Wide Gamut)",
            "ProPhoto RGB (Wide Gamut)",
            "CIE 1931 RGB primaries (Wide Gamut)",
            "DCI-P3 (Wide Gamut)",
            "DCI-P3 with D65 white point (Wide Gamut)",
            "Panasonic V-Gamut (Wide Gamut)",
            "Sony S-Gamut (Wide Gamut)",
            "Traditional film primaries with Illuminant C (Wide Gamut)",
            "ACES Primaries #0 (Wide Gamut)",
            "ACES Primaries #1 (Wide Gamut)"
          ],
          "Auto",
          "",
          {
            "source": "DisplaySettingsDialog.qml:37",
            "refs": [
              "displayTargetPrim"
            ],
            "defaultText": "Auto"
          }
        ],
        [
          "Target Transfer Characteristics",
          [
            "Auto",
            "ITU-R Rec. BT.1886 (SDR)",
            "IEC 61966-2-4 sRGB (SDR)",
            "Linear light content (SDR)",
            "IPure power gamma 1.8 (SDR)",
            "Pure power gamma 2.0 (SDR)",
            "Pure power gamma 2.2 (SDR)",
            "Pure power gamma 2.4 (SDR)",
            "Pure power gamma 2.6 (SDR)",
            "Pure power gamma 2.8 (SDR)",
            "ProPhoto RGB (SDR)",
            "Digital Cinema Distribution Master (SDR)",
            "ITU-R BT.2100 PQ / SMPTE ST2048 (HDR)",
            "ITU-R BT.2100 HLG / ARIB STD-B67 (HDR)",
            "Panasonic V-Log (HDR)",
            "Sony S-Log1 (HDR)",
            "Sony S-Log2 (HDR)"
          ],
          "Auto",
          "",
          {
            "source": "DisplaySettingsDialog.qml:55",
            "refs": [
              "displayTargetTrc"
            ],
            "defaultText": "Auto"
          }
        ],
        [
          "Target Peak",
          [
            "Auto",
            "Numeric Value"
          ],
          "Auto",
          "",
          {
            "source": "DisplaySettingsDialog.qml:72",
            "refs": [
              "displayTargetPeak"
            ],
            "defaultText": "Auto"
          }
        ],
        [
          "Target Peak Value",
          "number",
          1000.0,
          "nits",
          {
            "source": "DisplaySettingsDialog.qml:101",
            "refs": [
              "displayTargetPeak"
            ],
            "defaultText": "1000 nits",
            "condition": "When Target Peak is Numeric Value",
            "min": 10.0,
            "max": 10000.0,
            "step": 10.0
          }
        ],
        [
          "Target Contrast",
          [
            "Auto",
            "Infinity",
            "Numeric Value"
          ],
          "Auto",
          "",
          {
            "source": "DisplaySettingsDialog.qml:132",
            "refs": [
              "displayTargetContrast"
            ],
            "defaultText": "Auto"
          }
        ],
        [
          "Target Contrast Value",
          "number",
          1000.0,
          "",
          {
            "source": "DisplaySettingsDialog.qml:166",
            "refs": [
              "displayTargetContrast"
            ],
            "defaultText": "1000",
            "condition": "When Target Contrast is Numeric Value",
            "min": 10.0,
            "max": 1000000.0,
            "step": 1000.0
          }
        ]
      ]
    ]
  ],
  "Renderer": [
    [
      "Config",
      [
        [
          "Libplacebo Renderer Options Details",
          "action",
          "Open Documentation",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:106",
            "refs": [],
            "defaultText": ""
          }
        ],
        [
          "Copy documentation URL again",
          "action",
          "Click to Re-Copy URL",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:128",
            "refs": [],
            "defaultText": ""
          }
        ],
        [
          "Export Render Settings to File",
          "action",
          "Export",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:142",
            "refs": [
              "exportPlaceboSettings"
            ],
            "defaultText": ""
          }
        ],
        [
          "Import Render Settings from File",
          "action",
          "Import",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:157",
            "refs": [
              "importPlaceboSettings"
            ],
            "defaultText": ""
          }
        ]
      ]
    ],
    [
      "Scaling",
      [
        [
          "Upscaler",
          [
            "Custom",
            "Nearest",
            "Bilinear",
            "Oversample",
            "Bicubic",
            "Gaussian",
            "Catmull Rom",
            "Lanczos",
            "EwaLanczos",
            "EwaLanczosSharp",
            "EwaLanczosSharpest",
            "FSR",
            "FSRCNNX x2 8-0-4-1",
            "FSRCNNX x2 16-0-4-1"
          ],
          "EwaLanczosSharp",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:188",
            "refs": [
              "placeboUpscaler"
            ],
            "defaultText": "EwaLanczosSharp, FSR on modest upscale, or FSRCNNX with FSR fallback below 1.2x"
          }
        ],
        [
          "Plane Upscaler",
          [
            "Chosen Upscaler",
            "Nearest",
            "Bilinear",
            "Oversample",
            "Bicubic",
            "Gaussian",
            "Catmull Rom",
            "Lanczos",
            "EwaLanczos",
            "EwaLanczosSharp",
            "EwaLanczosSharpest"
          ],
          "Chosen Upscaler",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:206",
            "refs": [
              "placeboPlaneUpscaler"
            ],
            "defaultText": "Chosen Upscaler"
          }
        ],
        [
          "Downscaler",
          [
            "Custom",
            "Box",
            "Hermite",
            "Bilinear",
            "Bicubic",
            "Gaussian",
            "Catmull Rom",
            "Mitchell",
            "Lanczos"
          ],
          "Hermite",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:224",
            "refs": [
              "placeboDownscaler"
            ],
            "defaultText": "Hermite"
          }
        ],
        [
          "Plane Downscaler",
          [
            "Chosen Downscaler",
            "Box",
            "Hermite",
            "Bilinear",
            "Bicubic",
            "Gaussian",
            "Catmull Rom",
            "Mitchell",
            "Lanczos"
          ],
          "Chosen Downscaler",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:242",
            "refs": [
              "placeboPlaneDownscaler"
            ],
            "defaultText": "Chosen Downscaler"
          }
        ],
        [
          "Frame Mixer",
          [
            "None",
            "Oversample",
            "Hermite",
            "Linear",
            "Cubic"
          ],
          "Oversample",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:260",
            "refs": [
              "placeboFrameMixer"
            ],
            "defaultText": "Oversample"
          }
        ],
        [
          "Anti-ringing Strength",
          "number",
          0.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:277",
            "refs": [
              "placeboAntiringingStrength"
            ],
            "defaultText": "0.00",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ]
      ]
    ],
    [
      "Debanding",
      [
        [
          "Deband Enabled",
          "toggle",
          true,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:319",
            "refs": [
              "placeboDebandEnabled"
            ],
            "defaultText": "Checked"
          }
        ],
        [
          "Deband Preset",
          [
            "Custom",
            "Default"
          ],
          "Custom",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:337",
            "refs": [
              "placeboDebandEnabled",
              "placeboDebandPreset"
            ],
            "defaultText": "Custom",
            "condition": "When Deband Enabled"
          }
        ],
        [
          "Deband Iterations",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:357",
            "refs": [
              "placeboDebandEnabled",
              "placeboDebandIterations",
              "placeboDebandPreset"
            ],
            "defaultText": "1",
            "condition": "When Deband Enabled and Deband Preset is Custom",
            "min": 0.0,
            "max": 16.0,
            "step": 1.0
          }
        ],
        [
          "Deband Threshold",
          "number",
          3.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:388",
            "refs": [
              "placeboDebandEnabled",
              "placeboDebandPreset",
              "placeboDebandThreshold"
            ],
            "defaultText": "3.0",
            "condition": "When Deband Enabled and Deband Preset is Custom",
            "min": 0.0,
            "max": 1000.0,
            "step": 0.1
          }
        ],
        [
          "Deband Radius",
          "number",
          16.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:419",
            "refs": [
              "placeboDebandEnabled",
              "placeboDebandPreset",
              "placeboDebandRadius"
            ],
            "defaultText": "16.0",
            "condition": "When Deband Enabled and Deband Preset is Custom",
            "min": 0.0,
            "max": 1000.0,
            "step": 0.1
          }
        ],
        [
          "Deband Grain",
          "number",
          4.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:450",
            "refs": [
              "placeboDebandEnabled",
              "placeboDebandGrain",
              "placeboDebandPreset"
            ],
            "defaultText": "4.0",
            "condition": "When Deband Enabled and Deband Preset is Custom",
            "min": 0.0,
            "max": 1000.0,
            "step": 0.1
          }
        ]
      ]
    ],
    [
      "Sigmoidization",
      [
        [
          "Sigmoidization Enabled",
          "toggle",
          true,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:494",
            "refs": [
              "placeboSigmoidEnabled"
            ],
            "defaultText": "Checked"
          }
        ],
        [
          "Sigmoid Preset",
          [
            "Custom",
            "Default"
          ],
          "Custom",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:511",
            "refs": [
              "placeboSigmoidEnabled",
              "placeboSigmoidPreset"
            ],
            "defaultText": "Custom",
            "condition": "When Sigmoidization Enabled"
          }
        ],
        [
          "Sigmoid Center",
          "number",
          0.75,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:531",
            "refs": [
              "placeboSigmoidCenter",
              "placeboSigmoidEnabled",
              "placeboSigmoidPreset"
            ],
            "defaultText": "0.75",
            "condition": "When Sigmoidization Enabled and Sigmoid Preset is Custom",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Sigmoid Slope",
          "number",
          6.5,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:562",
            "refs": [
              "placeboSigmoidEnabled",
              "placeboSigmoidPreset",
              "placeboSigmoidSlope"
            ],
            "defaultText": "6.5",
            "condition": "When Sigmoidization Enabled and Sigmoid Preset is Custom",
            "min": 1.0,
            "max": 20.0,
            "step": 0.1
          }
        ]
      ]
    ],
    [
      "Color",
      [
        [
          "Color Adjustment Enabled",
          "toggle",
          true,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:606",
            "refs": [
              "placeboColorAdjustmentEnabled"
            ],
            "defaultText": "Checked"
          }
        ],
        [
          "Color Adjustment Preset",
          [
            "Custom",
            "Neutral"
          ],
          "Custom",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:623",
            "refs": [
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentPreset"
            ],
            "defaultText": "Custom",
            "condition": "When Color Adjustment Enabled"
          }
        ],
        [
          "Brightness",
          "number",
          0.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:643",
            "refs": [
              "placeboColorAdjustmentBrightness",
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentPreset"
            ],
            "defaultText": "0.00",
            "condition": "When Color Adjustment Enabled and Color Adjustment Preset is Custom",
            "min": -1.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Contrast",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:674",
            "refs": [
              "placeboColorAdjustmentContrast",
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentPreset"
            ],
            "defaultText": "1.0",
            "condition": "When Color Adjustment Enabled and Color Adjustment Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.1
          }
        ],
        [
          "Saturation",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:705",
            "refs": [
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentPreset",
              "placeboColorAdjustmentSaturation"
            ],
            "defaultText": "1.0",
            "condition": "When Color Adjustment Enabled and Color Adjustment Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.1
          }
        ],
        [
          "Hue",
          "number",
          0.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:736",
            "refs": [
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentHue",
              "placeboColorAdjustmentPreset"
            ],
            "defaultText": "0.00",
            "condition": "When Color Adjustment Enabled and Color Adjustment Preset is Custom",
            "min": 0.0,
            "max": 6.28,
            "step": 0.01
          }
        ],
        [
          "Gamma",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:767",
            "refs": [
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentGamma",
              "placeboColorAdjustmentPreset"
            ],
            "defaultText": "1.0",
            "condition": "When Color Adjustment Enabled and Color Adjustment Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.1
          }
        ],
        [
          "Temperature",
          "number",
          0.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:798",
            "refs": [
              "placeboColorAdjustmentEnabled",
              "placeboColorAdjustmentPreset",
              "placeboColorAdjustmentTemperature"
            ],
            "defaultText": "0.000",
            "condition": "When Color Adjustment Enabled and Color Adjustment Preset is Custom",
            "min": -1.143,
            "max": 5.286,
            "step": 0.001
          }
        ]
      ]
    ],
    [
      "Peak Detection",
      [
        [
          "HDR Peak Detection Enabled",
          "toggle",
          true,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:842",
            "refs": [
              "placeboPeakDetectionEnabled"
            ],
            "defaultText": "Checked"
          }
        ],
        [
          "Peak Detection Preset",
          [
            "Custom",
            "Default",
            "High Quality"
          ],
          "High Quality",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:859",
            "refs": [
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPreset"
            ],
            "defaultText": "High Quality",
            "condition": "When HDR Peak Detection Enabled"
          }
        ],
        [
          "Peak Smoothing Period",
          "number",
          20.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:879",
            "refs": [
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPeakSmoothingPeriod",
              "placeboPeakDetectionPreset"
            ],
            "defaultText": "20.0",
            "condition": "When HDR Peak Detection Enabled and Peak Detection Preset is Custom",
            "min": 0.0,
            "max": 1000.0,
            "step": 0.1
          }
        ],
        [
          "Scene Threshold Low",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:910",
            "refs": [
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPreset",
              "placeboPeakDetectionSceneThresholdLow"
            ],
            "defaultText": "1.0",
            "condition": "When HDR Peak Detection Enabled and Peak Detection Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.1
          }
        ],
        [
          "Scene Threshold High",
          "number",
          3.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:941",
            "refs": [
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPreset",
              "placeboPeakDetectionSceneThresholdHigh"
            ],
            "defaultText": "3.0",
            "condition": "When HDR Peak Detection Enabled and Peak Detection Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.1
          }
        ],
        [
          "Peak Percentile",
          "number",
          99.995,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:972",
            "refs": [
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPeakPercentile",
              "placeboPeakDetectionPreset"
            ],
            "defaultText": "99.995",
            "condition": "When HDR Peak Detection Enabled and Peak Detection Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.001
          }
        ],
        [
          "Black Cutoff",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:1003",
            "refs": [
              "placeboPeakDetectionBlackCutoff",
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPreset"
            ],
            "defaultText": "1.0",
            "condition": "When HDR Peak Detection Enabled and Peak Detection Preset is Custom",
            "min": 0.0,
            "max": 100.0,
            "step": 0.1
          }
        ],
        [
          "Allow Delayed Peak",
          "toggle",
          false,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:1034",
            "refs": [
              "placeboPeakDetectionAllowDelayedPeak",
              "placeboPeakDetectionEnabled",
              "placeboPeakDetectionPreset"
            ],
            "defaultText": "Unchecked",
            "condition": "When HDR Peak Detection Enabled and Peak Detection Preset is Custom"
          }
        ]
      ]
    ],
    [
      "Color Mapping",
      [
        [
          "Color Mapping Enabled",
          "toggle",
          true,
          "",
          {
            "source": "PlaceboSettingsDialog.qml:1066",
            "refs": [
              "placeboColorMappingEnabled"
            ],
            "defaultText": "Checked"
          }
        ],
        [
          "Color Mapping Preset",
          [
            "Custom",
            "Default",
            "High Quality"
          ],
          "High Quality",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:1083",
            "refs": [
              "placeboColorMappingEnabled",
              "placeboColorMappingPreset"
            ],
            "defaultText": "High Quality",
            "condition": "When Color Mapping Enabled"
          }
        ],
        [
          "Color Mapping Settings",
          "action",
          "Open",
          "",
          {
            "source": "PlaceboSettingsDialog.qml:1103",
            "refs": [
              "placeboColorMappingEnabled",
              "placeboColorMappingPreset"
            ],
            "defaultText": "",
            "condition": "When Color Mapping Enabled and Color Mapping Preset is Custom",
            "page": "Color mapping"
          }
        ]
      ]
    ]
  ],
  "Color mapping": [
    [
      "Display",
      [
        [
          "Gamut Mapping Function",
          [
            "Clip",
            "Perceptual",
            "Soft Clip",
            "Relative",
            "Saturation",
            "Absolute",
            "Desaturate",
            "Darken",
            "Highlight",
            "Linear"
          ],
          "Perceptual",
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:121",
            "refs": [
              "placeboGamutMappingFunction"
            ],
            "defaultText": "Perceptual"
          }
        ],
        [
          "Perceptual Deadzone",
          "number",
          0.3,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:140",
            "refs": [
              "placeboGamutMappingFunction",
              "placeboGamutMappingPerceptualDeadzone"
            ],
            "defaultText": "0.30",
            "condition": "When Gamut Mapping Function is Perceptual",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Perceptual Strength",
          "number",
          0.8,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:171",
            "refs": [
              "placeboGamutMappingFunction",
              "placeboGamutMappingPerceptualStrength"
            ],
            "defaultText": "0.80",
            "condition": "When Gamut Mapping Function is Perceptual",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Colorimetric Gamma",
          "number",
          1.8,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:203",
            "refs": [
              "placeboGamutMappingColorimetricGamma",
              "placeboGamutMappingFunction"
            ],
            "defaultText": "1.80",
            "condition": "When Gamut Mapping Function is Relative or Gamut Mapping Function is Absolute or Gamut Mapping Function is Darken",
            "min": 0.0,
            "max": 10.0,
            "step": 0.01
          }
        ],
        [
          "Soft Clip Knee",
          "number",
          0.7,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:234",
            "refs": [
              "placeboGamutMappingFunction",
              "placeboGamutMappingSoftClipKnee"
            ],
            "defaultText": "0.70",
            "condition": "When Gamut Mapping Function is Perceptual or Gamut Mapping Function is Soft Clip",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Soft Clip Desaturation",
          "number",
          0.35,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:265",
            "refs": [
              "placeboGamutMappingFunction",
              "placeboGamutMappingSoftClipDesat"
            ],
            "defaultText": "0.35",
            "condition": "When Gamut Mapping Function is Soft Clip",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "LUT 3D Size I",
          "number",
          48.0,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:309",
            "refs": [
              "placeboGamutMappingLut3dSizeI"
            ],
            "defaultText": "48",
            "min": 0.0,
            "max": 1024.0,
            "step": 1.0
          }
        ],
        [
          "LUT 3D Size C",
          "number",
          32.0,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:337",
            "refs": [
              "placeboGamutMappingLut3dSizeC"
            ],
            "defaultText": "32",
            "min": 0.0,
            "max": 1024.0,
            "step": 1.0
          }
        ],
        [
          "LUT 3D Size h",
          "number",
          256.0,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:365",
            "refs": [
              "placeboGamutMappingLut3dSizeH"
            ],
            "defaultText": "256",
            "min": 0.0,
            "max": 1024.0,
            "step": 1.0
          }
        ],
        [
          "LUT 3D Tricubic Enabled",
          "toggle",
          false,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:393",
            "refs": [
              "placeboGamutMappingLut3dTricubicEnabled"
            ],
            "defaultText": "Unchecked"
          }
        ],
        [
          "Gamut Expansion Enabled",
          "toggle",
          false,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:409",
            "refs": [
              "placeboGamutMappingGamutExpansionEnabled"
            ],
            "defaultText": "Unchecked"
          }
        ],
        [
          "Tone-mapping Function",
          [
            "Clip",
            "Spline",
            "St2094-40",
            "St-2094-10",
            "Bt2390",
            "Bt2446a",
            "Reinhard",
            "Mobius",
            "Hable",
            "Gamma",
            "Linear",
            "Linear Light"
          ],
          "Spline",
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:439",
            "refs": [
              "placeboToneMappingFunction"
            ],
            "defaultText": "Spline"
          }
        ],
        [
          "Knee Adaptation",
          "number",
          0.4,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:458",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingKneeAdaptation"
            ],
            "defaultText": "0.40",
            "condition": "When Tone-mapping Function is Spline or Tone-mapping Function is St209440 or Tone-mapping Function is St209410",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Knee Minimum",
          "number",
          0.1,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:490",
            "refs": [
              "placeboToneMappingKneeMinimum"
            ],
            "defaultText": "0.10",
            "condition": "When Tone-mapping Function is Spline, St2094-40 or St-2094-10",
            "min": 0.0,
            "max": 0.5,
            "step": 0.01
          }
        ],
        [
          "Knee Maximum",
          "number",
          0.8,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:521",
            "refs": [
              "placeboToneMappingKneeMaximum"
            ],
            "defaultText": "0.80",
            "condition": "When Tone-mapping Function is Spline, St2094-40 or St-2094-10",
            "min": 0.5,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Knee Default",
          "number",
          0.4,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:551",
            "refs": [
              "placeboToneMappingKneeDefault"
            ],
            "defaultText": "0.40",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Knee Offset",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:580",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingKneeOffset"
            ],
            "defaultText": "1.00",
            "condition": "When Tone-mapping Function is Bt2390",
            "min": 0.5,
            "max": 2.0,
            "step": 0.01
          }
        ],
        [
          "Slope Tuning",
          "number",
          1.5,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:625",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingSlopeTuning"
            ],
            "defaultText": "1.5",
            "condition": "When Tone-mapping Function is Spline",
            "min": 0.0,
            "max": 10.0,
            "step": 0.1
          }
        ],
        [
          "Slope Offset",
          "number",
          0.2,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:657",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingSlopeOffset"
            ],
            "defaultText": "0.20",
            "condition": "When Tone-mapping Function is Spline",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Spline Contrast",
          "number",
          0.5,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:688",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingSplineContrast"
            ],
            "defaultText": "0.50",
            "condition": "When Tone-mapping Function is Spline",
            "min": 0.0,
            "max": 1.5,
            "step": 0.01
          }
        ],
        [
          "Reinhard Contrast",
          "number",
          0.5,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:719",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingReinhardContrast"
            ],
            "defaultText": "0.50",
            "condition": "When Tone-mapping Function is Reinhard",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Linear Knee",
          "number",
          0.3,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:750",
            "refs": [
              "placeboToneMappingFunction",
              "placeboToneMappingLinearKnee"
            ],
            "defaultText": "0.3",
            "condition": "When Tone-mapping Function is Mobius or Tone-mapping Function is Gamma",
            "min": 0.0,
            "max": 1.0,
            "step": 0.01
          }
        ],
        [
          "Exposure",
          "number",
          1.0,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:781",
            "refs": [
              "placeboToneMappingExposure",
              "placeboToneMappingFunction"
            ],
            "defaultText": "1.0",
            "condition": "When Tone-mapping Function is Linear or Tone-mapping Function is Linear Light",
            "min": 0.0,
            "max": 10.0,
            "step": 0.1
          }
        ],
        [
          "Inverse Tone-mapping Enabled",
          "toggle",
          false,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:825",
            "refs": [
              "placeboToneMappingInverseToneMappingEnabled"
            ],
            "defaultText": "Unchecked"
          }
        ],
        [
          "Tone-mapping Metadata",
          [
            "Any",
            "None",
            "HDR10",
            "HDR10 Plus",
            "Cie_y"
          ],
          "Any",
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:841",
            "refs": [
              "placeboToneMappingMetadata"
            ],
            "defaultText": "Any"
          }
        ],
        [
          "Tone LUT Size",
          "number",
          256.0,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:859",
            "refs": [
              "placeboToneMappingToneLutSize"
            ],
            "defaultText": "256",
            "min": 0.0,
            "max": 4096.0,
            "step": 1.0
          }
        ],
        [
          "Contrast Recovery",
          "number",
          0.3,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:887",
            "refs": [
              "placeboToneMappingContrastRecovery"
            ],
            "defaultText": "0.30",
            "min": 0.0,
            "max": 2.0,
            "step": 0.01
          }
        ],
        [
          "Contrast Smoothness",
          "number",
          3.5,
          "",
          {
            "source": "PlaceboColorMappingDialog.qml:915",
            "refs": [
              "placeboToneMappingContrastSmoothness"
            ],
            "defaultText": "3.5",
            "min": 1.0,
            "max": 32.0,
            "step": 0.1
          }
        ]
      ]
    ]
  ]
});
