add_executable(examp_data examp_data.cpp)
set_target_properties(examp_data PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(examp_data ${LIBCORE} ${LIBCLASSIFIER} ${LIBVALIDATION})

add_executable(examp_exp examp_expression.cpp)
set_target_properties(examp_exp PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
target_link_libraries(examp_exp ${LIBCORE} ${LIBCLASSIFIER} ${LIBVALIDATION})

add_executable(bench_exptmp bench_exptmp.cpp)
set_target_properties(bench_exptmp PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(bench_exptmp PUBLIC ${LIBCORE} OpenMP::OpenMP_CXX ${LIBCLASSIFIER} ${LIBVALIDATION})
else()
    target_link_libraries(bench_exptmp ${LIBCORE} ${LIBCLASSIFIER} ${LIBVALIDATION})
endif()
add_test(bench_test bench_exptmp)
