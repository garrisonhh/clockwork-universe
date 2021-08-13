workspace "sdl-iso-project"
	configurations { "debug", "release" }

include "../external/libghhgfx"
-- include "../external/libghh" -- included in libghhgfx TODO figure out why tf premake compiles this twice?
include "../external/glad"
include "../external/cJSON"

project "iso"
	kind "ConsoleApp"
	language "C"
	cdialect "c11"
	targetdir ".."

	enablewarnings "all"
	floatingpoint "Fast"
	buildoptions "-pedantic-errors"

	links {
		"m", "ghh", "ghhgfx", "glad", "cJSON", "SDL2"
	}

	includedirs {
		"../external/**/include/",
		"../external/"
	}

	files {
		"../src/**.c",
		"../src/**.h"
	}

	if os.target() == "windows" then
		makesettings "CC=gcc" -- mingw32-make sets CC=cc by default
	else
		links { "dl" }
	end

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"
