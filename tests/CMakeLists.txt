set(TEST_FOLDING "unit_tests_folding")
add_executable(${TEST_FOLDING} TestFolding.cc TestUtils.cc)
target_include_directories(${TEST_FOLDING} PRIVATE 
    ${CMAKE_BINARY_DIR}/configured_files/include
    ${Folding_SOURCE_DIR}
    ${catch2_INCLUDE_DIRS_RELEASE}
    ${libtorch_INCLUDE_DIRS_RELEASE}
    ${arff-files_INCLUDE_DIRS_RELEASE}
    ${fimdlp_INCLUDE_DIRS_RELEASE}
)
target_link_libraries(${TEST_FOLDING} PUBLIC 
    ${Torch_LIBRARIES}
    arff-files::arff-files 
    fimdlp::fimdlp 
    Catch2::Catch2WithMain
)
add_test(NAME ${TEST_FOLDING} COMMAND ${TEST_FOLDING})
