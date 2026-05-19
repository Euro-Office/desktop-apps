---
id: 8-3
title: Test Reactive Primitives in a Root
category: Testing
priority: HIGH
description: Wrap signal, effect, and memo tests in createRoot or use renderHook
---

## Problem

Solid's reactive primitives (signals, effects, memos, stores) require an owning reactive scope to function. In a test file, top-level code has no reactive owner. Creating signals works (they are just getter/setter pairs), but `createEffect`, `createMemo` with side effects, and `onCleanup` all silently fail or warn when executed without an owner. The `renderHook` utility from `@solidjs/testing-library` or manual `createRoot` wrapping provides this scope.

## Incorrect

```tsx
import { createSignal, createEffect } from "solid-js";

// ❌ WRONG: No reactive owner -- effect never runs
test("tracks count changes", () => {
  const [count, setCount] = createSignal(0);
  const log: number[] = [];

  createEffect(() => {
    log.push(count()); // Never executes -- no owner
  });

  setCount(1);
  setCount(2);
  expect(log).toEqual([0, 1, 2]); // Fails: log is []
});
```

```tsx
import { createSignal, createMemo } from "solid-js";

// ❌ WRONG: Memo created without owner -- works but cleanup won't run
test("doubles value", () => {
  const [count, setCount] = createSignal(3);
  const doubled = createMemo(() => count() * 2);

  expect(doubled()).toBe(6);
  // Memo leaks -- no dispose, may interfere with other tests
});
```

## Correct

```tsx
import { createSignal, createEffect, createRoot } from "solid-js";

// ✅ CORRECT: createRoot provides reactive ownership
test("tracks count changes", () => {
  return new Promise<void>((resolve) => {
    createRoot((dispose) => {
      const [count, setCount] = createSignal(0);
      const log: number[] = [];
      let run = 0;

      createEffect(() => {
        log.push(count());
        run++;
        if (run === 3) {
          expect(log).toEqual([0, 1, 2]);
          dispose();
          resolve();
        }
      });

      setCount(1);
      setCount(2);
    });
  });
});
```

### Using renderHook

```tsx
import { renderHook } from "@solidjs/testing-library";
import { useCounter } from "./useCounter";

// ✅ CORRECT: renderHook manages reactive scope and cleanup
test("useCounter increments", () => {
  const { result } = renderHook(useCounter);

  expect(result.count()).toBe(0);
  result.increment();
  expect(result.count()).toBe(1);
});
```

```tsx
import { renderHook } from "@solidjs/testing-library";
import { useLocalStorage } from "./useLocalStorage";

// ✅ CORRECT: renderHook with initial arguments
test("useLocalStorage reads initial value", () => {
  localStorage.setItem("theme", '"dark"');

  const { result } = renderHook(() => useLocalStorage("theme", "light"));

  expect(result.value()).toBe("dark");
});
```

### testEffect Helper Pattern

```tsx
import { createRoot, createEffect, createSignal, createMemo } from "solid-js";

// ✅ CORRECT: Reusable testEffect utility
function testEffect(fn: (done: () => void) => void): Promise<void> {
  return new Promise<void>((resolve) => {
    createRoot((dispose) => {
      fn(() => {
        dispose();
        resolve();
      });
    });
  });
}

test("derived value updates", () =>
  testEffect((done) => {
    const [count, setCount] = createSignal(0);
    const doubled = createMemo(() => count() * 2);
    let runs = 0;

    createEffect(() => {
      const val = doubled();
      if (runs === 0) expect(val).toBe(0);
      if (runs === 1) {
        expect(val).toBe(10);
        done();
      }
      runs++;
    });

    setCount(5);
  })
);
```

### Synchronous Tests with createRoot

```tsx
import { createSignal, createMemo, createRoot } from "solid-js";

// ✅ CORRECT: Synchronous tests that don't need effects
test("memo computes derived value", () => {
  createRoot((dispose) => {
    const [count, setCount] = createSignal(3);
    const doubled = createMemo(() => count() * 2);

    expect(doubled()).toBe(6);
    setCount(10);
    expect(doubled()).toBe(20);

    dispose(); // Always dispose to prevent leaks
  });
});
```

## Why It Matters

1. **Silent Failures**: Without an owner, `createEffect` logs a warning and does nothing -- tests pass without running assertions inside effects.

2. **Memory Leaks**: Reactive roots that are not disposed leak subscriptions between tests, causing flaky behavior and degraded test runner performance.

3. **Escape Conditions**: Effect-based tests without a `done` callback or run counter cause test timeouts or infinite loops.

4. **Cleanup Parity**: `renderHook` automatically handles disposal, matching how `render` handles component cleanup.

## Related Rules

- [1-3: Effects for Side Effects](1-3-effects-for-side-effects.md) - Effects need reactive owners in production too
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Cleanup requires a reactive owner to register
- [8-2: Wrap Render in Arrow Functions](8-2-wrap-render-in-arrow.md) - Same ownership concept for components
