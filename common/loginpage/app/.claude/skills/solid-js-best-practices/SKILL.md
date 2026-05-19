---
name: solid-js-best-practices
description: "Solid.js best practices for AI-assisted code generation, code review, refactoring, and debugging reactivity issues. Use when writing Solid.js components, auditing SolidJS code, migrating from React to Solid, or fixing signals and fine-grained reactivity bugs. 52 rules across 8 categories (reactivity, components, control flow, state management, refs/DOM, performance, accessibility, testing) ranked by priority."
license: MIT
allowed-tools:
  - Read
  - Grep
  - Glob
metadata:
  topics:
    - claude-skills
    - claude-code-skill
---

# Solid.js Best Practices

Comprehensive best practices for building Solid.js applications and components, optimized for AI-assisted code generation, review, and refactoring.

## Quick Reference

### Essential Imports

```typescript
import {
  createSignal,
  createEffect,
  createMemo,
  createResource,
  onMount,
  onCleanup,
  Show,
  For,
  Switch,
  Match,
  Index,
  Suspense,
  ErrorBoundary,
  lazy,
  batch,
  untrack,
  mergeProps,
  splitProps,
  children,
} from "solid-js";

import { createStore, produce, reconcile } from "solid-js/store";
```

### Component Skeleton

```tsx
import { Component, JSX, mergeProps, splitProps } from "solid-js";

interface MyComponentProps {
  title: string;
  count?: number;
  onAction?: () => void;
  children?: JSX.Element;
}

const MyComponent: Component<MyComponentProps> = (props) => {
  // Merge default props
  const merged = mergeProps({ count: 0 }, props);

  // Split component props from passed-through props
  const [local, others] = splitProps(merged, ["title", "count", "onAction"]);

  // Local reactive state
  const [value, setValue] = createSignal("");

  // Derived/computed values
  const doubled = createMemo(() => local.count * 2);

  // Side effects
  createEffect(() => {
    console.log("Count changed:", local.count);
  });

  // Lifecycle
  onMount(() => {
    console.log("Component mounted");
  });

  onCleanup(() => {
    console.log("Component cleanup");
  });

  return (
    <div {...others}>
      <h1>{local.title}</h1>
      <p>Count: {local.count}, Doubled: {doubled()}</p>
      <input
        value={value()}
        onInput={(e) => setValue(e.currentTarget.value)}
      />
      <button onClick={local.onAction}>Action</button>
      {props.children}
    </div>
  );
};

export default MyComponent;
```

## Rules by Category

### 1. Reactivity (7 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [1-1](rules/1-1-use-signals-correctly.md) | Use Signals Correctly | CRITICAL | Always call signals as functions `count()` not `count` |
| [1-2](rules/1-2-use-memo-for-derived.md) | Use Memo for Derived Values | HIGH | Use `createMemo` for computed values, not `createEffect` |
| [1-3](rules/1-3-effects-for-side-effects.md) | Effects for Side Effects Only | HIGH | Use `createEffect` only for side effects, not derivations |
| [1-7](rules/1-7-no-primitives-in-reactive-contexts.md) | No Primitives in Reactive Contexts | HIGH | Don't call hooks or create reactive primitives inside effects or memos |
| [1-4](rules/1-4-avoid-signal-in-effect.md) | Avoid Setting Signals in Effects | MEDIUM | Setting signals in effects can cause infinite loops |
| [1-5](rules/1-5-use-untrack-when-needed.md) | Use Untrack When Needed | MEDIUM | Use `untrack()` to prevent unwanted reactive subscriptions |
| [1-6](rules/1-6-batch-signal-updates.md) | Batch Signal Updates | LOW | Use `batch()` for multiple synchronous signal updates |

### 2. Components (9 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [2-1](rules/2-1-never-destructure-props.md) | Never Destructure Props | CRITICAL | Destructuring props breaks reactivity |
| [2-6](rules/2-6-components-return-once.md) | Components Return Once | CRITICAL | Never use early returns — use `<Show>`, `<Switch>`, etc. in JSX |
| [2-9](rules/2-9-never-call-components-as-functions.md) | Never Call Components as Functions | CRITICAL | Always use JSX or `createComponent()` — direct calls leak reactive scope |
| [2-2](rules/2-2-use-merge-props.md) | Use mergeProps | HIGH | Use `mergeProps` for default prop values |
| [2-3](rules/2-3-use-split-props.md) | Use splitProps | HIGH | Use `splitProps` to separate prop groups safely |
| [2-7](rules/2-7-no-react-specific-props.md) | No React-Specific Props | HIGH | Use `class` not `className`, `for` not `htmlFor` |
| [2-4](rules/2-4-use-children-helper.md) | Use children Helper | MEDIUM | Use `children()` helper for safe children access |
| [2-5](rules/2-5-component-composition.md) | Prefer Composition | MEDIUM | Prefer composition and context over prop drilling |
| [2-8](rules/2-8-style-prop-conventions.md) | Style Prop Conventions | MEDIUM | Use object syntax with kebab-case properties for `style` |

