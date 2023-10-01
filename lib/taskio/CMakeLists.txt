file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.cpp)

target_sources(taskio PRIVATE ${sources})