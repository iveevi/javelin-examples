add_executable(features main.cpp)
set_property(TARGET features PROPERTY ENABLE_EXPORTS ON)
target_link_libraries(features javelin common)
target_compile_definitions(features PRIVATE $<$<CONFIG:DEBUG>:JVL_DEBUG=1>)
