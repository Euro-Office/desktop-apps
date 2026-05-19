---
id: 4-2
title: Use Store Path Syntax
category: State Management
priority: HIGH
description: Use path syntax for granular, efficient store updates
---

## Problem

Updating stores by replacing entire objects defeats the purpose of fine-grained reactivity. The path syntax allows targeted updates that only trigger re-renders for the specific properties that changed.

## Incorrect

```tsx
import { createStore } from "solid-js/store";

function Settings() {
  const [settings, setSettings] = createStore({
    theme: "light",
    notifications: {
      email: true,
      push: false,
      frequency: "daily"
    },
    privacy: {
      shareData: false,
      analytics: true
    }
  });

  // ❌ WRONG: Replacing entire object
  const togglePush = () => {
    setSettings({
      ...settings,
      notifications: {
        ...settings.notifications,
        push: !settings.notifications.push
      }
    });
  };

  // ❌ WRONG: Still replacing nested object
  const updateNotifications = (key: string, value: boolean) => {
    setSettings("notifications", {
      ...settings.notifications,
      [key]: value
    });
  };

  return <div>{/* ... */}</div>;
}
```

## Correct

```tsx
import { createStore } from "solid-js/store";

function Settings() {
  const [settings, setSettings] = createStore({
    theme: "light",
    notifications: {
      email: true,
      push: false,
      frequency: "daily"
    },
    privacy: {
      shareData: false,
      analytics: true
    }
  });

  // ✅ CORRECT: Path syntax for granular update
  const togglePush = () => {
    setSettings("notifications", "push", p => !p);
  };

  // ✅ CORRECT: Dynamic path
  const updateNotifications = (key: string, value: boolean) => {
    setSettings("notifications", key, value);
  };

  return <div>{/* ... */}</div>;
}
```

### Array Updates

```tsx
import { createStore } from "solid-js/store";

function TodoApp() {
  const [state, setState] = createStore({
    todos: [
      { id: 1, text: "Learn Solid", done: false },
      { id: 2, text: "Build app", done: false }
    ]
  });

  // ✅ CORRECT: Update by index
  const toggleTodo = (index: number) => {
    setState("todos", index, "done", d => !d);
  };

  // ✅ CORRECT: Append to array
  const addTodo = (text: string) => {
    setState("todos", state.todos.length, {
      id: Date.now(),
      text,
      done: false
    });
  };

  // ✅ CORRECT: Update multiple items with filter
  const markAllDone = () => {
    setState("todos", todo => !todo.done, "done", true);
  };

  // ✅ CORRECT: Update range of items
  const toggleRange = (from: number, to: number) => {
    setState("todos", { from, to }, "done", d => !d);
  };

  return <div>{/* ... */}</div>;
}
```

### Using Functional Updates

```tsx
// ✅ CORRECT: Functional update for current value
setSettings("notifications", "frequency", freq =>
  freq === "daily" ? "weekly" : "daily"
);

// ✅ CORRECT: Increment a counter
setState("stats", "views", v => v + 1);

// ✅ CORRECT: Toggle boolean
setState("ui", "sidebar", "open", open => !open);
```

### Complex Path Operations

```tsx
const [data, setData] = createStore({
  users: [
    { id: 1, name: "Alice", roles: ["admin"] },
    { id: 2, name: "Bob", roles: ["user"] }
  ]
});

// Update user by ID (using filter function)
const updateUserName = (id: number, name: string) => {
  setData("users", user => user.id === id, "name", name);
};

// Add role to specific user
const addRole = (userId: number, role: string) => {
  setData(
    "users",
    user => user.id === userId,
    "roles",
    roles => [...roles, role]
  );
};
```

## Path Syntax Reference

```tsx
// Basic path
setStore("key", value);
setStore("key1", "key2", value);
setStore("key1", "key2", "key3", value);

// Array index
setStore("items", 0, value);
setStore("items", 0, "prop", value);

// Filter function (updates all matching)
setStore("items", item => item.active, "prop", value);

// Range object
setStore("items", { from: 0, to: 5 }, "prop", value);
setStore("items", { from: 0, to: 10, by: 2 }, "prop", value);

// Multiple indices
setStore("items", [0, 2, 4], "prop", value);

// Functional update (receives current value)
setStore("key", current => newValue);
setStore("key1", "key2", current => current + 1);
```

## Why It Matters

1. **Fine-Grained Updates**: Only components subscribed to the changed property re-render.

2. **Performance**: No need to clone and spread nested objects.

3. **Clarity**: Path syntax clearly shows what's being updated.

4. **Batch Updates**: Multiple path updates in one `setStore` call are batched.

## Related Rules

- [4-1: Signals vs Stores](4-1-signals-vs-stores.md) - When to use stores
- [4-3: Use produce for Mutations](4-3-use-produce-for-mutations.md) - Alternative mutation style
- [3-2: Use For for Lists](3-2-use-for-for-lists.md) - Rendering store arrays
