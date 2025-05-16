-- VR - Renderer build file
-- Rodolphe VALICON
-- 2024

project "Renderer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    targetdir("%{build_dir}/bin/%{output_dir}/%{prj.name}")
    objdir("%{build_dir}/bin-int/%{output_dir}/%{prj.name}")

    files {
        "src/**.h",
        "src/**.cpp",
        "src/**.inl",

        -- Internal dependencies
        "vendor/glad/src/**.c",
        "vendor/mikktspace/src/**.c",
    }

    includedirs {
        "src"
    }

    externalincludedirs {
        "vendor/glfw/include",
        "vendor/glad/include",
        "vendor/glm/include",
        "vendor/imgui",
        "vendor/stb",
        "vendor/json",
        "vendor/mikktspace/include",
    }
    externalwarnings "Off"

    links {
        "GLFW",
        "ImGUI",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        runtime "Release"
        symbols "on"
        optimize "on"

    filter "configurations:Dist"
        runtime "Release"
        symbols "off"
        optimize "on"

