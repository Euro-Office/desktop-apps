# Start page unit tests

[Vitest](https://vitest.dev) tests for the start page scripts in `../src`, run
in a [jsdom](https://github.com/jsdom/jsdom) browser environment. CI runs them
on every change under `common/loginpage` (`.github/workflows/loginpage.yml`).

```bash
npm ci
npm test            # once
npm run test:watch  # rerun on changes
```

The scripts in `../src` are plain browser scripts that `index.html` loads in
order and that share globals (jQuery, `window.utils`, ...). `load.js` runs
them unchanged in the test's jsdom window: call `loadUtils()` for jQuery and
`utils.js`, and `loadScripts('src/<file>.js', ...)` for more, in the order
`index.html` loads them. Each test file gets its own window.

This is separate from `../build`, which bundles the page with Grunt.
