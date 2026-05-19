---
id: 8-10
title: Router Integration Testing
category: Testing
priority: HIGH
description: Use MemoryRouter with the root prop to provide router context to providers that call useLocation, useSearchParams, or render <A> links
---

## Problem

`@solidjs/router` primitives (`useLocation`, `useSearchParams`, `useParams`, `<A>`) can only be called inside a component that is rendered within a router context. When you wrap a page component in test providers (QueryClientProvider, theme context, etc.) and render them as direct children of `<MemoryRouter>` without using `<Route>`, those providers are outside the router context and throw:

```
Error: <A> and router primitives can only be used inside a Route.
```

Additionally, pages that read route params via `useParams()` require the URL to be set on the memory history **before** the component renders — not after.

## Incorrect

```tsx
// ❌ WRONG: Providers as direct children of MemoryRouter — outside router context
function renderWithProviders(ui: Component) {
  return render(() => (
    <MemoryRouter>
      <QueryClientProvider client={makeTestQueryClient()}>
        <PageContextProvider>   {/* calls useLocation() — throws here */}
          {ui({})}
        </PageContextProvider>
      </QueryClientProvider>
    </MemoryRouter>
  ));
}
```

```tsx
// ❌ WRONG: render(<Component />) without arrow function — broken reactivity
render(<RecipesListPage />);
```

## Correct

### Use the `root` Prop for Layout Providers

`MemoryRouter`'s `root` prop accepts a `Component<RouteSectionProps>` that renders inside the router context. Use a factory function so each test gets fresh context instances — never share context across tests.

```tsx
// src/test-helpers/renderHelpers.tsx
import {
  MemoryRouter,
  Route,
  createMemoryHistory,
  RouteSectionProps,
} from "@solidjs/router";
import { QueryClientProvider } from "@tanstack/solid-query";
import { render } from "@solidjs/testing-library";
import { Component } from "solid-js";
import { makeTestQueryClient } from "./queryHelpers";
import { PageContextProvider } from "../context/PageContext";
import { ActionMenuContext, createActionMenu } from "../context/ActionMenu";

/**
 * Returns a fresh layout component per call.
 * Call makeLayout() inside each render helper — never share across tests.
 */
function makeLayout(): Component<RouteSectionProps> {
  const actionMenu = createActionMenu();
  return (props) => (
    <PageContextProvider>            {/* calls useLocation() — safe inside root */}
      <ActionMenuContext.Provider value={actionMenu}>
        {props.children}
      </ActionMenuContext.Provider>
    </PageContextProvider>
  );
}

/** Render a page component at the root path (no route params). */
export function renderWithProviders(ui: Component) {
  const history = createMemoryHistory();
  history.set({ value: "/", replace: true });

  return render(() => (
    <QueryClientProvider client={makeTestQueryClient()}>
      <MemoryRouter history={history} root={makeLayout()}>
        <Route path="*" component={ui} />
      </MemoryRouter>
    </QueryClientProvider>
  ));
}

/** Render a page component at a specific URL (supports useParams). */
export function renderRoute(
  Page: Component,
  pathPattern: string,
  url: string,
) {
  const history = createMemoryHistory();
  history.set({ value: url, replace: true }); // set URL before render

  return render(() => (
    <QueryClientProvider client={makeTestQueryClient()}>
      <MemoryRouter history={history} root={makeLayout()}>
        <Route path={pathPattern} component={Page} />
      </MemoryRouter>
    </QueryClientProvider>
  ));
}
```

### Usage

```tsx
// RecipesListPage.integration.test.tsx
import { renderWithProviders, renderRoute } from "../test-helpers/renderHelpers";
import RecipesListPage from "./RecipesListPage";
import RecipeViewPage from "./RecipeViewPage";

it("renders the recipes list", async () => {
  renderWithProviders(RecipesListPage);
  await screen.findByRole("heading", { name: /recipes/i });
});

it("renders a recipe by id", async () => {
  await seedRecipe({ id: "abc123", title: "Pasta" });
  renderRoute(RecipeViewPage, "/recipes/:id", "/recipes/abc123");
  await screen.findByRole("heading", { name: "Pasta" });
});
```

## API Reference

All imports are from `@solidjs/router`:

```typescript
import {
  MemoryRouter,        // Router component backed by in-memory history
  Route,               // Route definition component
  createMemoryHistory, // Creates a controllable in-memory history object
  RouteSectionProps,   // Props type for components used as route sections / root
} from "@solidjs/router";
```

### `createMemoryHistory()` Methods

```typescript
const history = createMemoryHistory();

history.get()                           // → current path string
history.set({ value, replace?, scroll? }) // navigate to a path
history.back()                          // go(-1)
history.forward()                       // go(1)
history.go(n)                           // navigate n steps
history.listen(callback)                // register navigation listener
```

### `MemoryRouter` Props

| Prop | Type | Purpose |
| ---- | ---- | ------- |
| `history` | `MemoryHistory` | Pre-configured history (required for test URL control) |
| `root` | `Component<RouteSectionProps>` | Layout component rendered inside router context |
| `base` | `string` | Base URL prefix for route matching |
| `preload` | `boolean` | Enable/disable route preloading (default: `true`) |

## Why It Matters

1. **Context errors**: Without the `root` prop, any provider that calls `useLocation()` or `useParams()` throws immediately — the test can't even render.

2. **Stale URL on render**: Setting `history.set()` after render means the component reads the wrong initial path, causing `useParams()` to return empty params.

3. **Shared context leakage**: Reusing a `makeLayout()` instance across tests shares context state. The factory pattern ensures each test gets a clean slate.

## Related Rules

- [8-1: Configure Vitest for Solid](8-1-configure-vitest-for-solid.md) - Vitest workspace config prerequisite
- [8-11: TanStack Query Test Setup](8-11-tanstack-query-test-setup.md) - makeTestQueryClient used inside renderHelpers
