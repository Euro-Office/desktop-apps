---
id: 3-5
title: Provide Fallbacks
category: Control Flow
priority: LOW
description: Always provide fallback props for loading states
---

## Problem

When using conditional rendering or async data, failing to provide fallback content leads to empty UI states, confusing loading experiences, or jarring layout shifts. Always provide meaningful fallback content.

## Incorrect

```tsx
// ❌ WRONG: No fallback - blank screen while loading
function UserProfile(props) {
  return (
    <Show when={props.user}>
      <ProfileCard user={props.user} />
    </Show>
  );
}
```

```tsx
// ❌ WRONG: No fallback for lists
function ProductList(props) {
  return (
    <ul>
      <For each={props.products}>
        {(product) => <ProductItem product={product} />}
      </For>
    </ul>
  );
}
```

```tsx
// ❌ WRONG: Suspense without fallback
function Dashboard() {
  return (
    <Suspense>
      <AsyncContent />
    </Suspense>
  );
}
```

## Correct

```tsx
import { Show } from "solid-js";

// ✅ CORRECT: Loading state fallback
function UserProfile(props) {
  return (
    <Show
      when={props.user}
      fallback={<ProfileSkeleton />}
    >
      <ProfileCard user={props.user} />
    </Show>
  );
}
```

```tsx
import { For } from "solid-js";

// ✅ CORRECT: Empty state fallback
function ProductList(props) {
  return (
    <ul>
      <For
        each={props.products}
        fallback={<li class="empty">No products found</li>}
      >
        {(product) => <ProductItem product={product} />}
      </For>
    </ul>
  );
}
```

```tsx
import { Suspense } from "solid-js";

// ✅ CORRECT: Suspense with loading indicator
function Dashboard() {
  return (
    <Suspense fallback={<DashboardSkeleton />}>
      <AsyncContent />
    </Suspense>
  );
}
```

### Error Boundaries

```tsx
import { ErrorBoundary } from "solid-js";

// ✅ CORRECT: Error fallback
function App() {
  return (
    <ErrorBoundary
      fallback={(err, reset) => (
        <div class="error">
          <h2>Something went wrong</h2>
          <p>{err.message}</p>
          <button onClick={reset}>Try Again</button>
        </div>
      )}
    >
      <MainContent />
    </ErrorBoundary>
  );
}
```

### Skeleton Components

```tsx
// ✅ CORRECT: Matching skeleton structure prevents layout shift
function CardSkeleton() {
  return (
    <div class="card skeleton">
      <div class="skeleton-avatar" />
      <div class="skeleton-text skeleton-title" />
      <div class="skeleton-text skeleton-body" />
    </div>
  );
}

function UserCard(props) {
  return (
    <Show when={props.user} fallback={<CardSkeleton />}>
      {(user) => (
        <div class="card">
          <img src={user().avatar} class="avatar" />
          <h3>{user().name}</h3>
          <p>{user().bio}</p>
        </div>
      )}
    </Show>
  );
}
```

### Context-Appropriate Fallbacks

```tsx
// ✅ CORRECT: Different fallbacks for different contexts
function DataTable(props) {
  return (
    <table>
      <thead>
        <tr>
          <For each={props.columns}>
            {(col) => <th>{col.header}</th>}
          </For>
        </tr>
      </thead>
      <tbody>
        <Show
          when={!props.loading}
          fallback={
            <tr>
              <td colspan={props.columns.length}>
                <Spinner /> Loading data...
              </td>
            </tr>
          }
        >
          <For
            each={props.rows}
            fallback={
              <tr>
                <td colspan={props.columns.length} class="empty">
                  No data available
                </td>
              </tr>
            }
          >
            {(row) => <DataRow row={row} columns={props.columns} />}
          </For>
        </Show>
      </tbody>
    </table>
  );
}
```

## Why It Matters

1. **User Experience**: Users see meaningful feedback instead of blank screens.

2. **Layout Stability**: Skeleton fallbacks prevent content jumping.

3. **Error Recovery**: Error boundaries let users recover without page refresh.

4. **Accessibility**: Screen readers announce loading/empty states.

## Fallback Checklist

| Component | Needs Fallback For |
| --------- | ----------------- |
| `<Show>` | Loading, unauthorized, null data |
| `<For>` | Empty arrays |
| `<Switch>` | Unhandled cases |
| `<Suspense>` | Async loading |
| `<ErrorBoundary>` | Runtime errors |

## Best Practices

1. **Match Structure**: Skeletons should match the loaded content's layout.

2. **Be Informative**: Tell users what's happening ("Loading..." vs blank).

3. **Provide Actions**: Error states should offer recovery options.

4. **Use Transitions**: Smooth transitions between states when possible.

```tsx
// ✅ CORRECT: Informative empty state with action
<For
  each={searchResults()}
  fallback={
    <div class="empty-state">
      <SearchIcon />
      <h3>No results found</h3>
      <p>Try adjusting your search terms</p>
      <button onClick={clearFilters}>Clear Filters</button>
    </div>
  }
>
  {(result) => <SearchResult result={result} />}
</For>
```

## Related Rules

- [3-1: Use Show for Conditionals](3-1-use-show-for-conditionals.md) - Show's fallback prop
- [3-2: Use For for Lists](3-2-use-for-for-lists.md) - For's fallback prop
- [6-3: Use Suspense](6-3-use-suspense.md) - Async loading boundaries
