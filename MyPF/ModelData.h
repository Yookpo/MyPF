#pragma once
#include "MeshData.h"
#include <vector>
#include <string>

namespace My
{
	// Assimp의 Mesh 하나, 즉 SubMesh 하나를 표현
	struct ImportedMeshData
	{
		MeshData	meshData;
		std::string albedoTexturePath;
	};

	// 여러 subMesh 보관
	struct ModelData
	{
		std::vector<ImportedMeshData> meshes;
	};

} // namespace My
