// vcpkg install assimp:x64-windows
// Preprocessor definitions에 NOMINMAX 추가
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

#include "ModelLoader.h"
#include "ModelData.h"
#include <filesystem>

namespace My
{
	using namespace DirectX::SimpleMath;

	bool ModelLoader::Load(const std::string& filePath, ModelData& outModelData)
	{
		// 같은 ModelData 재사용 시 이전 모델 데이터 남아있으면 안된다
		outModelData.meshes.clear();

		if (filePath.empty())
		{
			return false;
		}

		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(filePath,
			aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);

		if (!pScene || !pScene->mRootNode)
		{
			const auto error = importer.GetErrorString();
			OutputDebugStringA(error);
			return false;
		}


		return true;
	}

}

