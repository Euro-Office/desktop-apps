---
id: 4-1
title: Signals vs Stores
category: State Management
priority: HIGH
description: Use signals for primitives, stores for nested objects
---

## Problem

Choosing the wrong state primitive leads to either verbosity (using stores for simple values) or lost reactivity (using signals for nested objects). Signals are for primitive values and simple objects; stores are for nested, complex state that needs granular updates.

## Incorrect

```tsx
import { createSignal } from "solid-js";

// ❌ WRONG: Signal for complex nested state
function UserManager() {
  const [user, setUser] = createSignal({
    profile: {
      name: "John",
      email: "john@example.com",
      preferences: {
        theme: "dark",
        notifications: true
      }
    },
    posts: []
  });

  // Updating nested values is verbose and replaces entire object
  const updateTheme = (theme: string) => {
    setUser(prev => ({
      ...prev,
      profile: {
        ...prev.profile,
        preferences: {
          ...prev.profile.preferences,
          theme
        }
      }
    }));
  };

  return <div>{user().profile.preferences.theme}</div>;
}
```

```tsx
import { createStore } from "solid-js/store";

// ❌ WRONG: Store for simple primitive
function Counter() {
  const [state, setState] = createStore({ count: 0 });

  // Overkill for a single value
  return (
    <button onClick={() => setState("count", c => c + 1)}>
      {state.count}
    </button>
  );
}
```

## Correct

```tsx
import { createSignal } from "solid-js";

// ✅ CORRECT: Signal for primitive/simple values
function Counter() {
  const [count, setCount] = createSignal(0);

  return (
    <button onClick={() => setCount(c => c + 1)}>
      {count()}
    </button>
  );
}
```

```tsx
import { createStore } from "solid-js/store";

// ✅ CORRECT: Store for complex nested state
function UserManager() {
  const [user, setUser] = createStore({
    profile: {
      name: "John",
      email: "john@example.com",
      preferences: {
        theme: "dark",
        notifications: true
      }
    },
    posts: []
  });

  // Granular update with path syntax
  const updateTheme = (theme: string) => {
    setUser("profile", "preferences", "theme", theme);
  };

  return <div>{user.profile.preferences.theme}</div>;
}
```

### Mixed Usage

```tsx
// ✅ CORRECT: Signals for primitives, store for complex state
function Dashboard() {
  // Simple values as signals
  const [isLoading, setIsLoading] = createSignal(false);
  const [activeTab, setActiveTab] = createSignal("overview");

  // Complex nested data as store
  const [data, setData] = createStore({
    users: [],
    stats: {
      daily: { views: 0, clicks: 0 },
      weekly: { views: 0, clicks: 0 }
    },
    filters: {
      dateRange: "week",
      category: "all"
    }
  });

  return (
    <Show when={!isLoading()}>
      <Tabs active={activeTab()} onChange={setActiveTab} />
      <StatsPanel stats={data.stats} />
      <Filters filters={data.filters} />
    </Show>
  );
}
```

## Decision Guide

| Data Type | Use | Example |
| --------- | --- | ------- |
| Primitive (number, string, boolean) | `createSignal` | `count`, `isOpen`, `userName` |
| Simple object (flat, rarely updated) | `createSignal` | `{ x: 0, y: 0 }` |
| Array of primitives | Either | `[1, 2, 3]` |
| Nested object | `createStore` | User profile, form state |
| Array of objects | `createStore` | Todo list, data table |
| Object with frequent partial updates | `createStore` | Settings, filters |

## Key Differences

| Aspect | Signal | Store |
| ------ | ------ | ----- |
| Access | `signal()` | `store.prop` |
| Update | `setSignal(newValue)` | `setStore("path", value)` |
| Reactivity | Whole value | Per-property |
| Nested updates | Replace entire object | Path-based granular |
| Best for | Primitives, simple values | Complex nested state |

## Store Advantages for Complex State

```tsx
const [form, setForm] = createStore({
  fields: {
    name: { value: "", error: null },
    email: { value: "", error: null }
  },
  isValid: false
});

// Granular update - only the name field re-renders
setForm("fields", "name", "value", "John");

// Accessing a value only subscribes to that value
<input value={form.fields.name.value} />
// This won't re-render when email changes
```

## Why It Matters

1. **Performance**: Stores provide fine-grained reactivity. Changing a nested value only updates subscribers to that specific property.

2. **Simplicity**: Signals are simpler for simple values. Don't add complexity where it's not needed.

3. **Update Ergonomics**: Stores have path-based updates. Signals require spreading nested objects.

## Related Rules

- [4-2: Store Path Updates](4-2-store-path-updates.md) - Efficient store updates
- [4-3: Use produce for Mutations](4-3-use-produce-for-mutations.md) - Mutable-style updates
- [1-1: Use Signals Correctly](1-1-use-signals-correctly.md) - Signal access patterns
