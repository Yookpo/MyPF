#pragma once
#include <string>
#include "ModelData.h"

struct aiMesh;
struct aiMaterial;

namespace My
{
	class ModelLoader
	{
	public:
		static bool Load(const std::string& filePath, ModelData& outModelData);

	private:
		static bool ProcessMesh(const aiMesh* sourceMesh, ImportedMeshData& outImportedMesh);
		static std::string ProcessMaterial(const aiMaterial* sourceMaterial, const std::string& modelDirectory);
	};
}


