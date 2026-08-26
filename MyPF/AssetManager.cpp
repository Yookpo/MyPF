#include "AssetManager.h"
#include "GraphicsResourceManager.h"
#include "Texture.h"
#include "Mesh.h"
#include "Material.h"
#include "ModelData.h"
#include "ModelLoader.h"
#include "Model.h"

namespace My
{
	AssetManager::AssetManager()
		: m_resourceManager{ nullptr }
	{
	}

	AssetManager::~AssetManager() = default;

	bool AssetManager::Initialize(GraphicsResourceManager& resourceManager)
	{
		if (m_resourceManager)
		{
			return false;
		}

		m_resourceManager = &resourceManager;

		return true;
	}

	const Texture* AssetManager::LoadTexture(const std::string& path)
	{
		if (!m_resourceManager || path.empty())
		{
			return nullptr;
		}

		// 캐시 검색 -> 캐시 히트시 기존 Texture 반환
		auto iter = m_textures.find(path);
		if (iter != m_textures.end())
		{
			return (iter->second).get();
		}

		auto texture = std::make_unique<Texture>();
		if (!texture->Initialize(*m_resourceManager, path))
		{
			OutputDebugStringW(L"AssetManager::LoadTexture() failed");
			return nullptr;
		}

		const Texture* loadedTex = texture.get();
		m_textures.emplace(path, std::move(texture));

		return loadedTex;
	}

	const Mesh* AssetManager::CreateMesh(const std::string& meshName, const MeshData& meshData)
	{
		if (!m_resourceManager || meshName.empty())
		{
			return nullptr;
		}

		// 캐시 검색 -> 캐시 히트시 기존 mesh 반환
		auto iter = m_meshes.find(meshName);
		if (iter != m_meshes.end())
		{
			return (iter->second).get();
		}

		auto mesh = std::make_unique<Mesh>();
		if (!mesh->Initialize(*m_resourceManager, meshData))
		{
			OutputDebugStringW(L"AssetManager::CreateMesh() failed");
			return nullptr;
		}

		const Mesh* loadedMesh = mesh.get();
		m_meshes.emplace(meshName, std::move(mesh));

		return loadedMesh;
	}

	Material* AssetManager::CreateMaterial(const std::string& key)
	{
		if (!m_resourceManager || key.empty())
		{
			return nullptr;
		}

		// 캐시 검색 -> 캐시 히트시 기존 Material 반환
		auto iter = m_materials.find(key);
		if (iter != m_materials.end())
		{
			return (iter->second).get();
		}

		auto	  material = std::make_unique<Material>();
		Material* createdMaterial = material.get();

		m_materials.emplace(key, std::move(material));

		return createdMaterial;
	}

	const Model* AssetManager::LoadModel(const std::string& filePath)
	{
		if (!m_resourceManager || filePath.empty())
		{
			return nullptr;
		}

		// 캐시 검색 -> 캐시 히트시 기존 model 반환
		auto iter = m_models.find(filePath);
		if (iter != m_models.end())
		{
			return (iter->second).get();
		}

		ModelData modelData{};

		if (!ModelLoader::Load(filePath, modelData))
		{
			return nullptr;
		}

		std::vector<ModelPart> parts;
		parts.reserve(modelData.meshes.size());

		for (std::size_t i = 0; i < modelData.meshes.size(); i++)
		{
			const ImportedMeshData& importedMesh = modelData.meshes[i];

			const std::string meshKey = filePath + "#mesh_" + std::to_string(i);
			const std::string materialKey = filePath + "#material_" + std::to_string(i);

			const Mesh* createdMesh = CreateMesh(meshKey, importedMesh.meshData);
			Material*	createdMaterial = CreateMaterial(materialKey);

			if (!createdMesh || !createdMaterial)
			{
				return nullptr;
			}

			if (!importedMesh.albedoTexturePath.empty())
			{
				const Texture* albedoTexture = LoadTexture(importedMesh.albedoTexturePath);

				if (!albedoTexture)
				{
					return nullptr;
				}

				createdMaterial->SetAlbedoTexture(albedoTexture);
			}

			ModelPart part{};
			part.mesh = createdMesh;
			part.material = createdMaterial;

			parts.push_back(part);
		}

		auto		 model = std::make_unique<Model>(std::move(parts));
		const Model* loadedModel = model.get();

		m_models.emplace(filePath, std::move(model));

		return loadedModel;
	}

} // namespace My
