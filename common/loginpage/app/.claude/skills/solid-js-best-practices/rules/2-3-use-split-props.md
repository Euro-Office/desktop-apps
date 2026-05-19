---
id: 2-3
title: Use splitProps
category: Components
priority: HIGH
description: Use splitProps to separate prop groups safely
---

## Problem

When creating wrapper components, you often need to separate "local" props (consumed by your component) from props that should pass through to a child element. Destructuring or manual picking breaks reactivity. The `splitProps` function safely divides props while preserving reactivity.

## Incorrect

```tsx
// ❌ WRONG: Destructuring breaks reactivity
function Input({ label, error, ...inputProps }) {
  return (
    <div>
      <label>{label}</label>
      <input {...inputProps} />
      {error && <span class="error">{error}</span>}
    </div>
  );
}
```

```tsx
// ❌ WRONG: Manual picking also breaks reactivity
function Input(props) {
  const label = props.label;
  const error = props.error;
  const inputProps = {
    type: props.type,
    value: props.value,
    // Have to manually list every possible input prop...
  };

  return (
    <div>
      <label>{label}</label>
      <input {...inputProps} />
    </div>
  );
}
```

## Correct

```tsx
import { splitProps } from "solid-js";

// ✅ CORRECT: splitProps preserves reactivity
function Input(props) {
  const [local, inputProps] = splitProps(props, ["label", "error"]);

  return (
    <div>
      <label>{local.label}</label>
      <input {...inputProps} />
      <Show when={local.error}>
        <span class="error">{local.error}</span>
      </Show>
    </div>
  );
}
```

### Multiple Groups

```tsx
import { splitProps } from "solid-js";

function Button(props) {
  // Split into three groups
  const [local, styleProps, buttonProps] = splitProps(
    props,
    ["loading", "icon"],           // Local props
    ["class", "style"]             // Style-related props
  );

  return (
    <button
      class={`btn ${styleProps.class ?? ""}`}
      style={styleProps.style}
      disabled={local.loading}
      {...buttonProps}
    >
      <Show when={local.loading}>
        <Spinner />
      </Show>
      <Show when={local.icon}>
        <Icon name={local.icon} />
      </Show>
      {props.children}
    </button>
  );
}
```

### With mergeProps

```tsx
import { mergeProps, splitProps } from "solid-js";

function Input(props) {
  // Merge defaults first
  const merged = mergeProps(
    { type: "text", variant: "outlined" },
    props
  );

  // Then split
  const [local, inputProps] = splitProps(merged, ["label", "error", "variant"]);

  return (
    <div class={`input-${local.variant}`}>
      <label>{local.label}</label>
      <input {...inputProps} />
    </div>
  );
}
```

### Type-Safe splitProps

```tsx
import { Component, splitProps, JSX } from "solid-js";

interface ButtonProps extends JSX.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: "primary" | "secondary";
  size?: "small" | "medium" | "large";
  loading?: boolean;
}

const Button: Component<ButtonProps> = (props) => {
  const [local, buttonProps] = splitProps(props, [
    "variant",
    "size",
    "loading",
    "children"
  ]);

  return (
    <button
      class={`btn btn-${local.variant ?? "primary"} btn-${local.size ?? "medium"}`}
      disabled={local.loading || buttonProps.disabled}
      {...buttonProps}
    >
      <Show when={local.loading} fallback={local.children}>
        <Spinner /> Loading...
      </Show>
    </button>
  );
};
```

## Why It Matters

1. **Reactivity Preserved**: Both the local props object and the rest object remain reactive.

2. **Clean Separation**: Explicitly define which props your component consumes vs. passes through.

3. **Spread Safety**: The remaining props object is safe to spread onto child elements.

4. **Type Safety**: TypeScript correctly infers the types of split prop groups.

## Pattern Comparison

| Pattern | Reactivity | Type Safety | Use Case |
| ------- | ---------- | ----------- | -------- |
| Destructuring | ❌ Broken | ✅ Good | Never in Solid |
| Manual picking | ❌ Broken | ⚠️ Manual | Never in Solid |
| `splitProps` | ✅ Preserved | ✅ Good | Wrapper components |

## Common Split Patterns

```tsx
// Split local vs forwarded
const [local, rest] = splitProps(props, ["onChange", "label"]);

// Split by concern
const [handlers, styles, rest] = splitProps(
  props,
  ["onClick", "onFocus", "onBlur"],
  ["class", "style", "classList"]
);

// Split everything except a few
// (Solid doesn't have this built-in, but you can list what you want)
```

## Related Rules

- [2-1: Never Destructure Props](2-1-never-destructure-props.md) - Why splitting is needed
- [2-2: Use mergeProps](2-2-use-merge-props.md) - Often used together
