---
id: 6-3
title: Use Suspense
category: Performance
priority: MEDIUM
description: Use <Suspense> for async loading boundaries
---

## Problem

Async operations (lazy components, data fetching with `createResource`) need loading states. Without `Suspense`, you must manually track loading states for each async operation. `Suspense` provides a declarative way to show fallback content while async children resolve.

## Incorrect

```tsx
// ❌ WRONG: Manual loading state management
function UserProfile(props) {
  const [user, setUser] = createSignal(null);
  const [loading, setLoading] = createSignal(true);

  onMount(async () => {
    setLoading(true);
    const data = await fetchUser(props.id);
    setUser(data);
    setLoading(false);
  });

  return (
    <Show when={!loading()} fallback={<Spinner />}>
      <ProfileCard user={user()} />
    </Show>
  );
}
```

```tsx
// ❌ WRONG: Lazy component without Suspense crashes
import { lazy } from "solid-js";

const HeavyComponent = lazy(() => import("./HeavyComponent"));

function App() {
  return (
    <div>
      <HeavyComponent />  {/* No Suspense boundary! */}
    </div>
  );
}
```

## Correct

```tsx
import { Suspense, createResource } from "solid-js";

// ✅ CORRECT: Suspense with createResource
function UserProfile(props) {
  const [user] = createResource(() => props.id, fetchUser);

  return (
    <Suspense fallback={<ProfileSkeleton />}>
      <ProfileCard user={user()} />
    </Suspense>
  );
}
```

```tsx
import { lazy, Suspense } from "solid-js";

// ✅ CORRECT: Suspense with lazy components
const HeavyComponent = lazy(() => import("./HeavyComponent"));

function App() {
  return (
    <Suspense fallback={<Loading />}>
      <HeavyComponent />
    </Suspense>
  );
}
```

### Multiple Async Children

```tsx
import { Suspense, createResource } from "solid-js";

// ✅ CORRECT: One Suspense for multiple async operations
function Dashboard() {
  const [users] = createResource(fetchUsers);
  const [stats] = createResource(fetchStats);
  const [activity] = createResource(fetchActivity);

  return (
    <Suspense fallback={<DashboardSkeleton />}>
      <div class="dashboard">
        <UserList users={users()} />
        <StatsPanel stats={stats()} />
        <ActivityFeed activity={activity()} />
      </div>
    </Suspense>
  );
}
```

### Nested Suspense Boundaries

```tsx
import { Suspense, createResource, lazy } from "solid-js";

const Comments = lazy(() => import("./Comments"));

function Article(props) {
  const [article] = createResource(() => props.id, fetchArticle);

  return (
    <Suspense fallback={<ArticleSkeleton />}>
      <article>
        <h1>{article()?.title}</h1>
        <div>{article()?.content}</div>

        {/* Nested Suspense: comments load independently */}
        <Suspense fallback={<CommentsSkeleton />}>
          <Comments articleId={props.id} />
        </Suspense>
      </article>
    </Suspense>
  );
}
```

### SuspenseList for Coordinated Loading

```tsx
import { Suspense, SuspenseList, createResource } from "solid-js";

// ✅ CORRECT: Coordinate multiple Suspense boundaries
function Feed() {
  return (
    <SuspenseList revealOrder="forwards" tail="collapsed">
      <Suspense fallback={<PostSkeleton />}>
        <Post id={1} />
      </Suspense>
      <Suspense fallback={<PostSkeleton />}>
        <Post id={2} />
      </Suspense>
      <Suspense fallback={<PostSkeleton />}>
        <Post id={3} />
      </Suspense>
    </SuspenseList>
  );
}
```

### Error Handling with ErrorBoundary

```tsx
import { Suspense, ErrorBoundary, createResource } from "solid-js";

// ✅ CORRECT: Combine Suspense with ErrorBoundary
function DataSection() {
  const [data] = createResource(fetchData);

  return (
    <ErrorBoundary
      fallback={(err, reset) => (
        <div class="error">
          <p>Error: {err.message}</p>
          <button onClick={reset}>Retry</button>
        </div>
      )}
    >
      <Suspense fallback={<Loading />}>
        <DataView data={data()} />
      </Suspense>
    </ErrorBoundary>
  );
}
```

## Suspense Props

| Prop | Type | Description |
| ---- | ---- | ----------- |
| `fallback` | `JSX.Element` | Content shown while children are loading |

## SuspenseList Props

| Prop | Type | Description |
| ---- | ---- | ----------- |
| `revealOrder` | `"forwards" \| "backwards" \| "together"` | Order to reveal children |
| `tail` | `"collapsed" \| "hidden"` | How to show loading placeholders |

## createResource Return Value

```tsx
const [data, { mutate, refetch }] = createResource(source, fetcher);

data()          // The data (or undefined while loading)
data.loading    // Boolean loading state
data.error      // Error if fetch failed
data.state      // "unresolved" | "pending" | "ready" | "refreshing" | "errored"
mutate(newData) // Manually update the data
refetch()       // Re-run the fetcher
```

## Why It Matters

1. **Declarative Loading**: Define fallbacks once, don't track loading states manually.

2. **Automatic Coordination**: Suspense knows when all children are ready.

3. **Progressive Loading**: Nested boundaries enable progressive content reveal.

4. **Required for lazy()**: Lazy components need a Suspense boundary.

## Related Rules

- [6-2: Use Lazy Components](6-2-use-lazy-components.md) - Code splitting
- [3-5: Provide Fallbacks](3-5-provide-fallbacks.md) - Fallback design
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Resource cleanup
