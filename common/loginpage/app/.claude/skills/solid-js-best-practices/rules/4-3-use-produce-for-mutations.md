---
id: 4-3
title: Use produce for Mutations
category: State Management
priority: MEDIUM
description: Use produce for complex mutable-style store updates
---

## Problem

When you need to make multiple changes to a store at once, or when mutable-style code is clearer, the path syntax can become verbose. The `produce` utility lets you write mutations in a mutable style that's automatically converted to immutable updates.

## Incorrect

```tsx
import { createStore } from "solid-js/store";

function FormEditor() {
  const [form, setForm] = createStore({
    fields: {
      name: { value: "", touched: false, error: null },
      email: { value: "", touched: false, error: null }
    },
    isSubmitting: false,
    submitCount: 0
  });

  // ❌ WRONG: Verbose multi-property update
  const submitForm = async () => {
    setForm("isSubmitting", true);
    setForm("submitCount", c => c + 1);
    setForm("fields", "name", "touched", true);
    setForm("fields", "email", "touched", true);
    // ... many more updates
  };

  // ❌ WRONG: Replacing objects defeats fine-grained reactivity
  const resetForm = () => {
    setForm({
      fields: {
        name: { value: "", touched: false, error: null },
        email: { value: "", touched: false, error: null }
      },
      isSubmitting: false,
      submitCount: 0
    });
  };
}
```

## Correct

```tsx
import { createStore, produce } from "solid-js/store";

function FormEditor() {
  const [form, setForm] = createStore({
    fields: {
      name: { value: "", touched: false, error: null },
      email: { value: "", touched: false, error: null }
    },
    isSubmitting: false,
    submitCount: 0
  });

  // ✅ CORRECT: produce for multiple mutations
  const submitForm = async () => {
    setForm(produce(draft => {
      draft.isSubmitting = true;
      draft.submitCount++;
      draft.fields.name.touched = true;
      draft.fields.email.touched = true;
    }));
  };

  // ✅ CORRECT: produce for complex reset
  const resetForm = () => {
    setForm(produce(draft => {
      draft.isSubmitting = false;
      draft.submitCount = 0;
      for (const key in draft.fields) {
        draft.fields[key].value = "";
        draft.fields[key].touched = false;
        draft.fields[key].error = null;
      }
    }));
  };
}
```

### With Path Prefix

```tsx
import { createStore, produce } from "solid-js/store";

function UserEditor() {
  const [state, setState] = createStore({
    users: [
      { id: 1, name: "Alice", roles: ["admin"], active: true },
      { id: 2, name: "Bob", roles: ["user"], active: false }
    ]
  });

  // ✅ CORRECT: Combine path with produce
  const updateUser = (index: number) => {
    setState("users", index, produce(user => {
      user.name = user.name.toUpperCase();
      user.roles.push("verified");
      user.active = true;
    }));
  };

  // ✅ CORRECT: Update matching items
  const activateAll = () => {
    setState("users", user => !user.active, produce(user => {
      user.active = true;
      user.roles.push("activated");
    }));
  };
}
```

### Array Mutations

```tsx
import { createStore, produce } from "solid-js/store";

function PlaylistManager() {
  const [state, setState] = createStore({
    playlists: [
      { id: 1, name: "Favorites", songs: ["song1", "song2"] },
      { id: 2, name: "Workout", songs: ["song3"] }
    ]
  });

  // ✅ CORRECT: Array mutations with produce
  const addSong = (playlistId: number, songId: string) => {
    setState("playlists", p => p.id === playlistId, produce(playlist => {
      playlist.songs.push(songId);
    }));
  };

  const removeSong = (playlistId: number, songId: string) => {
    setState("playlists", p => p.id === playlistId, produce(playlist => {
      const index = playlist.songs.indexOf(songId);
      if (index > -1) {
        playlist.songs.splice(index, 1);
      }
    }));
  };

  const reorderSongs = (playlistId: number, from: number, to: number) => {
    setState("playlists", p => p.id === playlistId, produce(playlist => {
      const [song] = playlist.songs.splice(from, 1);
      playlist.songs.splice(to, 0, song);
    }));
  };
}
```

## When to Use produce vs Path Syntax

| Scenario | Recommended |
| -------- | ----------- |
| Single property update | Path syntax |
| Multiple related updates | `produce` |
| Conditional mutations | `produce` |
| Array push/splice/sort | `produce` |
| Loop-based updates | `produce` |
| Simple increment/toggle | Path syntax with function |

## How produce Works

```tsx
// produce takes a mutator function and returns an updater
const updater = produce(draft => {
  // Mutations to draft are tracked
  draft.count++;
  draft.items.push(newItem);
});

// The updater is passed to setStore
setStore(updater);
// Or with path prefix
setStore("nested", "path", updater);
```

The draft is a proxy that records your mutations and applies them as immutable updates to the actual store.

## Why It Matters

1. **Readability**: Mutable-style code is often clearer for complex updates.

2. **Atomicity**: All mutations in a produce block are batched into one update.

3. **Array Operations**: Native array methods like `push`, `splice`, `sort` work naturally.

4. **Loops and Conditions**: Easy to use control flow in mutations.

## Related Rules

- [4-2: Store Path Updates](4-2-store-path-updates.md) - When path syntax is better
- [4-4: Use reconcile for Data](4-4-use-reconcile-for-data.md) - For replacing data from external sources
- [4-1: Signals vs Stores](4-1-signals-vs-stores.md) - When to use stores
