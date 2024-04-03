if(ENABLE_TESTING)
    include_directories(
        ${Folding_SOURCE_DIR}
        ${Folding_SOURCE_DIR}/lib/Files
        ${Folding_SOURCE_DIR}/lib/mdlp
        ${CMAKE_BINARY_DIR}/configured_files/include
    )
    set(TEST_FOLDING "unit_tests_folding")
    add_executable(${TEST_FOLDING} TestFolding.cc TestUtils.cc)
    target_link_libraries(${TEST_FOLDING} PUBLIC "${TORCH_LIBRARIES}" ArffFiles mdlp Catch2::Catch2WithMain)
    add_test(NAME ${TEST_FOLDING} COMMAND ${TEST_FOLDING})
endif(ENABLE_TESTING)
