---
id: 2-4
title: Use children Helper
category: Components
priority: MEDIUM
description: Use children() helper for safe children access
---

## Problem

Accessing `props.children` multiple times can cause child components to be recreated or re-evaluated each time. The `children` helper resolves and memoizes children, making them safe to access multiple times or pass to multiple locations.

## Incorrect

```tsx
function Layout(props) {
  // ❌ WRONG: Each access might re-evaluate children
  return (
    <div>
      <div class="preview">{props.children}</div>
      <div class="main">{props.children}</div>
      {/* Children rendered twice = potential duplicates or issues */}
    </div>
  );
}
```

```tsx
// ❌ WRONG: Iterating over children directly
function List(props) {
  return (
    <ul>
      {props.children.map((child, i) => (
        <li key={i}>{child}</li>
      ))}
    </ul>
  );
}
```

## Correct

```tsx
import { children } from "solid-js";

function Layout(props) {
  // ✅ CORRECT: children() resolves and memoizes
  const resolved = children(() => props.children);

  return (
    <div>
      <div class="preview">{resolved()}</div>
      <div class="main">{resolved()}</div>
    </div>
  );
}
```

### Transforming Children

```tsx
import { children, For } from "solid-js";

function List(props) {
  // ✅ CORRECT: Resolve children, then use toArray() to iterate
  const resolved = children(() => props.children);

  return (
    <ul>
      <For each={resolved.toArray()}>
        {(child) => <li>{child}</li>}
      </For>
    </ul>
  );
}
```

### Conditional Rendering Based on Children

```tsx
import { children, Show } from "solid-js";

function Card(props) {
  const header = children(() => props.header);
  const footer = children(() => props.footer);
  const content = children(() => props.children);

  return (
    <div class="card">
      <Show when={header()}>
        <div class="card-header">{header()}</div>
      </Show>

      <div class="card-body">{content()}</div>

      <Show when={footer()}>
        <div class="card-footer">{footer()}</div>
      </Show>
    </div>
  );
}

// Usage
<Card header={<h2>Title</h2>} footer={<button>Submit</button>}>
  <p>Card content here</p>
</Card>
```

### Counting and Accessing Children

```tsx
import { children, createMemo } from "solid-js";

function Tabs(props) {
  const tabs = children(() => props.children);

  const tabCount = createMemo(() => tabs.toArray().length);

  return (
    <div class="tabs">
      <div class="tab-count">
        {tabCount()} tabs
      </div>
      <div class="tab-content">
        {tabs()}
      </div>
    </div>
  );
}
```

## Why It Matters

1. **Memoization**: `children()` caches the resolved children, preventing unnecessary re-creation.

2. **Flattening**: Automatically handles fragments and arrays, giving you a flat list of elements.

3. **Safe Iteration**: Provides `toArray()` method for safely mapping over children.

4. **Reactivity**: Still reactive—updates when the actual children change.

## The children Helper API

```tsx
const resolved = children(() => props.children);

// Access as a whole
resolved()  // Returns resolved children

// Convert to array for iteration
resolved.toArray()  // Returns JSX.Element[]

// Both are reactive and can be used in JSX or effects
```

## When to Use children()

| Scenario | Use children()? |
| -------- | --------------- |
| Render children once | No, just use `{props.children}` |
| Render children multiple times | Yes |
| Iterate/map over children | Yes |
| Check if children exist | Yes |
| Transform children | Yes |
| Pass children to multiple slots | Yes |

## Related Rules

- [2-1: Never Destructure Props](2-1-never-destructure-props.md) - Related reactivity concept
- [2-5: Component Composition](2-5-component-composition.md) - Patterns using children
