workspace "sdl-iso-project"
	configurations { "debug", "release" }

include "../external/ghh-lib"
include "../external/glad"
include "../external/cJSON"

project "iso"
	if os.target() == "windows" then
		makesettings "CC=gcc" -- mingw32-make sets CC=cc by default
	else
		links { "dl" }
	end

	kind "ConsoleApp"
	language "C"
	cdialect "c99"
	targetdir ".."

	enablewarnings { "all" }
	buildoptions { "-pedantic-errors" }
	floatingpoint "Fast"

	links {
		"m", "ghh", "glad", "cJSON", "SDL2"
	}

	includedirs {
		"../external/**/include/",
		"../external/"
	}

	files {
		"../src/**.c",
		"../src/**.h"
	}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:release"
		defines { "NDEBUG" }
		optimize "On"
