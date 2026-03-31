cmake_minimum_required(VERSION 3.30)
project(AuthService)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ========== 输出目录统一配置 ==========
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/runtime)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ========== 子模块 ==========
add_subdirectory(libs/Bcrypt.cpp)
add_subdirectory(libs/redis-plus-plus)
add_subdirectory(libs/nlohmann_json)

# ========== 依赖查找 ==========
find_package(Drogon REQUIRED)
find_package(PostgreSQL REQUIRED)

# ========== 头文件包含 ==========
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/jwt-cpp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/redis-plus-plus
)

# ========== 可执行文件 ==========
set(EXECUTABLE_NAME my_drogon_app)

add_executable(${EXECUTABLE_NAME}
        src/main.cpp
        src/models/user.hpp
        src/dto/userDtro.hpp
        src/middlewares/ValidateRequestBodyMiddleware.hpp
        src/controllers/authController.cpp
        src/controllers/authController.h
        src/controllers/api_v1_health.cc
        src/controllers/api_v1_health.h
        src/repositories/userRepos.hpp
        src/repositories/userRepos.cpp
        src/services/serviceAuth.cpp
        src/services/serviceAuth.hpp
        src/repositories/sessionRepos.cpp
        src/repositories/sessionRepos.hpp
        src/utils/jwt/jwtToken.hpp
        src/middlewares/TokenExtractionMiddleware.hpp
        src/middlewares/ValidateTokensMiddleware.hpp
        src/middlewares/ValidatePasswordMiddleware.hpp
        src/middlewares/ValidateEmailAndUsernameMiddleware.hpp
        src/middlewares/ValidateRefreshMiddleware.hpp
)

# ========== hiredis + redis++ 依赖 ==========
find_path(HIREDIS_HEADER hiredis)
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${HIREDIS_HEADER})
find_library(HIREDIS_LIB hiredis)

# ========== 链接库 ==========
target_link_libraries(${EXECUTABLE_NAME}
        PRIVATE
        Drogon::Drogon
        nlohmann_json::nlohmann_json
        PostgreSQL::PostgreSQL
        bcrypt
        ${HIREDIS_LIB}
        redis++
)

# ========== ✅ 核心修复：RPATH 绝对正确 ==========
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
        # 关键：让运行时搜索 ./runtime 目录
        INSTALL_RPATH "\$ORIGIN/runtime"
        BUILD_RPATH "\$ORIGIN/runtime"

        # 强制启用 ORIGIN 相对路径
        BUILD_RPATH_USE_ORIGIN ON
        SKIP_BUILD_RPATH OFF
        NO_BUILD_RPATH OFF
        # 清除系统默认 RPATH，避免冲突
        CMAKE_BUILD_WITH_INSTALL_RPATH ON
)

# ========== Debug 宏 ==========
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${EXECUTABLE_NAME} PRIVATE DEBUG)
endif()

# ========== 拷贝配置文件 ==========
set(CONFIG_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/src/config.json")
if(NOT EXISTS ${CONFIG_FILE})
    message(FATAL_ERROR "配置文件不存在: ${CONFIG_FILE}")
endif()

add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CONFIG_FILE}"
        "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
        COMMENT "拷贝配置文件到目标目录"
        VERBATIM
)