---
id: 5-4
title: Use Directives
category: Refs & DOM
priority: MEDIUM
description: Use use: directives for reusable element behaviors
---

## Problem

When you need to apply the same DOM behavior to multiple elements (focus management, click outside detection, tooltips, etc.), duplicating the logic in each component leads to repetition. Directives provide a reusable, declarative way to attach behaviors to elements.

## Incorrect

```tsx
// ❌ WRONG: Duplicated click-outside logic
function Dropdown1() {
  let ref: HTMLDivElement;
  const [open, setOpen] = createSignal(false);

  onMount(() => {
    const handler = (e: MouseEvent) => {
      if (!ref.contains(e.target as Node)) {
        setOpen(false);
      }
    };
    document.addEventListener("click", handler);
    onCleanup(() => document.removeEventListener("click", handler));
  });

  return <div ref={ref}>{/* ... */}</div>;
}

// Same logic repeated in Dropdown2, Modal, Popover, etc.
```

## Correct

```tsx
import { Accessor, onCleanup } from "solid-js";

// ✅ CORRECT: Reusable directive
function clickOutside(el: HTMLElement, accessor: Accessor<() => void>) {
  const handler = (e: MouseEvent) => {
    if (!el.contains(e.target as Node)) {
      accessor()?.();
    }
  };

  document.addEventListener("click", handler);
  onCleanup(() => document.removeEventListener("click", handler));
}

// Register for TypeScript
declare module "solid-js" {
  namespace JSX {
    interface Directives {
      clickOutside: () => void;
    }
  }
}

// ✅ CORRECT: Use directive with use: prefix
function Dropdown() {
  const [open, setOpen] = createSignal(false);

  return (
    <div use:clickOutside={() => setOpen(false)}>
      <Show when={open()}>
        <DropdownMenu />
      </Show>
    </div>
  );
}

function Modal(props) {
  return (
    <div class="modal" use:clickOutside={props.onClose}>
      {props.children}
    </div>
  );
}
```

### Auto-Focus Directive

```tsx
// ✅ CORRECT: Simple auto-focus directive
function autofocus(el: HTMLElement) {
  onMount(() => el.focus());
}

declare module "solid-js" {
  namespace JSX {
    interface Directives {
      autofocus: boolean;
    }
  }
}

// Usage
<input use:autofocus />
<input use:autofocus={shouldFocus()} />
```

### Tooltip Directive

```tsx
import { Accessor, onCleanup } from "solid-js";

interface TooltipOptions {
  text: string;
  position?: "top" | "bottom" | "left" | "right";
}

function tooltip(el: HTMLElement, accessor: Accessor<TooltipOptions>) {
  let tooltipEl: HTMLDivElement | null = null;

  const show = () => {
    const options = accessor();
    tooltipEl = document.createElement("div");
    tooltipEl.className = `tooltip tooltip-${options.position || "top"}`;
    tooltipEl.textContent = options.text;
    document.body.appendChild(tooltipEl);
    // Position logic...
  };

  const hide = () => {
    tooltipEl?.remove();
    tooltipEl = null;
  };

  el.addEventListener("mouseenter", show);
  el.addEventListener("mouseleave", hide);

  onCleanup(() => {
    el.removeEventListener("mouseenter", show);
    el.removeEventListener("mouseleave", hide);
    hide();
  });
}

declare module "solid-js" {
  namespace JSX {
    interface Directives {
      tooltip: TooltipOptions;
    }
  }
}

// Usage
<button use:tooltip={{ text: "Click to save", position: "bottom" }}>
  Save
</button>
```

### Intersection Observer Directive

```tsx
import { Accessor, onCleanup } from "solid-js";

function inView(
  el: HTMLElement,
  accessor: Accessor<(visible: boolean) => void>
) {
  const observer = new IntersectionObserver(([entry]) => {
    accessor()(entry.isIntersecting);
  });

  observer.observe(el);
  onCleanup(() => observer.disconnect());
}

declare module "solid-js" {
  namespace JSX {
    interface Directives {
      inView: (visible: boolean) => void;
    }
  }
}

// Usage - lazy load images
function LazyImage(props) {
  const [visible, setVisible] = createSignal(false);

  return (
    <div use:inView={setVisible}>
      <Show when={visible()} fallback={<Placeholder />}>
        <img src={props.src} alt={props.alt} />
      </Show>
    </div>
  );
}
```

### Multiple Directives

```tsx
// ✅ CORRECT: Multiple directives on one element
<input
  use:autofocus
  use:validate={validationRules}
  use:mask="phone"
/>
```

## Directive Signature

```tsx
function directiveName(
  el: HTMLElement,                    // The DOM element
  accessor: Accessor<ValueType>       // Function returning the directive value
) {
  // Setup code
  const value = accessor();  // Get the value

  // Optional cleanup
  onCleanup(() => {
    // Cleanup code
  });
}
```

## When to Use Directives

| Use Case | Directive? |
| -------- | ---------- |
| Reusable DOM behavior | Yes |
| One-off DOM manipulation | `onMount` + ref |
| Reactive DOM updates | `createEffect` |
| Component-level logic | Regular component |

## Why It Matters

1. **Reusability**: Write once, use across many elements.

2. **Encapsulation**: Behavior logic is self-contained with cleanup.

3. **Declarative**: `use:directiveName` clearly shows element behaviors.

4. **Composable**: Multiple directives can be combined on one element.

## Related Rules

- [5-1: Use Refs Correctly](5-1-use-refs-correctly.md) - When to use refs vs directives
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Cleanup in directives
