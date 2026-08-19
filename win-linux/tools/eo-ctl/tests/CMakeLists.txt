# connectlogic is pure C++17 (no Qt dependency -- see connectlogic.h), so its test
# doesn't need find_package(Qt...) or AUTOMOC, unlike everything under tests/gateway.

add_executable(eo_ctl_connectlogic_test
    connectlogic_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/connectlogic.cpp
)

set_target_properties(eo_ctl_connectlogic_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

add_test(
    NAME eo_ctl_connectlogic
    COMMAND eo_ctl_connectlogic_test
)
