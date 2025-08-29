Testing
=======

The project uses **GoogleTest** for unit testing. Each individual C++ `TEST()`
case is discovered automatically at CMake *configure* time via
``gtest_discover_tests`` so that **CTest** lists and runs them separately.

Quick Start
-----------

.. code-block:: bash

   # Configure with tests enabled (default ENABLE_TESTING=ON)
   cmake -S . -B build -DENABLE_TESTING=ON

   # Build and run all tests using the aggregate target
   cmake --build build --target test_all -j 4

   # Or use the helper script (configures if needed)
   ./run_tests.sh

Running Tests
-------------

.. code-block:: bash

   # Run all tests with output on failure
   ctest --test-dir build --output-on-failure

   # List tests without running
   ctest --test-dir build -N

   # Run tests whose names match a regex
   ctest --test-dir build -R UtilsTests

   # Run a single test directly via GoogleTest filtering
   ./build/compi_tests --gtest_filter=TokenTests.BasicLexing

Convenience Targets
-------------------

.. code-block:: bash

   cmake --build build --target test_all   # Build and run all tests
   cmake --build build --target docs       # Build Sphinx docs

Adding New Tests
----------------

Add a new ``*.cpp`` file (or additional ``TEST`` blocks) under ``tests/``.
On the next configure or build, CMake re-discovers and registers them—no
manual edit to ``CMakeLists.txt`` is required.

Internals / Notes
-----------------

* Tests link against the reusable ``compi_core`` static library (all C sources
  except the CLI ``compi.c``) to avoid duplicating logic.
* Some tests touch internal global variables (e.g. ``g_array_count``,
  ``current_token``). Future refactors may wrap these with a fixture to
  improve isolation.
* Enable additional parser/code generation diagnostics by configuring with
  ``-DDEBUG=ON``.
* A future enhancement will introduce integration (end‑to‑end) tests comparing
  expected VHDL output for sample C inputs.

Planned Enhancements
--------------------

* Parser end‑to‑end tests (C → AST → VHDL golden output)
* Coverage reporting (gcov / lcov) optional target
* Test fixtures around global state
* Negative / error path tests for parser diagnostics
