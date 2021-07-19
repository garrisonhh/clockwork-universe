project "glad"
	kind "StaticLib"
	language "C"
	cdialect "c99"

	targetdir ("bin/%{prj.name}")
	objdir ("obj/%{prj.name}")

	files {
		"./include/**.h",
		"./src/**.c"
	}

	includedirs {
		"./include/"
	}

	warnings "Off"

	if os.target() == "windows" then
		makesettings { "CC=gcc" }
	else
		links { "dl" }
	end

	filter "configurations:debug"
		symbols "On"

	filter "configurations:release"
		optimize "On"
