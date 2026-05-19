---
id: 1-3
title: Effects for Side Effects Only
category: Reactivity
priority: HIGH
description: Use createEffect only for side effects, not derivations
---

## Problem

`createEffect` is designed for side effects—operations that interact with the outside world like API calls, DOM manipulation, logging, or subscriptions. Using effects to derive or synchronize state is an anti-pattern that leads to unnecessary updates and potential bugs.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

function UserProfile() {
  const [firstName, setFirstName] = createSignal("John");
  const [lastName, setLastName] = createSignal("Doe");
  const [fullName, setFullName] = createSignal("");

  // ❌ WRONG: Using effect to derive state
  createEffect(() => {
    setFullName(`${firstName()} ${lastName()}`);
  });

  return <h1>{fullName()}</h1>;
}
```

```tsx
// ❌ WRONG: Effect to transform data
function ItemList() {
  const [items, setItems] = createSignal([]);
  const [sortedItems, setSortedItems] = createSignal([]);

  createEffect(() => {
    setSortedItems([...items()].sort((a, b) => a.name.localeCompare(b.name)));
  });

  return <For each={sortedItems()}>{item => <div>{item.name}</div>}</For>;
}
```

## Correct

```tsx
import { createSignal, createMemo } from "solid-js";

function UserProfile() {
  const [firstName, setFirstName] = createSignal("John");
  const [lastName, setLastName] = createSignal("Doe");

  // ✅ CORRECT: Memo for derived values
  const fullName = createMemo(() => `${firstName()} ${lastName()}`);

  return <h1>{fullName()}</h1>;
}
```

```tsx
// ✅ CORRECT: Memo for transformed data
function ItemList() {
  const [items, setItems] = createSignal([]);

  const sortedItems = createMemo(() =>
    [...items()].sort((a, b) => a.name.localeCompare(b.name))
  );

  return <For each={sortedItems()}>{item => <div>{item.name}</div>}</For>;
}
```

### Valid Uses of createEffect

```tsx
import { createSignal, createEffect, onCleanup } from "solid-js";

function DataLogger() {
  const [data, setData] = createSignal(null);

  // ✅ CORRECT: Logging is a side effect
  createEffect(() => {
    console.log("Data changed:", data());
  });

  // ✅ CORRECT: API call is a side effect
  createEffect(() => {
    fetch(`/api/data/${data()?.id}`)
      .then(res => res.json())
      .then(result => console.log(result));
  });

  // ✅ CORRECT: DOM manipulation is a side effect
  createEffect(() => {
    document.title = `Count: ${count()}`;
  });

  // ✅ CORRECT: Subscriptions are side effects
  createEffect(() => {
    const unsubscribe = eventEmitter.subscribe(data());
    onCleanup(() => unsubscribe());
  });

  return <div>{JSON.stringify(data())}</div>;
}
```

### Using onMount for One-Time Effects

```tsx
import { onMount } from "solid-js";

function Dashboard() {
  const [data, setData] = createSignal(null);

  // ✅ CORRECT: One-time initialization
  onMount(async () => {
    const response = await fetch("/api/dashboard");
    setData(await response.json());
  });

  return <Show when={data()}>{d => <DashboardView data={d} />}</Show>;
}
```

## Why It Matters

1. **Double Updates**: Using effects to sync state causes an extra render cycle. The effect runs after the initial render, then sets state, causing another update.

2. **Glitches**: There's a brief moment where the derived value is stale or uninitialized.

3. **Complexity**: Managing synchronized state through effects requires more code and is harder to reason about.

4. **Performance**: Memos are synchronous and computed during the reactive graph update. Effects are deferred and create additional work.

## Decision Tree

```text
Need a value based on other reactive values?
├─ Yes → Use createMemo
└─ No → Need to interact with external systems?
         ├─ Yes → Use createEffect
         │        └─ One-time on mount? → Use onMount
         └─ No → You might not need reactivity here
```

## Related Rules

- [1-2: Use Memo for Derived Values](1-2-use-memo-for-derived.md) - The right tool for derivations
- [1-4: Avoid Signal in Effect](1-4-avoid-signal-in-effect.md) - Related anti-pattern
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Cleaning up effects