### 3. Control Flow (6 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [3-1](rules/3-1-use-show-for-conditionals.md) | Use Show for Conditionals | HIGH | Use `<Show>` instead of ternary operators |
| [3-2](rules/3-2-use-for-for-lists.md) | Use For for Lists | HIGH | Use `<For>` for referentially-keyed list rendering |
| [3-3](rules/3-3-use-index-for-primitives.md) | Use Index for Primitives | MEDIUM | Use `<Index>` when array index matters more than identity |
| [3-4](rules/3-4-use-switch-match.md) | Use Switch/Match | MEDIUM | Use `<Switch>`/`<Match>` for multiple conditions |
| [3-6](rules/3-6-stable-component-mount.md) | Stable Component Mount | MEDIUM | Avoid rendering the same component in multiple Switch/Show branches |
| [3-5](rules/3-5-provide-fallbacks.md) | Provide Fallbacks | LOW | Always provide `fallback` props for loading states |

### 4. State Management (5 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [4-1](rules/4-1-signals-vs-stores.md) | Signals vs Stores | HIGH | Use signals for primitives, stores for nested objects |
| [4-2](rules/4-2-store-path-updates.md) | Use Store Path Syntax | HIGH | Use path syntax for granular, efficient store updates |
| [4-3](rules/4-3-use-produce-for-mutations.md) | Use produce for Mutations | MEDIUM | Use `produce` for complex mutable-style store updates |
| [4-4](rules/4-4-use-reconcile-for-data.md) | Use reconcile for Server Data | MEDIUM | Use `reconcile` when integrating server/external data |
| [4-5](rules/4-5-use-context-for-global.md) | Use Context for Global State | MEDIUM | Use Context API for cross-component shared state |

### 5. Refs & DOM (6 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [5-1](rules/5-1-use-refs-correctly.md) | Use Refs Correctly | HIGH | Use callback refs for conditional elements |
| [5-2](rules/5-2-access-dom-in-onmount.md) | Access DOM in onMount | HIGH | Access DOM elements in `onMount`, not during render |
| [5-3](rules/5-3-cleanup-with-oncleanup.md) | Cleanup with onCleanup | HIGH | Always clean up subscriptions and timers |
| [5-5](rules/5-5-avoid-innerhtml.md) | Avoid innerHTML | HIGH | Avoid `innerHTML` to prevent XSS — use JSX or `textContent` |
| [5-4](rules/5-4-use-directives.md) | Use Directives | MEDIUM | Use `use:` directives for reusable element behaviors |
| [5-6](rules/5-6-event-handler-patterns.md) | Event Handler Patterns | MEDIUM | Use `on:`/`oncapture:` namespaces and array handler syntax correctly |

### 6. Performance (5 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [6-1](rules/6-1-avoid-unnecessary-tracking.md) | Avoid Unnecessary Tracking | HIGH | Don't access signals outside reactive contexts |
| [6-2](rules/6-2-use-lazy-components.md) | Use Lazy Components | MEDIUM | Use `lazy()` for code splitting large components |
| [6-3](rules/6-3-use-suspense.md) | Use Suspense | MEDIUM | Use `<Suspense>` for async loading boundaries |
| [6-4](rules/6-4-optimize-store-access.md) | Optimize Store Access | LOW | Access only the store properties you need |
| [6-5](rules/6-5-prefer-classlist.md) | Prefer classList | LOW | Use `classList` prop for conditional class toggling |

### 7. Accessibility (3 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [7-1](rules/7-1-semantic-html.md) | Use Semantic HTML | HIGH | Use appropriate semantic HTML elements |
| [7-2](rules/7-2-aria-attributes.md) | Use ARIA Attributes | MEDIUM | Apply appropriate ARIA attributes for custom controls |
| [7-3](rules/7-3-keyboard-navigation.md) | Support Keyboard Navigation | MEDIUM | Ensure all interactive elements are keyboard accessible |

