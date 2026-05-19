---
id: 1-2
title: Use Memo for Derived Values
category: Reactivity
priority: HIGH
description: Use createMemo for computed values, not createEffect
---

## Problem

When you need to compute a value based on reactive state, you should use `createMemo` to cache the result. Using a plain function recalculates on every access, while using `createEffect` is semantically incorrect and can cause issues.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

function PriceDisplay() {
  const [price, setPrice] = createSignal(100);
  const [quantity, setQuantity] = createSignal(2);

  // ❌ WRONG: Recalculates on every access
  const total = () => price() * quantity();

  // Used multiple times = calculated multiple times
  return (
    <div>
      <p>Total: ${total()}</p>
      <p>With tax: ${total() * 1.1}</p>
      <p>Savings: ${total() > 200 ? total() * 0.1 : 0}</p>
    </div>
  );
}
```

```tsx
// ❌ WRONG: Using effect for derived values
function PriceDisplay() {
  const [price, setPrice] = createSignal(100);
  const [quantity, setQuantity] = createSignal(2);
  const [total, setTotal] = createSignal(0);

  // Anti-pattern: effect to sync derived state
  createEffect(() => {
    setTotal(price() * quantity());
  });

  return <p>Total: ${total()}</p>;
}
```

## Correct

```tsx
import { createSignal, createMemo } from "solid-js";

function PriceDisplay() {
  const [price, setPrice] = createSignal(100);
  const [quantity, setQuantity] = createSignal(2);

  // ✅ CORRECT: Memo caches the computation
  const total = createMemo(() => price() * quantity());

  // Value is computed once and cached
  return (
    <div>
      <p>Total: ${total()}</p>
      <p>With tax: ${total() * 1.1}</p>
      <p>Savings: ${total() > 200 ? total() * 0.1 : 0}</p>
    </div>
  );
}
```

### Expensive Computations

```tsx
// ✅ CORRECT: Memo prevents expensive recalculation
function FilteredList() {
  const [items, setItems] = createSignal([...]);
  const [filter, setFilter] = createSignal("");

  const filteredItems = createMemo(() => {
    console.log("Filtering..."); // Only runs when items or filter change
    return items().filter(item =>
      item.name.toLowerCase().includes(filter().toLowerCase())
    );
  });

  return (
    <ul>
      <For each={filteredItems()}>
        {item => <li>{item.name}</li>}
      </For>
    </ul>
  );
}
```

### Chained Memos

```tsx
// ✅ CORRECT: Memos can depend on other memos
const [items, setItems] = createSignal([1, 2, 3, 4, 5]);

const evenItems = createMemo(() => items().filter(n => n % 2 === 0));
const sumOfEvens = createMemo(() => evenItems().reduce((a, b) => a + b, 0));
const averageOfEvens = createMemo(() => sumOfEvens() / evenItems().length);
```

## Why It Matters

1. **Performance**: Memos cache their result and only recompute when dependencies change. Plain functions recalculate on every access.

2. **Consistency**: Memos ensure all consumers see the same computed value. With plain functions, if state changes between accesses, you could get inconsistent results.

3. **Semantic Correctness**: `createEffect` is for side effects (API calls, DOM manipulation), not for deriving state. Using effects for derived values creates an unnecessary signal and update cycle.

## When to Use What

| Use Case | Solution |
| -------- | -------- |
| Simple, cheap computation used once | Plain function `() => a() + b()` |
| Computation used multiple times | `createMemo` |
| Expensive computation | `createMemo` |
| Derived value passed as prop | `createMemo` |
| Side effects (API, DOM, logging) | `createEffect` |

## Related Rules

- [1-3: Effects for Side Effects Only](1-3-effects-for-side-effects.md) - When to use effects instead
- [1-1: Use Signals Correctly](1-1-use-signals-correctly.md) - Remember to call memo as function
- [6-4: Optimize Store Access](6-4-optimize-store-access.md) - Memos with stores
