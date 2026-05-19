---
id: 8-8
title: Testing Headless UI Libraries with Non-Standard ARIA
category: Testing
priority: MEDIUM
description: Headless UI libraries use non-obvious ARIA structures and render into portals — query with screen and inspect the actual ARIA tree before writing assertions
---

## Problem

Headless UI libraries (Kobalte, Ark UI, Radix-for-Solid, etc.) provide unstyled, accessible components. Because they control their own ARIA implementation, their structures don't always match what you'd expect from native HTML equivalents. Tests written against assumed roles (`combobox`, `dialog`) will fail when the library uses a different structure.

Additionally, these libraries frequently render interactive content (option lists, tooltips, popovers) into a portal at the document root — outside the render container returned by `render()`.

## Incorrect

```tsx
import { render, screen } from "@solidjs/testing-library";
import { userEvent } from "vitest/browser";
import RecipesListPage from "./RecipesListPage";

// ❌ WRONG: Kobalte Select is not a combobox — getByRole throws
test("filters by category", async () => {
  render(() => <RecipesListPage />);
  const select = await screen.findByRole("combobox", { name: /categories/i });
  await userEvent.click(select);
  // Also fails: options render in a portal outside the render container
  const option = screen.getByRole("option", { name: "Beef" });
});
```

## Correct

Before writing role-based queries for a headless UI component, check the actual ARIA structure using `screen.debug()` or the browser's accessibility panel (DevTools → Accessibility tree).

### Kobalte Select Example

Kobalte's `<Select>` renders as a `role="group"` with `aria-label` on the root — not `role="combobox"`. The trigger is a `<button>` inside that group. Options render in a portal.

```tsx
// ✅ CORRECT: query Kobalte Select by its actual ARIA structure
test("filters by category", async () => {
  render(() => <RecipesListPage />);

  // 1. Find the group by its accessible label
  const categoryGroup = await screen.findByRole("group", { name: /categories/i });

  // 2. Access the trigger — a button inside the group (acceptable querySelector use)
  //    Kobalte renders exactly one trigger button; this is a well-known library internal.
  const trigger = categoryGroup.querySelector("button")!;
  await userEvent.click(trigger);

  // 3. Options render in a portal — use screen (full document), not container
  const option = await screen.findByRole("option", { name: /beef/i });
  await userEvent.click(option);

  // 4. Assert the filtered result
  await waitFor(() => expect(screen.getAllByRole("listitem")).toHaveLength(2));
});
```

### When `.querySelector()` Is Acceptable

Directly querying with `.querySelector()` is normally a smell (implementation coupling, not semantic). It's acceptable here because:

- The trigger button is a **well-known structural internal** of the library component
- The Kobalte Select group will always contain exactly one trigger button
- The library's own docs and tests rely on this structure

Document the reason with a comment so future readers understand the exception.

### Portal Content

Always use `screen` (which queries the full `document`) — never the `container` returned by `render()`. Portal content is appended to `document.body`, outside the container:

```tsx
// ❌ WRONG: container only covers the render root
const { container } = render(() => <MyPage />);
container.querySelector("[role='option']"); // null — options are in a portal

// ✅ CORRECT: screen covers the full document
const option = await screen.findByRole("option", { name: /beef/i });
```

### Debugging Unknown ARIA Structures

When `getByRole` fails unexpectedly, use `screen.debug()` to print the rendered ARIA tree:

```tsx
await screen.findByRole("group"); // wait for component to render
screen.debug();                   // prints full ARIA tree to console
```

Or inspect via browser DevTools: open the **Accessibility** panel, navigate to the component, and read the computed role and accessible name.

## Common ARIA Structure Differences

| Component type | Native HTML role | Kobalte role |
| -------------- | ---------------- | ------------ |
| Select | `combobox` | `group` (with `aria-label`) |
| Select trigger | (part of combobox) | `button` inside group |
| Select option | `option` | `option` ✅ (correct) |
| Dialog | `dialog` | `dialog` ✅ (correct) |
| Tooltip content | `tooltip` | renders in portal at `document.body` |

Other libraries will differ — always verify against the actual output.

## Why It Matters

1. **Role mismatches cause test failures**: Writing queries against assumed roles leads to immediate failures with unhelpful messages like `Unable to find an accessible element with the role "combobox"`.

2. **Portal content is invisible to container queries**: Using `container.querySelector` for portal-rendered content always returns null. `screen` is the correct API.

3. **Incorrect queries produce false confidence**: If you stub around the real structure rather than querying the actual ARIA, tests can pass while accessibility is broken.

## Related Rules

- [8-4: Handle Async in Tests](8-4-handle-async-in-tests.md) - `findBy` queries for content that renders after interaction
- [8-7: Browser Mode for Web Components and PWA APIs](8-7-browser-mode-for-web-components-and-pwa-apis.md) - Running tests in real Chromium
- [7-2: Use ARIA Attributes](7-2-aria-attributes.md) - ARIA patterns for custom controls
