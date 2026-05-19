---
id: 6-4
title: Optimize Store Access
category: Performance
priority: LOW
description: Access only the store properties you need
---

## Problem

Solid stores provide fine-grained reactivity at the property level. When you access more properties than needed, components subscribe to all of them, causing unnecessary re-renders when any of those properties change.

## Incorrect

```tsx
import { createStore } from "solid-js/store";
import { For } from "solid-js";

// ❌ WRONG: Spreading entire store
function UserCard(props) {
  // Every property of user is tracked
  return <Card {...props.user} />;
}

function UserList() {
  const [state] = createStore({
    users: [
      { id: 1, name: "Alice", email: "alice@example.com", lastLogin: "..." },
      { id: 2, name: "Bob", email: "bob@example.com", lastLogin: "..." }
    ]
  });

  return (
    <For each={state.users}>
      {(user) => <UserCard user={user} />}
    </For>
  );
}
```

```tsx
// ❌ WRONG: Accessing unused properties
function UserName(props) {
  const user = props.user;

  // Even though we only display name,
  // we've accessed the entire user object
  console.log(user);  // Tracks all properties

  return <span>{user.name}</span>;
}
```

## Correct

```tsx
import { createStore } from "solid-js/store";
import { For } from "solid-js";

// ✅ CORRECT: Access only needed properties
function UserCard(props) {
  return (
    <div class="card">
      <h3>{props.user.name}</h3>
      <p>{props.user.email}</p>
      {/* lastLogin not accessed = not tracked */}
    </div>
  );
}

function UserList() {
  const [state] = createStore({
    users: [
      { id: 1, name: "Alice", email: "alice@example.com", lastLogin: "..." },
      { id: 2, name: "Bob", email: "bob@example.com", lastLogin: "..." }
    ]
  });

  return (
    <For each={state.users}>
      {(user) => <UserCard user={user} />}
    </For>
  );
}
```

```tsx
// ✅ CORRECT: Logging without creating subscription
import { untrack } from "solid-js";

function UserName(props) {
  // Debug log without tracking
  console.log(untrack(() => props.user));

  // Only name is tracked
  return <span>{props.user.name}</span>;
}
```

### Passing Specific Properties

```tsx
// ✅ CORRECT: Pass only what child needs
function ParentComponent() {
  const [state] = createStore({
    user: {
      profile: { name: "Alice", avatar: "..." },
      settings: { theme: "dark", notifications: true },
      activity: { lastLogin: "...", actions: [] }
    }
  });

  return (
    <>
      {/* AvatarDisplay only re-renders when avatar changes */}
      <AvatarDisplay
        name={state.user.profile.name}
        avatar={state.user.profile.avatar}
      />

      {/* SettingsPanel only re-renders when settings change */}
      <SettingsPanel settings={state.user.settings} />
    </>
  );
}
```

### Memoizing Derived Data

```tsx
import { createStore } from "solid-js/store";
import { createMemo, For } from "solid-js";

function FilteredUserList() {
  const [state] = createStore({
    users: [...],
    filter: ""
  });

  // ✅ CORRECT: Memo tracks only users and filter
  const filteredUsers = createMemo(() =>
    state.users.filter(user =>
      user.name.toLowerCase().includes(state.filter.toLowerCase())
    )
  );

  return (
    <For each={filteredUsers()}>
      {(user) => <UserRow name={user.name} />}
    </For>
  );
}
```

### Using Selectors

```tsx
import { createStore } from "solid-js/store";
import { createMemo } from "solid-js";

function Dashboard() {
  const [state] = createStore({
    metrics: {
      daily: { visits: 100, sales: 50, revenue: 1000 },
      weekly: { visits: 700, sales: 350, revenue: 7000 },
      monthly: { visits: 3000, sales: 1500, revenue: 30000 }
    },
    period: "daily"
  });

  // ✅ CORRECT: Selector memo for current period
  const currentMetrics = createMemo(() => state.metrics[state.period]);

  return (
    <div>
      <MetricsCard
        visits={currentMetrics().visits}
        sales={currentMetrics().sales}
      />
    </div>
  );
}
```

## Tracking Rules

| Access Pattern | Creates Subscription |
| -------------- | ------------------- |
| `store.prop` in JSX | Yes, to `prop` |
| `store.nested.prop` in JSX | Yes, to `nested.prop` |
| `{...store}` spread | Yes, to all current properties |
| `console.log(store)` | No (not in reactive context) |
| `untrack(() => store.prop)` | No |

## Why It Matters

1. **Fewer Re-renders**: Components only update for properties they actually use.

2. **Better Performance**: Less work for the reactive system to track.

3. **Predictable Updates**: Easier to understand what triggers re-renders.

4. **Memory Efficiency**: Fewer subscriptions = less memory overhead.

## Debugging Store Access

```tsx
// See what's being tracked
import { createEffect } from "solid-js";

createEffect(() => {
  console.log("This effect tracks:", {
    name: state.user.name,  // Tracked
    // email: state.user.email  // Not tracked (commented)
  });
});
```

## Related Rules

- [4-1: Signals vs Stores](4-1-signals-vs-stores.md) - When to use stores
- [4-2: Store Path Updates](4-2-store-path-updates.md) - Efficient updates
- [1-5: Use Untrack When Needed](1-5-use-untrack-when-needed.md) - Preventing tracking
