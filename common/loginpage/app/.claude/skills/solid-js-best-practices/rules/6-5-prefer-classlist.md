---
id: 6-5
title: Prefer classList
category: Performance
priority: LOW
description: Use the classList prop for conditional class toggling instead of string concatenation
---

## Problem

When toggling CSS classes conditionally, developers often resort to string concatenation or template literals. Solid.js provides a `classList` prop that maps class names to boolean expressions, producing cleaner code and more efficient DOM updates. The `classList` prop calls `element.classList.toggle()` under the hood, which is more efficient than replacing the entire `className` string.

## Incorrect

```tsx
// ❌ WRONG: String concatenation for conditional classes
function Button(props) {
  return (
    <button
      class={
        "btn" +
        (props.primary ? " btn-primary" : "") +
        (props.large ? " btn-lg" : "") +
        (props.disabled ? " btn-disabled" : "")
      }
    >
      {props.children}
    </button>
  );
}
```

```tsx
// ❌ WRONG: Template literal with conditionals
function NavItem(props) {
  return (
    <a class={`nav-item ${props.active ? "active" : ""} ${props.highlighted ? "highlighted" : ""}`}>
      {props.label}
    </a>
  );
}
```

```tsx
// ❌ WRONG: Using a classnames/clsx helper library
import clsx from "clsx";

function Card(props) {
  return (
    <div class={clsx("card", { "card-elevated": props.elevated, "card-bordered": props.bordered })}>
      {props.children}
    </div>
  );
}
```

## Correct

```tsx
// ✅ CORRECT: classList for conditional toggling
function Button(props) {
  return (
    <button
      class="btn"
      classList={{
        "btn-primary": props.primary,
        "btn-lg": props.large,
        "btn-disabled": props.disabled,
      }}
    >
      {props.children}
    </button>
  );
}
```

```tsx
// ✅ CORRECT: classList with reactive conditions
function NavItem(props) {
  return (
    <a
      class="nav-item"
      classList={{
        active: props.active,
        highlighted: props.highlighted,
      }}
    >
      {props.label}
    </a>
  );
}
```

### Combining class and classList

```tsx
// ✅ CORRECT: class for static classes, classList for dynamic ones
function Card(props) {
  return (
    <div
      class="card"
      classList={{
        "card-elevated": props.elevated,
        "card-bordered": props.bordered,
        "card-loading": props.loading,
      }}
    >
      {props.children}
    </div>
  );
}
```

### Reactive Signal-Based Classes

```tsx
import { createSignal } from "solid-js";

// ✅ CORRECT: classList with signal-driven conditions
function Toggle() {
  const [active, setActive] = createSignal(false);
  const [expanded, setExpanded] = createSignal(false);

  return (
    <div
      class="toggle-container"
      classList={{
        "is-active": active(),
        "is-expanded": expanded(),
      }}
    >
      <button onClick={() => setActive(!active())}>Toggle</button>
      <button onClick={() => setExpanded(!expanded())}>Expand</button>
    </div>
  );
}
```

### Computed Class Conditions

```tsx
import { createSignal, createMemo } from "solid-js";

// ✅ CORRECT: Derived conditions in classList
function PasswordStrength(props) {
  const strength = createMemo(() => {
    const len = props.password.length;
    if (len > 12) return "strong";
    if (len > 6) return "medium";
    return "weak";
  });

  return (
    <div
      class="password-meter"
      classList={{
        "strength-weak": strength() === "weak",
        "strength-medium": strength() === "medium",
        "strength-strong": strength() === "strong",
      }}
    />
  );
}
```

## classList vs class

| Feature | `class` | `classList` |
| ------- | ------- | ----------- |
| Static classes | Yes | Not recommended |
| Conditional classes | Possible (string concat) | Yes (cleaner) |
| DOM update | Replaces entire `className` | Toggles individual classes |
| Reactive granularity | Entire string recalculated | Per-class toggle |
| Combinable | Yes, use together | Yes, use together |

## Why It Matters

1. **Readability**: `classList` is declarative — you see each class and its condition at a glance.

2. **Efficiency**: `classList.toggle()` is faster than rebuilding and setting the entire class string.

3. **Fine-Grained Updates**: Each class is toggled independently, so changing one condition doesn't touch the others.

4. **No Dependencies**: Eliminates the need for `classnames`/`clsx` helper libraries.

## Related Rules

- [2-8: Style Prop Conventions](2-8-style-prop-conventions.md) - Solid style object conventions
- [2-7: No React-Specific Props](2-7-no-react-specific-props.md) - Use `class` not `className`
