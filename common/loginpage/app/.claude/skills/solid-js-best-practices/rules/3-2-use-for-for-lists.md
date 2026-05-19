---
id: 3-2
title: Use For for Lists
category: Control Flow
priority: HIGH
description: Use <For> for referentially-keyed list rendering
---

## Problem

Using JavaScript's `.map()` method for rendering lists works, but it recreates all DOM nodes when the array changes. The `<For>` component is optimized for list rendering, updating only the items that actually changed.

## Incorrect

```tsx
// ❌ WRONG: .map() recreates all items on any change
function TodoList(props) {
  return (
    <ul>
      {props.items.map((item) => (
        <li>{item.text}</li>
      ))}
    </ul>
  );
}
```

```tsx
// ❌ WRONG: Even with keys, .map() isn't optimized in Solid
function UserList(props) {
  return (
    <ul>
      {props.users.map((user) => (
        <li key={user.id}>{user.name}</li>
      ))}
    </ul>
  );
}
```

## Correct

```tsx
import { For } from "solid-js";

// ✅ CORRECT: For component with referential keying
function TodoList(props) {
  return (
    <ul>
      <For each={props.items}>
        {(item) => <li>{item.text}</li>}
      </For>
    </ul>
  );
}
```

### With Index Access

```tsx
import { For } from "solid-js";

// ✅ CORRECT: Second parameter is reactive index accessor
function NumberedList(props) {
  return (
    <ol>
      <For each={props.items}>
        {(item, index) => (
          <li>
            {index() + 1}. {item.name}
          </li>
        )}
      </For>
    </ol>
  );
}
```

### With Fallback

```tsx
import { For } from "solid-js";

// ✅ CORRECT: Fallback for empty arrays
function ProductList(props) {
  return (
    <div>
      <For each={props.products} fallback={<p>No products found</p>}>
        {(product) => (
          <ProductCard
            name={product.name}
            price={product.price}
          />
        )}
      </For>
    </div>
  );
}
```

### Nested Lists

```tsx
import { For } from "solid-js";

function CategoryList(props) {
  return (
    <div>
      <For each={props.categories}>
        {(category) => (
          <section>
            <h2>{category.name}</h2>
            <For each={category.items}>
              {(item) => <ItemCard item={item} />}
            </For>
          </section>
        )}
      </For>
    </div>
  );
}
```

### With Reactive Item Properties

```tsx
import { For } from "solid-js";
import { createStore } from "solid-js/store";

function EditableList() {
  const [items, setItems] = createStore([
    { id: 1, text: "First", done: false },
    { id: 2, text: "Second", done: true }
  ]);

  return (
    <ul>
      <For each={items}>
        {(item, i) => (
          <li>
            <input
              type="checkbox"
              checked={item.done}
              onChange={() => setItems(i(), "done", d => !d)}
            />
            {item.text}
          </li>
        )}
      </For>
    </ul>
  );
}
```

## Why It Matters

1. **Referential Keying**: `<For>` tracks items by reference. When items move or change, only affected DOM nodes update.

2. **Performance**: Adding/removing/reordering items doesn't recreate the entire list.

3. **State Preservation**: Component instances and their local state are preserved when items move.

4. **Reactive Index**: The index accessor is reactive, updating only when position changes.

## For vs Index

| Component | Keys By | Best For |
| --------- | ------- | -------- |
| `<For>` | Object reference | Objects, database records |
| `<Index>` | Array index | Primitives, fixed-position data |

```tsx
// For: item identity matters
<For each={users}>{user => <UserRow user={user} />}</For>

// Index: position matters more than identity
<Index each={numbers}>{(num, i) => <Cell value={num()} row={i} />}</Index>
```

## For Props

| Prop | Type | Description |
| ---- | ---- | ----------- |
| `each` | `T[]` | Array to iterate over |
| `fallback` | `JSX.Element` | Rendered when array is empty |
| `children` | `(item: T, index: () => number) => JSX.Element` | Render function |

## Related Rules

- [3-3: Use Index for Primitives](3-3-use-index-for-primitives.md) - When to use Index instead
- [3-5: Provide Fallbacks](3-5-provide-fallbacks.md) - Empty state handling
- [4-2: Store Path Updates](4-2-store-path-updates.md) - Updating items in lists
