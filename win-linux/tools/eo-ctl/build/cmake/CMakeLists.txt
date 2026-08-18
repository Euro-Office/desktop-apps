# Standalone CLI target, shaped after core/X2tConverter/build/cmake/CMakeLists.txt
# (see cdp-gateway-cli-plan.md §3/§8 for why this precedent was chosen over folding
# eo-ctl's sources into DesktopEditors' own COMMON_SOURCES the way update-daemon is).

project(eo-ctl)

set(EO_CTL_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

add_executable(eo-ctl
    "${EO_CTL_ROOT_DIR}/src/main.cpp"
    "${EO_CTL_ROOT_DIR}/src/connectlogic.cpp"
)

set_target_properties(eo-ctl PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    AUTOMOC ON
)

target_link_libraries(eo-ctl PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
)
