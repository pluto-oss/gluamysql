workspace "gluamysql"

	configurations { "Debug32", "Release32", "Debug64", "Release64" }

	language "C++"
	location "./proj"

	filter "configurations:*32"
		architecture "x86"

	filter "configurations:*64"
		architecture "x86_64"

	project "gluamysql"
		flags { "NoPCH", "NoImportLib"}

		symbols "On"
		vectorextensions "SSE"
		optimize "On"

		staticruntime "on"
		kind "SharedLib"

		targetdir "bin"

		includedirs {
			"gmod-module-base/include",
			"src",
			"mysql",
			"garrysmod_common",
			"luajit",
		}

		files {
			"src/**.cpp",
			"src/**.hpp",
			"src/**.h",

			"garrysmod_common/**.cpp",
			"garrysmod_common/**.hpp",
			"garrysmod_common/**.h",

			"luajit/**.hpp",
			"luajit/**.h",
		}

		defines {
			"GMMODULE",
			"_WINDOWS",
			"NDEBUG",
		}

		targetprefix "gmsv_"
		targetextension "_win32.dll"
		links "mariadbclient"

		if os.target() == "windows" then
			links { "ws2_32.lib", "shlwapi.lib" }
		elseif os.target() == "macosx" or os.target() == "linux" then
			links { "pthread", "dl" }
		end

		filter "configurations:*64"
			libdirs "lib64"

		filter "configurations:*32"
			libdirs "lib"