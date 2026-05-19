---
id: 7-3
title: Support Keyboard Navigation
category: Accessibility
priority: MEDIUM
description: Ensure all interactive elements are keyboard accessible
---

## Problem

Interactive elements must be usable without a mouse. Users with motor disabilities, power users, and anyone with a broken mouse rely on keyboard navigation. Custom components built with non-interactive elements often lack keyboard support.

## Incorrect

```tsx
// ❌ WRONG: Div not keyboard accessible
function ClickableCard(props) {
  return (
    <div class="card" onClick={props.onClick}>
      {props.children}
    </div>
  );
}
```

```tsx
// ❌ WRONG: Custom select without keyboard
function CustomSelect(props) {
  const [open, setOpen] = createSignal(false);
  const [selected, setSelected] = createSignal(null);

  return (
    <div class="select">
      <div class="trigger" onClick={() => setOpen(!open())}>
        {selected()?.label || "Select..."}
      </div>
      <Show when={open()}>
        <div class="options">
          <For each={props.options}>
            {option => (
              <div
                class="option"
                onClick={() => {
                  setSelected(option);
                  setOpen(false);
                }}
              >
                {option.label}
              </div>
            )}
          </For>
        </div>
      </Show>
    </div>
  );
}
```

## Correct

```tsx
// ✅ CORRECT: Use native interactive element
function ClickableCard(props) {
  return (
    <button class="card" onClick={props.onClick}>
      {props.children}
    </button>
  );
}

// Or if it's a link
function ClickableCard(props) {
  return (
    <a href={props.href} class="card">
      {props.children}
    </a>
  );
}
```

```tsx
// ✅ CORRECT: Custom select with full keyboard support
function CustomSelect(props) {
  const [open, setOpen] = createSignal(false);
  const [selected, setSelected] = createSignal(null);
  const [activeIndex, setActiveIndex] = createSignal(0);
  let triggerRef: HTMLButtonElement;
  let listRef: HTMLUListElement;

  const handleKeyDown = (e: KeyboardEvent) => {
    switch (e.key) {
      case "ArrowDown":
        e.preventDefault();
        if (!open()) {
          setOpen(true);
        } else {
          setActiveIndex(i => Math.min(i + 1, props.options.length - 1));
        }
        break;
      case "ArrowUp":
        e.preventDefault();
        setActiveIndex(i => Math.max(i - 1, 0));
        break;
      case "Enter":
      case " ":
        e.preventDefault();
        if (open()) {
          setSelected(props.options[activeIndex()]);
          setOpen(false);
          triggerRef.focus();
        } else {
          setOpen(true);
        }
        break;
      case "Escape":
        setOpen(false);
        triggerRef.focus();
        break;
      case "Tab":
        setOpen(false);
        break;
    }
  };

  return (
    <div class="select" onKeyDown={handleKeyDown}>
      <button
        ref={triggerRef}
        type="button"
        aria-haspopup="listbox"
        aria-expanded={open()}
        onClick={() => setOpen(!open())}
      >
        {selected()?.label || "Select..."}
      </button>

      <Show when={open()}>
        <ul
          ref={listRef}
          role="listbox"
          tabindex="-1"
        >
          <For each={props.options}>
            {(option, index) => (
              <li
                role="option"
                aria-selected={selected() === option}
                class={activeIndex() === index() ? "active" : ""}
                onClick={() => {
                  setSelected(option);
                  setOpen(false);
                }}
                onMouseEnter={() => setActiveIndex(index())}
              >
                {option.label}
              </li>
            )}
          </For>
        </ul>
      </Show>
    </div>
  );
}
```

### Focus Management for Modals

```tsx
// ✅ CORRECT: Trap focus in modal
function Modal(props) {
  let dialogRef: HTMLDivElement;
  let previousFocus: HTMLElement;

  onMount(() => {
    previousFocus = document.activeElement as HTMLElement;
    dialogRef.focus();
  });

  onCleanup(() => {
    previousFocus?.focus();
  });

  const handleKeyDown = (e: KeyboardEvent) => {
    if (e.key === "Escape") {
      props.onClose();
      return;
    }

    if (e.key === "Tab") {
      const focusable = dialogRef.querySelectorAll(
        'button, [href], input, select, textarea, [tabindex]:not([tabindex="-1"])'
      );
      const first = focusable[0] as HTMLElement;
      const last = focusable[focusable.length - 1] as HTMLElement;

      if (e.shiftKey && document.activeElement === first) {
        e.preventDefault();
        last.focus();
      } else if (!e.shiftKey && document.activeElement === last) {
        e.preventDefault();
        first.focus();
      }
    }
  };

  return (
    <Show when={props.open}>
      <div class="modal-backdrop" aria-hidden="true" />
      <div
        ref={dialogRef}
        role="dialog"
        aria-modal="true"
        tabindex="-1"
        onKeyDown={handleKeyDown}
      >
        {props.children}
        <button onClick={props.onClose}>Close</button>
      </div>
    </Show>
  );
}
```

### Skip Links

```tsx
// ✅ CORRECT: Skip to main content
function Layout(props) {
  return (
    <>
      <a href="#main-content" class="skip-link">
        Skip to main content
      </a>
      <header>
        <nav>{/* Navigation links */}</nav>
      </header>
      <main id="main-content" tabindex="-1">
        {props.children}
      </main>
    </>
  );
}

// CSS
// .skip-link {
//   position: absolute;
//   top: -40px;
// }
// .skip-link:focus {
//   top: 0;
// }
```

### Visible Focus Indicators

```tsx
// ✅ CORRECT: Custom focus style (CSS)
// button:focus-visible {
//   outline: 2px solid var(--focus-color);
//   outline-offset: 2px;
// }

// ❌ WRONG: Removing focus indicator
// *:focus { outline: none; }
```

## Keyboard Patterns

| Key | Common Action |
| --- | ------------- |
| Tab | Move to next focusable element |
| Shift+Tab | Move to previous focusable |
| Enter/Space | Activate button/link |
| Escape | Close modal/dropdown |
| Arrow keys | Navigate within widgets |
| Home/End | First/last item in list |

## Focus Management

| Scenario | Action |
| -------- | ------ |
| Modal opens | Focus first focusable or dialog |
| Modal closes | Restore previous focus |
| Dropdown opens | Focus first option |
| Delete item | Focus next/previous item |
| Form error | Focus first invalid field |

## Why It Matters

1. **Accessibility**: Required for users who can't use a mouse.

2. **Power Users**: Many prefer keyboard for speed.

3. **Assistive Tech**: Screen readers rely on keyboard navigation.

4. **Legal**: Required by WCAG and accessibility laws.

## Testing Keyboard Access

1. Unplug your mouse
2. Tab through the entire page
3. Verify all interactive elements are reachable
4. Check focus indicators are visible
5. Test Escape closes modals/dropdowns
6. Verify arrow key navigation in widgets

## Related Rules

- [7-1: Semantic HTML](7-1-semantic-html.md) - Native keyboard support
- [7-2: ARIA Attributes](7-2-aria-attributes.md) - Announce focus changes
