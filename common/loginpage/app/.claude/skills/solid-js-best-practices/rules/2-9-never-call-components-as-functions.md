---
id: 2-9
title: Never Call Components as Functions
category: Components
priority: CRITICAL
description: Always use JSX or createComponent() — calling components as plain functions leaks reactive scope
---

## Problem

Calling a component as a plain function (`MyComp(props)`) instead of using JSX (`<MyComp />`) runs the component body inside the **parent's** reactive tracking scope instead of its own isolated reactive owner. This causes:

- Signals created inside the called component are owned by the parent — destroyed when the parent re-runs
- Any signal reads during initialization are tracked as parent dependencies
- When those signals change, the parent re-runs, effectively unmounting and remounting the "component"

This bug is subtle: the component appears to work on first render but silently resets state whenever the parent re-runs.

## Incorrect

```tsx
// ❌ Compiled into a reactive memo — tracks MyComp's internal signal reads as parent deps
const Layout = () => (
  <div>{override.component?.(layoutProps)}</div>
);

// ❌ Same problem in a Switch fallback
<Switch fallback={<main>{MyComp(props)}</main>}>
  ...
</Switch>
```

## Correct

```tsx
// ✅ JSX compiles to createComponent() — isolated reactive owner, no scope leak
const Layout = () => (
  <div><MyComp {...layoutProps} /></div>
);

// ✅ For conditional dynamic components, capture the reference first
const Comp = override.component;
// then render conditionally:
<Show when={Comp}>{(C) => <C {...layoutProps} />}</Show>

// ✅ Or use <Dynamic> for truly dynamic component references
<Dynamic component={override.component} {...layoutProps} />
```

## Why It Matters

JSX `<MyComp />` compiles to `createComponent(MyComp, props)`, which establishes an isolated reactive owner for the component. A plain function call `MyComp(props)` has no such isolation — the callee inherits the caller's reactive context.

Consequences of calling a component as a function:

1. **Silent state resets** — local signals inside the called component are owned by the parent and destroyed on every parent re-run
2. **Unexpected dependency tracking** — library hooks that eagerly read signals during init (e.g., TanStack Query's `queryOptions`) register those reads as parent-scope dependencies
3. **Cascading re-renders** — when the leaked dependencies change, the parent invalidates, causing a full unmount/remount cycle instead of a targeted update

## Related Rules

- [2-1: Never Destructure Props](2-1-never-destructure-props.md) - Another pattern that silently breaks reactive ownership
- [1-5: Use Untrack When Needed](1-5-use-untrack-when-needed.md) - Controlling what gets tracked in a reactive scope
