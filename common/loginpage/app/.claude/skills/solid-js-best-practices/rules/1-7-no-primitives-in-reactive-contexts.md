---
id: 1-7
title: No Primitives in Reactive Contexts
category: Reactivity
priority: HIGH
description: Don't call hooks or create reactive primitives inside effects or memos — they are recreated and disposed on every run
---

## Problem

Calling hooks or creating reactive primitives (`createSignal`, `createMemo`, `useMatch`, `useQuery`, etc.) inside a reactive computation (`createEffect`, `createComputed`, `createMemo`) causes them to be created fresh on every re-run and immediately disposed when the computation runs again.

## Incorrect

```tsx
// ❌ useMatch() creates a new internal memo every time windowWidth changes
createComputed(() => {
  setIsMobile(windowWidth() < 960);
  setIsRoot(Boolean(Router.useMatch(() => path)())); // ❌ recreated on every run
});

// ❌ Creating a signal inside an effect
createEffect(() => {
  const [local, setLocal] = createSignal(props.value); // ❌ orphaned on re-run
  setLocal(props.value + 1);
});
```

## Correct

```tsx
// ✅ Call hooks once at component initialization level
const matchRoute = Router.useMatch(() => path);        // called once
const isMobile = createMemo(() => windowWidth() < 960);
const isRoot   = createMemo(() => Boolean(matchRoute()));

// ✅ Derive values with createMemo instead of effects
const adjusted = createMemo(() => props.value + 1);
```

## Why It Matters

Every reactive computation owns the reactive nodes created during its execution. When the computation re-runs:

1. **Old nodes are disposed** — their subscriptions and state are silently dropped
2. **New nodes are created** — they start fresh with no accumulated reactive history
3. **State is lost** — any value the previous node had tracked is gone

This is especially dangerous with third-party hooks that create internal memos or effects (`useMatch`, `useQuery`, `useStore`, custom hooks). The hook appears to work initially but loses its reactive subscriptions on the next re-run of its enclosing computation.

Applies to any hook that internally calls `createSignal`, `createMemo`, `createEffect`, or `createResource`.

## Related Rules

- [1-3: Effects for Side Effects Only](1-3-effects-for-side-effects.md) - Use `createMemo` for derivations instead
- [1-2: Use Memo for Derived Values](1-2-use-memo-for-derived.md) - The correct alternative for computed values
- [1-4: Avoid Setting Signals in Effects](1-4-avoid-signal-in-effect.md) - Related pitfall with effects and signals
