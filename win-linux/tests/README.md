# desktop-apps unit tests

GoogleTest suites registered with CTest. Every suite can run in two builds:

| Build | SDK | Where it runs |
|---|---|---|
| Unit test build (`fake-sdk/`) | a fake desktop-sdk | `.github/workflows/unit-tests.yml` on every PR, with coverage |
| Full build (`../CMakeLists.txt`, `EO_BUILD_TESTS=ON`) | the real desktop-sdk and CEF | the Linux docker build in DesktopEditors |

The full build also runs the desktop-sdk suites in
`desktop-sdk/ChromiumBasedEditors/lib/tests`, since desktop-apps builds the
SDK as a subdirectory.

## Unit test build

`fake-sdk/CMakeLists.txt` compiles the application code (the sources in
`../desktopeditors.cmake`) against the real desktop-sdk headers, but links a
fake SDK instead of the real one. It needs Qt (the version the release
build uses), GoogleTest and the GTK, X11 and D-Bus development packages, but
not core, CEF or docker. desktop-sdk and core are expected next to
desktop-apps, as in the DesktopEditors superproject; of core only a few
header-only files are used (see the workflow for the list).

```bash
cmake -S win-linux/tests/fake-sdk -B build -G Ninja [-DEO_COVERAGE=ON]
cmake --build build
ctest --test-dir build --output-on-failure

# with EO_COVERAGE=ON, a coverage report for the app code:
gcovr --root . --object-directory build --filter win-linux/src/ --html-details coverage.html
```

The fake SDK (`fake-sdk/*.cpp`) records what the app asks the SDK to do and
does nothing else. It also fakes core's `CFileDownloader`, which the app
still uses directly for the editor's avatar. Suites control it through `fake-sdk/fakesdk.h`:

- `FakeSdk::calls("CCefView::load")`: arguments of the calls the app made
- `FakeSdk::sendEvent(event)`: send an SDK event to the app's listener
- `FakeSdk::view(id)`, `FakeSdk::setFileFormat(path, format)`, `FakeSdk::reset()`

The app's `AscAppManager` is a singleton, so it lives for the whole suite;
call `FakeSdk::reset()` in `SetUp()` to forget recorded calls.

When the app starts using an SDK function the fake doesn't have yet, the unit
test build fails to link; add the function to the fake then.

## Full build

The Linux docker build (`build/linux/build.sh` in DesktopEditors) configures
with tests enabled and runs `ctest` after compiling, so a failing test fails
the build. In an existing build tree:

```bash
cmake -DEO_BUILD_TESTS=ON -DVCPKG_MANIFEST_FEATURES="desktop-editors;tests" .
cmake --build .
ctest --output-on-failure
```

## Adding a suite

1. Add `<name>_test.cpp` here.
2. Register it in `CMakeLists.txt` with `desktop_test()`:
   - add `${DESKTOP_TEST_MAIN}` to `SOURCES` if the code under test needs a
     `QApplication` (widgets, event loop, `QSettings`). It starts Qt on the
     offscreen platform. Otherwise pass `GTEST_MAIN`.
   - link `desktopeditors_core` in `LIBS` for application code (everything
     but `main.cpp`, with all include paths and dependencies of the app).
   - `SCOPE unit` if the suite uses `fakesdk.h`, `SCOPE full` if it needs the
     real SDK or CEF. Without `SCOPE`, both builds run it.

Prefer unit tests: only they run on PRs and count towards the coverage the
workflow checks. It fails when less than 80% of a PR's changed lines in
`win-linux/src` are covered. Use `SCOPE full` for what the fake can't show, for example
how the real SDK or CEF behaves.

`setup_test.cpp` only checks the test setup itself (GoogleTest, Qt, headless
widgets) and is the minimal example; `tabpanel_test.cpp` shows the fake SDK.
