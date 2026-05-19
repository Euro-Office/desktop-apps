---
id: 8-5
title: Use Accessible Query Priority
category: Testing
priority: MEDIUM
description: Prefer role and label queries over test IDs for resilient, accessible tests
---

## Problem

Tests that query by test ID (`getByTestId`) or raw DOM structure (`querySelector`) are brittle -- they break when markup changes and they do not verify that the UI is accessible. The Testing Library query priority (Role > LabelText > Text > TestId) is designed so that tests simultaneously verify accessibility. In Solid apps, this is especially important because control flow components (`<Show>`, `<For>`, `<Switch>`) change DOM structure; accessible queries are resilient to those structural changes.

## Incorrect

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import LoginForm from "./LoginForm";

// ❌ WRONG: Fragile test IDs that don't verify accessibility
test("submits form", async () => {
  const user = userEvent.setup();
  render(() => <LoginForm />);

  await user.type(screen.getByTestId("email-input"), "user@test.com");
  await user.type(screen.getByTestId("password-input"), "secret");
  await user.click(screen.getByTestId("submit-btn"));

  expect(screen.getByTestId("success-msg")).toBeInTheDocument();
});
```

```tsx
// ❌ WRONG: DOM structure queries break with Solid control flow
test("shows items", () => {
  const { container } = render(() => <ItemList items={mockItems} />);

  // querySelector breaks when <For> restructures the DOM
  const items = container.querySelectorAll(".item-row");
  expect(items.length).toBe(3);
});
```

## Correct

```tsx
import { render, screen } from "@solidjs/testing-library";
import userEvent from "@testing-library/user-event";
import LoginForm from "./LoginForm";

// ✅ CORRECT: Accessible queries that also verify a11y
test("submits form", async () => {
  const user = userEvent.setup();
  render(() => <LoginForm />);

  await user.type(
    screen.getByRole("textbox", { name: /email/i }),
    "user@test.com"
  );
  await user.type(screen.getByLabelText(/password/i), "secret");
  await user.click(screen.getByRole("button", { name: /sign in/i }));

  expect(await screen.findByRole("alert")).toHaveTextContent(/welcome/i);
});
```

```tsx
// ✅ CORRECT: Role-based queries work regardless of DOM structure
test("shows items", () => {
  render(() => <ItemList items={mockItems} />);

  const items = screen.getAllByRole("listitem");
  expect(items).toHaveLength(3);
});
```

### Query Priority Decision Table

| Content Type | Best Query | Example |
| ------------ | ---------- | ------- |
| Buttons, links, headings | `getByRole` | `getByRole('button', { name: /save/i })` |
| Form fields with labels | `getByLabelText` | `getByLabelText(/email/i)` |
| Form field placeholders | `getByPlaceholderText` | `getByPlaceholderText(/search/i)` |
| Static display text | `getByText` | `getByText(/no results/i)` |
| Form values | `getByDisplayValue` | `getByDisplayValue(/draft/i)` |
| Images | `getByAltText` | `getByAltText(/profile photo/i)` |
| Tooltips, titles | `getByTitle` | `getByTitle(/close/i)` |
| Last resort only | `getByTestId` | `getByTestId('complex-canvas')` |

### Common ARIA Roles for Solid Components

```tsx
// ✅ CORRECT: Query by semantic role
screen.getByRole("button", { name: /submit/i });      // <button>
screen.getByRole("textbox", { name: /email/i });       // <input type="text">
screen.getByRole("checkbox", { name: /agree/i });      // <input type="checkbox">
screen.getByRole("heading", { level: 2 });             // <h2>
screen.getByRole("navigation");                         // <nav>
screen.getByRole("list");                               // <ul> or <ol>
screen.getByRole("listitem");                           // <li>
screen.getByRole("dialog");                             // Portal modal with role="dialog"
screen.getByRole("alert");                              // <div role="alert">
screen.getByRole("tab", { name: /settings/i });        // Tab panels
```

### When TestId Is Acceptable

```tsx
// ✅ ACCEPTABLE: Complex visual elements with no semantic role
test("renders chart", () => {
  render(() => <Chart data={mockData} />);

  // Canvas/SVG charts have no meaningful ARIA role
  expect(screen.getByTestId("revenue-chart")).toBeInTheDocument();
});
```

## Why It Matters

1. **Accessibility Verification**: Role-based queries ensure your components are screen-reader navigable as a side effect of testing.

2. **Structural Resilience**: Solid's control flow components (`<Show>`, `<For>`, `<Switch>`) restructure the DOM differently than static HTML. Semantic queries target meaning, not DOM position.

3. **Maintenance**: Test IDs add noise to production markup and create a parallel naming system that drifts from the actual UI.

4. **User Perspective**: Role and label queries mirror how real users (including assistive technology users) interact with the UI.

## Related Rules

- [7-1: Semantic HTML](7-1-semantic-html.md) - Semantic elements provide implicit ARIA roles
- [7-2: ARIA Attributes](7-2-aria-attributes.md) - Custom ARIA roles enable better queries
- [8-4: Handle Async in Tests](8-4-handle-async-in-tests.md) - findBy variants for async content
