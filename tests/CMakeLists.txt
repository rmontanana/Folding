if(ENABLE_TESTING)
    include_directories(
        ${Folding_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/configured_files/include
        lib/Files
        lib/mdlp/src
    )
    set(TEST_FOLDING "unit_tests_folding")
    add_executable(${TEST_FOLDING} TestFolding.cc TestUtils.cc)
    target_link_libraries(${TEST_FOLDING} PUBLIC "${TORCH_LIBRARIES}" ArffFiles fimdlp Catch2::Catch2WithMain)
    add_test(NAME ${TEST_FOLDING} COMMAND ${TEST_FOLDING})
endif(ENABLE_TESTING)
