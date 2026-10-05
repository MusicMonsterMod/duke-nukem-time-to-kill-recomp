cmake_minimum_required(VERSION 3.20)
project(duke_nukem_time_to_kill_recomp LANGUAGES C CXX)

# Keep debug symbols/server in the player build, but optimize the generated
# instruction helpers for throughput. The opening scene exhausted its CPU
# budget with the default RelWithDebInfo -O2 build (D17B, 2026-10-03).
# Preserve additional caller-supplied flags and non-GNU compiler defaults.
foreach(_ttk_lang C CXX)
    if(CMAKE_${_ttk_lang}_COMPILER_ID MATCHES "GNU|Clang" AND NOT MSVC)
        string(REGEX REPLACE "(^| )-O2( |$)" "\\1-O3\\2"
            CMAKE_${_ttk_lang}_FLAGS_RELWITHDEBINFO "${CMAKE_${_ttk_lang}_FLAGS_RELWITHDEBINFO}")
    endif()
endforeach()

set(PSXRECOMP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/psxrecomp" CACHE PATH
    "Path to psxrecomp")

# --- Opt-in product surfaces (OFF by default in the framework) -------------
# Scaffold prompts / flags fill these. Without recomp-ui, PSX_RECOMP_UI is OFF.
# Setup-host CI requires WIZARD_CMAKE_BLOCK / ENABLE_SETUP_WIZARD (FORCE_SETUP_HOST
# alone does not open first-run / Generate & rebuild).
# recomp-ui submodule present (PSX_RECOMP_UI defaults ON).
# Full netplay UI — enable after adding recomp-ui + testing:
# if(EXISTS "${PSXRECOMP_ROOT}/lib/recomp-net/CMakeLists.txt")
#     set(PSX_NETPLAY ON CACHE BOOL "…" FORCE)
# endif()
set(PSX_SETUP_WIZARD ON CACHE BOOL
    "Advertise first-run setup wizard + Generate & rebuild in recomp-ui" FORCE)
include("${PSXRECOMP_ROOT}/runtime/runtime.cmake")

psxrecomp_add_game_runtime(psx-runtime
    # ENABLE_NETPLAY_IF_PRESENT
    # NETPLAY_LOBBY_URL "ws://netplay.retcomm.net:8765"
    ENABLE_SETUP_WIZARD
    WINDOW_TITLE "Duke Nukem: Time to Kill Recompiled"
    EXE_NAME "Duke_Nukem__Time_to_Kill_Recompiled"
    DEFAULT_GAME_CONFIG_PATH "game.toml"
    MAX_PLAYERS 1
    APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/psxrecomp.ico"
    GEN_MARKER "generated/SLUS_005.83_dispatch.c"
    GEN_FULL_GLOB "generated/SLUS_005.83_full_*.c"
    CODEGEN_SETUP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/codegen_setup.c"
)

# D15: retain projection provenance through resident mesh drawing/subdivision.
# Compile authored build definitions, never edit generated C or change save ABI.
# D23F developer calibration build: measure the accurate model instead.
option(TTK_FT_CALIBRATE "D23F: build the accurate-model timing calibration (developer only)" OFF)
if(TTK_FT_CALIBRATE)
    set(_ttk_timing_header fast_timing_calibrate.h)
else()
    set(_ttk_timing_header fast_timing.h)
