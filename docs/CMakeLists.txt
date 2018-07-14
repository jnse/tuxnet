option(BUILD_DOC "Build documentation" ON)

find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_SOURCE_DIR}/)
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation."
        VERBATIM)
else (DOXYGEN_FOUND)
    message("Doxygen is required to generate documentation.")
endif (DOXYGEN_FOUND)
