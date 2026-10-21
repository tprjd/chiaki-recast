# Chiaki Recast

Chiaki Recast connects to PlayStation consoles for Remote Play. It is a fork of chiaki-ng.

## Language

**Console login PIN**:
The four-digit PIN used to sign in to a console user. A saved console login PIN can be used automatically when that console requests sign-in.
_Avoid_: Remote Play PIN, pairing code

**Remote Play pairing code**:
The eight-digit code displayed by a console to register a Remote Play client.
_Avoid_: Console login PIN

**Auto-connect**:
An automatic connection attempt to a selected registered console when the app starts.

**Console wake-up**:
The transition of a console from rest mode so that it becomes available for Remote Play.

**Sign-in redirect URL**:
The complete browser URL returned after PSN sign-in. It contains the authorization code used to complete sign-in in Chiaki Recast.
_Avoid_: Token, token string

**Stream input owner**:
A local interaction that temporarily blocks console controls during Remote Play. Several stream input owners can be active at the same time.

**Stream input presentation**:
One open-to-close occurrence of a stream input owner. A presentation can remain visible after it releases console input.

**Keyboard and mouse mapping**:
The conversion of PC key presses, mouse buttons, and mouse movement into PlayStation controller input for a Remote Play session.

**Gameplay capture**:
The mode in which Recast directs keyboard and mouse input to the game. Leaving this mode returns mouse control to the desktop and releases held game inputs.
