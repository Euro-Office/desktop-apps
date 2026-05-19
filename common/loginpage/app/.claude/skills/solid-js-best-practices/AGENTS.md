# Solid.js Best Practices - Contributor Guide

> **For AI agents:** The primary skill content is in [SKILL.md](SKILL.md), which includes the rule index, task-based rule selection, common mistakes, and React comparison tables.

This document is for contributors working on the skill itself.

## Skill File Structure

```text
solid-js-best-practices/
├── SKILL.md           # Primary skill file (start here)
├── AGENTS.md          # This file - contributor guidance
├── README.md          # Human documentation
├── CHANGELOG.md       # Version history
└── rules/             # Individual rule files (each with YAML frontmatter)
    ├── 1-*.md         # Reactivity rules
    ├── 2-*.md         # Component rules
    ├── 3-*.md         # Control flow rules
    ├── 4-*.md         # State management rules
    ├── 5-*.md         # Refs & DOM rules
    ├── 6-*.md         # Performance rules
    ├── 7-*.md         # Accessibility rules
    └── 8-*.md         # Testing rules
```

## Rule File Naming

Files follow the pattern: `[category]-[number]-[hyphenated-topic].md`

- Category 1: Reactivity
- Category 2: Components
- Category 3: Control Flow
- Category 4: State Management
- Category 5: Refs & DOM
- Category 6: Performance
- Category 7: Accessibility
- Category 8: Testing

## Rule File Structure

Each rule file should contain:

1. **YAML frontmatter** — id, title, category, priority, description
2. **Problem Section** — explanation of what goes wrong
3. **Incorrect Code Examples** — with `❌ WRONG` annotations
4. **Correct Code Examples** — with `✅ CORRECT` annotations
5. **Advanced Patterns** — multiple variations showing nuanced usage
6. **Why It Matters** — performance, maintainability, and correctness context
7. **Related Rules** — cross-references to similar or dependent rules
