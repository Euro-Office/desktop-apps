---
id: 8-4
title: Handle Async Reactivity in Tests
category: Testing
priority: HIGH
description: Use findBy queries and proper timer configuration for async Solid behavior
---

## Problem

Solid batches signal updates synchronously, but many real-world patterns involve async behavior: `createResource`, lazy components, router transitions, and timer-driven effects. Tests that use `getBy` queries immediately after triggering async operations fail because the DOM has not yet updated. Additionally, using fake timers with `userEvent` requires explicit `advanceTimers` configuration, or user interactions silently hang.

## Incorrect

```tsx
import { render, screen } from "@solidjs/testing-library";
import UserProfile from "./UserProfile";

// ❌ WRONG: getBy doesn't wait for async resource to load
test("shows user name after fetch", async () => {
  render(() => <UserProfile id={1} />);

  // Fails: resource hasn't resolved yet
  expect(screen.getByText("Alice")).toBeInTheDocument();
});
```

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import SearchBox from "./SearchBox";

// ❌ WRONG: Fake timers break userEvent without advanceTimers
test("debounced search", async () => {
  vi.useFakeTimers();
  const user = userEvent.setup(); // Missing advanceTimers config

  render(() => <SearchBox />);
  await user.type(screen.getByRole("textbox"), "solid");
  // Hangs forever: userEvent uses internal delays that need timer advancement
});
```

```tsx
import { render, screen } from "@solidjs/testing-library";
import Modal from "./Modal";

// ❌ WRONG: Portal content queried from render container
test("opens modal", async () => {
  const user = userEvent.setup();
  const { container } = render(() => <Modal />);

  await user.click(screen.getByRole("button", { name: /open/i }));

  // Fails: portal renders outside the container
  expect(container.querySelector("[role='dialog']")).toBeInTheDocument();
});
```

## Correct

```tsx
import { render, screen } from "@solidjs/testing-library";
import UserProfile from "./UserProfile";

// ✅ CORRECT: findBy waits for element to appear
test("shows user name after fetch", async () => {
  render(() => <UserProfile id={1} />);

  expect(await screen.findByText("Alice")).toBeInTheDocument();
});
```

### Fake Timers with userEvent

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import SearchBox from "./SearchBox";

// ✅ CORRECT: Configure advanceTimers for fake timer + userEvent
test("debounced search", async () => {
  vi.useFakeTimers();
  const user = userEvent.setup({
    advanceTimers: vi.advanceTimersByTime, // Required for fake timers
  });

  render(() => <SearchBox />);
  await user.type(screen.getByRole("textbox"), "solid");
  vi.advanceTimersByTime(300); // Advance past debounce delay

  expect(await screen.findByText('Results for "solid"')).toBeInTheDocument();
  vi.useRealTimers();
});
```

### Portal Content

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import Modal from "./Modal";

// ✅ CORRECT: Use screen for portal content (modals, tooltips)
test("opens modal", async () => {
  const user = userEvent.setup();
  render(() => <Modal />);

  await user.click(screen.getByRole("button", { name: /open/i }));

  // screen queries the entire document, not just the render container
  expect(screen.getByRole("dialog")).toBeInTheDocument();
});
```

### Router Components

```tsx
import { render, screen } from "@solidjs/testing-library";
import UserPage from "./UserPage";

// ✅ CORRECT: Router loads lazily -- first query must be async
test("renders user page", async () => {
  render(() => <UserPage />, {
    location: "/users/1",
  });

  // findBy required because router resolves asynchronously
  expect(await screen.findByRole("heading", { name: /user/i }))
    .toBeInTheDocument();
});
```

### Mocking createResource Fetchers

```tsx
import { render, screen } from "@solidjs/testing-library";
import UserList from "./UserList";
import * as api from "./api";

// ✅ CORRECT: Mock the fetcher, not createResource
vi.mock("./api", () => ({
  fetchUsers: vi.fn(),
}));

test("renders user list", async () => {
  vi.mocked(api.fetchUsers).mockResolvedValue([
    { id: 1, name: "Alice" },
    { id: 2, name: "Bob" },
  ]);

  render(() => <UserList />);

  expect(await screen.findByText("Alice")).toBeInTheDocument();
  expect(screen.getByText("Bob")).toBeInTheDocument();
});
```

### Testing Empty and Absent States

Testing that something is **not** present is subtle. The naive pattern fails silently:

```tsx
// ❌ WRONG: passes immediately before data even loads
// Initial render (before fetch) is also empty — this assertion is vacuously true
await waitFor(() =>
  expect(screen.queryAllByRole("listitem")).toHaveLength(0),
);
```

The problem: the component starts empty (no data yet), so the assertion passes before the query settles. You're not testing "loaded and found nothing" — you're testing "hasn't loaded yet."

**Fix**: wait for a stable "settled anchor" — an element that only renders after queries resolve — then assert absence synchronously:

```tsx
// ✅ CORRECT: wait for a settled anchor, then assert absence
// The categories filter renders once queries settle, even when the list is empty.
// With <Suspense>, anything from the component tree indicates queries have settled.
await screen.findByRole("group", { name: /categories/i }); // settled anchor
expect(screen.queryAllByRole("listitem")).toHaveLength(0);  // synchronous
```

Choosing a settled anchor:

1. A heading or control that **always renders** after the query settles (regardless of data)
2. With `<Suspense>`: once any component content appears, all queries within the boundary have settled
3. Avoid `setTimeout` hacks — they create implicit timing assumptions that break under slower environments

## Query Selection Guide

| Scenario | Query | Why |
| -------- | ----- | --- |
| Element present on render | `getBy` | Synchronous, throws if absent |
| Element appears after async load | `findBy` | Polls with timeout |
| Element should NOT exist | `queryBy` | Returns null instead of throwing |
| Multiple elements expected | `getAllBy` / `findAllBy` | Returns arrays |

## Why It Matters

1. **False Failures**: `getBy` throws immediately when the element is absent. `findBy` polls with a configurable timeout, matching async rendering patterns.

2. **Frozen Tests**: Fake timers intercept `setTimeout`/`setInterval` globally. `userEvent` uses internal delays that freeze without `advanceTimers`, causing indefinite hangs.

3. **Invisible Content**: Portal-rendered content (modals, tooltips, dropdowns) exists outside the render container. Only `screen` queries the full document.

4. **Flaky Tests**: Async tests without proper waiting produce race conditions that pass locally but fail in CI.

## Related Rules

- [6-3: Use Suspense](6-3-use-suspense.md) - Suspense boundaries affect async test timing
- [8-2: Wrap Render in Arrow Functions](8-2-wrap-render-in-arrow.md) - Prerequisite for reactive rendering
- [8-5: Use Accessible Queries](8-5-use-accessible-queries.md) - Query selection best practices
- [8-8: Testing Headless UI Libraries](8-8-testing-headless-ui-libraries.md) - Portal content and non-standard ARIA
- [8-11: TanStack Query Test Setup](8-11-tanstack-query-test-setup.md) - Configuring QueryClient for deterministic async behavior
