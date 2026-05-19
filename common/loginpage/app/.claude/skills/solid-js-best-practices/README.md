# Solid.js Best Practices - An Agent Skill

An [Agent Skill](https://platform.claude.com/docs/en/agents-and-tools/agent-skills/overview) (for [Claude Code](https://claude.ai), but quietly becoming an industry standard), `solid-js-best-practices` enables the use of comprehensive best practices for building [Solid.js](https://www.solidjs.com/) applications and components. It is optimized for AI-assisted code generation, review, and refactoring.

> **NOTE** Generative AI was used to create this skill, with information published in the resources listed at the bottom of this README. The rules have been expanded and refined from there, based on performance being used in a personal project of mine.

## Features

- **44 actionable rules** organized into 8 categories
- **Priority levels** (CRITICAL, HIGH, MEDIUM, LOW) for focused code review
- **Before/after code examples** showing incorrect vs correct patterns
- **Detailed explanations** of why each practice matters
- **Cross-references** between related rules

## Categories

| Category | Rules | Focus Area |
| -------- | ----- | ---------- |
| [Reactivity](rules/1-1-use-signals-correctly.md) | 6 | Signals, effects, memos, batching |
| [Components](rules/2-1-never-destructure-props.md) | 8 | Props, composition, children, return-once, style |
| [Control Flow](rules/3-1-use-show-for-conditionals.md) | 5 | Show, For, Switch, Index |
| [State Management](rules/4-1-signals-vs-stores.md) | 5 | Stores, context, reconcile |
| [Refs & DOM](rules/5-1-use-refs-correctly.md) | 6 | Refs, lifecycle, directives, events, security |
| [Performance](rules/6-1-avoid-unnecessary-tracking.md) | 5 | Lazy loading, Suspense, optimization, classList |
| [Accessibility](rules/7-1-semantic-html.md) | 3 | Semantic HTML, ARIA, keyboard |
| [Testing](rules/8-1-configure-vitest-for-solid.md) | 6 | Vitest setup, render patterns, async, queries |

## Installation

### Claude Code CLI

```bash
npx skills add richardcarls/solid-js-best-practices
```

### Manual Integration

1. Clone or download this repository
2. Reference [SKILL.md](SKILL.md) for quick rule lookup
3. Load specific rules from `rules/` directory as needed

## Usage

The skill automatically activates when Claude detects tasks involving Solid.js.

### Priority Levels

Rules are categorized by impact:

| Priority | Description | Action |
| -------- | ----------- | ------ |
| CRITICAL | Major performance or correctness issues | Fix immediately |
| HIGH | Significant impact on maintainability/performance | Address in current PR |
| MEDIUM | Best practice violations | Address when touching related code |
| LOW | Style preferences and micro-optimizations | Consider during refactoring |

### For AI Agents

[SKILL.md](SKILL.md) contains the complete rule index, task-based rule selection, common mistakes table, and React-to-Solid comparison. See [AGENTS.md](AGENTS.md) for contributor guidance on the skill file structure.

## Example Prompts

```text
"Create a multiselect dropdown component"
"Help me fix accessibility issues"
"Help me refactor my app to follow best practices"
```

## Contributing

Contributions are welcome! There is also a Fork button 😁.

## Resources

- [Solid.js Documentation](https://docs.solidjs.com/)
- [Solid.js Tutorial](https://www.solidjs.com/tutorial)
- [eslint-plugin-solid](https://github.com/solidjs-community/eslint-plugin-solid) — Companion ESLint plugin for automated linting

## License

MIT
