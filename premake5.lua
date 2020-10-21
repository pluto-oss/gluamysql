workspace "gluamysql"

	configurations { "Debug32", "Release32", "Debug64", "Release64" }

	language "C++"
	location "./proj"

	filter "configurations:*32"
		architecture "x86"

	filter "configurations:*64"
		architecture "x86_64"

	project "gluamysql"
		flags { "NoPCH", "NoImportLib" }

		symbols "On"
		vectorextensions "SSE"

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
		
		links "mariadbclient"

		targetprefix "gmsv_"

		filter "system:windows"
			links { "ws2_32.lib", "shlwapi.lib" }

		filter "system:linux"
			linkoptions { "-Wl,--as-needed" }
			links { "pthread", "dl", "z" }

		filter "configurations:*64"
			libdirs "lib64"

		filter "configurations:*32"
			libdirs "lib"

		filter "configurations:Debug*"
			optimize "Debug"

		filter "configurations:Release*"
			optimize "On"


		filter { "system:windows", "configurations:*32" }
			targetextension "_win32.dll"
		
		filter { "system:windows", "configurations:*64" }
			targetextension "_win64.dll"
		
		filter { "system:linux", "configurations:*32" }
			targetextension "_linux32.dll"
		
		filter { "system:linux", "configurations:*64" }
			targetextension "_linux64.dll"