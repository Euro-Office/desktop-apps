---
id: 2-1
title: Never Destructure Props
category: Components
priority: CRITICAL
description: Destructuring props breaks reactivity
---

## Problem

In Solid.js, components render once. Unlike React, components are not re-executed when props change. Destructuring props captures their values at render time, breaking reactivity because subsequent prop changes won't be reflected.

## Incorrect

```tsx
// ❌ WRONG: Destructured props lose reactivity
function Greeting({ name, age }) {
  return (
    <div>
      <h1>Hello {name}</h1>
      <p>Age: {age}</p>
    </div>
  );
}

// ❌ WRONG: Destructuring in function body
function Greeting(props) {
  const { name, age } = props;  // Still breaks reactivity

  return (
    <div>
      <h1>Hello {name}</h1>
      <p>Age: {age}</p>
    </div>
  );
}
```

## Correct

```tsx
// ✅ CORRECT: Access props directly
function Greeting(props) {
  return (
    <div>
      <h1>Hello {props.name}</h1>
      <p>Age: {props.age}</p>
    </div>
  );
}
```

### Using Accessor Pattern

If you prefer shorter names, wrap in a function:

```tsx
// ✅ CORRECT: Accessor pattern preserves reactivity
function Greeting(props) {
  const name = () => props.name;
  const age = () => props.age;

  return (
    <div>
      <h1>Hello {name()}</h1>
      <p>Age: {age()}</p>
    </div>
  );
}
```

### Using mergeProps for Defaults

```tsx
import { mergeProps } from "solid-js";

// ✅ CORRECT: mergeProps preserves reactivity
function Greeting(props) {
  const merged = mergeProps({ name: "Guest", age: 0 }, props);

  return (
    <div>
      <h1>Hello {merged.name}</h1>
      <p>Age: {merged.age}</p>
    </div>
  );
}
```

### Using splitProps

```tsx
import { splitProps } from "solid-js";

// ✅ CORRECT: splitProps preserves reactivity
function Button(props) {
  const [local, others] = splitProps(props, ["onClick", "children"]);

  return (
    <button onClick={local.onClick} {...others}>
      {local.children}
    </button>
  );
}
```

## Why It Matters

Solid.js achieves its performance by running component functions only once. Reactivity happens at the signal/prop access level, not the component level. When you destructure:

```tsx
const { name } = props;  // 'name' is now a static string value
```

The variable `name` holds the value at render time. If the parent component updates the `name` prop, your component won't see the change because it never runs again.

By accessing `props.name` directly in JSX, Solid can track that dependency and update just that DOM node when the prop changes.

## Common Patterns

### Passing Props to Child Components

```tsx
// ✅ CORRECT: Spread remaining props
function Input(props) {
  const [local, others] = splitProps(props, ["label"]);

  return (
    <label>
      {local.label}
      <input {...others} />
    </label>
  );
}
```

### Conditional Props

```tsx
// ✅ CORRECT: Access in expression
function Alert(props) {
  return (
    <div class={props.type === "error" ? "alert-error" : "alert-info"}>
      {props.message}
    </div>
  );
}
```

## Related Rules

- [2-2: Use mergeProps](2-2-use-merge-props.md) - Safe way to add default props
- [2-3: Use splitProps](2-3-use-split-props.md) - Safe way to separate props
- [1-1: Use Signals Correctly](1-1-use-signals-correctly.md) - Related reactivity concept
