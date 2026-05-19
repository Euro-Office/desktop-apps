---
id: 8-6
title: Separate Logic from UI Tests
category: Testing
priority: MEDIUM
description: Test reactive primitives and hooks independently from component rendering
---

## Problem

Testing all behavior through rendered components is slow, fragile, and conflates logic bugs with rendering bugs. Solid's architecture naturally separates reactive logic (custom primitives/hooks) from presentation (components). Custom primitives can be tested with `renderHook()` without any DOM rendering. Directives can be tested with `renderDirective()` or by attaching to a bare DOM element. Reserving component-level `render()` tests for actual UI integration behavior keeps the test suite fast and pinpoints failures precisely.

## Incorrect

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import PaginatedList from "./PaginatedList";

// ❌ WRONG: Testing business logic through a full component render
test("pagination logic", async () => {
  const user = userEvent.setup();
  render(() => <PaginatedList items={mockItems} pageSize={10} />);

  // Testing the pagination hook's math through click sequences
  await user.click(screen.getByRole("button", { name: /next/i }));
  await user.click(screen.getByRole("button", { name: /next/i }));
  expect(screen.getByText("Page 3 of 5")).toBeInTheDocument();

  await user.click(screen.getByRole("button", { name: /next/i }));
  await user.click(screen.getByRole("button", { name: /next/i }));
  await user.click(screen.getByRole("button", { name: /next/i }));
  expect(screen.getByText("Page 5 of 5")).toBeInTheDocument();
});
```

```tsx
// ❌ WRONG: Testing form validation logic through the UI
test("validates email format", async () => {
  const user = userEvent.setup();
  render(() => <RegistrationForm />);

  await user.type(screen.getByLabelText(/email/i), "not-an-email");
  await user.click(screen.getByRole("button", { name: /submit/i }));

  // Testing the validation function through DOM interaction
  expect(screen.getByText(/invalid email/i)).toBeInTheDocument();
});
```

## Correct

### Test Primitives with renderHook

```tsx
import { renderHook } from "@solidjs/testing-library";
import { usePagination } from "./usePagination";

// ✅ CORRECT: Test the primitive directly -- fast, precise
test("pagination clamps to last page", () => {
  const { result } = renderHook(() =>
    usePagination({ total: 50, pageSize: 10 })
  );

  expect(result.page()).toBe(1);
  expect(result.totalPages()).toBe(5);

  result.next();
  result.next();
  expect(result.page()).toBe(3);

  result.next();
  result.next();
  result.next(); // Beyond bounds
  expect(result.page()).toBe(5); // Clamped
});

// ✅ CORRECT: Component test only for UI integration
test("PaginatedList renders current page items", () => {
  render(() => <PaginatedList items={mockItems} pageSize={10} />);

  expect(screen.getAllByRole("listitem")).toHaveLength(10);
});
```

### Test Validation Logic Directly

```tsx
import { validateEmail, validatePassword } from "./validators";

// ✅ CORRECT: Pure function tests -- no DOM needed
test("validates email format", () => {
  expect(validateEmail("user@test.com")).toBe(true);
  expect(validateEmail("not-an-email")).toBe(false);
  expect(validateEmail("")).toBe(false);
});

test("validates password strength", () => {
  expect(validatePassword("short")).toBe(false);
  expect(validatePassword("LongEnough123!")).toBe(true);
});
```

### Test Directives in Isolation

```tsx
import { renderDirective } from "@solidjs/testing-library";
import { clickOutside } from "./clickOutside";

// ✅ CORRECT: Test directive without a full component
test("clickOutside calls handler on outside click", () => {
  const handler = vi.fn();
  const { container } = renderDirective(clickOutside, {
    initialValue: handler,
  });

  document.body.click(); // Outside the directive element
  expect(handler).toHaveBeenCalledOnce();

  handler.mockClear();
  container.click(); // Inside the directive element
  expect(handler).not.toHaveBeenCalled();
});
```

```tsx
// ✅ CORRECT: Alternative -- test directive with bare DOM element
import { createRoot } from "solid-js";
import { tooltip } from "./tooltip";

test("tooltip directive sets title attribute", () => {
  createRoot((dispose) => {
    const el = document.createElement("div");
    tooltip(el, () => "Help text");

    expect(el.getAttribute("title")).toBe("Help text");
    dispose();
  });
});
```

### Mock Stores with Getters

```tsx
import { createSignal } from "solid-js";

// ✅ CORRECT: Mock stores with getter functions to preserve reactivity
vi.mock("./authStore", () => {
  const [user, setUser] = createSignal<string | null>(null);
  return {
    authStore: {
      get user() { return user(); },         // Getter, not static value
      get isLoggedIn() { return user() !== null; },
      login: (name: string) => setUser(name),
      logout: () => setUser(null),
    },
  };
});

test("shows login button when logged out", () => {
  render(() => <NavBar />);
  expect(screen.getByRole("button", { name: /log in/i })).toBeInTheDocument();
});

test("shows username when logged in", async () => {
  const { authStore } = await import("./authStore");
  authStore.login("Alice");

  render(() => <NavBar />);
  expect(screen.getByText("Alice")).toBeInTheDocument();
});
```

## Test Pyramid for Solid Apps

| Layer | What to Test | Tool | Speed |
| ----- | ------------ | ---- | ----- |
| Unit | Pure functions (validators, formatters) | Direct function calls | Fastest |
| Primitive | Reactive hooks (signals, memos, effects) | `renderHook` / `createRoot` | Fast |
| Directive | Reusable DOM behaviors | `renderDirective` / bare element | Fast |
| Component | UI integration (rendering, interaction) | `render` + queries | Slower |
| Integration | Full page flows with routing/context | `render` + providers | Slowest |

## Why It Matters

1. **Speed**: Primitive/hook tests run in microseconds with no DOM overhead; component tests are 10-100x slower.

2. **Precision**: When a primitive test fails, the bug is in logic. When a component test fails, the cause could be logic, rendering, or test setup.

3. **Solid's Design**: Solid's reactive primitive pattern is built for extraction and reuse -- testing them independently validates that design.

4. **Store Mocking**: Static mock values silently break reactivity. Getter-based mocks preserve signal tracking so components react to store changes in tests.

## Related Rules

- [5-4: Use Directives](5-4-use-directives.md) - Directives are testable in isolation
- [4-5: Use Context for Global State](4-5-use-context-for-global.md) - Context mocking patterns
- [8-3: Test Primitives in a Root](8-3-test-primitives-in-root.md) - Reactive scope requirement for primitives
