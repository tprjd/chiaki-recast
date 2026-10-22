# Ticket 08 native request seams

Source `bf4c7e01251168b12a5ddcf26f1d63ea96902bf4`.

`PsnRequests` retains `networkManager` and `listDevices`. The device-list
function now uses `std::function` so a fixture can retain per-instance state.
Its default remains `chiaki_holepunch_list_devices`.

`PsnRequests::connect` is optional. PsnConnectionWorker copies it at
construction. When absent, the worker calls the real
`StreamSession::ConnectPsnConnection`. The existing resultReady signal and
attempt/session/profile guards own completion.

`PsnRequests::startSession` defaults to `chiaki_session_start`. QmlBackend
passes it to the StreamSession constructor. StreamSession stores it and uses
it for the existing raw start invocation. Start logic, events, media, stop,
and join remain unchanged. The ticket 08 fixture records and rejects any
unexpected call, then asserts zero calls after cancelled and stale success.

`DiscoveryRequests` is in `discoverymanager.h`. It has per-instance init and
fini functions with the raw discovery-service signatures. Defaults remain
`chiaki_discovery_service_init` and `chiaki_discovery_service_fini`.
DiscoveryManager receives it before any service starts. IPv4, IPv6, manual
services, destructor cleanup, and restart use the same dependency.

On this branch, before ticket 07 is integrated, the constructor order is
PsnRequests, ProcessLauncher, DiscoveryRequests. Ticket 07 adds its independent
RegistrationRequests. The combined order must be PsnRequests, ProcessLauncher,
RegistrationRequests, DiscoveryRequests. Update fixture argument positions
at that merge. Do not replace either transport with the other.

Production SetupBridge remains the only registered setup object. No fixture
transport, session-start function, token, or media stream is exposed through
WebChannel.
