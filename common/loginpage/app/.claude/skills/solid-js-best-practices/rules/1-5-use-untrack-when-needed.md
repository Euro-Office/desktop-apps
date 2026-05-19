---
id: 1-5
title: Use Untrack When Needed
category: Reactivity
priority: MEDIUM
description: Use untrack() to prevent unwanted reactive subscriptions
---

## Problem

Sometimes you need to read a signal's value without subscribing to its changes. By default, any signal accessed within a reactive context (effect, memo, JSX) creates a dependency. The `untrack` function lets you read values without creating subscriptions.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

function Logger() {
  const [value, setValue] = createSignal(0);
  const [logLevel, setLogLevel] = createSignal("info");

  // ❌ WRONG: Effect re-runs when logLevel changes, even though we only care about value
  createEffect(() => {
    console.log(`[${logLevel()}] Value changed to: ${value()}`);
  });

  return (
    <div>
      <button onClick={() => setValue(v => v + 1)}>Increment</button>
      <select onChange={(e) => setLogLevel(e.currentTarget.value)}>
        <option value="info">Info</option>
        <option value="debug">Debug</option>
      </select>
    </div>
  );
}
```

## Correct

```tsx
import { createSignal, createEffect, untrack } from "solid-js";

function Logger() {
  const [value, setValue] = createSignal(0);
  const [logLevel, setLogLevel] = createSignal("info");

  // ✅ CORRECT: Only re-runs when value changes, logLevel is read without tracking
  createEffect(() => {
    console.log(`[${untrack(logLevel)}] Value changed to: ${value()}`);
  });

  return (
    <div>
      <button onClick={() => setValue(v => v + 1)}>Increment</button>
      <select onChange={(e) => setLogLevel(e.currentTarget.value)}>
        <option value="info">Info</option>
        <option value="debug">Debug</option>
      </select>
    </div>
  );
}
```

### Preventing Cycles

```tsx
import { createSignal, createEffect, untrack } from "solid-js";

function FormWithValidation() {
  const [input, setInput] = createSignal("");
  const [errors, setErrors] = createSignal<string[]>([]);

  createEffect(() => {
    const value = input();
    const currentErrors = untrack(errors);  // Read without tracking

    const newErrors: string[] = [];
    if (value.length < 3) newErrors.push("Too short");
    if (value.length > 100) newErrors.push("Too long");

    // Only update if errors actually changed
    if (JSON.stringify(newErrors) !== JSON.stringify(currentErrors)) {
      setErrors(newErrors);
    }
  });

  return (
    <div>
      <input value={input()} onInput={(e) => setInput(e.currentTarget.value)} />
      <For each={errors()}>{error => <p class="error">{error}</p>}</For>
    </div>
  );
}
```

### Conditionally Skipping Work

```tsx
function DataFetcher() {
  const [id, setId] = createSignal(1);
  const [cache, setCache] = createSignal<Record<number, Data>>({});

  createEffect(() => {
    const currentId = id();
    const currentCache = untrack(cache);

    // Skip fetch if already cached
    if (currentCache[currentId]) {
      return;
    }

    fetch(`/api/data/${currentId}`)
      .then(r => r.json())
      .then(data => setCache(c => ({ ...c, [currentId]: data })));
  });

  return <div>{/* ... */}</div>;
}
```

### Using on() as Alternative

```tsx
import { createEffect, on } from "solid-js";

function Logger() {
  const [value, setValue] = createSignal(0);
  const [logLevel, setLogLevel] = createSignal("info");

  // ✅ ALTERNATIVE: on() makes tracked dependencies explicit
  createEffect(on(value, (v) => {
    console.log(`[${logLevel()}] Value changed to: ${v}`);
    // logLevel is accessed but not in a tracking context here
  }));

  return <div>{/* ... */}</div>;
}
```

## Why It Matters

1. **Control Over Dependencies**: Sometimes you need to read a value for reference without wanting updates when it changes.

2. **Prevent Infinite Loops**: When an effect needs to read and potentially write to the same signal.

3. **Performance**: Reduce unnecessary effect re-runs by only tracking what you need.

4. **Initial Values**: Read current state for comparison without subscribing.

## Common Use Cases

| Use Case | Pattern |
| -------- | ------- |
| Read config/settings without tracking | `untrack(config)` |
| Compare before updating | `if (newVal !== untrack(signal))` |
| Use value for logging/debugging | `console.log(untrack(state))` |
| Access previous value | `const prev = untrack(signal)` |

## Related Rules

- [1-4: Avoid Signal in Effect](1-4-avoid-signal-in-effect.md) - Related cycle prevention
- [1-6: Batch Signal Updates](1-6-batch-signal-updates.md) - Another update control mechanism
