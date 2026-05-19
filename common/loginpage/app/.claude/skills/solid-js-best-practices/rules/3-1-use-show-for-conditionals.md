---
id: 3-1
title: Use Show for Conditionals
category: Control Flow
priority: HIGH
description: Use <Show> instead of ternary operators
---

## Problem

Using JavaScript ternary operators or logical AND (`&&`) for conditional rendering works but bypasses Solid's optimizations. The `<Show>` component is optimized for conditional rendering and provides better performance and semantics.

## Incorrect

```tsx
// ❌ WRONG: Ternary operator
function UserGreeting(props) {
  return props.user
    ? <WelcomeMessage user={props.user} />
    : <LoginPrompt />;
}
```

```tsx
// ❌ WRONG: Logical AND - also has falsy value issues
function Notification(props) {
  return props.count && <Badge count={props.count} />;
  // When count is 0, this renders "0" instead of nothing!
}
```

## Correct

```tsx
import { Show } from "solid-js";

// ✅ CORRECT: Show component with fallback
function UserGreeting(props) {
  return (
    <Show when={props.user} fallback={<LoginPrompt />}>
      <WelcomeMessage user={props.user} />
    </Show>
  );
}
```

```tsx
// ✅ CORRECT: Show handles falsy values properly
function Notification(props) {
  return (
    <Show when={props.count > 0}>
      <Badge count={props.count} />
    </Show>
  );
}
```

### Accessing the Condition Value

```tsx
import { Show } from "solid-js";

// ✅ CORRECT: Use callback to access narrowed type
function UserProfile(props) {
  return (
    <Show when={props.user} fallback={<p>Loading...</p>}>
      {(user) => (
        <div>
          <h1>{user().name}</h1>
          <p>{user().email}</p>
        </div>
      )}
    </Show>
  );
}
```

### Keyed Show for Object Identity

```tsx
import { Show } from "solid-js";

// ✅ CORRECT: keyed=true rerenders when object reference changes
function ItemDetail(props) {
  return (
    <Show when={props.item} keyed>
      {(item) => <ItemView item={item} />}
    </Show>
  );
}
```

### Nested Conditions

```tsx
import { Show } from "solid-js";

function Dashboard(props) {
  return (
    <Show when={props.user} fallback={<LoginPage />}>
      {(user) => (
        <Show when={user().isAdmin} fallback={<UserDashboard user={user()} />}>
          <AdminDashboard user={user()} />
        </Show>
      )}
    </Show>
  );
}
```

## Why It Matters

1. **Optimization**: `<Show>` preserves DOM nodes and component instances when toggling. Ternaries may recreate elements.

2. **Type Narrowing**: The callback form provides a narrowed, non-null accessor for the condition value.

3. **Falsy Safety**: Properly handles `0`, `""`, and other falsy values that `&&` renders incorrectly.

4. **Readability**: Explicit `fallback` prop is clearer than ternary's `:` alternative.

5. **Keyed Updates**: The `keyed` prop gives control over when to recreate content.

## Show Props

| Prop | Type | Description |
| ---- | ---- | ----------- |
| `when` | `T \| undefined \| null \| false` | Condition to evaluate |
| `fallback` | `JSX.Element` | Rendered when `when` is falsy |
| `keyed` | `boolean` | Rerender when `when` reference changes |

## Pattern Comparison

| Pattern | Performance | Type Safe | Falsy Safe |
| ------- | ----------- | --------- | ---------- |
| `condition ? <A/> : <B/>` | ⚠️ May recreate | ❌ No narrowing | ✅ Yes |
| `condition && <A/>` | ⚠️ May recreate | ❌ No narrowing | ❌ Renders "0" |
| `<Show when={condition}>` | ✅ Optimized | ✅ With callback | ✅ Yes |

## When to Use Ternaries

Ternaries are acceptable for:

- Simple attribute values: `class={active ? "on" : "off"}`
- Inline text: `{count === 1 ? "item" : "items"}`
- Very simple toggles where performance doesn't matter

But for component rendering, prefer `<Show>`.

## Related Rules

- [3-4: Use Switch/Match](3-4-use-switch-match.md) - For multiple conditions
- [3-5: Provide Fallbacks](3-5-provide-fallbacks.md) - Always include loading states
