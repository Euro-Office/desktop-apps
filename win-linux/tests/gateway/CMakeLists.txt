# First automated-test target in desktop-apps (none existed before this — see
# cdp-gateway-cli-plan.md investigation notes). Uses CTest, which ships with CMake
# itself, rather than adding gtest/Catch2/QtTest as a new external test-framework
# dependency — YAGNI given the harness needs are currently just "run these functions,
# report pass/fail" (see word_document_properties_test.cpp's own header comment for
# what is and isn't testable yet).

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(gateway_word_document_properties_test
    word_document_properties_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_document_properties_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_document_properties_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_document_properties
    COMMAND gateway_word_document_properties_test
)

add_executable(gateway_word_content_enumeration_test
    word_content_enumeration_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_content_enumeration_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_content_enumeration_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_content_enumeration
    COMMAND gateway_word_content_enumeration_test
)

add_executable(gateway_word_insert_edit_text_test
    word_insert_edit_text_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_insert_edit_text_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_insert_edit_text_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_insert_edit_text
    COMMAND gateway_word_insert_edit_text_test
)
