if(NOT DEFINED XW_BUNDLE_CONTENT_DIR OR NOT DEFINED XW_SHADER_SOURCE_DIR)
    message(FATAL_ERROR "macOS bundle staging paths are incomplete")
endif()

set(bundle_resource_dir "${XW_BUNDLE_CONTENT_DIR}/Resources")
set(bundle_shader_dir "${bundle_resource_dir}/shaders")

file(GLOB shader_files "${XW_SHADER_SOURCE_DIR}/*.msl")
if(NOT shader_files)
    message(FATAL_ERROR "No compiled MSL shaders found in ${XW_SHADER_SOURCE_DIR}")
endif()

file(MAKE_DIRECTORY "${bundle_shader_dir}")
file(COPY ${shader_files} DESTINATION "${bundle_shader_dir}")
