-- VR main build file
-- Rodolphe VALICON
-- 2025

workspace "VR"
    architecture "x64"

    startproject "Renderer"

    configurations {
        "Debug",    -- For debugging (non-optimized bytecode with symbols)
        "Release",  -- For pre-release tests (optimized bytecode with symbols)
        "Dist"      -- For productions builds (optimized bytecode without symbols)
    }

    -- Workspace variables
    build_dir = "%{wks.location}/build"
    output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    Includes = {}

    -- Assemblies
    group "Dependencies"
        include "assemblies/Renderer/vendor/glfw"
        include "assemblies/Renderer/vendor/imgui"
    group ""
        include "assemblies/Renderer"