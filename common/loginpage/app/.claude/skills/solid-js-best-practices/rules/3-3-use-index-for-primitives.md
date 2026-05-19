---
id: 3-3
title: Use Index for Primitives
category: Control Flow
priority: MEDIUM
description: Use <Index> when array index matters more than identity
---

## Problem

`<For>` tracks items by reference, which works well for objects. But for primitive arrays (numbers, strings), values don't have stable references. `<Index>` tracks by array index instead, which is more appropriate when the position matters more than the value's identity.

## Incorrect

```tsx
import { For } from "solid-js";

// ❌ WRONG: For with primitives - values aren't stable references
function NumberGrid() {
  const [numbers, setNumbers] = createSignal([1, 2, 3, 4, 5]);

  return (
    <div>
      <For each={numbers()}>
        {(num) => <Cell value={num} />}
      </For>
    </div>
  );
}
```

```tsx
// ❌ WRONG: For treats identical values as same item
function TagList() {
  const [tags, setTags] = createSignal(["react", "solid", "react"]); // duplicate

  return (
    <For each={tags()}>
      {(tag) => <Tag name={tag} />}
    </For>
  );
}
```

## Correct

```tsx
import { Index } from "solid-js";

// ✅ CORRECT: Index tracks by position
function NumberGrid() {
  const [numbers, setNumbers] = createSignal([1, 2, 3, 4, 5]);

  return (
    <div>
      <Index each={numbers()}>
        {(num, i) => <Cell value={num()} position={i} />}
      </Index>
    </div>
  );
}
```

```tsx
// ✅ CORRECT: Index handles duplicate primitives correctly
function TagList() {
  const [tags, setTags] = createSignal(["react", "solid", "react"]);

  return (
    <Index each={tags()}>
      {(tag, i) => <Tag name={tag()} key={i} />}
    </Index>
  );
}
```

### Note the Accessor Pattern

```tsx
import { Index } from "solid-js";

// In Index, the item is a signal (accessor), index is static
function ScoreBoard() {
  const [scores, setScores] = createSignal([100, 85, 92, 78]);

  return (
    <table>
      <Index each={scores()}>
        {(score, i) => (
          <tr>
            <td>Player {i + 1}</td>
            {/* score is an accessor - call it! */}
            <td>{score()}</td>
          </tr>
        )}
      </Index>
    </table>
  );
}
```

### Updating Values

```tsx
import { Index, createSignal } from "solid-js";

function EditableGrid() {
  const [values, setValues] = createSignal(["a", "b", "c"]);

  const updateValue = (index: number, newValue: string) => {
    setValues(prev => {
      const next = [...prev];
      next[index] = newValue;
      return next;
    });
  };

  return (
    <Index each={values()}>
      {(value, i) => (
        <input
          value={value()}
          onInput={(e) => updateValue(i, e.currentTarget.value)}
        />
      )}
    </Index>
  );
}
```

## For vs Index Comparison

| Aspect | `<For>` | `<Index>` |
| ------ | ------- | --------- |
| Tracks by | Reference | Index |
| Item param | Direct value | Accessor function |
| Index param | Accessor function | Static number |
| Best for | Objects | Primitives |
| On reorder | Moves DOM nodes | Updates content |

### Behavior Difference

```tsx
// Array: ["a", "b", "c"] → ["x", "b", "c"]

// For: Creates new DOM node for "x", removes "a" node
<For each={arr}>{item => <span>{item}</span>}</For>

// Index: Updates first span's content from "a" to "x"
<Index each={arr}>{item => <span>{item()}</span>}</Index>
```

## When to Use Each

| Data Type | Recommended |
| --------- | ----------- |
| Array of objects with IDs | `<For>` |
| Array of primitives (numbers, strings) | `<Index>` |
| Fixed-size grid or matrix | `<Index>` |
| List where items move/reorder | `<For>` |
| List where values change in place | `<Index>` |

## Common Patterns

```tsx
// Spreadsheet-like grid (values change at positions)
<Index each={row()}>
  {(cell, col) => <Cell value={cell()} row={rowIndex} col={col} />}
</Index>

// Character-by-character display
<Index each={word().split("")}>
  {(char) => <span class="letter">{char()}</span>}
</Index>

// Numeric data series
<Index each={dataPoints()}>
  {(value, i) => <Bar height={value()} position={i} />}
</Index>
```

## Related Rules

- [3-2: Use For for Lists](3-2-use-for-for-lists.md) - When to use For instead
- [4-2: Store Path Updates](4-2-store-path-updates.md) - Efficient list updates
