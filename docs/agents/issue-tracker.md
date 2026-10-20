# Issue tracker: Local Markdown

Issues and specs live as Markdown files under `.scratch/`.

## Conventions

- Use one directory per feature: `.scratch/<feature-slug>/`.
- Store the spec in `spec.md` within that directory.
- Store each implementation ticket in `issues/<NN>-<slug>.md`.
- Number tickets from `01`. Keep one file per ticket.
- Record triage state in a `Status:` line near the top.
  Use the values in `triage-labels.md`.
- Append comments under a `## Comments` heading.

## Skill operations

When a skill says "publish to the issue tracker", create the appropriate
file under `.scratch/<feature-slug>/`.

When a skill says "fetch the relevant ticket", read the referenced file.
Resolve ticket numbers within the relevant feature directory.

## Wayfinding operations

- Store the map in `.scratch/<effort>/map.md`.
- Store child tickets in `.scratch/<effort>/issues/NN-<slug>.md`.
- Record each ticket's type in `Type:`. Use `research`, `prototype`,
  `grilling`, or `task`.
- Record dependencies in `Blocked by: NN, NN`.
- A ticket is unblocked when every listed blocker is `resolved`.
- Select the first open, unblocked, unclaimed ticket by number.
- To claim a ticket, set `Status: claimed` and save before work.
- To resolve a ticket, append an `## Answer` section and set
  `Status: resolved`. Add the decision summary and ticket link to the map.

Wayfinding uses `claimed` and `resolved` as workflow states.
Use the five triage labels for triage.
