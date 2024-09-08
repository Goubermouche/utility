project "utility"
    kind "StaticLib" -- or "SharedLib" if it's a dynamic library
    language "C++"
    targetdir "output/bin/utility"
    objdir "output/obj/utility"

    files { "utility/**.cpp", "utility/**.h" }
    includedirs { "utility/" }

