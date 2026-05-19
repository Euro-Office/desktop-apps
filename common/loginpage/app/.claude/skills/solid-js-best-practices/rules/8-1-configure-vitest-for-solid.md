---
id: 8-1
title: Configure Vitest for Solid
category: Testing
priority: CRITICAL
description: Configure Vitest with Solid-specific resolve conditions and plugin settings
---

## Problem

Vitest does not understand Solid's compilation model out of the box. Without the correct `resolve.conditions`, `vite-plugin-solid`, jsdom environment, and dependency optimization settings, tests silently import the server or production build of Solid instead of the development/browser build. This causes effects to never run, reactivity to break, and hydration-mode behavior in what should be client-side tests -- all with no error message.

## Incorrect

```ts
// vitest.config.ts
// ❌ WRONG: Missing Solid-specific configuration
import { defineConfig } from "vitest/config";

export default defineConfig({
  test: {
    environment: "jsdom",
  },
});
// Tests run but signals don't track, effects never fire,
// components render static HTML with no reactivity
```

```ts
// vitest.config.ts
// ❌ WRONG: Has plugin but missing resolve conditions
import { defineConfig } from "vitest/config";
import solidPlugin from "vite-plugin-solid";

export default defineConfig({
  plugins: [solidPlugin()],
  test: {
    environment: "jsdom",
  },
  // Missing resolve.conditions -- wrong module build imported
});
```

## Correct

```ts
// vitest.config.ts
// ✅ CORRECT: Full Solid-specific Vitest configuration
import { defineConfig } from "vitest/config";
import solidPlugin from "vite-plugin-solid";

export default defineConfig({
  plugins: [solidPlugin()],
  resolve: {
    conditions: ["development", "browser"],
  },
  test: {
    environment: "jsdom",
    globals: true,
    setupFiles: "./vitest.setup.ts",
    deps: {
      optimizer: {
        web: {
          include: ["solid-js", "solid-testing-library"],
        },
      },
    },
    transformMode: {
      web: [/\.[jt]sx?$/],
    },
  },
});
```

### Setup File

```ts
// vitest.setup.ts
// ✅ CORRECT: Import DOM matchers globally
import "@testing-library/jest-dom";
```

### Required Dependencies

```json
{
  "devDependencies": {
    "vitest": "^1.0.0",
    "jsdom": "^24.0.0",
    "vite-plugin-solid": "^2.0.0",
    "@solidjs/testing-library": "^0.8.0",
    "@testing-library/jest-dom": "^6.0.0",
    "@testing-library/user-event": "^14.0.0"
  }
}
```

### SolidStart Projects

```ts
// vitest.config.ts
// ✅ CORRECT: SolidStart uses its own Vite plugin
import { defineConfig } from "vitest/config";
import solid from "vite-plugin-solid";

export default defineConfig({
  plugins: [solid()],
  resolve: {
    conditions: ["development", "browser"],
  },
  test: {
    environment: "jsdom",
    globals: true,
    deps: {
      optimizer: {
        web: {
          include: ["solid-js", "@solidjs/router"],
        },
      },
    },
  },
});
```

### Browser Mode: Workspace Config for Unit + Integration Split

When a project has both fast unit tests (jsdom) and real-browser integration tests, use `defineWorkspace` to run them in separate projects with disjoint file globs.

```ts
// vitest.workspace.ts
// ✅ CORRECT: Split unit (jsdom) and integration (browser/Chromium) projects
import { defineWorkspace } from "vitest/config";
import solidPlugin from "vite-plugin-solid";

export default defineWorkspace([
  {
    // Unit tests — fast, jsdom
    plugins: [solidPlugin() as any], // `as any` needed if project Vite ≠ Vitest bundled Vite
    resolve: { conditions: ["development", "browser"] },
    test: {
      name: "unit",
      environment: "jsdom",
      include: ["src/**/*.unit.test.{ts,tsx}"],
      globals: true,
      setupFiles: "./vitest.setup.ts",
      deps: { optimizer: { web: { include: ["solid-js"] } } },
    },
  },
  {
    // Integration tests — real Chromium via Playwright
    plugins: [solidPlugin() as any],
    resolve: { conditions: ["development", "browser"] },
    test: {
      name: "integration",
      browser: {
        enabled: true,
        provider: "playwright",
        headless: true, // Required for CI — prevents browser window from opening
        instances: [{ browser: "chromium" }],
      },
      include: ["src/**/*.integration.test.{ts,tsx}"],
      globals: true,
      setupFiles: "./vitest.setup.ts",
      deps: {
        optimizer: {
          web: {
            // Prevents Vite from restarting mid-test when this dep is first imported
            include: ["solid-js", "@testing-library/jest-dom/vitest"],
          },
        },
      },
    },
  },
]);
```

Additional devDependencies for browser mode:

```json
{
  "devDependencies": {
    "@vitest/browser": "^2.0.0",
    "@vitest/browser-playwright": "^2.0.0"
  }
}
```

Key notes:

- The `as any` cast on `solidPlugin()` is needed when the project's Vite version differs from Vitest's bundled Vite — without it you get a type error on `plugins`
- `include` globs must be disjoint — each test file should run in exactly one project
- `headless: true` is required for CI/CD; omitting it opens a visible browser window during tests
- Adding `@testing-library/jest-dom/vitest` to `optimizeDeps.include` prevents Vite from triggering a full server restart the first time it is imported in a test

## Symptom Diagnosis

| Symptom | Likely Cause | Fix |
| ------- | ------------ | --- |
| Effects never fire | Missing `resolve.conditions` | Add `['development', 'browser']` |
| `createSignal` updates don't reach DOM | Wrong Solid build (server) | Add `vite-plugin-solid` and conditions |
| `document is not defined` | Missing jsdom environment | Add `environment: 'jsdom'` |
| Module resolution errors for solid-js | Missing deps optimizer | Add `solid-js` to `deps.optimizer.web.include` |
| `toBeInTheDocument` not found | Missing setup file | Create `vitest.setup.ts` with jest-dom import |
| TypeScript errors in test files | Missing globals config | Add `globals: true` or import from `vitest` |
| Browser window opens in CI | Missing `headless` config | Add `browser.headless: true` to the browser project |

## Why It Matters

1. **Silent Failures**: The wrong Solid build causes tests to pass with broken reactivity -- you're testing static HTML, not a reactive app.

2. **Effects and Memos**: The development/browser build is required for `createEffect` and `createMemo` to register reactive tracking.

3. **Root Cause Confusion**: Configuration bugs look like component bugs. Developers waste hours debugging "broken" components when the test setup is at fault.

4. **CI Reliability**: Tests that pass locally but use the wrong build produce meaningless coverage and green CI pipelines that miss real bugs.

## Related Rules

- [8-2: Wrap Render in Arrow Functions](8-2-wrap-render-in-arrow.md) - Correct config is a prerequisite for reactive rendering
- [8-3: Test Primitives in a Root](8-3-test-primitives-in-root.md) - Effects need both config and reactive owners
- [8-7: Browser Mode for Web Components and PWA APIs](8-7-browser-mode-for-web-components-and-pwa-apis.md) - When to use browser mode vs jsdom
