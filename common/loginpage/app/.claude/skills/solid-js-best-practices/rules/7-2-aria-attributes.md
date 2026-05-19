---
id: 7-2
title: Use ARIA Attributes
category: Accessibility
priority: MEDIUM
description: Apply appropriate ARIA attributes for custom controls
---

## Problem

Custom interactive components (dropdowns, modals, tabs) built with generic elements lack the semantic information screen readers need. ARIA (Accessible Rich Internet Applications) attributes bridge this gap by describing roles, states, and relationships.

## Incorrect

```tsx
// ❌ WRONG: Custom button without accessibility
function IconButton(props) {
  return (
    <div class="icon-button" onClick={props.onClick}>
      <Icon name={props.icon} />
    </div>
  );
}
```

```tsx
// ❌ WRONG: Dropdown without ARIA
function Dropdown(props) {
  const [open, setOpen] = createSignal(false);

  return (
    <div class="dropdown">
      <div class="trigger" onClick={() => setOpen(!open())}>
        {props.label}
      </div>
      <Show when={open()}>
        <div class="menu">
          <For each={props.items}>
            {item => <div onClick={item.onClick}>{item.label}</div>}
          </For>
        </div>
      </Show>
    </div>
  );
}
```

## Correct

```tsx
// ✅ CORRECT: Accessible icon button
function IconButton(props) {
  return (
    <button
      class="icon-button"
      onClick={props.onClick}
      aria-label={props.label}
      type="button"
    >
      <Icon name={props.icon} aria-hidden="true" />
    </button>
  );
}

// Usage
<IconButton icon="close" label="Close dialog" onClick={handleClose} />
```

```tsx
// ✅ CORRECT: Accessible dropdown
function Dropdown(props) {
  const [open, setOpen] = createSignal(false);
  const menuId = `menu-${props.id}`;

  return (
    <div class="dropdown">
      <button
        type="button"
        aria-haspopup="listbox"
        aria-expanded={open()}
        aria-controls={menuId}
        onClick={() => setOpen(!open())}
      >
        {props.label}
        <Icon name="chevron-down" aria-hidden="true" />
      </button>

      <Show when={open()}>
        <ul
          id={menuId}
          role="listbox"
          aria-label={props.label}
        >
          <For each={props.items}>
            {item => (
              <li role="option" onClick={item.onClick}>
                {item.label}
              </li>
            )}
          </For>
        </ul>
      </Show>
    </div>
  );
}
```

### Modal Dialog

```tsx
// ✅ CORRECT: Accessible modal
function Modal(props) {
  let dialogRef: HTMLDivElement;
  const titleId = `modal-title-${props.id}`;
  const descId = `modal-desc-${props.id}`;

  onMount(() => {
    dialogRef.focus();
  });

  return (
    <Show when={props.open}>
      <div
        class="modal-backdrop"
        onClick={props.onClose}
        aria-hidden="true"
      />
      <div
        ref={dialogRef}
        role="dialog"
        aria-modal="true"
        aria-labelledby={titleId}
        aria-describedby={descId}
        tabindex="-1"
      >
        <h2 id={titleId}>{props.title}</h2>
        <p id={descId}>{props.description}</p>
        {props.children}
        <button onClick={props.onClose}>Close</button>
      </div>
    </Show>
  );
}
```

### Tabs

```tsx
// ✅ CORRECT: Accessible tabs
function Tabs(props) {
  const [activeTab, setActiveTab] = createSignal(0);

  return (
    <div class="tabs">
      <div role="tablist" aria-label={props.label}>
        <For each={props.tabs}>
          {(tab, index) => (
            <button
              role="tab"
              id={`tab-${index()}`}
              aria-selected={activeTab() === index()}
              aria-controls={`panel-${index()}`}
              tabindex={activeTab() === index() ? 0 : -1}
              onClick={() => setActiveTab(index())}
            >
              {tab.label}
            </button>
          )}
        </For>
      </div>

      <For each={props.tabs}>
        {(tab, index) => (
          <div
            role="tabpanel"
            id={`panel-${index()}`}
            aria-labelledby={`tab-${index()}`}
            hidden={activeTab() !== index()}
            tabindex="0"
          >
            {tab.content}
          </div>
        )}
      </For>
    </div>
  );
}
```

### Live Regions

```tsx
// ✅ CORRECT: Announce dynamic updates
function Notifications() {
  const [message, setMessage] = createSignal("");

  return (
    <div
      role="status"
      aria-live="polite"
      aria-atomic="true"
    >
      {message()}
    </div>
  );
}

// For urgent announcements
function Alert(props) {
  return (
    <div role="alert" aria-live="assertive">
      {props.message}
    </div>
  );
}
```

### Form Accessible Name

A `<form>` element without an accessible name is **not exposed** as `role="form"` in the accessibility tree per the ARIA spec. This has two consequences:

1. `screen.getByRole("form")` throws in tests — the form isn't queryable by role
2. Screen readers don't announce the form as a landmark, making navigation harder

```tsx
// ❌ WRONG: not exposed as role="form" — getByRole('form') throws
<form use:form>
  {/* ... */}
</form>

// ✅ CORRECT: accessible name exposes it as role="form"
<form aria-label="Recipe form" use:form>
  {/* ... */}
</form>

// ✅ CORRECT: using aria-labelledby with a visible heading
<h2 id="recipe-form-heading">New Recipe</h2>
<form aria-labelledby="recipe-form-heading" use:form>
  {/* ... */}
</form>
```

### Form Validation

```tsx
// ✅ CORRECT: Form field with error
function FormField(props) {
  const errorId = `error-${props.name}`;

  return (
    <div class="field">
      <label for={props.name}>{props.label}</label>
      <input
        id={props.name}
        type={props.type}
        aria-invalid={!!props.error}
        aria-describedby={props.error ? errorId : undefined}
        value={props.value}
        onInput={props.onInput}
      />
      <Show when={props.error}>
        <span id={errorId} class="error" role="alert">
          {props.error}
        </span>
      </Show>
    </div>
  );
}
```

## Common ARIA Patterns

| Component | Key ARIA Attributes |
| --------- | ----------------- |
| Modal | `role="dialog"`, `aria-modal`, `aria-labelledby` |
| Dropdown | `aria-haspopup`, `aria-expanded`, `aria-controls` |
| Tabs | `role="tablist/tab/tabpanel"`, `aria-selected` |
| Menu | `role="menu/menuitem"`, `aria-activedescendant` |
| Alert | `role="alert"`, `aria-live="assertive"` |
| Status | `role="status"`, `aria-live="polite"` |
| Toggle | `aria-pressed` or `aria-checked` |
| Loading | `aria-busy="true"` |
| Form | `aria-label` or `aria-labelledby` (required for `role="form"` exposure) |

## ARIA Rules

1. **First Rule**: Don't use ARIA if native HTML works.
2. **No Fake Roles**: ARIA changes announcements, not behavior.
3. **Visible Labels**: `aria-label` or `aria-labelledby` for unnamed elements.
4. **State Updates**: Keep `aria-*` attributes in sync with visual state.

## Why It Matters

1. **Screen Readers**: ARIA provides missing semantic information.

2. **State Communication**: Announces changes (expanded/collapsed, selected).

3. **Relationships**: Connects labels, descriptions, and controls.

4. **Interactive Patterns**: Standard patterns users expect.

## Related Rules

- [7-1: Semantic HTML](7-1-semantic-html.md) - Use before ARIA
- [7-3: Keyboard Navigation](7-3-keyboard-navigation.md) - ARIA needs keyboard support
