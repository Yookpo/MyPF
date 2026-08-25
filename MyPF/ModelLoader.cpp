// vcpkg install assimp:x64-windows
// Preprocessor definitions에 NOMINMAX 추가
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>

#include "ModelLoader.h"
#include "ModelData.h"
#include <Windows.h>

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

		// 모델 파일 전체 불러오기
		// face를 삼각형으로 변환, Left-Handed로 변환, Normal이 없는 모델은 Smooth Normal 생성
		const aiScene* pScene = importer.ReadFile(filePath,
			aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_GenSmoothNormals);

		// Scene 유효성 검사
		if (!pScene || (pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !pScene->mRootNode)
		{
			const auto error = importer.GetErrorString();
			OutputDebugStringA(error);
			return false;
		}

		// Scene에 mesh가 있는지 검사
		if (!pScene->HasMeshes())
		{
			return false;
		}

		// 모델에 포함된 모든 mesh 처리
		for (UINT i = 0; i < pScene->mNumMeshes; i++)
		{
			ImportedMeshData importedMesh{};

			if (!ProcessMesh(pScene->mMeshes[i], importedMesh))
			{
				outModelData.meshes.clear();
				return false;
			}

			outModelData.meshes.push_back(std::move(importedMesh));

		}

		return !outModelData.meshes.empty();
	}

	bool ModelLoader::ProcessMesh(const aiMesh* sourceMesh, ImportedMeshData& outImportedMesh)
	{
		outImportedMesh = ImportedMeshData{};

		// Position/Face/Normal 검사
		if (!sourceMesh || !sourceMesh->HasPositions() || !sourceMesh->HasFaces() || !sourceMesh->HasNormals())
		{
			return false;
		}

		// Assimp Vertex -> Vertex 변환
		MeshData meshData;

		const bool hasTexCoords = sourceMesh->HasTextureCoords(0);

		// Walk through each of the mesh's vertices
		for (UINT i = 0; i < sourceMesh->mNumVertices; i++)
		{
			Vertex v{};

			v.position.x = sourceMesh->mVertices[i].x;
			v.position.y = sourceMesh->mVertices[i].y;
			v.position.z = sourceMesh->mVertices[i].z;

			v.normal.x = sourceMesh->mNormals[i].x;
			v.normal.y = sourceMesh->mNormals[i].y;
			v.normal.z = sourceMesh->mNormals[i].z;

			v.normal.Normalize();

			if (hasTexCoords)
			{
				v.uv.x = static_cast<float>(sourceMesh->mTextureCoords[0][i].x);
				v.uv.y = static_cast<float>(sourceMesh->mTextureCoords[0][i].y);
			}

			v.color = Vector3{ 1.0f };

			meshData.vertices.push_back(v);
		}

		for (UINT i = 0; i < sourceMesh->mNumFaces; i++)
		{
			const aiFace& face = sourceMesh->mFaces[i];

			// 삼각형 판별
			if (face.mNumIndices != 3)
			{
				return false;
			}

			for (UINT j = 0; j < face.mNumIndices; j++)
			{
				meshData.indices.push_back(face.mIndices[j]);
			}
		}

		outImportedMesh.meshData = meshData;

		return true;
	}

}

