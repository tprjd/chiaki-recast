# 26: Complete the web-only application and verify native performance

Superseded: historical draft. Use the [23 published tickets](../ticket-breakdown.md) for implementation.

**What to build:** The complete packaged application provides GPU-accelerated web setup and separate native playback and overlays, with the required feature and performance evidence.

**Blocked by:** 08: Retrieve an Account ID during registration; 11: Register a console automatically through PSN; 14: Configure stream profiles and audio devices; 16: Configure native color mapping from web setup; 18: Tune mouse response and reset keyboard and mouse controls; 19: Configure controllers and their mappings; 20: Manage profiles, settings files, and logs; 21: Complete setup reminders and Steam shortcuts; 22: Ship the Windows web and native application; 23: Ship the Linux web and native application; 24: Ship the macOS web and native application; 25: Resume interrupted sessions and recover web setup.

**Status:** needs-info

**Spec stories:** 3, 48, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95.

- [ ] Every specification story is mapped to an implemented flow and acceptance evidence. The application offers one web setup path without a QML setup fallback or runtime UI selector.
- [ ] Retired setup QML, obsolete callback adapters, resource entries, build references, and tests are removed when no caller needs them. Native stream QML, overlays, live settings, and their tests remain.
- [ ] All settings categories, linked search routes, input flows, account routes, reminders, confirmations, profiles, files, and Steam actions are reachable in the packaged application.
- [ ] Access checks cover keyboard, controller, touch, screen-reader labels, focus return, reduced motion, privacy, long text, required sizes, display scaling, and 200 percent zoom.
- [ ] The production embedded application test suite exercises the real bridge and native services. Existing native input ownership and output checks remain effective on their supported toolchains.
- [ ] Final Windows, Linux, and macOS package smoke checks are repeated with all migrated features and actual Chromium GPU acceleration enabled.
- [ ] At least three matched native baseline comparisons show no repeatable regression beyond predefined variation. Record frame timing, queue depth, pending frame age, dropped frames, audio underruns, CPU, GPU, memory including Chromium, and time to first frame.
- [ ] Each validated renderer path completes a 30-minute stream and 20 connect-stop cycles without retained windows, callbacks, threads, GPU resources, or held inputs. Cover supported SDR, HDR, decoder, and fallback cases.
- [ ] Measure input latency externally when equipment is available. Report unavailable measurements and unsupported combinations explicitly instead of treating them as passed.