### 8. Testing (11 rules)

| # | Rule | Priority | Description |
| - | ---- | -------- | ----------- |
| [8-1](rules/8-1-configure-vitest-for-solid.md) | Configure Vitest for Solid | CRITICAL | Configure Vitest with Solid-specific resolve conditions and plugin |
| [8-2](rules/8-2-wrap-render-in-arrow.md) | Wrap Render in Arrow Functions | CRITICAL | Always use `render(() => <C />)` not `render(<C />)` |
| [8-3](rules/8-3-test-primitives-in-root.md) | Test Primitives in a Root | HIGH | Wrap signal/effect/memo tests in `createRoot` or `renderHook` |
| [8-4](rules/8-4-handle-async-in-tests.md) | Handle Async in Tests | HIGH | Use `findBy` queries and proper timer config for async behavior |
| [8-5](rules/8-5-use-accessible-queries.md) | Use Accessible Queries | MEDIUM | Prefer role and label queries over test IDs |
| [8-6](rules/8-6-separate-logic-from-ui-tests.md) | Separate Logic from UI Tests | MEDIUM | Test primitives/hooks independently from component rendering |
| [8-7](rules/8-7-browser-mode-for-web-components-and-pwa-apis.md) | Browser Mode for Web Components and PWA APIs | HIGH | Use Vitest browser mode (real Chromium) for custom elements, shadow DOM, and browser-native APIs |
| [8-8](rules/8-8-testing-headless-ui-libraries.md) | Testing Headless UI Libraries with Non-Standard ARIA | MEDIUM | Headless UI libraries use non-obvious ARIA structures and portals — inspect the actual tree before querying |
| [8-9](rules/8-9-browser-native-api-test-isolation.md) | Browser-Native API Test Isolation | HIGH | Clear IndexedDB and localStorage between tests — close connection before deleteDatabase |
| [8-10](rules/8-10-router-integration-testing.md) | Router Integration Testing | HIGH | Use MemoryRouter `root` prop to provide router context to layout providers |
| [8-11](rules/8-11-tanstack-query-test-setup.md) | TanStack Query Test Setup | HIGH | Create a fresh QueryClient per test with retry and caching disabled |

## Task-Based Rule Selection

### Writing New Components

Load these rules when creating new Solid.js components:

| Rule | Why |
| ---- | --- |
| [1-1](rules/1-1-use-signals-correctly.md) | Ensure signals are called as functions |
| [2-1](rules/2-1-never-destructure-props.md) | Prevent reactivity breakage |
| [2-6](rules/2-6-components-return-once.md) | No early returns — use control flow in JSX |
| [2-9](rules/2-9-never-call-components-as-functions.md) | Never call components as plain functions |
| [2-2](rules/2-2-use-merge-props.md) | Handle default props correctly |
| [2-3](rules/2-3-use-split-props.md) | Separate local and forwarded props |
| [3-1](rules/3-1-use-show-for-conditionals.md) | Proper conditional rendering |
| [3-2](rules/3-2-use-for-for-lists.md) | Efficient list rendering |
| [5-3](rules/5-3-cleanup-with-oncleanup.md) | Prevent memory leaks |

### Code Review

Focus on these rules during code review:

| Priority | Rules |
| -------- | ----- |
| CRITICAL | [1-1](rules/1-1-use-signals-correctly.md), [2-1](rules/2-1-never-destructure-props.md), [2-6](rules/2-6-components-return-once.md), [2-9](rules/2-9-never-call-components-as-functions.md) |
| HIGH | [1-2](rules/1-2-use-memo-for-derived.md), [1-3](rules/1-3-effects-for-side-effects.md), [1-7](rules/1-7-no-primitives-in-reactive-contexts.md), [2-7](rules/2-7-no-react-specific-props.md), [5-2](rules/5-2-access-dom-in-onmount.md), [5-3](rules/5-3-cleanup-with-oncleanup.md), [5-5](rules/5-5-avoid-innerhtml.md) |

### Performance Optimization

Load these rules when optimizing performance:

| Rule | Focus |
| ---- | ----- |
| [1-2](rules/1-2-use-memo-for-derived.md) | Prevent unnecessary recomputation |
| [1-6](rules/1-6-batch-signal-updates.md) | Reduce update cycles |
| [4-2](rules/4-2-store-path-updates.md) | Granular store updates |
| [6-1](rules/6-1-avoid-unnecessary-tracking.md) | Prevent unwanted subscriptions |
| [6-2](rules/6-2-use-lazy-components.md) | Code splitting |
| [6-4](rules/6-4-optimize-store-access.md) | Efficient store access |

