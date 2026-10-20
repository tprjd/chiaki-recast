# Domain docs

## Layout

Use one root `GLOSSARY.md` and `docs/adr/` for architecture decision records.

## Before code exploration

Read `GLOSSARY.md` and the ADRs relevant to the area being changed.

If these files do not exist, proceed silently. Create them through
`domain-modeling` when terms or decisions are resolved.

## Vocabulary

Use the glossary's terms in issues, proposals, and tests.
If a required term is missing, check existing project usage.
Record unresolved terminology for `domain-modeling`.

## Decision conflicts

If a proposal conflicts with an existing ADR, identify that ADR
and explain the conflict before proceeding.
