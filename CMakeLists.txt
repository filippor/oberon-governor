cmake_minimum_required(VERSION 3.20.0)
project(oberon-governor VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(LIBDRM_AMDGPU REQUIRED libdrm_amdgpu)

# Executable
add_executable(oberon-governor
	src/governor.cpp
	src/governor.hpp
	src/oberon.cpp
	src/oberon.hpp
	src/main.cpp
)
target_compile_definitions(oberon-governor PUBLIC GOVERNOR_VERSION="${PROJECT_VERSION}")
target_include_directories(oberon-governor PUBLIC ${LIBDRM_INCLUDE_DIRS} ${LIBDRM_AMDGPU_INCLUDE_DIRS})
target_link_libraries(oberon-governor ${LIBDRM_LIBRARIES} ${LIBDRM_AMDGPU_LIBRARIES} yaml-cpp::yaml-cpp)
install(
	TARGETS oberon-governor
	DESTINATION "bin"
)

# Service
configure_file(init/oberon-governor.service.in oberon-governor.service)
install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/oberon-governor.service"
	DESTINATION "/usr/lib/systemd/system"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

# Config file
configure_file(init/oberon-config.yaml.in oberon-config.yaml)
install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/oberon-config.yaml"
	DESTINATION "/etc/"
	PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)