endif()
file(GLOB _ttk_generated_mesh "${CMAKE_CURRENT_SOURCE_DIR}/generated/SLUS_005.83_full_*.c")
foreach(_ttk_source IN LISTS _ttk_generated_mesh)
    file(STRINGS "${_ttk_source}" _ttk_mesh_entry REGEX "^void func_800(10000|11020|1205C|12960)")
    if(_ttk_mesh_entry)
        set_property(SOURCE "${_ttk_source}" APPEND PROPERTY COMPILE_DEFINITIONS PSX_PGXP=1)
    endif()
    # D23F: optional fast CPU timing for the game code (runtime-selected,
    # Modernized gameplay only). The generated C itself is unchanged.
    set_property(SOURCE "${_ttk_source}" APPEND PROPERTY COMPILE_OPTIONS
        -include "${CMAKE_CURRENT_SOURCE_DIR}/src/ttk/${_ttk_timing_header}")
    set_property(SOURCE "${_ttk_source}" APPEND PROPERTY OBJECT_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ttk/${_ttk_timing_header}")
endforeach()
target_sources(psx-runtime PRIVATE src/ttk/fast_timing.c)
if(TTK_FT_CALIBRATE)
    set_property(SOURCE src/ttk/fast_timing.c APPEND PROPERTY COMPILE_DEFINITIONS TTK_FT_CALIBRATE=1)
endif()

# Repair the pinned fetched SDL3 X11 raw-event millisecond/nanosecond mismatch.
# Never modify an installed/system SDL library.
if(PSX_SDL3 AND SDL3_SOURCE_DIR AND EXISTS "${SDL3_SOURCE_DIR}/src/video/x11/SDL_x11events.c")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    execute_process(COMMAND "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/local/fix_sdl_x11_time.py" "${SDL3_SOURCE_DIR}"
        RESULT_VARIABLE _ttk_sdl_time_result)
    if(NOT _ttk_sdl_time_result EQUAL 0)
        message(FATAL_ERROR "SDL3 X11 timestamp repair needs review")
    endif()
endif()

# Stage runtime configs next to the exe for direct build/ launches. Only copy
# files that exist — never fail POST_BUILD when game_options.toml is absent.
foreach(_PSX_RUNTIME_CONFIG game.toml game_options.toml)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_PSX_RUNTIME_CONFIG}")
        add_custom_command(TARGET psx-runtime POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${_PSX_RUNTIME_CONFIG}"
                "$<TARGET_FILE_DIR:psx-runtime>/${_PSX_RUNTIME_CONFIG}"
            COMMENT "Staging ${_PSX_RUNTIME_CONFIG}"
            VERBATIM)
    endif()
endforeach()

# Time to Kill-specific, byte-guarded movie-ring wait acceleration.
target_sources(psx-runtime PRIVATE src/ttk/fmv_poll.c src/ttk/scene_lifetime.cpp)
# Title-owned preloaded mod packages (D14 widescreen hook), staged after the
# framework catalog so they sit beside the executable in mods/packages.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mods/preloaded/packages")
    add_custom_command(TARGET psx-runtime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/mods/preloaded/packages"
            "$<TARGET_FILE_DIR:psx-runtime>/mods/packages"
        COMMENT "Staging Time to Kill preloaded mod packages"
        VERBATIM)
endif()

# Native MOVIE.OVR shard (intro FMV decoder). Its cache namespace includes the
# game.local.toml hook list, so re-check it after every runtime build instead of
# only from build.py; a stranded shard silently drops the intro to the
# interpreter. Skips without a prepared disc/recompiler; a failed compile fails
# the build. See documentation/59-fmv-shard-namespace.md.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(ttk-movie-shard ALL
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/local/build_movie_overlay.py"
        --if-ready --quiet
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Checking the native movie shard"
    VERBATIM)
add_dependencies(ttk-movie-shard psx-runtime)

# The geometry library modifies two opcodes during initialization. Retain an
# exactly guarded native version so fresh games do not fall back to interpreting
# this hot routine, and keep it in the same checked cache namespace as the FMV.
add_custom_target(ttk-math-shard ALL
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/local/build_math_overlay.py"
        --if-ready --quiet
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Checking the initialized geometry math shard"
    VERBATIM)
add_dependencies(ttk-math-shard psx-runtime)

# D04 PC action input; framework seams are retained in patches/.
target_sources(psx-runtime PRIVATE src/ttk/pc_input.cpp src/ttk/modern_controls.cpp src/ttk/weapon_aim.cpp src/ttk/near_clip.cpp src/ttk/frame_replay.cpp src/ttk/level_select.cpp)
target_include_directories(psx-runtime PRIVATE src/ttk)

