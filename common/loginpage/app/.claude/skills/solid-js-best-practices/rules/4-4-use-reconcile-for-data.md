---
id: 4-4
title: Use reconcile for Server Data
category: State Management
priority: MEDIUM
description: Use reconcile when integrating server/external data
---

## Problem

When replacing store data with data from an external source (API, WebSocket, etc.), using direct assignment or spreading causes all subscribed components to re-render, even for unchanged values. The `reconcile` utility diffs the data and only updates what actually changed.

## Incorrect

```tsx
import { createStore } from "solid-js/store";

function UserDashboard() {
  const [data, setData] = createStore({
    users: [],
    lastUpdated: null
  });

  const fetchUsers = async () => {
    const response = await fetch("/api/users");
    const users = await response.json();

    // ❌ WRONG: Replaces entire array, all items re-render
    setData("users", users);
  };

  const pollForUpdates = async () => {
    const response = await fetch("/api/users");
    const newUsers = await response.json();

    // ❌ WRONG: Even unchanged users trigger updates
    setData({
      users: newUsers,
      lastUpdated: Date.now()
    });
  };
}
```

## Correct

```tsx
import { createStore, reconcile } from "solid-js/store";

function UserDashboard() {
  const [data, setData] = createStore({
    users: [],
    lastUpdated: null
  });

  const fetchUsers = async () => {
    const response = await fetch("/api/users");
    const users = await response.json();

    // ✅ CORRECT: Only updates changed users
    setData("users", reconcile(users));
  };

  const pollForUpdates = async () => {
    const response = await fetch("/api/users");
    const newUsers = await response.json();

    // ✅ CORRECT: Diff-based update
    setData("users", reconcile(newUsers));
    setData("lastUpdated", Date.now());
  };
}
```

### With Custom Key

```tsx
import { createStore, reconcile } from "solid-js/store";

function OrderList() {
  const [state, setState] = createStore({
    orders: []
  });

  const refreshOrders = async () => {
    const orders = await fetchOrders();

    // ✅ CORRECT: Use 'id' to match items
    setState("orders", reconcile(orders, { key: "id" }));
  };
}
```

### Merging Partial Updates

```tsx
import { createStore, reconcile } from "solid-js/store";

function RealTimeData() {
  const [state, setState] = createStore({
    stocks: {
      AAPL: { price: 150, change: 0 },
      GOOGL: { price: 2800, change: 0 }
    }
  });

  // WebSocket update with partial data
  socket.onmessage = (event) => {
    const update = JSON.parse(event.data);

    // ✅ CORRECT: Merge partial update, preserve unchanged
    setState("stocks", reconcile(
      { ...state.stocks, ...update },
      { merge: true }
    ));
  };
}
```

### With Nested Data

```tsx
import { createStore, reconcile } from "solid-js/store";

function ComplexDashboard() {
  const [data, setData] = createStore({
    departments: [
      {
        id: 1,
        name: "Engineering",
        employees: [
          { id: 101, name: "Alice", role: "Lead" },
          { id: 102, name: "Bob", role: "Developer" }
        ]
      }
    ]
  });

  const refreshDepartment = async (deptId: number) => {
    const dept = await fetchDepartment(deptId);

    // ✅ CORRECT: Reconcile nested data
    setState(
      "departments",
      d => d.id === deptId,
      reconcile(dept, { key: "id" })
    );
  };
}
```

## reconcile Options

| Option | Type | Default | Description |
| ------ | ---- | ------- | ----------- |
| `key` | `string` | `"id"` | Property to match array items |
| `merge` | `boolean` | `false` | Merge with existing instead of replace |

## How reconcile Works

```tsx
// Old data
const old = [
  { id: 1, name: "Alice", score: 100 },
  { id: 2, name: "Bob", score: 85 }
];

// New data from API
const new = [
  { id: 1, name: "Alice", score: 105 },  // score changed
  { id: 2, name: "Bob", score: 85 },     // unchanged
  { id: 3, name: "Carol", score: 90 }    // new item
];

setData("users", reconcile(new, { key: "id" }));
// Result:
// - User 1: Only 'score' property updates
// - User 2: No update (unchanged)
// - User 3: New item added
```

## When to Use reconcile

| Scenario | Use reconcile? |
| -------- | -------------- |
| Initial data fetch | Optional (no previous data to diff) |
| Polling/refresh | Yes |
| WebSocket updates | Yes |
| User-triggered refresh | Yes |
| Complete data replacement | Yes (minimizes re-renders) |
| Known single-item update | Path syntax may be simpler |

## Why It Matters

1. **Performance**: Only changed properties trigger reactive updates.

2. **DOM Stability**: List items aren't recreated if their data hasn't changed.

3. **State Preservation**: Component local state survives data refreshes for unchanged items.

4. **Network Efficiency Pattern**: Encourages fetching full data without performance penalty.

## Related Rules

- [4-2: Store Path Updates](4-2-store-path-updates.md) - For known changes
- [4-3: Use produce for Mutations](4-3-use-produce-for-mutations.md) - For local mutations
- [3-2: Use For for Lists](3-2-use-for-for-lists.md) - List rendering with reconciled data
