# ---------------------------------------------------------------------------
# utils_xml -- XML parser and object binding
# ---------------------------------------------------------------------------

file(GLOB UTILS_XML_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB UTILS_XML_PRIVATE_HEADERS CONFIGURE_DEPENDS "src/*.h")
file(GLOB_RECURSE UTILS_XML_HEADERS CONFIGURE_DEPENDS "include/*.hpp")

add_library(utils_xml STATIC
    ${UTILS_XML_SOURCES}
    ${UTILS_XML_PRIVATE_HEADERS}
    ${UTILS_XML_HEADERS}
)
add_library(utils::xml ALIAS utils_xml)

target_include_directories(utils_xml
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(utils_xml PUBLIC utils::cpp)

set_target_properties(utils_xml PROPERTIES EXPORT_NAME xml)

install(TARGETS utils_xml
        EXPORT utils_cppTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set_source_files_properties(${UTILS_XML_HEADERS} PROPERTIES HEADER_FILE_ONLY ON)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    ${UTILS_XML_SOURCES} ${UTILS_XML_PRIVATE_HEADERS} ${UTILS_XML_HEADERS})

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(UTILS_BUILD_TESTS)
    file(GLOB UTILS_XML_TEST_SOURCES CONFIGURE_DEPENDS "test_src/*.cpp")

    add_executable(utils_xml_tests ${UTILS_XML_TEST_SOURCES})
    target_link_libraries(utils_xml_tests PRIVATE utils::xml)

    add_test(NAME utils_xml_tests COMMAND utils_xml_tests)
endif()
