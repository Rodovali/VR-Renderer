// VR Renderer - Material Registry
// Rodolphe VALICON
// 2025

#pragma once

#include "renderer/Material.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace vr {

	class MaterialRegistry {
	public:
		static void registerMaterial(std::string name, std::shared_ptr<Material> material);
		static std::shared_ptr<Material> getMaterial(std::string name);

		static void loadMaterials();
		static void reloadMaterials();

	private:
		static std::unordered_map<std::string, std::shared_ptr<Material>> s_materials;
	};

}