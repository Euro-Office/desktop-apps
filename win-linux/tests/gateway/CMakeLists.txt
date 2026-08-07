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

add_executable(gateway_word_character_formatting_test
    word_character_formatting_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_character_formatting_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_character_formatting_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_character_formatting
    COMMAND gateway_word_character_formatting_test
)

add_executable(gateway_word_paragraph_formatting_test
    word_paragraph_formatting_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_paragraph_formatting_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_paragraph_formatting_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_paragraph_formatting
    COMMAND gateway_word_paragraph_formatting_test
)

add_executable(gateway_word_search_replace_test
    word_search_replace_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_search_replace_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_search_replace_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_search_replace
    COMMAND gateway_word_search_replace_test
)

add_executable(gateway_word_table_test
    word_table_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_table_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_table_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_table
    COMMAND gateway_word_table_test
)

add_executable(gateway_word_style_test
    word_style_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_style_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_style_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_style
    COMMAND gateway_word_style_test
)

add_executable(gateway_word_image_shape_test
    word_image_shape_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_image_shape_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_image_shape_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_image_shape
    COMMAND gateway_word_image_shape_test
)

add_executable(gateway_word_page_setup_test
    word_page_setup_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_page_setup_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_page_setup_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_page_setup
    COMMAND gateway_word_page_setup_test
)

add_executable(gateway_word_bookmark_hyperlink_test
    word_bookmark_hyperlink_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_bookmark_hyperlink_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_bookmark_hyperlink_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_bookmark_hyperlink
    COMMAND gateway_word_bookmark_hyperlink_test
)

add_executable(gateway_word_form_field_test
    word_form_field_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_form_field_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_form_field_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_form_field
    COMMAND gateway_word_form_field_test
)

add_executable(gateway_word_comment_revision_test
    word_comment_revision_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/wordcommands.cpp
)

set_target_properties(gateway_word_comment_revision_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_word_comment_revision_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_word_comment_revision
    COMMAND gateway_word_comment_revision_test
)

add_executable(gateway_cell_sheet_management_test
    cell_sheet_management_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_sheet_management_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_sheet_management_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_sheet_management
    COMMAND gateway_cell_sheet_management_test
)

add_executable(gateway_cell_range_read_write_test
    cell_range_read_write_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_range_read_write_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_range_read_write_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_range_read_write
    COMMAND gateway_cell_range_read_write_test
)

add_executable(gateway_cell_number_format_merge_clear_test
    cell_number_format_merge_clear_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_number_format_merge_clear_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_number_format_merge_clear_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_number_format_merge_clear
    COMMAND gateway_cell_number_format_merge_clear_test
)

add_executable(gateway_cell_copy_find_replace_test
    cell_copy_find_replace_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_copy_find_replace_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_copy_find_replace_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_copy_find_replace
    COMMAND gateway_cell_copy_find_replace_test
)

add_executable(gateway_cell_formatting_test
    cell_formatting_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_formatting_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_formatting_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_formatting
    COMMAND gateway_cell_formatting_test
)

add_executable(gateway_cell_conditional_formatting_test
    cell_conditional_formatting_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_conditional_formatting_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_conditional_formatting_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_conditional_formatting
    COMMAND gateway_cell_conditional_formatting_test
)

add_executable(gateway_cell_validation_defname_test
    cell_validation_defname_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_validation_defname_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_validation_defname_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_validation_defname
    COMMAND gateway_cell_validation_defname_test
)

add_executable(gateway_cell_autofilter_test
    cell_autofilter_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_autofilter_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_autofilter_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_autofilter
    COMMAND gateway_cell_autofilter_test
)

add_executable(gateway_cell_pivot_table_test
    cell_pivot_table_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_pivot_table_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_pivot_table_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_pivot_table
    COMMAND gateway_cell_pivot_table_test
)

add_executable(gateway_cell_freeze_panes_test
    cell_freeze_panes_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_freeze_panes_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_freeze_panes_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_freeze_panes
    COMMAND gateway_cell_freeze_panes_test
)

add_executable(gateway_cell_image_ole_test
    cell_image_ole_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_image_ole_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_image_ole_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_image_ole
    COMMAND gateway_cell_image_ole_test
)

add_executable(gateway_cell_comment_reply_test
    cell_comment_reply_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_comment_reply_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_comment_reply_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_comment_reply
    COMMAND gateway_cell_comment_reply_test
)

add_executable(gateway_cell_insert_delete_rowcol_test
    cell_insert_delete_rowcol_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_insert_delete_rowcol_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_insert_delete_rowcol_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_insert_delete_rowcol
    COMMAND gateway_cell_insert_delete_rowcol_test
)

add_executable(gateway_cell_recalculate_test
    cell_recalculate_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_recalculate_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_recalculate_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_recalculate
    COMMAND gateway_cell_recalculate_test
)

add_executable(gateway_cell_chart_test
    cell_chart_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_chart_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_chart_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_chart
    COMMAND gateway_cell_chart_test
)

add_executable(gateway_cell_smartart_test
    cell_smartart_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/cellcommands.cpp
)

set_target_properties(gateway_cell_smartart_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_cell_smartart_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_cell_smartart
    COMMAND gateway_cell_smartart_test
)

add_executable(gateway_slide_management_test
    slide_management_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_management_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_management_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_management
    COMMAND gateway_slide_management_test
)

add_executable(gateway_slide_content_enumeration_test
    slide_content_enumeration_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_content_enumeration_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_content_enumeration_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_content_enumeration
    COMMAND gateway_slide_content_enumeration_test
)

add_executable(gateway_slide_layout_master_test
    slide_layout_master_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_layout_master_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_layout_master_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_layout_master
    COMMAND gateway_slide_layout_master_test
)

add_executable(gateway_slide_transition_test
    slide_transition_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_transition_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_transition_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_transition
    COMMAND gateway_slide_transition_test
)

add_executable(gateway_slide_shape_test
    slide_shape_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_shape_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_shape_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_shape
    COMMAND gateway_slide_shape_test
)

add_executable(gateway_slide_text_formatting_test
    slide_text_formatting_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_text_formatting_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_text_formatting_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_text_formatting
    COMMAND gateway_slide_text_formatting_test
)

add_executable(gateway_slide_image_test
    slide_image_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_image_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_image_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_image
    COMMAND gateway_slide_image_test
)

add_executable(gateway_slide_table_test
    slide_table_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_table_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_table_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_table
    COMMAND gateway_slide_table_test
)

add_executable(gateway_slide_notes_test
    slide_notes_test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/allowlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/gateway/commands/slidecommands.cpp
)

set_target_properties(gateway_slide_notes_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(gateway_slide_notes_test PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

add_test(
    NAME gateway_slide_notes
    COMMAND gateway_slide_notes_test
)
