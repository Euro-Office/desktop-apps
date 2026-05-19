---
id: 6-2
title: Use Lazy Components
category: Performance
priority: MEDIUM
description: Use lazy() for code splitting large components
---

## Problem

Large applications that load all components upfront have slow initial load times. The `lazy` function enables code splitting, loading components only when they're actually needed.

## Incorrect

```tsx
// ❌ WRONG: All components loaded immediately
import Dashboard from "./Dashboard";
import Settings from "./Settings";
import Analytics from "./Analytics";
import AdminPanel from "./AdminPanel";
import UserProfile from "./UserProfile";

function App() {
  const [route] = useRoute();

  return (
    <Switch>
      <Match when={route() === "dashboard"}>
        <Dashboard />
      </Match>
      <Match when={route() === "settings"}>
        <Settings />
      </Match>
      <Match when={route() === "analytics"}>
        <Analytics />
      </Match>
      {/* All components are in the initial bundle */}
    </Switch>
  );
}
```

## Correct

```tsx
import { lazy, Suspense } from "solid-js";

// ✅ CORRECT: Components loaded on demand
const Dashboard = lazy(() => import("./Dashboard"));
const Settings = lazy(() => import("./Settings"));
const Analytics = lazy(() => import("./Analytics"));
const AdminPanel = lazy(() => import("./AdminPanel"));
const UserProfile = lazy(() => import("./UserProfile"));

function App() {
  const [route] = useRoute();

  return (
    <Suspense fallback={<PageLoader />}>
      <Switch>
        <Match when={route() === "dashboard"}>
          <Dashboard />
        </Match>
        <Match when={route() === "settings"}>
          <Settings />
        </Match>
        <Match when={route() === "analytics"}>
          <Analytics />
        </Match>
        {/* Components loaded only when route matches */}
      </Switch>
    </Suspense>
  );
}
```

### With Preloading

```tsx
import { lazy, Suspense } from "solid-js";

const Settings = lazy(() => import("./Settings"));

function Navigation() {
  // Preload on hover/focus
  const preloadSettings = () => {
    import("./Settings");
  };

  return (
    <nav>
      <a
        href="/settings"
        onMouseEnter={preloadSettings}
        onFocus={preloadSettings}
      >
        Settings
      </a>
    </nav>
  );
}
```

### Nested Lazy Loading

```tsx
import { lazy, Suspense } from "solid-js";

// Main routes
const Dashboard = lazy(() => import("./Dashboard"));

// Dashboard sub-routes (lazy loaded within Dashboard)
function Dashboard() {
  const Overview = lazy(() => import("./dashboard/Overview"));
  const Metrics = lazy(() => import("./dashboard/Metrics"));
  const Reports = lazy(() => import("./dashboard/Reports"));

  return (
    <div class="dashboard">
      <Sidebar />
      <Suspense fallback={<ContentLoader />}>
        <Switch>
          <Match when={tab() === "overview"}>
            <Overview />
          </Match>
          <Match when={tab() === "metrics"}>
            <Metrics />
          </Match>
          <Match when={tab() === "reports"}>
            <Reports />
          </Match>
        </Switch>
      </Suspense>
    </div>
  );
}
```

### Heavy Components

```tsx
import { lazy, Suspense, Show } from "solid-js";

// ✅ CORRECT: Lazy load heavy third-party components
const Chart = lazy(() => import("./Chart"));       // ChartJS
const Editor = lazy(() => import("./Editor"));     // Monaco/CodeMirror
const Map = lazy(() => import("./Map"));           // Mapbox/Leaflet

function DataVisualization(props) {
  return (
    <Suspense fallback={<ChartSkeleton />}>
      <Show when={props.showChart}>
        <Chart data={props.data} />
      </Show>
    </Suspense>
  );
}
```

### Conditional Lazy Loading

```tsx
import { lazy, Suspense, Show } from "solid-js";

const AdminTools = lazy(() => import("./AdminTools"));

function UserArea(props) {
  return (
    <div>
      <UserContent />

      {/* Only load admin tools for admins */}
      <Show when={props.user.isAdmin}>
        <Suspense fallback={<Loading />}>
          <AdminTools />
        </Suspense>
      </Show>
    </div>
  );
}
```

## Lazy Loading Best Practices

1. **Route-Level Splitting**: Each major route should be lazy loaded.

2. **Feature Splitting**: Large features (analytics, admin) should be separate chunks.

3. **Heavy Dependencies**: Components with large libraries (charts, editors) should be lazy.

4. **Suspense Boundaries**: Wrap lazy components in Suspense with meaningful fallbacks.

5. **Preloading**: Preload on hover/focus for perceived performance.

## What to Lazy Load

| Component Type | Lazy Load? |
| -------------- | ---------- |
| Route pages | Yes |
| Admin-only features | Yes |
| Heavy visualizations (charts, maps) | Yes |
| Modals/dialogs | Often yes |
| Core UI components | No |
| Frequently used components | No |
| Above-the-fold content | No |

## Why It Matters

1. **Faster Initial Load**: Smaller initial bundle means faster Time to Interactive.

2. **Reduced Bandwidth**: Users only download code for features they use.

3. **Better Caching**: Smaller chunks can be cached more effectively.

4. **Mobile Performance**: Critical for slower networks and devices.

## Bundle Analysis

```bash
# Analyze your bundle to find lazy loading opportunities
npm run build -- --analyze
# or
npx vite-bundle-visualizer
```

Look for:

- Large route components
- Heavy third-party libraries
- Features not used by all users

## Related Rules

- [6-3: Use Suspense](6-3-use-suspense.md) - Required for lazy components
- [3-5: Provide Fallbacks](3-5-provide-fallbacks.md) - Loading states
