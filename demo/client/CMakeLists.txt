file(GLOB sources *.cpp)

set(name client)
add_executable(${name} ${sources})

target_link_libraries(${name} sses_client ${COMMON_LIBS})
