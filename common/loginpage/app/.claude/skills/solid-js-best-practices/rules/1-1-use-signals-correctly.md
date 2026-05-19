---
id: 1-1
title: Use Signals Correctly
category: Reactivity
priority: CRITICAL
description: Always call signals as functions count() not count
---

## Problem

Signals in Solid.js are getter/setter pairs returned by `createSignal`. The getter must be called as a function to access the value. Forgetting to call the getter function results in passing the function itself rather than its value, breaking reactivity and causing unexpected behavior.

## Incorrect

```tsx
import { createSignal } from "solid-js";

function Counter() {
  const [count, setCount] = createSignal(0);

  // ❌ WRONG: Passing the getter function, not the value
  return (
    <div>
      <p>Count: {count}</p>  {/* Displays "[Function]" or nothing */}
      <button onClick={() => setCount(count + 1)}>  {/* NaN result */}
        Increment
      </button>
    </div>
  );
}
```

## Correct

```tsx
import { createSignal } from "solid-js";

function Counter() {
  const [count, setCount] = createSignal(0);

  // ✅ CORRECT: Calling the getter function to access value
  return (
    <div>
      <p>Count: {count()}</p>
      <button onClick={() => setCount(count() + 1)}>
        Increment
      </button>
    </div>
  );
}
```

### Using Functional Updates

```tsx
// ✅ CORRECT: Functional update pattern
<button onClick={() => setCount(prev => prev + 1)}>
  Increment
</button>
```

## Why It Matters

Solid.js tracks dependencies by detecting when getter functions are called within reactive contexts (like JSX expressions or `createEffect`). When you pass the getter function without calling it:

1. **No value is displayed** - The function object is rendered, not its return value
2. **No reactivity** - Solid can't track the dependency because the getter was never called
3. **Arithmetic errors** - Operations like `count + 1` produce `NaN` because you're adding a number to a function

This is the most common mistake when transitioning from React, where `useState` returns the value directly.

## TypeScript Help

TypeScript can help catch this error. When a signal should return a number but you see type errors about functions, you likely forgot to call the getter:

```tsx
const [count, setCount] = createSignal(0);

// TypeScript error: Operator '+' cannot be applied to types '() => number' and 'number'
setCount(count + 1);

// ✅ No error
setCount(count() + 1);
```

## Related Rules

- [1-2: Use Memo for Derived Values](1-2-use-memo-for-derived.md) - Signals in memos also need to be called
- [2-1: Never Destructure Props](2-1-never-destructure-props.md) - Another common reactivity pitfall
