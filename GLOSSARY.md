# chiaki-ng

chiaki-ng connects to PlayStation consoles for Remote Play.

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
The complete browser URL returned after PSN sign-in. It contains the authorization code used to complete sign-in in chiaki-ng.
_Avoid_: Token, token string
