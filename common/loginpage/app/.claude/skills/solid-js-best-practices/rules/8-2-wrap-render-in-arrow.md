---
id: 8-2
title: Wrap Render Calls in Arrow Functions
category: Testing
priority: CRITICAL
description: Always use render(() => <Component />) not render(<Component />)
---

## Problem

Solid components are functions that execute once to set up their reactive graph. When you pass JSX directly to `render()` as `render(<MyComponent />)`, the component executes immediately in the caller's scope, outside the testing library's reactive root. The rendered output becomes static: signals never update the DOM, effects never register, and assertions on reactive behavior silently pass or fail for the wrong reasons. The arrow wrapper `render(() => <MyComponent />)` ensures the component runs inside the testing library's managed reactive scope.

## Incorrect

```tsx
import { render, screen } from "@solidjs/testing-library";
import Counter from "./Counter";

// ❌ WRONG: Component executes outside reactive scope
test("increments count", async () => {
  render(<Counter />);

  const button = screen.getByRole("button");
  button.click();

  // This may pass because the DOM never updates --
  // the initial "0" is still there, but reactivity is broken
  expect(screen.getByText("0")).toBeInTheDocument();
});
```

```tsx
import { render, screen } from "@solidjs/testing-library";
import Greeting from "./Greeting";

// ❌ WRONG: Props won't update reactively
test("shows greeting", () => {
  const name = "Alice";
  render(<Greeting name={name} />);

  // Static render -- changing the variable won't update the DOM
  expect(screen.getByText("Hello, Alice")).toBeInTheDocument();
});
```

## Correct

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import Counter from "./Counter";

// ✅ CORRECT: Arrow wrapper creates proper reactive scope
test("increments count", async () => {
  const user = userEvent.setup();

  render(() => <Counter />);

  const button = screen.getByRole("button", { name: /increment/i });
  await user.click(button);

  expect(screen.getByText("1")).toBeInTheDocument();
});
```

### Reactive Props in Tests

```tsx
import { createSignal } from "solid-js";
import { render, screen } from "@solidjs/testing-library";
import Display from "./Display";

// ✅ CORRECT: Props update reactively through the arrow wrapper
test("responds to prop changes", () => {
  const [count, setCount] = createSignal(0);

  render(() => <Display value={count()} />);

  expect(screen.getByText("0")).toBeInTheDocument();

  setCount(5);
  expect(screen.getByText("5")).toBeInTheDocument();
});
```

### With Context Providers

```tsx
import { render, screen } from "@solidjs/testing-library";
import { ThemeProvider } from "./ThemeContext";
import ThemedButton from "./ThemedButton";

// ✅ CORRECT: Arrow wrapper works with context providers
test("renders with theme", () => {
  render(() => (
    <ThemeProvider theme="dark">
      <ThemedButton>Click me</ThemedButton>
    </ThemeProvider>
  ));

  expect(screen.getByRole("button")).toHaveClass("dark");
});
```

### Using the wrapper Option

```tsx
import { render, screen } from "@solidjs/testing-library";
import { ThemeProvider } from "./ThemeContext";
import ThemedButton from "./ThemedButton";

// ✅ CORRECT: wrapper option for reusable context
test("renders with theme", () => {
  render(() => <ThemedButton>Click me</ThemedButton>, {
    wrapper: (props) => (
      <ThemeProvider theme="dark">{props.children}</ThemeProvider>
    ),
  });

  expect(screen.getByRole("button")).toHaveClass("dark");
});
```

## Why It Matters

1. **Reactive Ownership**: Without the arrow wrapper, the component runs with no reactive owner -- identical to calling the function directly in a plain script.

2. **Signal Updates**: Signal changes never propagate to the DOM because the component was never registered in a reactive scope.

3. **False Positives**: Tests that check initial render pass, but tests that verify updates after interactions silently fail or give misleading results.

4. **React Migration Pitfall**: This is the most common mistake when porting React Testing Library patterns, where `render(<Component />)` is standard.

## Related Rules

- [1-1: Use Signals Correctly](1-1-use-signals-correctly.md) - Signals must be called in reactive contexts
- [8-1: Configure Vitest for Solid](8-1-configure-vitest-for-solid.md) - Correct config is a prerequisite
- [8-3: Test Primitives in a Root](8-3-test-primitives-in-root.md) - Same reactive ownership requirement for primitives
