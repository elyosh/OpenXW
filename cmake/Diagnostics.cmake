# Opt-in sanitizer instrumentation, matching OpenXvT; include before all targets.
option(XW_ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(XW_ENABLE_UBSAN "Build with UndefinedBehaviorSanitizer (excluding alignment)" OFF)

if(XW_ENABLE_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
    message(STATUS "AddressSanitizer enabled (XW_ENABLE_ASAN=ON)")
endif()
if(XW_ENABLE_UBSAN AND NOT MSVC)
    # OpenTIE excludes alignment checks for the recovered serialized layouts.
    add_compile_options(-fsanitize=undefined -fno-sanitize=alignment -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=undefined)
    message(STATUS "UndefinedBehaviorSanitizer enabled (alignment excluded)")
endif()
