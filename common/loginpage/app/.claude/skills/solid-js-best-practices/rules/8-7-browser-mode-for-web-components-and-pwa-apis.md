---
id: 8-7
title: Use Browser Mode for Web Components and PWA APIs
category: Testing
priority: HIGH
description: Use Vitest browser mode (real Chromium) when testing custom elements, shadow DOM, or browser-native APIs like IndexedDB
---

## Problem

jsdom is a Node.js DOM simulation. It covers most standard HTML/CSS APIs well enough for component testing, but it does not faithfully implement the Web Components spec or browser-native APIs used by PWAs. Tests that pass in jsdom can silently fail or behave incorrectly when those features are involved.

Vitest browser mode runs tests inside a real Chromium instance via Playwright, giving you the same environment your users have.

## When to Use Browser Mode vs jsdom

| Use case | jsdom | Browser mode |
| -------- | :---: | :----------: |
| Standard Solid components | ✅ | ✅ |
| TanStack Query / async data | ✅ | ✅ |
| Custom elements / Web Components | ❌ | ✅ |
| Shadow DOM | ❌ | ✅ |
| IndexedDB (real, not shimmed) | ❌ | ✅ |
| `crypto.subtle` / SubtleCrypto | ❌ | ✅ |
| ServiceWorker registration | ❌ | ✅ |
| CSS `getComputedStyle` (real layout) | ❌ | ✅ |

## Incorrect

```tsx
// ❌ WRONG: Testing a custom element in jsdom — lifecycle callbacks never fire
import { render, screen } from "@solidjs/testing-library";
import "./my-icon"; // customElements.define('my-icon', ...)

test("renders icon", () => {
  render(() => <my-icon name="close" />);
  // connectedCallback never runs in jsdom — element stays as HTMLUnknownElement
  // Any assertions about shadow DOM or instance methods will fail or silently pass
  expect(screen.getByRole("img")).toBeInTheDocument();
});
```

```tsx
// ❌ WRONG in browser mode: simulated events, not real browser dispatch
import userEvent from "@testing-library/user-event";
```

## Correct

### Custom Elements / Web Components

Run tests that depend on custom elements in Vitest browser mode (see rule 8-1 for workspace config). The element's full lifecycle runs correctly in Chromium:

- `connectedCallback` fires when the element is inserted into the DOM
- `disconnectedCallback` fires on removal
- `attributeChangedCallback` fires on attribute mutation
- `attachShadow()` creates a real shadow root
- `customElements.whenDefined()` resolves correctly

```tsx
// my-icon.integration.test.tsx — runs in browser project (Chromium)
import { render, screen } from "@solidjs/testing-library";
import "./my-icon"; // registers custom element

test("renders icon via shadow DOM", async () => {
  render(() => <my-icon name="close" />);
  // connectedCallback has fired — shadow root is live
  const el = document.querySelector("my-icon")!;
  expect(el.shadowRoot).not.toBeNull();
});
```

### userEvent in Browser Mode

In browser mode, import `userEvent` from `vitest/browser`, not `@testing-library/user-event`:

```tsx
// ✅ CORRECT in browser mode: real browser event dispatch
import { userEvent } from "vitest/browser";

test("clicks button", async () => {
  render(() => <my-button />);
  await userEvent.click(document.querySelector("my-button")!);
});
```

`@testing-library/user-event` simulates events programmatically via JavaScript. Vitest's `userEvent` dispatches real browser events (pointer events, focus/blur sequences, etc.). They have similar APIs but are not interchangeable in browser mode.

### No Fakes Needed for Browser-Native APIs

In browser mode, native browser APIs are available without shimming:

```tsx
// ❌ jsdom: requires fake-indexeddb package
import "fake-indexeddb/auto";

// ✅ browser mode: real IDB, no shim needed
const db = await openDB("my-app", 1, { upgrade(db) { ... } });
```

APIs available natively in browser mode (no polyfills needed):

- `indexedDB` — real IndexedDB with full transaction support
- `crypto.subtle` — real SubtleCrypto
- `localStorage` / `sessionStorage` — real storage (must clear between tests — see rule 8-9)
- `fetch` — real network fetch (mock at the service layer for unit tests)

## jsdom Limitations for Custom Elements (Detail)

| Failure mode | Cause |
| ------------ | ----- |
| `connectedCallback` not called | jsdom doesn't trigger lifecycle callbacks on insert |
| Element stays `HTMLUnknownElement` | Custom element upgrade doesn't run on existing nodes |
| `shadowRoot` is null | `attachShadow()` creates a node but it doesn't participate in rendering |
| Slot assignment doesn't work | Light DOM / slot distribution not implemented |
| CSS custom properties not resolved | jsdom doesn't compute layout or cascade |

## Why It Matters

1. **Silent failures**: jsdom doesn't throw when custom element lifecycle methods don't run — tests pass while testing nothing.

2. **Real behavior**: Browser mode catches bugs that only appear in a real rendering environment (slot distribution, focus management, pointer events on shadow roots).

3. **No shim drift**: Shims like `fake-indexeddb` diverge from real browser behavior over time. Browser mode eliminates an entire class of shim-vs-reality bugs.

## Related Rules

- [8-1: Configure Vitest for Solid](8-1-configure-vitest-for-solid.md) - Workspace config for browser mode projects
- [8-9: Browser-Native API Test Isolation](8-9-browser-native-api-test-isolation.md) - Clearing IDB and localStorage between tests
