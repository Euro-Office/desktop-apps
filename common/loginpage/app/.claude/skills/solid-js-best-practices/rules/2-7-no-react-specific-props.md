---
id: 2-7
title: No React-Specific Props
category: Components
priority: HIGH
description: Use class instead of className, for instead of htmlFor, and other Solid-native prop names
---

## Problem

Solid.js uses standard HTML attribute names, not React's camelCase alternatives. Using React-specific props like `className`, `htmlFor`, or `onChange` (for input tracking) results in broken styling, broken form associations, or unexpected behavior. This is one of the most common mistakes when migrating from React.

## Incorrect

```tsx
// ❌ WRONG: React-specific prop names
function LoginForm() {
  return (
    <form className="login-form">
      <label htmlFor="email">Email</label>
      <input
        id="email"
        className="input"
        onChange={(e) => setEmail(e.target.value)}
      />

      <label htmlFor="password">Password</label>
      <input
        id="password"
        type="password"
        className="input"
      />

      <button className="btn btn-primary" type="submit">
        Log In
      </button>
    </form>
  );
}
```

```tsx
// ❌ WRONG: tabIndex, encType — React camelCase conventions
<div tabIndex={0} />
<form encType="multipart/form-data" />
```

## Correct

```tsx
// ✅ CORRECT: Standard HTML attribute names
function LoginForm() {
  return (
    <form class="login-form">
      <label for="email">Email</label>
      <input
        id="email"
        class="input"
        onInput={(e) => setEmail(e.currentTarget.value)}
      />

      <label for="password">Password</label>
      <input
        id="password"
        type="password"
        class="input"
      />

      <button class="btn btn-primary" type="submit">
        Log In
      </button>
    </form>
  );
}
```

```tsx
// ✅ CORRECT: Standard HTML attributes
<div tabindex={0} />
<form enctype="multipart/form-data" />
```

## React to Solid Prop Mapping

| React Prop | Solid Prop | Notes |
| ---------- | ---------- | ----- |
| `className` | `class` | Standard HTML attribute |
| `htmlFor` | `for` | Standard HTML attribute |
| `onChange` (input) | `onInput` | `onChange` fires on blur in Solid (native behavior) |
| `tabIndex` | `tabindex` | Lowercase HTML attribute |
| `encType` | `enctype` | Lowercase HTML attribute |
| `crossOrigin` | `crossorigin` | Lowercase HTML attribute |
| `dateTime` | `datetime` | Lowercase HTML attribute |
| `readOnly` | `readonly` | Lowercase HTML attribute |
| `maxLength` | `maxlength` | Lowercase HTML attribute |

## Event Target Differences

```tsx
// ❌ WRONG: React uses e.target (synthetic events)
<input onChange={(e) => setValue(e.target.value)} />

// ✅ CORRECT: Solid uses e.currentTarget (native events, typed correctly)
<input onInput={(e) => setValue(e.currentTarget.value)} />
```

In Solid, `e.currentTarget` is typed to the element the handler is attached to, while `e.target` is typed as `EventTarget` and requires casting.

## Why It Matters

1. **Broken Styling**: `className` is silently ignored — no class is applied, so styles break with no error.

2. **Broken Forms**: `htmlFor` doesn't associate labels with inputs, hurting both UX and accessibility.

3. **Wrong Event Timing**: `onChange` fires on blur in Solid (native behavior), not on every keystroke like React. Use `onInput` for live updates.

4. **Migration Speed**: These are mechanical find-and-replace fixes, but they're easy to miss without a rule.

## Related Rules

- [2-1: Never Destructure Props](2-1-never-destructure-props.md) - Other prop handling pitfalls
- [7-1: Use Semantic HTML](7-1-semantic-html.md) - Proper HTML attribute usage
- [2-8: Style Prop Conventions](2-8-style-prop-conventions.md) - Style object differences from React
