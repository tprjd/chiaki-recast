# Main console screen design

Status: base design selected

## Confirmed scope

- Design first, then prepare an implementation plan for the app.
- Start with the main console screen.
- Prioritize desktop mouse and keyboard use.
- Build three HTML/CSS/JavaScript designs that are quick to edit and compare.
- Refine the chosen design before planning Qt/QML changes.
- Use functional labels and instructions. Omit slogans, decorative headings,
  and repeated tips.
- Use B, Card library, as the implementation base.
- Test the real QML console screen with sample host data and action-target
  assertions, then check discovery and streaming in the native app.

## Prototype

Source: `gui/prototype/`.

- A: compact console list with actions on each row.
- B: card library with console artwork and a navigation rail.
- C: console selector with a larger details and connection area.

All designs share the same sample consoles, statuses, and simulated actions.
The user selected B after removal of the decorative text. Its card layout,
sidebar, dark palette, blue accent, and functional text form the design baseline.
Selection of the visual design does not make its simulated connection rules
the native application's behavior.

## Decision

Implement B in native Qt/QML. Preserve existing connection, registration,
PSN, privacy, and controller behavior. Keep A and C as design references.
The implementation specification is `spec.md` in this directory.

## Open decisions

- Minor spacing and typography adjustments after native rendering.
- Local Qt toolchain setup and access to consoles for native verification.

## Limits

The browser prototype uses sample data. It does not verify Qt rendering,
controller input, discovery, PSN, or streaming. The native UI is unchanged.
