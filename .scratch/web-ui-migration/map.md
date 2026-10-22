# Web UI migration implementation

Status: active
Integration branch: codex/web-ui-migration-integration
Implementation baseline: d0e2f052cb076b7fda85c51f60dab1cc428951fb
Started: 2026-10-22

The user invoked implement-spec for the full approved specification. This
supersedes the earlier planning-only task restriction. Use the approved
[23-ticket graph](ticket-breakdown.md) and [specification](spec.md).

The integration baseline preserves the source workspace, including pending
keyboard and mouse changes. The original main checkout remains unchanged.

## Test policy

Use the approved embedded application boundary and existing native input
checks. Run focused checks for changed behavior and affected callers.
Reuse valid passing results. Broaden or repeat checks only for relevant
changes, failures, or unresolved concerns. Record hardware limitations.

## Frontier

- [01: Application-owned native services](issues/01-application-owned-native-services.md): claimed.
- Tickets 02 through 23: waiting for their declared blockers.

## Integration history

- d0e2f052: preserved current source and approved planning baseline.