### State Management

Load these rules when working with application state:

| Rule | Focus |
| ---- | ----- |
| [4-1](rules/4-1-signals-vs-stores.md) | Choose the right primitive |
| [4-2](rules/4-2-store-path-updates.md) | Efficient updates |
| [4-3](rules/4-3-use-produce-for-mutations.md) | Complex mutations |
| [4-4](rules/4-4-use-reconcile-for-data.md) | External data integration |
| [4-5](rules/4-5-use-context-for-global.md) | Cross-component state |

### Accessibility Audit

Load these rules when auditing accessibility:

| Rule | Focus |
| ---- | ----- |
| [7-1](rules/7-1-semantic-html.md) | Semantic structure |
| [7-2](rules/7-2-aria-attributes.md) | Screen reader support |
| [7-3](rules/7-3-keyboard-navigation.md) | Keyboard users |

### Writing Tests

Load these rules when writing or reviewing tests:

| Rule | Focus |
| ---- | ----- |
| [8-1](rules/8-1-configure-vitest-for-solid.md) | Correct Vitest configuration |
| [8-2](rules/8-2-wrap-render-in-arrow.md) | Reactive render scope |
| [8-3](rules/8-3-test-primitives-in-root.md) | Reactive ownership for primitives |
| [8-4](rules/8-4-handle-async-in-tests.md) | Async queries and timers |
| [8-5](rules/8-5-use-accessible-queries.md) | Accessible query selection |
| [8-6](rules/8-6-separate-logic-from-ui-tests.md) | Test architecture |
| [8-7](rules/8-7-browser-mode-for-web-components-and-pwa-apis.md) | When to use browser mode vs jsdom |
| [8-8](rules/8-8-testing-headless-ui-libraries.md) | Portals and non-standard ARIA structures |
| [8-9](rules/8-9-browser-native-api-test-isolation.md) | IDB and localStorage cleanup patterns |
| [8-10](rules/8-10-router-integration-testing.md) | MemoryRouter setup for integration tests |
| [8-11](rules/8-11-tanstack-query-test-setup.md) | QueryClient configuration for tests |

## Common Mistakes to Catch

| Mistake | Rule | Solution |
| ------- | ---- | -------- |
| Forgetting `()` on signal access | [1-1](rules/1-1-use-signals-correctly.md) | Always call signals: `count()` |
| Destructuring props | [2-1](rules/2-1-never-destructure-props.md) | Access via `props.name` |
| Using ternaries for conditionals | [3-1](rules/3-1-use-show-for-conditionals.md) | Use `<Show>` component |
| `.map()` for lists | [3-2](rules/3-2-use-for-for-lists.md) | Use `<For>` component |
| Deriving values in effects | [1-2](rules/1-2-use-memo-for-derived.md) | Use `createMemo` |
| Setting signals in effects | [1-4](rules/1-4-avoid-signal-in-effect.md) | Use `createMemo` or external triggers |
| Accessing DOM during render | [5-2](rules/5-2-access-dom-in-onmount.md) | Use `onMount` |
| Forgetting cleanup | [5-3](rules/5-3-cleanup-with-oncleanup.md) | Use `onCleanup` |
| Early returns in components | [2-6](rules/2-6-components-return-once.md) | Use `<Show>`, `<Switch>` in JSX instead |
| Using `className` or `htmlFor` | [2-7](rules/2-7-no-react-specific-props.md) | Use `class` and `for` (standard HTML) |
| `style="color: red"` or camelCase styles | [2-8](rules/2-8-style-prop-conventions.md) | Use `style={{ color: "red" }}` with kebab-case |
| Using `innerHTML` with user data | [5-5](rules/5-5-avoid-innerhtml.md) | Use JSX or sanitize with DOMPurify |
| Spreading whole store | [6-4](rules/6-4-optimize-store-access.md) | Access specific properties |
| String concatenation for class toggling | [6-5](rules/6-5-prefer-classlist.md) | Use `classList={{ active: isActive() }}` |
| `render(<Comp />)` without arrow | [8-2](rules/8-2-wrap-render-in-arrow.md) | Use `render(() => <Comp />)` |
| Effects in tests without owner | [8-3](rules/8-3-test-primitives-in-root.md) | Wrap in `createRoot` or use `renderHook` |
| `getBy` for async content | [8-4](rules/8-4-handle-async-in-tests.md) | Use `findBy` queries |
| `MyComp(props)` instead of `<MyComp />` | [2-9](rules/2-9-never-call-components-as-functions.md) | Always use JSX syntax or `createComponent()` |
| Calling `useMatch()`/`useQuery()` inside `createEffect`/`createComputed` | [1-7](rules/1-7-no-primitives-in-reactive-contexts.md) | Call hooks once at component init, not inside reactive computations |
| Same component in Switch fallback and Match branch | [3-6](rules/3-6-stable-component-mount.md) | Keep component in one stable position; use CSS for layout changes |
| Custom elements don't upgrade / lifecycle doesn't fire in tests | [8-7](rules/8-7-browser-mode-for-web-components-and-pwa-apis.md) | Use Vitest browser mode (real Chromium) instead of jsdom |
| IDB state persists between tests causing order-dependent failures | [8-9](rules/8-9-browser-native-api-test-isolation.md) | Close connection before `deleteDatabase`; use `useCleanDb()` |
| Router primitives throw "can only be used inside a Route" | [8-10](rules/8-10-router-integration-testing.md) | Use MemoryRouter `root` prop with a layout factory |
| QueryClient retries mask errors / cache leaks between tests | [8-11](rules/8-11-tanstack-query-test-setup.md) | Use `makeTestQueryClient()` with `retry: false`, `gcTime: 0` |
| `waitFor(length === 0)` passes before data loads | [8-4](rules/8-4-handle-async-in-tests.md) | Use a settled anchor with `findBy` before asserting absence |
| `getByRole('form')` throws even though the form exists | [7-2](rules/7-2-aria-attributes.md) | Add `aria-label` or `aria-labelledby` to expose `role="form"` |

