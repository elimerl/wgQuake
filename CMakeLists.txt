cmake_minimum_required(VERSION 3.0...3.25)
project(
	wgQuake
	VERSION 0.1.0
	LANGUAGES C
)

# Include webgpu directory, to define the 'webgpu' target
add_subdirectory(webgpu)
add_subdirectory(sdl2webgpu)

add_executable(wgQuake main.c wgpu_app.h wgpu_app.c wgpu_utils.h wgpu_utils.c HandmadeMath.h)

set(SRC "${CMAKE_SOURCE_DIR}")
set(COMMON_SOURCES
"${SRC}/Quake/bgmusic.c"
"${SRC}/Quake/cd_null.c"
"${SRC}/Quake/cfgfile.c"
"${SRC}/Quake/chase.c"
"${SRC}/Quake/cl_demo.c"
"${SRC}/Quake/cl_input.c"
"${SRC}/Quake/cl_main.c"
"${SRC}/Quake/cl_parse.c"
"${SRC}/Quake/cl_tent.c"
"${SRC}/Quake/cmd.c"
"${SRC}/Quake/common.c"
"${SRC}/Quake/console.c"
"${SRC}/Quake/crc.c"
"${SRC}/Quake/cvar.c"
"${SRC}/Quake/gl_draw.c"
"${SRC}/Quake/gl_fog.c"
"${SRC}/Quake/gl_mesh.c"
"${SRC}/Quake/gl_model.c"
"${SRC}/Quake/gl_refrag.c"
"${SRC}/Quake/gl_rlight.c"
"${SRC}/Quake/gl_rmain.c"
"${SRC}/Quake/gl_rmisc.c"
"${SRC}/Quake/gl_screen.c"
"${SRC}/Quake/gl_shaders.c"
"${SRC}/Quake/gl_sky.c"
"${SRC}/Quake/gl_texmgr.c"
"${SRC}/Quake/gl_vidsdl.c"
"${SRC}/Quake/gl_warp.c"
"${SRC}/Quake/host_cmd.c"
"${SRC}/Quake/host.c"
"${SRC}/Quake/image.c"
"${SRC}/Quake/in_sdl.c"
"${SRC}/Quake/json.c"
"${SRC}/Quake/keys.c"
"${SRC}/Quake/main_sdl.c"
"${SRC}/Quake/mathlib.c"
"${SRC}/Quake/menu.c"
"${SRC}/Quake/miniz.c"
"${SRC}/Quake/net_bsd.c"
"${SRC}/Quake/net_dgrm.c"
"${SRC}/Quake/net_loop.c"
"${SRC}/Quake/net_main.c"
"${SRC}/Quake/pl_web.c"
"${SRC}/Quake/pr_cmds.c"
"${SRC}/Quake/pr_edict.c"
"${SRC}/Quake/pr_exec.c"
"${SRC}/Quake/r_alias.c"
"${SRC}/Quake/r_brush.c"
"${SRC}/Quake/r_part.c"
"${SRC}/Quake/r_sprite.c"
"${SRC}/Quake/r_world.c"
"${SRC}/Quake/sbar.c"
"${SRC}/Quake/snd_codec.c"
"${SRC}/Quake/snd_dma.c"
"${SRC}/Quake/snd_flac.c"
"${SRC}/Quake/snd_mem.c"
"${SRC}/Quake/snd_mikmod.c"
"${SRC}/Quake/snd_mix.c"
"${SRC}/Quake/snd_mp3tag.c"
"${SRC}/Quake/snd_opus.c"
"${SRC}/Quake/snd_sdl.c"
"${SRC}/Quake/snd_umx.c"
"${SRC}/Quake/snd_vorbis.c"
"${SRC}/Quake/snd_wave.c"
"${SRC}/Quake/snd_xmp.c"
"${SRC}/Quake/strlcat.c"
"${SRC}/Quake/strlcpy.c"
"${SRC}/Quake/sv_main.c"
"${SRC}/Quake/sv_move.c"
"${SRC}/Quake/sv_phys.c"
"${SRC}/Quake/sv_user.c"
"${SRC}/Quake/view.c"
"${SRC}/Quake/wad.c"
"${SRC}/Quake/world.c"
"${SRC}/Quake/zone.c"
)

if (WIN32) 
set(COMMON_SOURCES ${COMMON_SOURCES} "${SRC}/Quake/sys_sdl_win.c")
else()
set(COMMON_SOURCES ${COMMON_SOURCES} "${SRC}/Quake/sys_sdl_unix.c")
endif()

target_sources(wgQuake PRIVATE ${COMMON_SOURCES})

# Add the 'webgpu' target as a dependency of our wgQuake
target_link_libraries(wgQuake PRIVATE webgpu)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
target_copy_webgpu_binaries(wgQuake)

set_target_properties(wgQuake PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
	COMPILE_WARNING_AS_ERROR ON
	EXPORT_COMPILE_COMMANDS ON
)

if (XCODE)
	set_target_properties(wgQuake PROPERTIES
		XCODE_GENERATE_SCHEME ON
		XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
	)
endif()

set(COMMON_LIBRARIES "sdl2webgpu" "webgpu")

if (WIN32)
set(PLATFORM_LIBRARIES wsock32 winmm)
else()
set(PLATFORM_LIBRARIES "") 
endif()

# Options that are specific to Emscripten
if (EMSCRIPTEN)
	# Generate a full web page rather than a simple WebAssembly module
	set_target_properties(wgQuake PROPERTIES SUFFIX ".html")
	target_compile_options(wgQuake PRIVATE --use-port=sdl2)
	target_link_options(wgQuake PRIVATE -sUSE_SDL=2 -sUSE_WEBGPU -sASYNCIFY -sINITIAL_MEMORY=512MB -sSTACK_SIZE=4MB --preload-file=../id1)
	target_link_libraries(wgQuake PRIVATE ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES})
else()
	set(SDL_STATIC ON)
	set(SDL_SHARED OFF)
	set(SDL_RENDER_D3D OFF)
	set(SDL_RENDER_METAL OFF)
	# set(SDL_OPENGL OFF)
	set(SDL_VULKAN OFF)
	add_subdirectory(SDL)
	find_package(OpenGL REQUIRED)
	if (NOT WIN32)
		target_compile_options(wgQuake PRIVATE -fsanitize=address)
		target_link_options(wgQuake PRIVATE -fsanitize=address)
	endif()
	target_link_libraries(wgQuake PRIVATE ${COMMON_LIBRARIES} ${PLATFORM_LIBRARIES} ${OPENGL_LIBRARIES} SDL2::SDL2-static)
endif()

set_property(TARGET wgQuake PROPERTY COMPILE_WARNING_AS_ERROR OFF)

add_custom_command(TARGET wgQuake POST_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_if_different
	"${SRC}/Quake/quakespasm.pak"
	"$<TARGET_FILE_DIR:wgQuake>"
)

add_custom_command(TARGET wgQuake POST_BUILD COMMAND
	${CMAKE_COMMAND} -E copy_if_different
	"${SRC}/Quake/ironwail.pak"
	"$<TARGET_FILE_DIR:wgQuake>"
)

