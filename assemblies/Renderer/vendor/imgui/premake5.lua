project "ImGUI"
	kind "StaticLib"
	language "C++"
	staticruntime "On"
	warnings "off"

    targetdir("%{build_dir}/bin/%{output_dir}/%{prj.name}")
    objdir("%{build_dir}/bin-int/%{output_dir}/%{prj.name}")

	files
	{
		"*.cpp",

		"backends/imgui_impl_glfw.cpp",
		"backends/imgui_impl_opengl3.cpp"
	}

	includedirs {
		"./",
		"../glfw/include"
	}

	filter "system:windows"
		systemversion "latest"
		defines 
		{ 
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "speed"

    filter "configurations:Dist"
		runtime "Release"
		optimize "speed"
        symbols "off"