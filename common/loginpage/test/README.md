# Start page unit tests

[Vitest](https://vitest.dev) tests for the start page scripts in `../src`, run
in a [jsdom](https://github.com/jsdom/jsdom) browser environment. CI runs them
on every change under `common/loginpage` (`.github/workflows/loginpage.yml`).

```bash
npm ci
npm test            # once
npm run test:watch  # rerun on changes
npm run coverage    # once, with a coverage report in coverage/
```

On pull requests, CI fails when the tests cover less than 80% of the
changed lines in `../src`.

The scripts in `../src` are plain browser scripts that `index.html` loads in
order and that share globals (jQuery, `window.utils`, ...). `load.js` runs
them unchanged in the test's jsdom window: call `loadUtils()` for jQuery and
`utils.js`, and `loadScripts('src/<file>.js', ...)` for more, in the order
`index.html` loads them. Each test file gets its own window. `load.js` runs
them under their file names, so coverage counts them like any other file.

This is separate from `../build`, which bundles the page with Grunt.
