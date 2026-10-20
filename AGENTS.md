# Default response style

Use ASD-STE100 Simplified Technical English when explaining technical topics.

## Agent skills

### Issue tracker

Track issues and specs as local Markdown files under `.scratch/`.
Before issue operations, read `docs/agents/issue-tracker.md`.

### Triage labels

Use the default five triage labels.
Before triage, read `docs/agents/triage-labels.md`.

### Domain docs

Use a single-context layout with root `GLOSSARY.md` and `docs/adr/`.
Before code exploration, read `docs/agents/domain.md`.

## Subagent model selection

When subagents are needed, select the model and reasoning effort
from the task's complexity, uncertainty, and cost.

| Task | Model | Effort |
|---|---|---|
| Simple edits, styling, and routine fixes | gpt-6.1-sol | medium |
| Implementation, components, refactoring, and debugging | gpt-6.1-sol | high |
| Difficult debugging, state or binding problems, and complex implementation | gpt-6.1-sol | xhigh |
| Major architecture decisions and critical design or code reviews | gpt-6-astra | high |

- Default to Sol High for substantive delegated work.
- Use Astra High only when its reasoning advantage justifies the cost.
- Use Sol for implementation after architecture decisions are complete.
- Use only High effort for Astra. Never use a higher effort.
- Keep small tasks in the main agent when delegation adds no clear value.
- If the selected model or effort is unavailable, report the limitation.
  Do not silently select a more expensive option.
