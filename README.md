
# Chiaki Recast

A PlayStation Remote Play client with a redesigned desktop interface, forked from
[chiaki-ng](https://github.com/chiaki-ng/chiaki-ng).

Chiaki Recast starts with a reworked console library, settings, connection dialogs,
and stream controls. Further work will focus on the application architecture.

The repository name is `chiaki-recast`.

## Project status

This fork is in development. The interface changes are implemented in the native
Qt application. Future architecture changes are planned.

Build targets, executable names, settings locations, and package identifiers still
use inherited Chiaki names. Existing profiles and console registrations keep their
current storage locations. The release workflows and installer configuration are
inherited from upstream and need configuration before publishing Recast binaries.
The logo and Steam artwork are inherited from chiaki-ng.

## Development

See the [contributor guide](CONTRIBUTOR_GUIDE.md) for the source layout and
[screen test guide](gui/tests/README.md) for production QML checks.
For the Windows launcher, see the [local launch instructions](docs/setup/launch-local-windows.md).
The [upstream documentation](https://chiaki-ng.github.io/chiaki-ng/) describes
Remote Play setup. Its interface and release instructions can differ from this fork.

## Credits and license

Chiaki Recast builds on chiaki-ng by Street Pea and contributors, and the original
Chiaki by Florian Märkl and contributors. Their work provides the Remote Play
implementation and the foundation for this fork.

The inherited license terms and copyright notices remain in place. See
[COPYING](COPYING) and [LICENSES](LICENSES).

This project is not endorsed or certified by Sony Interactive Entertainment LLC.
