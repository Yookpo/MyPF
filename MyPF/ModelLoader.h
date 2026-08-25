#pragma once
#include <string>
#include "ModelData.h"

struct aiScene;
struct aiMesh;

namespace My
{
	class ModelLoader
	{
	public:
		static bool Load(const std::string& filePath, ModelData& outModelData);

	private:
		static bool ProcessMesh(const aiMesh* sourceMesh, ImportedMeshData& outImportedMesh);
	};
}


