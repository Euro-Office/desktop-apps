---
id: 5-6
title: Event Handler Patterns
category: Refs & DOM
priority: MEDIUM
description: Understand delegated vs native events, on:/oncapture: namespaces, and array handler syntax
---

## Problem

Solid.js has a unique event system that differs from both React and vanilla DOM. It uses **event delegation** for common events (`onClick`, `onInput`, etc.) and supports **native event binding** via `on:` and `oncapture:` namespaces. It also provides an array handler syntax `[handler, data]` for passing data without creating closures. Misunderstanding these patterns leads to missed events, incorrect behavior, or performance issues.

## Incorrect

```tsx
// ❌ WRONG: Creating a new closure on every render for data passing
function TodoList() {
  const [todos] = createSignal([
    { id: 1, text: "Buy milk" },
    { id: 2, text: "Walk dog" },
  ]);

  return (
    <For each={todos()}>
      {(todo) => (
        <button onClick={() => handleDelete(todo.id)}>
          Delete {todo.text}
        </button>
      )}
    </For>
  );
}
```

```tsx
// ❌ WRONG: Using delegated event for non-bubbling event
<div onScroll={(e) => handleScroll(e)}>
  <LongContent />
</div>

// ❌ WRONG: Expecting custom element events to work with delegation
<my-component onCustomEvent={handler} />
```

## Correct

### Array Handler Syntax

```tsx
// ✅ CORRECT: Array handler avoids closure allocation
function handleDelete(id: number, e: MouseEvent) {
  // Note: data is the first parameter, event is the second
  console.log("Deleting", id);
}

function TodoList() {
  const [todos] = createSignal([
    { id: 1, text: "Buy milk" },
    { id: 2, text: "Walk dog" },
  ]);

  return (
    <For each={todos()}>
      {(todo) => (
        <button onClick={[handleDelete, todo.id]}>
          Delete {todo.text}
        </button>
      )}
    </For>
  );
}
```

### Native Events with on: Namespace

```tsx
// ✅ CORRECT: Use on: for non-bubbling events (scroll, focus, blur, etc.)
<div on:scroll={(e) => handleScroll(e)}>
  <LongContent />
</div>

// ✅ CORRECT: Use on: for custom element events
<my-component on:customevent={handler} />

// ✅ CORRECT: Use on: when you need the native event (not delegated)
<button on:click={handler}>Native click</button>
```

### Capture Phase with oncapture: Namespace

```tsx
// ✅ CORRECT: Capture phase event handling
<div oncapture:click={(e) => {
  // Fires during capture phase, before the target element's handler
  if (shouldBlock()) {
    e.stopPropagation();
  }
}}>
  <ChildComponent />
</div>
```

### Delegated vs Native Events

```tsx
import { createSignal } from "solid-js";

function EventExample() {
  const [log, setLog] = createSignal<string[]>([]);

  const addLog = (msg: string) => setLog((prev) => [...prev, msg]);

  return (
    <div>
      {/* Delegated: Solid attaches one listener on document */}
      <button onClick={() => addLog("delegated click")}>
        Delegated
      </button>

      {/* Native: listener directly on the element */}
      <button on:click={() => addLog("native click")}>
        Native
      </button>

      {/* Capture: fires before the target's handler */}
      <div oncapture:click={() => addLog("capture phase")}>
        <button onClick={() => addLog("target click")}>
          Capture Example
        </button>
      </div>
    </div>
  );
}
```

## Event System Reference

| Syntax | Type | Use Case |
| ------ | ---- | -------- |
| `onClick={handler}` | Delegated | Common events (click, input, keydown, etc.) |
| `on:click={handler}` | Native | Non-bubbling events, custom elements, Web Components |
| `oncapture:click={handler}` | Capture | Intercept events before they reach target |
| `onClick={[handler, data]}` | Delegated + data | Pass data without creating a closure |

## Delegated Events in Solid

Solid delegates these common events to the document root for performance:

`beforeinput`, `click`, `dblclick`, `focusin`, `focusout`, `input`, `keydown`, `keyup`, `mousedown`, `mousemove`, `mouseout`, `mouseover`, `mouseup`, `pointerdown`, `pointermove`, `pointerout`, `pointerover`, `pointerup`, `touchend`, `touchmove`, `touchstart`

All other events (e.g., `scroll`, `resize`, `animationend`, custom events) are always bound natively regardless of syntax.

## Array Handler Details

```tsx
// The array syntax: [handler, data]
// handler receives (data, event) — data first, event second

function handleClick(itemId: number, event: MouseEvent) {
  console.log("Item:", itemId, "Target:", event.currentTarget);
}

// These are equivalent, but the array version avoids creating a closure:
<button onClick={() => handleClick(item.id)}>Closure</button>
<button onClick={[handleClick, item.id]}>Array</button>
```

## Why It Matters

1. **Performance**: Event delegation reduces the number of event listeners. Array handlers avoid closure allocation inside loops.

2. **Correctness**: Non-bubbling events like `scroll` and `focus` don't work with delegation. Use `on:` for these.

3. **Web Components**: Custom element events require `on:` syntax since Solid cannot delegate events it doesn't know about.

4. **Capture Phase**: `oncapture:` enables event interception patterns that aren't possible with delegated events.

## Related Rules

- [5-4: Use Directives](5-4-use-directives.md) - Reusable event-based behaviors
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Cleaning up manual event listeners
