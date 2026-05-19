---
id: 8-11
title: TanStack Query Test Setup
category: Testing
priority: HIGH
description: Create a fresh QueryClient per test with retry and caching disabled to prevent flaky tests and cross-test state leakage
---

## Problem

The default `QueryClient` configuration is optimized for production: it retries failed requests three times, keeps data in cache while components are mounted, and holds garbage-collected data for five minutes. In tests, these defaults cause three distinct problems:

1. **Slow error tests**: A query that should fail immediately retries silently three times before surfacing the error, adding seconds of wait time.
2. **Stale data masking writes**: With `staleTime > 0`, a test that writes data and then re-reads it may get the cached pre-write result instead of the fresh post-write result.
3. **Cross-test cache leakage**: If a `QueryClient` is shared across tests (e.g. created at module scope), data fetched in one test remains in cache for the next.

## Incorrect

```typescript
// ❌ WRONG: Module-scope client — shared state leaks between tests
const queryClient = new QueryClient();

function renderPage(ui: Component) {
  return render(() => (
    <QueryClientProvider client={queryClient}>
      {ui({})}
    </QueryClientProvider>
  ));
}
```

```typescript
// ❌ WRONG: Default QueryClient — retries mask errors, cache leaks across tests
function renderPage(ui: Component) {
  return render(() => (
    <QueryClientProvider client={new QueryClient()}>
      {ui({})}
    </QueryClientProvider>
  ));
}
```

## Correct

```typescript
// src/test-helpers/queryHelpers.ts
import { QueryClient } from "@tanstack/solid-query";

/**
 * Creates a QueryClient configured for deterministic testing:
 * - retry: false    — errors surface immediately (no silent 3-retry wait)
 * - staleTime: 0   — always re-fetches (no stale-while-revalidate masking)
 * - gcTime: 0      — cache cleared after last subscriber unsubscribes
 *
 * Call makeTestQueryClient() inside each render helper — never share across tests.
 */
export function makeTestQueryClient(): QueryClient {
  return new QueryClient({
    defaultOptions: {
      queries: {
        retry: false,
        staleTime: 0,
        gcTime: 0,
      },
      mutations: {
        retry: false,
      },
    },
  });
}
```

### Use Inside Render Helpers

Create a new `QueryClient` per render call — not at module scope and not shared between tests:

```tsx
// src/test-helpers/renderHelpers.tsx
import { QueryClientProvider } from "@tanstack/solid-query";
import { makeTestQueryClient } from "./queryHelpers";

export function renderWithProviders(ui: Component) {
  return render(() => (
    <QueryClientProvider client={makeTestQueryClient()}>
      {/* ... */}
    </QueryClientProvider>
  ));
}
```

### Assertions on Query Errors

With `retry: false`, errors surface on the first attempt:

```tsx
it("shows error state when fetch fails", async () => {
  vi.mocked(api.fetchRecipes).mockRejectedValue(new Error("Network error"));

  renderWithProviders(RecipesListPage);

  // No need to wait for 3 retries — error appears immediately
  expect(await screen.findByRole("alert")).toHaveTextContent(/network error/i);
});
```

## Configuration Options Explained

| Option | Default | Test value | Why |
| ------ | ------- | ---------- | --- |
| `queries.retry` | `3` | `false` | Errors surface on first failure; no 3× slow retry loop |
| `queries.staleTime` | `0` | `0` | (Already 0 by default) Re-fetch on mount; no stale cache masking writes |
| `queries.gcTime` | `5 min` | `0` | Cache cleared immediately after unsubscribe; no cross-test leakage |
| `mutations.retry` | `0` | `false` | Consistent with query setting; explicit is better |

## Why It Matters

1. **Deterministic failures**: `retry: false` makes query error tests fast and predictable. Without it, a test that expects an error state waits through three retries before the error renders.

2. **Write-then-read correctness**: `staleTime: 0` ensures a re-render after a mutation fetches fresh data rather than returning a stale cached value.

3. **Test isolation**: A new `QueryClient` per test means no data from a previous test can influence the current one. Shared clients are a common source of order-dependent test failures.

## Related Rules

- [8-4: Handle Async in Tests](8-4-handle-async-in-tests.md) - `findBy` queries for async query results
- [8-9: Browser-Native API Test Isolation](8-9-browser-native-api-test-isolation.md) - Complementary IDB/localStorage isolation
- [8-10: Router Integration Testing](8-10-router-integration-testing.md) - `makeTestQueryClient` used inside render helpers
