workspace("borepack")
configurations({ "Debug", "Release" })
architecture("x64")

project("borepack")
kind("WindowedApp")
language("C++")
targetdir("bin/%{cfg.buildcfg}")

-- Common include directories
includedirs({ "deps/glad/include", "deps/glm" })

-- Common files
files({
	"src/**.h",
	"src/**.cpp",
	"deps/glad/src/glad.cpp",
})

filter("configurations:Debug")
defines({ "DEBUG" })
symbols("On")

filter("configurations:Release")
defines({ "NDEBUG" })
optimize("On")

-- Linux-specific setup
filter("action:gmake")
defines({ "LINUX" })
buildoptions({ "-std=c++20", "-g", "-Wall", "-Wformat", "`sdl2-config --cflags`" })
links({ "GL", "SDL2" })

-- Windows-specific setup for Visual Studio
-- probably will dynamically link SDL2 in the future
filter("action:vs2022")
defines({ "WINDOWS" })
buildoptions({ "/std:c++20" })
links({ "opengl32.lib", "glu32.lib", "SDL2.lib", "SDL2main.lib" })
includedirs({ "deps/SDL2/include", "src" })
libdirs({ "deps/SDL2/lib" })

-- Reset filters to avoid affecting other projects
filter({})
