---
id: 5-1
title: Use Refs Correctly
category: Refs & DOM
priority: HIGH
description: Use callback refs for conditional elements
---

## Problem

Refs provide direct access to DOM elements, but accessing them incorrectly can lead to undefined references, especially with conditional rendering. Use callback refs or signals when elements may not exist immediately.

## Incorrect

```tsx
// ❌ WRONG: Ref may be undefined when accessed
function AutoFocus() {
  let inputRef: HTMLInputElement;

  // inputRef is undefined here - element not yet mounted
  inputRef.focus();

  return <input ref={inputRef} />;
}
```

```tsx
// ❌ WRONG: Ref with conditional element
function ConditionalInput(props) {
  let inputRef: HTMLInputElement;

  const focusInput = () => {
    inputRef.focus();  // Error if Show is false
  };

  return (
    <Show when={props.visible}>
      <input ref={inputRef} />
    </Show>
  );
}
```

## Correct

```tsx
import { onMount } from "solid-js";

// ✅ CORRECT: Access ref in onMount
function AutoFocus() {
  let inputRef: HTMLInputElement;

  onMount(() => {
    inputRef.focus();  // Element is mounted here
  });

  return <input ref={inputRef} />;
}
```

```tsx
// ✅ CORRECT: Callback ref for immediate access
function AutoFocus() {
  return (
    <input ref={(el) => {
      // el is the DOM element, already mounted
      el.focus();
    }} />
  );
}
```

```tsx
import { createSignal } from "solid-js";

// ✅ CORRECT: Signal ref for conditional elements
function ConditionalInput(props) {
  const [inputRef, setInputRef] = createSignal<HTMLInputElement>();

  const focusInput = () => {
    const el = inputRef();
    if (el) {
      el.focus();
    }
  };

  return (
    <Show when={props.visible}>
      <input ref={setInputRef} />
      <button onClick={focusInput}>Focus</button>
    </Show>
  );
}
```

### TypeScript Definite Assignment

```tsx
// ✅ CORRECT: Use definite assignment assertion
function Component() {
  let divRef!: HTMLDivElement;  // ! tells TS it will be assigned

  onMount(() => {
    console.log(divRef.offsetWidth);
  });

  return <div ref={divRef}>Content</div>;
}
```

### Forwarding Refs

```tsx
// ✅ CORRECT: Accept ref as prop and forward
function FancyInput(props) {
  return (
    <div class="fancy-wrapper">
      <input
        ref={props.ref}  // Forward to parent
        class="fancy-input"
      />
    </div>
  );
}

function Parent() {
  let inputRef: HTMLInputElement;

  return <FancyInput ref={inputRef} />;
}
```

### Multiple Refs

```tsx
// ✅ CORRECT: Array of refs
function DynamicInputs() {
  const [inputs, setInputs] = createSignal(["", "", ""]);
  const refs: HTMLInputElement[] = [];

  const focusNext = (index: number) => {
    if (refs[index + 1]) {
      refs[index + 1].focus();
    }
  };

  return (
    <For each={inputs()}>
      {(_, i) => (
        <input
          ref={(el) => refs[i()] = el}
          onKeyDown={(e) => e.key === "Enter" && focusNext(i())}
        />
      )}
    </For>
  );
}
```

## Ref Patterns

| Pattern | Use Case |
| ------- | -------- |
| Variable ref (`let ref`) | Always-rendered elements |
| Callback ref (`ref={(el) => ...}`) | Immediate access on mount |
| Signal ref (`createSignal`) | Conditional elements |
| Array of refs | Lists/dynamic elements |
| Forwarding refs | Wrapper components |

## Why It Matters

1. **Timing**: DOM elements don't exist until after render. Accessing refs too early causes errors.

2. **Conditional Elements**: Elements in `<Show>` may not exist. Signal refs handle this gracefully.

3. **TypeScript**: Proper typing prevents accessing potentially undefined refs.

## Common Ref Use Cases

```tsx
// Focus management
onMount(() => inputRef.focus());

// Scroll into view
onMount(() => elementRef.scrollIntoView());

// Third-party library initialization
onMount(() => {
  const chart = new Chart(canvasRef, config);
  onCleanup(() => chart.destroy());
});

// Measuring elements
const [width, setWidth] = createSignal(0);
onMount(() => setWidth(elementRef.offsetWidth));

// Canvas drawing
onMount(() => {
  const ctx = canvasRef.getContext("2d");
  ctx.fillRect(0, 0, 100, 100);
});
```

## Related Rules

- [5-2: Access DOM in onMount](5-2-access-dom-in-onmount.md) - When to access refs
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Cleaning up ref-based resources
- [5-4: Use Directives](5-4-use-directives.md) - Reusable ref behaviors
