---
id: 2-6
title: Components Return Once
category: Components
priority: CRITICAL
description: Never use early returns in components — use Show, Switch, or other control flow in JSX
---

## Problem

Solid components execute their body **once** — they are not re-invoked when state changes. If you use early returns like `if (!data) return null`, the component will never re-run that check. The returned JSX is static; only the reactive expressions within it update. This is the biggest mental model difference from React, where components re-render on every state change.

## Incorrect

```tsx
// ❌ WRONG: Early return based on signal — will never re-evaluate
function UserProfile(props) {
  const [user] = createResource(() => fetchUser(props.id));

  if (user.loading) return <Spinner />;
  if (user.error) return <ErrorMessage error={user.error} />;

  return (
    <div>
      <h1>{user().name}</h1>
      <p>{user().email}</p>
    </div>
  );
}
```

```tsx
// ❌ WRONG: Guard clause that breaks reactivity
function Dashboard(props) {
  if (!props.isLoggedIn) {
    return <LoginPrompt />;
  }

  return (
    <div>
      <h1>Welcome back</h1>
      <Stats />
    </div>
  );
}
```

```tsx
// ❌ WRONG: Multiple returns based on state
function StatusBadge(props) {
  if (props.status === "error") return <span class="red">Error</span>;
  if (props.status === "loading") return <span class="gray">Loading</span>;
  return <span class="green">Ready</span>;
}
```

## Correct

### Using Show

```tsx
import { Show, createResource } from "solid-js";

// ✅ CORRECT: All conditional rendering in JSX with control flow components
function UserProfile(props) {
  const [user] = createResource(() => fetchUser(props.id));

  return (
    <Show when={!user.error} fallback={<ErrorMessage error={user.error} />}>
      <Show when={!user.loading} fallback={<Spinner />}>
        <div>
          <h1>{user().name}</h1>
          <p>{user().email}</p>
        </div>
      </Show>
    </Show>
  );
}
```

### Using Suspense and ErrorBoundary

```tsx
import { Suspense, ErrorBoundary, createResource } from "solid-js";

// ✅ CORRECT: Idiomatic Solid pattern for async data
function UserProfile(props) {
  const [user] = createResource(() => fetchUser(props.id));

  return (
    <ErrorBoundary fallback={(err) => <ErrorMessage error={err} />}>
      <Suspense fallback={<Spinner />}>
        <div>
          <h1>{user().name}</h1>
          <p>{user().email}</p>
        </div>
      </Suspense>
    </ErrorBoundary>
  );
}
```

### Using Show for Guards

```tsx
import { Show } from "solid-js";

// ✅ CORRECT: Guard condition as Show wrapper
function Dashboard(props) {
  return (
    <Show when={props.isLoggedIn} fallback={<LoginPrompt />}>
      <div>
        <h1>Welcome back</h1>
        <Stats />
      </div>
    </Show>
  );
}
```

### Using Switch/Match for Multiple States

```tsx
import { Switch, Match } from "solid-js";

// ✅ CORRECT: Switch/Match replaces multiple if-returns
function StatusBadge(props) {
  return (
    <Switch fallback={<span class="green">Ready</span>}>
      <Match when={props.status === "error"}>
        <span class="red">Error</span>
      </Match>
      <Match when={props.status === "loading"}>
        <span class="gray">Loading</span>
      </Match>
    </Switch>
  );
}
```

## Common Pitfall: Conditional Hooks

```tsx
// ❌ WRONG: Effect inside a condition that never re-evaluates
function Timer(props) {
  if (!props.enabled) return null;

  createEffect(() => {
    const id = setInterval(() => console.log("tick"), 1000);
    onCleanup(() => clearInterval(id));
  });

  return <span>Timer running</span>;
}

// ✅ CORRECT: Always return JSX, conditionally render
function Timer(props) {
  createEffect(() => {
    if (!props.enabled) return;
    const id = setInterval(() => console.log("tick"), 1000);
    onCleanup(() => clearInterval(id));
  });

  return (
    <Show when={props.enabled}>
      <span>Timer running</span>
    </Show>
  );
}
```

## Why It Matters

1. **Correctness**: Early returns produce static output that never updates when state changes.

2. **Migration Safety**: This is the single most common mistake when migrating from React to Solid.

3. **Reactivity**: Solid's reactive system only tracks signals accessed within reactive contexts (JSX expressions, effects, memos). Code in the component body runs once and is never revisited.

4. **Debugging**: Bugs from early returns are subtle — the component appears to work on first render but never updates.

## Related Rules

- [3-1: Use Show for Conditionals](3-1-use-show-for-conditionals.md) - The Show component for conditional rendering
- [3-4: Use Switch/Match](3-4-use-switch-match.md) - Multi-branch conditional rendering
- [6-3: Use Suspense](6-3-use-suspense.md) - Suspense for async loading boundaries
