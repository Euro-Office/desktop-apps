---
id: 5-3
title: Cleanup with onCleanup
category: Refs & DOM
priority: HIGH
description: Always clean up subscriptions and timers
---

## Problem

Components that create subscriptions, timers, event listeners, or other resources must clean them up when the component unmounts. Failing to do so causes memory leaks, stale callbacks, and unexpected behavior.

## Incorrect

```tsx
// ❌ WRONG: Timer never cleaned up
function Clock() {
  const [time, setTime] = createSignal(new Date());

  onMount(() => {
    setInterval(() => setTime(new Date()), 1000);
    // Timer continues after component unmounts!
  });

  return <span>{time().toLocaleTimeString()}</span>;
}
```

```tsx
// ❌ WRONG: Event listener not removed
function WindowSize() {
  const [size, setSize] = createSignal({ w: 0, h: 0 });

  onMount(() => {
    const update = () => setSize({
      w: window.innerWidth,
      h: window.innerHeight
    });
    window.addEventListener("resize", update);
    // Listener stays attached forever!
  });

  return <span>{size().w} x {size().h}</span>;
}
```

```tsx
// ❌ WRONG: Subscription not unsubscribed
function LiveData() {
  const [data, setData] = createSignal(null);

  onMount(() => {
    const subscription = dataStream.subscribe(setData);
    // Subscription never cancelled!
  });

  return <pre>{JSON.stringify(data())}</pre>;
}
```

## Correct

```tsx
import { onMount, onCleanup, createSignal } from "solid-js";

// ✅ CORRECT: Timer cleaned up
function Clock() {
  const [time, setTime] = createSignal(new Date());

  onMount(() => {
    const id = setInterval(() => setTime(new Date()), 1000);
    onCleanup(() => clearInterval(id));
  });

  return <span>{time().toLocaleTimeString()}</span>;
}
```

```tsx
// ✅ CORRECT: Event listener removed
function WindowSize() {
  const [size, setSize] = createSignal({ w: 0, h: 0 });

  onMount(() => {
    const update = () => setSize({
      w: window.innerWidth,
      h: window.innerHeight
    });

    window.addEventListener("resize", update);
    onCleanup(() => window.removeEventListener("resize", update));

    update(); // Initial value
  });

  return <span>{size().w} x {size().h}</span>;
}
```

```tsx
// ✅ CORRECT: Subscription cancelled
function LiveData() {
  const [data, setData] = createSignal(null);

  onMount(() => {
    const subscription = dataStream.subscribe(setData);
    onCleanup(() => subscription.unsubscribe());
  });

  return <pre>{JSON.stringify(data())}</pre>;
}
```

### Cleanup in Effects

```tsx
import { createEffect, onCleanup, createSignal } from "solid-js";

// ✅ CORRECT: Cleanup in effect runs before each re-run
function WebSocketComponent(props) {
  const [messages, setMessages] = createSignal([]);

  createEffect(() => {
    const ws = new WebSocket(props.url);  // Tracks props.url

    ws.onmessage = (e) => {
      setMessages(msgs => [...msgs, e.data]);
    };

    // Cleanup runs when:
    // 1. props.url changes (before new effect runs)
    // 2. Component unmounts
    onCleanup(() => ws.close());
  });

  return (
    <ul>
      <For each={messages()}>{msg => <li>{msg}</li>}</For>
    </ul>
  );
}
```

### Third-Party Libraries

```tsx
// ✅ CORRECT: Library instance destroyed
function Chart(props) {
  let canvasRef: HTMLCanvasElement;

  onMount(() => {
    const chart = new ChartJS(canvasRef, {
      type: props.type,
      data: props.data
    });

    onCleanup(() => chart.destroy());
  });

  return <canvas ref={canvasRef} />;
}
```

### Abort Controllers for Fetch

```tsx
// ✅ CORRECT: Abort in-flight requests
function DataFetcher(props) {
  const [data, setData] = createSignal(null);

  createEffect(() => {
    const controller = new AbortController();

    fetch(props.url, { signal: controller.signal })
      .then(r => r.json())
      .then(setData)
      .catch(e => {
        if (e.name !== "AbortError") throw e;
      });

    onCleanup(() => controller.abort());
  });

  return <Show when={data()}>{d => <pre>{JSON.stringify(d)}</pre>}</Show>;
}
```

## What to Clean Up

| Resource | Cleanup Method |
| -------- | -------------- |
| `setInterval` | `clearInterval(id)` |
| `setTimeout` | `clearTimeout(id)` |
| `addEventListener` | `removeEventListener(...)` |
| `WebSocket` | `ws.close()` |
| `fetch` | `controller.abort()` |
| `MutationObserver` | `observer.disconnect()` |
| `ResizeObserver` | `observer.disconnect()` |
| `IntersectionObserver` | `observer.disconnect()` |
| Custom subscriptions | `subscription.unsubscribe()` |
| Chart.js, Map libraries | `instance.destroy()` |

## Why It Matters

1. **Memory Leaks**: Unreleased resources accumulate over time.

2. **Stale Updates**: Old callbacks may try to update unmounted components.

3. **Resource Exhaustion**: Too many open connections or timers degrade performance.

4. **Correct Behavior**: Effects with reactive deps need cleanup before re-running.

## onCleanup Timing

```tsx
// In onMount: runs on unmount only
onMount(() => {
  const timer = setInterval(...);
  onCleanup(() => clearInterval(timer));  // Unmount only
});

// In createEffect: runs before each re-run AND on unmount
createEffect(() => {
  const value = signal();  // Dependency
  const ws = connect(value);
  onCleanup(() => ws.close());  // Before re-run AND unmount
});
```

## Related Rules

- [5-2: Access DOM in onMount](5-2-access-dom-in-onmount.md) - Setting up resources
- [1-3: Effects for Side Effects](1-3-effects-for-side-effects.md) - Effect cleanup
