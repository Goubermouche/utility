workspace "utility"
    configurations { "Release", "Debug" }
    startproject "tests"

    architecture "x64"
    language "C++"
    cppdialect "C++20"
    warnings "Extra"

    flags {
        "MultiProcessorCompile"
    }

    -- buildoptions { "-fsanitize=address" }
    -- linkoptions { "-fsanitize=address" }
    -- debugformat "C7"

    filter "toolset:gcc or clang"
        buildoptions { "-Wno-comment", "-Wno-missing-field-initializers" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "Off"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        runtime "Debug"
        defines { "DEBUG", "_DEBUG" }

project "utility"
    kind "Utility"
    location "source"

    targetdir "bin/%{cfg.buildcfg}"

    files { "utility/**.cpp", "utility/**.h" }
    includedirs { "utility/" }

    targetdir "output/utility/bin/%{cfg.buildcfg}"
    objdir "output/utility/obj/%{cfg.buildcfg}"

project "tests"
    kind "ConsoleApp"
    location "tests"

    targetdir "bin/%{cfg.buildcfg}"
    files { "tests/**.cpp" }
    includedirs { "../utility" }
    links { "utility" }

    targetdir "output/tests/bin/%{cfg.buildcfg}"
    objdir "output/tests/obj/%{cfg.buildcfg}"