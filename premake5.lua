workspace "gluamysql"

	configurations { "Debug32", "Release32", "Debug64", "Release64" }

	language "C++"
	location "./proj"

	filter "configurations:*32"
		architecture "x86"

	filter "configurations:*64"
		architecture "x86_64"

	project "gm_gluamysql"
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
		}

		files {
			"src/**.cpp",
			"src/**.hpp",
			"src/**.h",
		}

		defines {
			"GMMODULE",
			"_WINDOWS",
			"NDEBUG",
		}

		targetprefix ""
		targetextension ".dll"
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