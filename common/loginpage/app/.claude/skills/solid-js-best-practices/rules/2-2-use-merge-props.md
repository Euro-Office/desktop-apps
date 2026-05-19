---
id: 2-2
title: Use mergeProps
category: Components
priority: HIGH
description: Use mergeProps for default prop values
---

## Problem

When setting default values for props, you can't use destructuring with defaults (e.g., `{ name = "Guest" }`) because destructuring breaks reactivity. The `mergeProps` function lets you provide defaults while preserving reactivity.

## Incorrect

```tsx
// ❌ WRONG: Destructuring with defaults breaks reactivity
function Greeting({ name = "Guest", age = 0 }) {
  return <p>Hello {name}, you are {age} years old</p>;
}
```

```tsx
// ❌ WRONG: Conditional assignment also breaks reactivity
function Greeting(props) {
  const name = props.name || "Guest";  // Captured once
  const age = props.age ?? 0;  // Captured once

  return <p>Hello {name}, you are {age} years old</p>;
}
```

## Correct

```tsx
import { mergeProps } from "solid-js";

// ✅ CORRECT: mergeProps preserves reactivity
function Greeting(props) {
  const merged = mergeProps({ name: "Guest", age: 0 }, props);

  return <p>Hello {merged.name}, you are {merged.age} years old</p>;
}
```

### With Multiple Sources

```tsx
import { mergeProps } from "solid-js";

function Button(props) {
  const defaultProps = { variant: "primary", size: "medium" };
  const themeProps = { variant: "secondary" };  // From theme context

  // Later sources override earlier ones
  const merged = mergeProps(defaultProps, themeProps, props);

  return (
    <button class={`btn-${merged.variant} btn-${merged.size}`}>
      {merged.children}
    </button>
  );
}
```

### Typed Props with Defaults

```tsx
import { Component, mergeProps } from "solid-js";

interface CardProps {
  title: string;
  subtitle?: string;
  elevation?: number;
  children?: JSX.Element;
}

const Card: Component<CardProps> = (props) => {
  const merged = mergeProps(
    { subtitle: "", elevation: 1 },
    props
  );

  return (
    <div class="card" style={{ "box-shadow": `0 ${merged.elevation * 2}px ${merged.elevation * 4}px rgba(0,0,0,0.1)` }}>
      <h2>{merged.title}</h2>
      <Show when={merged.subtitle}>
        <h3>{merged.subtitle}</h3>
      </Show>
      {merged.children}
    </div>
  );
};
```

### Combining with splitProps

```tsx
import { mergeProps, splitProps } from "solid-js";

function Input(props) {
  // First merge defaults
  const merged = mergeProps(
    { type: "text", placeholder: "" },
    props
  );

  // Then split local from pass-through props
  const [local, others] = splitProps(merged, ["label", "error"]);

  return (
    <div class="input-wrapper">
      <Show when={local.label}>
        <label>{local.label}</label>
      </Show>
      <input {...others} />
      <Show when={local.error}>
        <span class="error">{local.error}</span>
      </Show>
    </div>
  );
}
```

## Why It Matters

1. **Reactivity Preserved**: `mergeProps` returns a reactive object that updates when source props change.

2. **Proper Defaults**: You get the default value when the prop is `undefined`, but still react to updates.

3. **Type Safety**: Works well with TypeScript—optional props with defaults are properly typed.

4. **Composability**: Can merge multiple sources (defaults, theme, props) in order.

## How mergeProps Works

```tsx
// Conceptually similar to:
const merged = new Proxy({}, {
  get(_, key) {
    // Check sources in reverse order (last wins)
    for (let i = sources.length - 1; i >= 0; i--) {
      if (key in sources[i]) return sources[i][key];
    }
  }
});
```

The returned object is reactive because property access goes through the proxy, which accesses the original reactive props.

## Related Rules

- [2-1: Never Destructure Props](2-1-never-destructure-props.md) - Why defaults via destructuring fail
- [2-3: Use splitProps](2-3-use-split-props.md) - Often used together with mergeProps
