---
id: 6-1
title: Avoid Unnecessary Tracking
category: Performance
priority: HIGH
description: Don't access signals outside reactive contexts
---

## Problem

Accessing signals in contexts where you don't need reactivity creates unnecessary subscriptions. This leads to re-renders, effect re-runs, and wasted computation when those signals change.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

// ❌ WRONG: Logging creates unwanted subscription
function ClickCounter() {
  const [count, setCount] = createSignal(0);
  const [clicks, setClicks] = createSignal(0);

  // This effect re-runs when EITHER signal changes
  createEffect(() => {
    console.log(`Count: ${count()}, Total clicks: ${clicks()}`);
    // But we only wanted to log on count changes!
  });

  return (
    <button onClick={() => {
      setCount(c => c + 1);
      setClicks(c => c + 1);
    }}>
      {count()}
    </button>
  );
}
```

```tsx
// ❌ WRONG: Signal access in event handler creates subscription
function Form() {
  const [formData, setFormData] = createSignal({ name: "", email: "" });
  const [isValid, setIsValid] = createSignal(false);

  createEffect(() => {
    // This tracks BOTH formData AND isValid
    if (isValid()) {
      console.log("Form is valid:", formData());
    }
  });

  return <form>{/* ... */}</form>;
}
```

## Correct

```tsx
import { createSignal, createEffect, on, untrack } from "solid-js";

// ✅ CORRECT: Use on() to explicitly track only count
function ClickCounter() {
  const [count, setCount] = createSignal(0);
  const [clicks, setClicks] = createSignal(0);

  createEffect(on(count, (c) => {
    // Only runs when count changes
    console.log(`Count: ${c}, Total clicks: ${clicks()}`);
    // clicks() is accessed but not tracked because we're in on()'s callback
  }));

  return (
    <button onClick={() => {
      setCount(c => c + 1);
      setClicks(c => c + 1);
    }}>
      {count()}
    </button>
  );
}
```

```tsx
// ✅ CORRECT: Use untrack to prevent subscription
function Form() {
  const [formData, setFormData] = createSignal({ name: "", email: "" });
  const [isValid, setIsValid] = createSignal(false);

  createEffect(() => {
    if (isValid()) {  // Track this
      console.log("Form is valid:", untrack(formData));  // Don't track this
    }
  });

  return <form>{/* ... */}</form>;
}
```

### Preventing Subscription in Callbacks

```tsx
// ✅ CORRECT: Event handlers don't create subscriptions
function Counter() {
  const [count, setCount] = createSignal(0);
  const [multiplier, setMultiplier] = createSignal(2);

  // onClick is not a reactive context, so this is fine
  const handleClick = () => {
    console.log("Current multiplier:", multiplier());
    setCount(c => c * multiplier());
  };

  return <button onClick={handleClick}>{count()}</button>;
}
```

### Using on() for Multiple Dependencies

```tsx
import { createEffect, on } from "solid-js";

// ✅ CORRECT: Explicit dependency list with on()
function DataSync() {
  const [userId, setUserId] = createSignal(1);
  const [timestamp, setTimestamp] = createSignal(Date.now());
  const [config, setConfig] = createSignal({ debug: false });

  // Only re-run when userId OR timestamp change, not config
  createEffect(on(
    [userId, timestamp],
    ([id, ts]) => {
      const cfg = config();  // Read without tracking
      if (cfg.debug) console.log(`Syncing user ${id} at ${ts}`);
      syncData(id, ts);
    }
  ));
}
```

### Memoization to Prevent Tracking

```tsx
import { createMemo } from "solid-js";

// ✅ CORRECT: Memo isolates expensive computation
function FilteredList() {
  const [items, setItems] = createSignal([]);
  const [searchTerm, setSearchTerm] = createSignal("");
  const [sortOrder, setSortOrder] = createSignal("asc");

  // Each memo only tracks what it accesses
  const filtered = createMemo(() =>
    items().filter(item =>
      item.name.includes(searchTerm())
    )
  );

  const sorted = createMemo(() =>
    [...filtered()].sort((a, b) =>
      sortOrder() === "asc" ? a.name.localeCompare(b.name) : b.name.localeCompare(a.name)
    )
  );

  // Components using sorted() only re-render when sorted changes
  return <List items={sorted()} />;
}
```

## Tracking Contexts

| Context | Tracks Dependencies? |
| ------- | ------------------- |
| JSX expressions `{signal()}` | Yes |
| `createEffect(() => ...)` | Yes |
| `createMemo(() => ...)` | Yes |
| `createComputed(() => ...)` | Yes |
| Event handlers `onClick={...}` | No |
| `onMount(() => ...)` | No |
| `untrack(() => ...)` | No |
| `on(deps, callback)` callback | Only explicit deps |

## Why It Matters

1. **Excess Re-renders**: Unnecessary subscriptions cause components to update when unrelated signals change.

2. **Performance**: More subscriptions = more work for the reactive system.

3. **Predictability**: Understanding what triggers updates makes debugging easier.

4. **Resource Efficiency**: Effects doing unnecessary work waste CPU and battery.

## Related Rules

- [1-5: Use Untrack When Needed](1-5-use-untrack-when-needed.md) - Breaking dependency tracking
- [1-2: Use Memo for Derived Values](1-2-use-memo-for-derived.md) - Isolating computations
- [1-4: Avoid Signal in Effect](1-4-avoid-signal-in-effect.md) - Related effect patterns
