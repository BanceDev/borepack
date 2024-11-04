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
buildoptions({ "-std=c++20", "-g", "-Wall", "-Wformat" })
links({ "GL", "glfw" })
linkoptions({ "`pkg-config --static --libs glfw3`" })
includedirs({ "`pkg-config --cflags glfw3`" })

-- Windows-specific setup for Visual Studio
filter("action:vs2022")
defines({ "WINDOWS" })
buildoptions({ "/std:c++20" })
links({ "opengl32.lib", "glfw3.lib" })
includedirs({ "deps/glfw/include", "src" })
libdirs({ "deps/glfw/lib" })

-- Reset filters to avoid affecting other projects
filter({})
