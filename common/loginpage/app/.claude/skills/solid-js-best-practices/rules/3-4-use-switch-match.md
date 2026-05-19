---
id: 3-4
title: Use Switch/Match
category: Control Flow
priority: MEDIUM
description: Use <Switch>/<Match> for multiple conditions
---

## Problem

When rendering different content based on multiple conditions, nested `<Show>` components or chained ternaries become hard to read and maintain. The `<Switch>` and `<Match>` components provide a cleaner pattern for multiple mutually exclusive conditions.

## Incorrect

```tsx
// ❌ WRONG: Nested Show components
function StatusBadge(props) {
  return (
    <Show when={props.status === "success"} fallback={
      <Show when={props.status === "error"} fallback={
        <Show when={props.status === "loading"} fallback={
          <span class="badge-unknown">Unknown</span>
        }>
          <span class="badge-loading">Loading...</span>
        </Show>
      }>
        <span class="badge-error">Error</span>
      </Show>
    }>
      <span class="badge-success">Success</span>
    </Show>
  );
}
```

```tsx
// ❌ WRONG: Chained ternaries
function StatusBadge(props) {
  return props.status === "success" ? (
    <span class="badge-success">Success</span>
  ) : props.status === "error" ? (
    <span class="badge-error">Error</span>
  ) : props.status === "loading" ? (
    <span class="badge-loading">Loading...</span>
  ) : (
    <span class="badge-unknown">Unknown</span>
  );
}
```

## Correct

```tsx
import { Switch, Match } from "solid-js";

// ✅ CORRECT: Switch/Match for multiple conditions
function StatusBadge(props) {
  return (
    <Switch fallback={<span class="badge-unknown">Unknown</span>}>
      <Match when={props.status === "success"}>
        <span class="badge-success">Success</span>
      </Match>
      <Match when={props.status === "error"}>
        <span class="badge-error">Error</span>
      </Match>
      <Match when={props.status === "loading"}>
        <span class="badge-loading">Loading...</span>
      </Match>
    </Switch>
  );
}
```

### With Value Access

```tsx
import { Switch, Match } from "solid-js";

// ✅ CORRECT: Access the matched value via callback
function UserStatus(props) {
  return (
    <Switch>
      <Match when={props.user?.isAdmin}>
        {(isAdmin) => <AdminBadge verified={isAdmin()} />}
      </Match>
      <Match when={props.user?.role}>
        {(role) => <RoleBadge role={role()} />}
      </Match>
      <Match when={props.user}>
        {(user) => <span>Welcome, {user().name}</span>}
      </Match>
    </Switch>
  );
}
```

### Order Matters

```tsx
import { Switch, Match } from "solid-js";

// ✅ CORRECT: First matching condition wins
function PricingTier(props) {
  return (
    <Switch fallback={<FreeTier />}>
      {/* Check most specific conditions first */}
      <Match when={props.plan === "enterprise"}>
        <EnterpriseTier />
      </Match>
      <Match when={props.plan === "pro"}>
        <ProTier />
      </Match>
      <Match when={props.plan === "basic"}>
        <BasicTier />
      </Match>
    </Switch>
  );
}
```

### Complex Conditions

```tsx
import { Switch, Match } from "solid-js";

function ContentRenderer(props) {
  return (
    <Switch>
      <Match when={props.loading}>
        <Skeleton />
      </Match>
      <Match when={props.error}>
        {(error) => <ErrorDisplay message={error().message} />}
      </Match>
      <Match when={props.data?.length === 0}>
        <EmptyState />
      </Match>
      <Match when={props.data}>
        {(data) => <DataList items={data()} />}
      </Match>
    </Switch>
  );
}
```

### Type-Safe Pattern Matching

```tsx
import { Switch, Match } from "solid-js";

type State =
  | { status: "idle" }
  | { status: "loading" }
  | { status: "success"; data: string }
  | { status: "error"; error: Error };

function AsyncContent(props: { state: State }) {
  return (
    <Switch>
      <Match when={props.state.status === "loading"}>
        <Spinner />
      </Match>
      <Match when={props.state.status === "error" && props.state}>
        {(state) => <Error message={(state() as { error: Error }).error.message} />}
      </Match>
      <Match when={props.state.status === "success" && props.state}>
        {(state) => <Content data={(state() as { data: string }).data} />}
      </Match>
      <Match when={props.state.status === "idle"}>
        <Placeholder />
      </Match>
    </Switch>
  );
}
```

## Why It Matters

1. **Readability**: Flat structure is easier to scan than nested conditionals.

2. **Maintainability**: Adding or removing conditions is straightforward.

3. **Explicit Fallback**: The `fallback` prop makes the default case clear.

4. **First Match Wins**: Clear precedence when conditions might overlap.

## Switch/Match Props

### Switch

| Prop | Type | Description |
| ---- | ---- | ----------- |
| `fallback` | `JSX.Element` | Rendered when no Match conditions are true |

### Match

| Prop | Type | Description |
| ---- | ---- | ----------- |
| `when` | `T \| undefined \| null \| false` | Condition to evaluate |
| `keyed` | `boolean` | Rerender when reference changes |
| `children` | `JSX.Element \| (item: T) => JSX.Element` | Content or render function |

## When to Use Switch vs Show

| Scenario | Recommended |
| -------- | ----------- |
| Simple true/false | `<Show>` |
| Two conditions (if/else) | `<Show>` with `fallback` |
| Three or more conditions | `<Switch>/<Match>` |
| Enum or union type matching | `<Switch>/<Match>` |

## Related Rules

- [3-1: Use Show for Conditionals](3-1-use-show-for-conditionals.md) - For simple conditions
- [3-5: Provide Fallbacks](3-5-provide-fallbacks.md) - Always handle default case