## Solid.js vs React Mental Model

When helping users familiar with React, keep these differences in mind:

| React | Solid.js |
| ----- | -------- |
| Components re-render on state change | Components run once, signals update DOM directly |
| `useState` returns `[value, setter]` | `createSignal` returns `[getter, setter]` |
| `useMemo` with deps array | `createMemo` with automatic tracking |
| `useEffect(fn, [deps])` | `createEffect(fn)` (no deps array — automatic tracking) |
| Destructure props freely | Never destructure props |
| Early returns (`if (!x) return null`) | `<Show>` / `<Switch>` in JSX (components return once) |
| `{condition && <Component />}` | `<Show when={condition}><Component /></Show>` |
| `{items.map(item => ...)}` | `<For each={items}>{item => ...}</For>` |
| `className` | `class` |
| `htmlFor` | `for` |
| `style={{ fontSize: 14 }}` | `style={{ "font-size": "14px" }}` |
| Context requires `useContext` hook | Context works with `useContext` or direct access |

## Priority Levels

- **CRITICAL**: Fix immediately. Causes bugs, broken reactivity, or runtime errors.
- **HIGH**: Address in code reviews. Important for correctness and maintainability.
- **MEDIUM**: Apply when relevant. Improves code quality and performance.
- **LOW**: Consider during refactoring. Nice-to-have optimizations.

## Key Solid.js Concepts

### Fine-Grained Reactivity

Solid.js updates only the specific DOM elements that depend on changed data, not entire component trees. This is achieved through:

- **Signals**: Reactive primitives that track dependencies
- **Effects**: Side effects that automatically re-run when dependencies change
- **Memos**: Cached derived values that only recompute when dependencies change

### Components Render Once

Unlike React, Solid components are functions that run once during initial render. Reactivity happens at the signal level, not the component level. This is why:

- Props must not be destructured (would capture static values)
- Signals must be called as functions (to maintain reactive tracking)
- Control flow uses special components (`<Show>`, `<For>`) instead of JS expressions

### Stores for Complex State

For nested objects and arrays, Solid provides stores with:

- Fine-grained updates via path syntax
- Automatic proxy wrapping for nested reactivity
- Utilities like `produce` and `reconcile` for common patterns

## Tooling

For automated linting alongside these best practices, use [eslint-plugin-solid](https://github.com/solidjs-community/eslint-plugin-solid). The plugin catches many of the same issues this skill covers (destructured props, early returns, React-specific props, innerHTML usage, style prop format, etc.) and provides auto-fixable rules.

## Resources

- [Solid.js Documentation](https://docs.solidjs.com/)
