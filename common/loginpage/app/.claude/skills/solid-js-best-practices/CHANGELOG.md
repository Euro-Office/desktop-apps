# Changelog

All notable changes to this skill will be documented in this file.

## [1.5.0] - Integration testing rules

### Added

- 5 new rules for integration testing patterns from real browser-mode test experience
  - 8-7: Browser Mode for Web Components and PWA APIs (HIGH) - When jsdom fails (custom element lifecycle, shadow DOM, IDB, crypto); `userEvent` from `vitest/browser`
  - 8-8: Testing Headless UI Libraries with Non-Standard ARIA (MEDIUM) - Portal content, non-obvious roles (Kobalte example), when `.querySelector()` is acceptable
  - 8-9: Browser-Native API Test Isolation (HIGH) - Close IDB connection before `deleteDatabase`; `onblocked` no-op pattern; `useCleanDb()` helper
  - 8-10: Router Integration Testing (HIGH) - `MemoryRouter` `root` prop pattern; `makeLayout()` factory; `renderWithProviders`/`renderRoute` helpers
  - 8-11: TanStack Query Test Setup (HIGH) - `makeTestQueryClient()` with `retry: false`, `staleTime: 0`, `gcTime: 0`
- Vitest workspace config section in rule 8-1: `defineWorkspace` split for unit (jsdom) and integration (browser/Chromium) projects, `as any` plugin cast, `headless: true` for CI
- "Testing Empty and Absent States" section in rule 8-4: settled anchor pattern for asserting absence after async load
- "Form Accessible Name" section and table row in rule 7-2: `<form>` requires `aria-label`/`aria-labelledby` to be exposed as `role="form"`
- 6 new entries in Common Mistakes table

### Changed

- Rule count updated from 47 to 52
- Testing category expanded from 6 to 11 rules
- "Writing Tests" task-based rule selection updated with 8-7 through 8-11

## [1.4.0] - Reactive scope and mount stability rules

### Added

- 3 new rules derived from real-world reactive scope bugs
  - 2-9: Never Call Components as Functions (CRITICAL) - JSX/`createComponent()` only; direct calls leak reactive scope
  - 1-7: No Primitives in Reactive Contexts (HIGH) - Don't call hooks or create reactive primitives inside effects/memos
  - 3-6: Stable Component Mount (MEDIUM) - Avoid same component in multiple Switch/Show branches; use CSS for layout changes
- 3 new entries in Common Mistakes table

### Changed

- Rule count updated from 44 to 47
- Components category expanded from 8 to 9 rules
- Reactivity category expanded from 6 to 7 rules
- Control Flow category expanded from 5 to 6 rules
- Updated Code Review task-based selection with new CRITICAL (2-9) and HIGH (1-7) rules
- Updated Writing New Components task-based selection with 2-9

## [1.3.0] - ESLint plugin gap analysis

### Added

- 6 new rules based on [eslint-plugin-solid](https://github.com/solidjs-community/eslint-plugin-solid) gap analysis
  - 2-6: Components Return Once (CRITICAL) - Never use early returns; use `<Show>`, `<Switch>` in JSX
  - 2-7: No React-Specific Props (HIGH) - Use `class` not `className`, `for` not `htmlFor`
  - 2-8: Style Prop Conventions (MEDIUM) - Object syntax with kebab-case and string values
  - 5-5: Avoid innerHTML (HIGH) - Prevent XSS; use JSX or sanitize with DOMPurify
  - 5-6: Event Handler Patterns (MEDIUM) - Delegated vs native events, `on:`/`oncapture:`, array handlers
  - 6-5: Prefer classList (LOW) - Use `classList` prop for conditional class toggling
- Tooling section in SKILL.md recommending `eslint-plugin-solid`
- 5 new entries in Common Mistakes table
- `eslint-plugin-solid` link in README.md Resources section

### Changed

- Rule count updated from 38 to 44
- Components category expanded from 5 to 8 rules
- Refs & DOM category expanded from 4 to 6 rules
- Performance category expanded from 4 to 5 rules
- Expanded React vs Solid comparison table with `className`/`class`, `htmlFor`/`for`, style syntax, and early returns
- Updated Code Review task-based selection with new CRITICAL and HIGH rules
- Updated Writing New Components task-based selection with 2-6

## [1.2.0] - Testing category

### Added

- New category 8: Testing, with 6 rules (8-1 through 8-6)
  - 8-1: Configure Vitest for Solid (CRITICAL) - Vitest config with resolve conditions, plugin, deps
  - 8-2: Wrap Render in Arrow Functions (CRITICAL) - `render(() => <C />)` not `render(<C />)`
  - 8-3: Test Primitives in a Root (HIGH) - `createRoot`/`renderHook` for reactive ownership
  - 8-4: Handle Async in Tests (HIGH) - `findBy` queries, fake timer config, portal queries
  - 8-5: Use Accessible Queries (MEDIUM) - Role/label query priority over test IDs
  - 8-6: Separate Logic from UI Tests (MEDIUM) - `renderHook`/`renderDirective` for isolated testing
- "Writing Tests" task-based rule selection in SKILL.md
- 3 testing-related entries in Common Mistakes table

### Changed

- Rule count updated from 32 to 38, categories from 7 to 8
- Updated SKILL.md frontmatter description
- Updated README.md features, categories table, and TODO checklist
- Updated AGENTS.md file structure and category list

## [1.1.0] - Skill structure improvements

### Changed

- Fixed SKILL.md frontmatter to use only valid Claude Code fields (`name`, `description`, `allowed-tools`)
- Expanded description with discovery-relevant phrases for better skill activation
- Capitalized tool names to match Claude Code identifiers (`Read`, `Grep`, `Glob`)
- Merged task-based rule selection, common mistakes table, and React comparison from AGENTS.md into SKILL.md
- Trimmed AGENTS.md to contributor-focused content only
- Added YAML frontmatter (id, title, category, priority, description) to all 32 rule files
- Updated README.md to reflect new file structure

### Removed

- Removed invalid frontmatter fields (`trigger_phrases`, `requirements`, `keywords`, `license`, `author`, `version`)
- Removed inline `# Title` and `**Priority:** X` lines from rule files (now in frontmatter)
- Deleted `rules/index.json` (metadata now lives in each rule file's frontmatter)
- Deleted `metadata.json` (duplicated frontmatter)
- Deleted `examples.md` (trivial duplicates of rule file content)
- Deleted `reference.md` (stub with placeholder text)
- Deleted empty `scripts/` directory
- Deleted empty `.github/` directory

## [1.0.0] - Initial release

- Initial set of 32 rules and documentation
