---
id: 3-6
title: Stable Component Mount
category: Control Flow
priority: MEDIUM
description: Avoid rendering the same component in multiple Switch/Show branches — it unmounts and remounts, destroying all local state
---

## Problem

Rendering the same component in multiple branches of `<Switch>` or `<Show>` causes it to unmount when the active branch changes and remount fresh in the new branch. All local signal state, scroll position, and focus are lost, and `onMount`/`onCleanup` run again.

## Incorrect

```tsx
// ❌ List appears in both fallback AND Match — unmounts on every nav between branches
<Switch
  fallback={<main><List /></main>}     // branch A
>
  <Match when={!isRoot()}>
    <div class="grid">
      <List />                          // branch B — new instance!
      <Detail />
    </div>
  </Match>
</Switch>
```

## Correct

```tsx
// ✅ List is always in the same DOM position; layout changes via classList
<div
  class={styles.root}
  classList={{
    [styles.singleList]: isRoot(),
    [styles.withDetail]: !isRoot(),
  }}
>
  <List />                              {/* always mounted */}
  <Show when={!isRoot()}>
    <Detail />
  </Show>
</div>
```

```css
/* CSS for hiding without unmounting (when display:none is needed) */
.hidden { display: none; }
```

## Why It Matters

`<Switch>` and `<Show>` achieve conditional rendering by mounting and unmounting their content. The same component appearing in two branches is treated as two separate instances — one is destroyed when the other is created.

Consequences:

1. **Local state reset** — all `createSignal` values inside the component revert to their initial values
2. **Scroll position lost** — the DOM element is destroyed and recreated
3. **Lifecycle re-fires** — `onMount` runs again, potentially triggering duplicate network requests or unwanted side effects
4. **Flicker** — a visible flash as the old element unmounts and the new one mounts and paints

**When unmounting IS correct**: if you explicitly want state to reset on branch change (e.g., navigating between unrelated views), `<Switch>` and `<Show>` are the right tools. This rule applies only when you want state to **survive** the layout change.

## Related Rules

- [3-1: Use Show for Conditionals](3-1-use-show-for-conditionals.md) - When Show/Switch is the right choice
- [3-4: Use Switch/Match](3-4-use-switch-match.md) - Proper use of Switch for multiple conditions
- [6-5: Prefer classList](6-5-prefer-classlist.md) - CSS-based approach to conditional styling