# Explicit opt-in native input checks use the same SDL backend as the player.
if(TARGET SDL3::SDL3)
    add_executable(ttk-input-test EXCLUDE_FROM_ALL tests/local/pc_input_native.cpp src/ttk/pc_input.cpp)
    target_include_directories(ttk-input-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
    target_compile_definitions(ttk-input-test PRIVATE PSX_SDL3=1)
    target_compile_features(ttk-input-test PRIVATE cxx_std_17)
    target_link_libraries(ttk-input-test PRIVATE SDL3::SDL3)
endif()

if(TARGET SDL3::SDL3)
    add_executable(ttk-controls-test EXCLUDE_FROM_ALL tests/local/modern_controls_native.cpp
        src/ttk/modern_controls.cpp src/ttk/near_clip.cpp ${PSXRECOMP_ROOT}/runtime/src/psx_sha256.c)
    target_include_directories(ttk-controls-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
    target_compile_definitions(ttk-controls-test PRIVATE PSX_SDL3=1)
    target_compile_options(ttk-controls-test PRIVATE -UNDEBUG)
    target_compile_features(ttk-controls-test PRIVATE cxx_std_17)
    target_link_libraries(ttk-controls-test PRIVATE SDL3::SDL3)
endif()

if(TARGET SDL3::SDL3)
add_executable(ttk-near-test EXCLUDE_FROM_ALL tests/local/near_clip_native.cpp src/ttk/near_clip.cpp
    ${PSXRECOMP_ROOT}/runtime/src/psx_sha256.c)
target_include_directories(ttk-near-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
target_compile_options(ttk-near-test PRIVATE -UNDEBUG)
target_compile_features(ttk-near-test PRIVATE cxx_std_17)
target_compile_definitions(ttk-near-test PRIVATE PSX_SDL3=1)
target_link_libraries(ttk-near-test PRIVATE SDL3::SDL3)
endif()

if(TARGET SDL3::SDL3)
    add_executable(ttk-aim-test EXCLUDE_FROM_ALL tests/local/weapon_aim_native.cpp
        src/ttk/weapon_aim.cpp ${PSXRECOMP_ROOT}/runtime/src/psx_sha256.c)
    target_include_directories(ttk-aim-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
    target_compile_definitions(ttk-aim-test PRIVATE PSX_SDL3=1)
    target_compile_options(ttk-aim-test PRIVATE -UNDEBUG)
    target_compile_features(ttk-aim-test PRIVATE cxx_std_17)
    target_link_libraries(ttk-aim-test PRIVATE SDL3::SDL3)
endif()

add_executable(ttk-scene-test EXCLUDE_FROM_ALL tests/local/scene_lifetime_native.cpp
    src/ttk/scene_lifetime.cpp ${PSXRECOMP_ROOT}/runtime/src/psx_sha256.c)
target_include_directories(ttk-scene-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
target_compile_options(ttk-scene-test PRIVATE -UNDEBUG)
target_compile_features(ttk-scene-test PRIVATE cxx_std_17)

# Title-specific cheat feedback uses the existing host OSD without launcher UI.
set_source_files_properties("${PSXRECOMP_ROOT}/runtime/src/host_osd.c" PROPERTIES COMPILE_DEFINITIONS TTK_CHEAT_OSD=1)

# Owned local Duke sprites. Keep generated art/provenance beside the executable;
# a checkout without supplied art retains the original host-font fallback.
target_sources(psx-runtime PRIVATE src/ttk/duke_font.cpp src/ttk/inventory_hud.cpp)
# Font ZIP/PK3 still live under research/fonts for the Atomic menu set; message
# glyphs are owned copies under assets/fonts/Messages (never link research/inv).
set(TTK_FONT_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/../research/fonts/MS-DOS - Duke Nukem 3D - Miscellaneous - Fonts.zip")
set(TTK_FONT_MESSAGES "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Messages")
if(EXISTS "${TTK_FONT_ARCHIVE}")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(TTK_FONT_BUILD_ARGS
        "${TTK_FONT_ARCHIVE}" "${CMAKE_CURRENT_BINARY_DIR}/ttk-fonts.pack")
    set(TTK_FONT_EXTRA_DEPS
        "${CMAKE_CURRENT_SOURCE_DIR}/../research/fonts/DukeNukemSmallFont.pk3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../research/fonts/DukeNukemAtomicFont.pk3")
    if(EXISTS "${TTK_FONT_MESSAGES}")
        list(APPEND TTK_FONT_BUILD_ARGS --messages "${TTK_FONT_MESSAGES}")
        file(GLOB TTK_FONT_MESSAGE_PNGS CONFIGURE_DEPENDS
            "${TTK_FONT_MESSAGES}/TILES011_*.png"
            "${TTK_FONT_MESSAGES}/Atomic Edition/TILES011_*.png")
        list(APPEND TTK_FONT_EXTRA_DEPS ${TTK_FONT_MESSAGE_PNGS}
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Messages.provenance.json")
    endif()
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ttk-fonts.pack" "${CMAKE_CURRENT_BINARY_DIR}/ttk-fonts.json"
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/local/build_duke_fonts.py"
            ${TTK_FONT_BUILD_ARGS}
        DEPENDS "${TTK_FONT_ARCHIVE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/local/build_duke_fonts.py"
            ${TTK_FONT_EXTRA_DEPS}
        VERBATIM)
    add_custom_target(ttk-font-assets DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ttk-fonts.pack" "${CMAKE_CURRENT_BINARY_DIR}/ttk-fonts.json")
    add_dependencies(psx-runtime ttk-font-assets)
endif()
# Owned Duke3D THREEBYFIVE extract (tiles 3010–3021) for EDuke32-style inventory %.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/ttk-inv-digits.pack")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ttk-inv-digits.pack"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/ttk-inv-digits.pack"
            "${CMAKE_CURRENT_BINARY_DIR}/ttk-inv-digits.pack"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/ttk-inv-digits.pack"
        VERBATIM)
    add_custom_target(ttk-inv-digit-assets DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ttk-inv-digits.pack")
    add_dependencies(psx-runtime ttk-inv-digit-assets)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/ttk-inv-icons.pack")
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/ttk-inv-icons.pack"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/ttk-inv-icons.pack"
            "${CMAKE_CURRENT_BINARY_DIR}/ttk-inv-icons.pack"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/ttk-inv-icons.pack"
        VERBATIM)
    add_custom_target(ttk-inv-icon-assets DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/ttk-inv-icons.pack")
    add_dependencies(psx-runtime ttk-inv-icon-assets)
endif()
if(TARGET SDL3::SDL3)
    add_executable(ttk-font-test EXCLUDE_FROM_ALL tests/local/duke_font_native.cpp src/ttk/duke_font.cpp)
    target_include_directories(ttk-font-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
    target_compile_definitions(ttk-font-test PRIVATE PSX_SDL3=1)
    target_compile_options(ttk-font-test PRIVATE -UNDEBUG)
    target_compile_features(ttk-font-test PRIVATE cxx_std_17)
    target_link_libraries(ttk-font-test PRIVATE SDL3::SDL3)
endif()

if(TARGET SDL3::SDL3)
    add_executable(ttk-inventory-test EXCLUDE_FROM_ALL tests/local/inventory_hud_native.cpp
        src/ttk/inventory_hud.cpp src/ttk/duke_font.cpp)
    target_include_directories(ttk-inventory-test PRIVATE src/ttk ${PSXRECOMP_ROOT}/runtime/include)
    target_compile_definitions(ttk-inventory-test PRIVATE PSX_SDL3=1)
    target_compile_options(ttk-inventory-test PRIVATE -UNDEBUG)
    target_compile_features(ttk-inventory-test PRIVATE cxx_std_17)
    target_link_libraries(ttk-inventory-test PRIVATE SDL3::SDL3)
endif()
