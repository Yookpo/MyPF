#include "AssetManager.h"
#include "GraphicsResourceManager.h"
#include "Texture.h"
#include "Mesh.h"

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

}

