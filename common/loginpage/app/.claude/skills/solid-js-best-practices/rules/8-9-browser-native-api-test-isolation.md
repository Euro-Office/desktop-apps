---
id: 8-9
title: Browser-Native API Test Isolation
category: Testing
priority: HIGH
description: Clear IndexedDB and localStorage between tests in browser mode — real browser storage persists across tests and causes cross-test contamination
---

## Problem

In Vitest browser mode, tests run in a real Chromium instance. Browser-native storage APIs — IndexedDB and localStorage — persist across tests within the same session unless you explicitly clear them. State left by one test contaminates the next, causing intermittent failures that depend on test order.

IndexedDB has an additional gotcha: you cannot delete a database while a connection to it is open. The `deleteDatabase` request blocks silently if any connection holds a transaction. You must close the connection first.

## Incorrect

```typescript
// ❌ WRONG: No cleanup — IDB data from test A leaks into test B
beforeEach(async () => {
  // Nothing here
});
```

```typescript
// ❌ WRONG: Resolving in onblocked skips the actual deletion
async function resetDb(): Promise<void> {
  await new Promise<void>((resolve, reject) => {
    const req = indexedDB.deleteDatabase("my-app");
    req.onsuccess = () => resolve();
    req.onerror = () => reject(req.error);
    req.onblocked = () => resolve(); // ❌ resolves before deletion completes
  });
}
```

```typescript
// ❌ WRONG: Deleting without closing the connection first — blocks indefinitely
async function resetDb(): Promise<void> {
  await new Promise<void>((resolve, reject) => {
    const req = indexedDB.deleteDatabase("my-app"); // blocks if db is open
    req.onsuccess = () => resolve();
    req.onerror = () => reject(req.error);
  });
}
```

## Correct

### Step 1 — Test Escape Hatch in db.ts

Export a reset function from your database module that closes the connection and clears the module-level promise. Do not call this in production code.

```typescript
// src/db.ts
let dbPromise: Promise<IDBDatabase> | null = null;

export function getDb(): Promise<IDBDatabase> {
  if (!dbPromise) {
    dbPromise = openDatabase();
  }
  return dbPromise;
}

// Test-only escape hatch — not exported from the production bundle entry point
export async function _resetDbForTest(): Promise<void> {
  if (dbPromise) {
    const db = await dbPromise;
    db.close();       // Must close before deleteDatabase or the request blocks
    dbPromise = null; // Allow getDb() to re-open on next call
  }
}
```

### Step 2 — resetDb and useCleanDb Helpers

```typescript
// src/test-helpers/dbHelpers.ts
import { _resetDbForTest } from "../db";

export async function resetDb(): Promise<void> {
  await _resetDbForTest(); // close connection first

  await new Promise<void>((resolve, reject) => {
    const req = indexedDB.deleteDatabase("my-app");
    req.onsuccess = () => resolve();
    req.onerror = () => reject(req.error);
    // onblocked must be a no-op — NOT resolve().
    // The delete is NOT complete when onblocked fires; onsuccess fires once
    // all pending transactions drain. Resolving early skips the deletion.
    req.onblocked = () => {};
  });
}

/** Call inside a describe block to reset storage before each test. */
export function useCleanDb() {
  beforeEach(async () => {
    await resetDb();
    localStorage.clear(); // Clear any persisted strategy or session data
  });
}
```

### Step 3 — Use in Tests

```typescript
// src/routes/RecipesListPage.integration.test.tsx
import { useCleanDb } from "../test-helpers/dbHelpers";

describe("RecipesListPage", () => {
  useCleanDb(); // resets IDB + localStorage before each test

  it("shows empty list when no recipes exist", async () => {
    renderWithProviders(RecipesListPage);
    await screen.findByRole("heading", { name: /recipes/i });
    expect(screen.queryAllByRole("listitem")).toHaveLength(0);
  });

  it("shows seeded recipes", async () => {
    await seedRecipes([{ title: "Pasta", category: "Italian" }]);
    renderWithProviders(RecipesListPage);
    await screen.findByText("Pasta");
  });
});
```

## Why `onblocked` Must Be a No-Op

The `IDBOpenDBRequest.onblocked` callback fires when `deleteDatabase` cannot proceed because another connection is still open. At this point the deletion has **not** happened — calling `resolve()` here would make the promise resolve before the database is actually deleted. The pattern is:

1. Close all connections (`db.close()`) before calling `deleteDatabase`
2. Set `onblocked = () => {}` — the no-op means "I've already closed, this should drain soon"
3. The `onsuccess` callback fires once all pending transactions complete and the database is gone

## Why It Matters

1. **Order-dependent failures**: Tests that assume a clean database pass in isolation but fail when run after a test that writes data.

2. **Silent contamination**: Stale IDB data doesn't throw — queries return old data, causing assertion failures that are hard to trace.

3. **Blocking deletes**: Deleting IDB without closing the connection first causes `deleteDatabase` to block indefinitely, hanging the test suite.

## Related Rules

- [8-7: Browser Mode for Web Components and PWA APIs](8-7-browser-mode-for-web-components-and-pwa-apis.md) - Why browser mode is needed (real IDB)
- [8-11: TanStack Query Test Setup](8-11-tanstack-query-test-setup.md) - Complementary cache isolation for query state
