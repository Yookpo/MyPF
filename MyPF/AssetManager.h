#pragma once
#include <unordered_map>
#include <memory>
#include <string>

namespace My
{
	class GraphicsResourceManager;
	class Texture;
	class Mesh;
	struct MeshData;

	class AssetManager
	{
	public:
		AssetManager();
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		~AssetManager();


		bool Initialize(GraphicsResourceManager&);
		const Texture* LoadTexture(const std::string&);
		const Mesh* CreateMesh(const std::string&, const MeshData&);

	private:
		GraphicsResourceManager* m_resourceManager;
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
		std::unordered_map<std::string, std::unique_ptr<Mesh>> m_meshes;
	};

}



