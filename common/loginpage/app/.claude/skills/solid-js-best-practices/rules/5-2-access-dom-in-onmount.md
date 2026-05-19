---
id: 5-2
title: Access DOM in onMount
category: Refs & DOM
priority: HIGH
description: Access DOM elements in onMount, not during render
---

## Problem

DOM elements don't exist during the component's initial execution—they're created after the JSX is processed. Accessing refs or performing DOM operations during render leads to undefined errors or incorrect behavior.

## Incorrect

```tsx
// ❌ WRONG: DOM access during component execution
function Chart(props) {
  let canvasRef: HTMLCanvasElement;

  // Element doesn't exist yet!
  const ctx = canvasRef.getContext("2d");
  ctx.fillRect(0, 0, 100, 100);

  return <canvas ref={canvasRef} />;
}
```

```tsx
// ❌ WRONG: Reading layout during render
function ResponsiveComponent() {
  let containerRef: HTMLDivElement;

  // containerRef is undefined, throws error
  const [width, setWidth] = createSignal(containerRef.offsetWidth);

  return <div ref={containerRef}>{width()}</div>;
}
```

```tsx
// ❌ WRONG: Setting up event listeners too early
function Scrollable() {
  let scrollRef: HTMLDivElement;

  // Element doesn't exist yet
  scrollRef.addEventListener("scroll", handleScroll);

  return <div ref={scrollRef}>Content</div>;
}
```

## Correct

```tsx
import { onMount, onCleanup } from "solid-js";

// ✅ CORRECT: DOM access in onMount
function Chart(props) {
  let canvasRef: HTMLCanvasElement;

  onMount(() => {
    const ctx = canvasRef.getContext("2d");
    ctx.fillRect(0, 0, 100, 100);
  });

  return <canvas ref={canvasRef} />;
}
```

```tsx
import { onMount, createSignal } from "solid-js";

// ✅ CORRECT: Read layout in onMount
function ResponsiveComponent() {
  let containerRef: HTMLDivElement;
  const [width, setWidth] = createSignal(0);

  onMount(() => {
    setWidth(containerRef.offsetWidth);

    // Optional: update on resize
    const observer = new ResizeObserver(entries => {
      setWidth(entries[0].contentRect.width);
    });
    observer.observe(containerRef);
    onCleanup(() => observer.disconnect());
  });

  return <div ref={containerRef}>{width()}px wide</div>;
}
```

```tsx
import { onMount, onCleanup } from "solid-js";

// ✅ CORRECT: Event listeners in onMount with cleanup
function Scrollable() {
  let scrollRef: HTMLDivElement;

  const handleScroll = (e: Event) => {
    console.log("Scrolled:", (e.target as HTMLDivElement).scrollTop);
  };

  onMount(() => {
    scrollRef.addEventListener("scroll", handleScroll);
    onCleanup(() => scrollRef.removeEventListener("scroll", handleScroll));
  });

  return <div ref={scrollRef} style={{ overflow: "auto", height: "200px" }}>
    {/* Long content */}
  </div>;
}
```

### One-Time Initialization

```tsx
import { onMount } from "solid-js";

// ✅ CORRECT: Third-party library initialization
function MapComponent(props) {
  let mapContainer: HTMLDivElement;

  onMount(() => {
    const map = new MapLibrary(mapContainer, {
      center: props.center,
      zoom: props.zoom
    });

    // Cleanup handled by onCleanup inside onMount
    onCleanup(() => map.destroy());
  });

  return <div ref={mapContainer} class="map" />;
}
```

### Async Operations

```tsx
import { onMount, createSignal } from "solid-js";

// ✅ CORRECT: Async work in onMount
function DataLoader() {
  const [data, setData] = createSignal(null);

  onMount(async () => {
    const response = await fetch("/api/data");
    const json = await response.json();
    setData(json);
  });

  return (
    <Show when={data()} fallback={<Loading />}>
      <DataDisplay data={data()} />
    </Show>
  );
}
```

### Multiple onMount Calls

```tsx
// ✅ CORRECT: Multiple onMount calls are valid
function Component() {
  onMount(() => {
    console.log("First initialization");
  });

  onMount(() => {
    console.log("Second initialization");
  });

  // Both run in order when component mounts
  return <div>Content</div>;
}
```

## onMount vs createEffect

| Aspect | onMount | createEffect |
| ------ | ------- | ------------ |
| Runs | Once on mount | On mount + when deps change |
| Tracking | None (not reactive) | Auto-tracks signal access |
| Use for | DOM setup, fetch, libraries | Reactive side effects |

```tsx
// Use onMount for one-time setup
onMount(() => {
  initLibrary(ref);
});

// Use createEffect for reactive DOM updates
createEffect(() => {
  // Re-runs when color() changes
  ref.style.backgroundColor = color();
});
```

## Why It Matters

1. **Element Existence**: DOM elements are created after component function runs.

2. **Error Prevention**: Accessing undefined refs throws runtime errors.

3. **Correct Timing**: `onMount` guarantees the DOM is ready.

4. **Cleanup Pairing**: `onCleanup` inside `onMount` ensures proper resource cleanup.

## Related Rules

- [5-1: Use Refs Correctly](5-1-use-refs-correctly.md) - Ref patterns
- [5-3: Cleanup with onCleanup](5-3-cleanup-with-oncleanup.md) - Resource cleanup
- [1-3: Effects for Side Effects](1-3-effects-for-side-effects.md) - When to use effects instead
