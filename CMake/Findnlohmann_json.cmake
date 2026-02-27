# - Try to find nlohmann_json (header-only)
# Once done, this will define:
#  nlohmann_json_FOUND
#  NLOHMANN_JSON_INCLUDE_DIRS
#  nlohmann_json::nlohmann_json (INTERFACE target)

find_path(
    NLOHMANN_JSON_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    PATH_SUFFIXES include
)

include(FindPackageHandleStandardArgs)
# 只要NLOHMANN_JSON_INCLUDE_DIR找到，就认为nlohmann_json_FOUND=TRUE；否则find_package(nlohmann_json REQUIRED)会失败并报错
find_package_handle_standard_args(
    nlohmann_json
    REQUIRED_VARS NLOHMANN_JSON_INCLUDE_DIR
)

if(nlohmann_json_FOUND)
    set(NLOHMANN_JSON_INCLUDE_DIRS "${NLOHMANN_JSON_INCLUDE_DIR}")
    # 防止重复定义同名target（比如系统或其他模块已经提供了这个target）
    if(NOT TARGET nlohmann_json::nlohmann_json)
        # 创建一个仅头文件的导入目标。INTERFACE表示没有实际编译产物，只传递用到的属性；IMPORTED表示它来自外部
        add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
        # 把NLOHMANN_JSON_INCLUDE_DIR作为头文件搜索路径绑定到这个target上
        set_target_properties(
            nlohmann_json::nlohmann_json
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${NLOHMANN_JSON_INCLUDE_DIR}"
        )
    endif()
endif()
