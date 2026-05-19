---
id: 1-6
title: Batch Signal Updates
category: Reactivity
priority: LOW
description: Use batch() for multiple synchronous signal updates
---

## Problem

By default, each signal update triggers an immediate reactive update. When updating multiple signals in sequence, this causes multiple update cycles. The `batch` function groups updates together so the reactive system only runs once.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

function Form() {
  const [firstName, setFirstName] = createSignal("");
  const [lastName, setLastName] = createSignal("");
  const [email, setEmail] = createSignal("");

  // This effect runs 3 times during reset
  createEffect(() => {
    console.log("Form updated:", firstName(), lastName(), email());
  });

  const resetForm = () => {
    // ❌ Each update triggers a separate reactive cycle
    setFirstName("");
    setLastName("");
    setEmail("");
  };

  return (
    <div>
      <input value={firstName()} onInput={(e) => setFirstName(e.currentTarget.value)} />
      <input value={lastName()} onInput={(e) => setLastName(e.currentTarget.value)} />
      <input value={email()} onInput={(e) => setEmail(e.currentTarget.value)} />
      <button onClick={resetForm}>Reset</button>
    </div>
  );
}
```

## Correct

```tsx
import { createSignal, createEffect, batch } from "solid-js";

function Form() {
  const [firstName, setFirstName] = createSignal("");
  const [lastName, setLastName] = createSignal("");
  const [email, setEmail] = createSignal("");

  // This effect runs only once during reset
  createEffect(() => {
    console.log("Form updated:", firstName(), lastName(), email());
  });

  const resetForm = () => {
    // ✅ All updates batched into single reactive cycle
    batch(() => {
      setFirstName("");
      setLastName("");
      setEmail("");
    });
  };

  return (
    <div>
      <input value={firstName()} onInput={(e) => setFirstName(e.currentTarget.value)} />
      <input value={lastName()} onInput={(e) => setLastName(e.currentTarget.value)} />
      <input value={email()} onInput={(e) => setEmail(e.currentTarget.value)} />
      <button onClick={resetForm}>Reset</button>
    </div>
  );
}
```

### With Async Operations

```tsx
import { createSignal, batch } from "solid-js";

async function fetchUserData(userId: string) {
  const [user, setUser] = createSignal(null);
  const [posts, setPosts] = createSignal([]);
  const [loading, setLoading] = createSignal(true);

  try {
    const [userData, userPosts] = await Promise.all([
      fetch(`/api/users/${userId}`).then(r => r.json()),
      fetch(`/api/users/${userId}/posts`).then(r => r.json())
    ]);

    // ✅ Batch the state updates after async operations
    batch(() => {
      setUser(userData);
      setPosts(userPosts);
      setLoading(false);
    });
  } catch (error) {
    batch(() => {
      setUser(null);
      setPosts([]);
      setLoading(false);
    });
  }
}
```

### Store Updates Are Already Batched

```tsx
import { createStore } from "solid-js/store";

function Form() {
  const [form, setForm] = createStore({
    firstName: "",
    lastName: "",
    email: ""
  });

  const resetForm = () => {
    // ✅ Store updates within setStore are automatically batched
    setForm({
      firstName: "",
      lastName: "",
      email: ""
    });
  };

  return <div>{/* ... */}</div>;
}
```

## Why It Matters

1. **Performance**: Multiple reactive updates mean multiple DOM updates, effect runs, and memo recalculations.

2. **Consistency**: Without batching, intermediate states are visible. Components might render with `firstName=""` but old `lastName` values.

3. **Predictability**: Effects see all changes at once rather than being called multiple times with partial updates.

## When to Use Batch

| Scenario | Use Batch? |
| -------- | ---------- |
| Event handlers updating multiple signals | Yes |
| After async operations (fetch, timers) | Yes |
| Store updates via setStore | No (auto-batched) |
| Single signal update | No |
| Inside effects | Usually not needed |

## Note on Automatic Batching

Solid automatically batches updates within:

- Store `setStore` calls
- Transitions
- The synchronous execution of effects

You primarily need `batch()` for:

- Event handlers updating multiple signals
- After `await` in async functions
- Callbacks from external libraries

## Related Rules

- [1-4: Avoid Signal in Effect](1-4-avoid-signal-in-effect.md) - Related update patterns
- [4-2: Store Path Updates](4-2-store-path-updates.md) - Stores batch automatically
