# First red identity

Before the first real QtTest run, tool output recorded:

- Base source `691342dfac10d6e5857190ba40808247a2e8a879`.
- The sole application-source change was the initial `registrationForm` case.
- Test source file SHA256 `ba5cede5914fdcf827afab761710a7293bcbbdbf1c24c7bbdd2afd89c5363d24`.
- Test-only diff Git hash `5c48ecdb288fa0f7001173a33e7d6fa566d4eec0`.
- Executable SHA256 `f676d3617483c0e6b0bc94d79eb3ee05c0a686cb411fb4a76d5cf645c0b1ef7d`.

The executable was later replaced by normal builds. The original test-only
source patch was not saved. These values come from the before-run tool output,
not a later binary reconstruction. The first QtTest report proves the missing
Register console entry. Later regression source is retained in Git.

The first launch failed in the Windows loader with `0xc0000135`. Adding this
build's Steam tools directory to PATH fixed the missing DLL. A console-output
retry needed QtTest's explicit `-o <file>,txt` option to save the report.
The source and executable did not change during those launcher corrections.
