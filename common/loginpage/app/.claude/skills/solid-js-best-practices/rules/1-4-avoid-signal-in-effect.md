---
id: 1-4
title: Avoid Setting Signals in Effects
category: Reactivity
priority: MEDIUM
description: Setting signals in effects can cause infinite loops
---

## Problem

Setting signals inside `createEffect` can lead to infinite loops, unexpected behavior, or inefficient update cycles. Effects run whenever their dependencies change, so setting a signal that the effect reads creates a cycle.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

function Counter() {
  const [count, setCount] = createSignal(0);

  // ❌ WRONG: Infinite loop - reads and writes count
  createEffect(() => {
    setCount(count() + 1);
  });

  return <p>{count()}</p>;
}
```

```tsx
// ❌ WRONG: Hidden infinite loop through derived value
function Temperature() {
  const [celsius, setCelsius] = createSignal(0);
  const [fahrenheit, setFahrenheit] = createSignal(32);

  createEffect(() => {
    setFahrenheit(celsius() * 9/5 + 32);
  });

  createEffect(() => {
    setCelsius((fahrenheit() - 32) * 5/9);
  });

  return <div>{celsius()}°C = {fahrenheit()}°F</div>;
}
```

## Correct

### Use createMemo for Derived Values

```tsx
import { createSignal, createMemo } from "solid-js";

function Temperature() {
  const [celsius, setCelsius] = createSignal(0);

  // ✅ CORRECT: Derived value, no signal setting
  const fahrenheit = createMemo(() => celsius() * 9/5 + 32);

  return <div>{celsius()}°C = {fahrenheit()}°F</div>;
}
```

### Use on() for Explicit Dependencies

```tsx
import { createSignal, createEffect, on } from "solid-js";

function LogChanges() {
  const [value, setValue] = createSignal(0);
  const [log, setLog] = createSignal<string[]>([]);

  // ✅ CORRECT: on() makes dependencies explicit, defers: true prevents immediate run
  createEffect(on(value, (v, prev) => {
    setLog(logs => [...logs, `Changed from ${prev} to ${v}`]);
  }, { defer: true }));

  return (
    <div>
      <button onClick={() => setValue(v => v + 1)}>Increment</button>
      <ul>
        <For each={log()}>{msg => <li>{msg}</li>}</For>
      </ul>
    </div>
  );
}
```

### Use untrack() to Break Cycles

```tsx
import { createSignal, createEffect, untrack } from "solid-js";

function Synced() {
  const [source, setSource] = createSignal(0);
  const [derived, setDerived] = createSignal(0);

  // ✅ CORRECT: untrack prevents reading derived from creating dependency
  createEffect(() => {
    const newValue = source() * 2;
    if (untrack(derived) !== newValue) {
      setDerived(newValue);
    }
  });

  return <div>{source()} → {derived()}</div>;
}
```

### Use External Triggers

```tsx
// ✅ CORRECT: Signal set from event, not from effect reading
function AutoSave() {
  const [content, setContent] = createSignal("");
  const [saved, setSaved] = createSignal(false);

  createEffect(() => {
    const text = content();  // Track content changes
    // Don't read 'saved' here - would create dependency

    fetch("/api/save", {
      method: "POST",
      body: JSON.stringify({ content: text })
    }).then(() => setSaved(true));
  });

  return (
    <textarea
      value={content()}
      onInput={(e) => {
        setContent(e.currentTarget.value);
        setSaved(false);  // Set from event, not effect
      }}
    />
  );
}
```

## Why It Matters

1. **Infinite Loops**: Reading and writing the same signal in an effect creates an endless cycle that can crash your app.

2. **Extra Render Cycles**: Even when not infinite, setting signals in effects causes additional updates after the initial render.

3. **Hard to Debug**: The control flow becomes implicit and hard to trace.

4. **Performance**: Each signal set triggers a new reactive update cycle.

## Safe Patterns

| Pattern | Use Case |
| ------- | -------- |
| `createMemo` | Derive values from other signals |
| `on()` with defer | React to specific changes without creating cycles |
| `untrack()` | Read a signal without tracking it |
| Event handlers | Set signals from user interactions |
| `onMount` | One-time initialization |

## Related Rules

- [1-2: Use Memo for Derived Values](1-2-use-memo-for-derived.md) - Better alternative
- [1-3: Effects for Side Effects Only](1-3-effects-for-side-effects.md) - When effects are appropriate
- [1-5: Use Untrack When Needed](1-5-use-untrack-when-needed.md) - Breaking dependency tracking
