# desktop-apps unit tests

GoogleTest suites registered with CTest, following the same setup as core
(see `core/TESTING.md`). They are off by default and enabled with
`-DEO_BUILD_TESTS=ON` plus the vcpkg `tests` manifest feature. This also
builds the desktop-sdk suites in `desktop-sdk/ChromiumBasedEditors/lib/tests`,
since desktop-apps builds the SDK as a subdirectory.

## Running

The Linux docker build (`build/linux/build.sh`) configures with tests enabled
and runs `ctest` after compiling, so a failing test fails the build.

In an existing build tree:

```bash
cmake -DEO_BUILD_TESTS=ON -DVCPKG_MANIFEST_FEATURES="desktop-editors;tests" .
cmake --build .
ctest --output-on-failure                       # all suites
ctest -R desktopeditors_setup_test --output-on-failure  # one suite
```

## Adding a suite

1. Add `<name>_test.cpp` here.
2. Register it in `CMakeLists.txt` with `add_core_gtest()`:
   - add `${DESKTOP_TEST_MAIN}` to `SOURCES` if the code under test needs a
     `QApplication` (widgets, event loop, `QSettings`). It starts Qt on the
     offscreen platform.
   - otherwise pass `GTEST_MAIN`.
3. List what it links in `LIBS`. Application code is in the
   `desktopeditors_core` object library (everything but `main.cpp`), which
   carries all include paths, definitions and dependencies of the app.
4. Call `use_build_tree_qt(<name>)`, so the test finds Qt when it runs from
   the build tree.

`setup_test.cpp` only checks the test setup itself (GoogleTest, Qt, headless
widgets) and is the minimal example.
