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
	class Material;

	class AssetManager
	{
	public:
		AssetManager();
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		~AssetManager();

		bool		   Initialize(GraphicsResourceManager&);
		const Texture* LoadTexture(const std::string&);
		const Mesh*	   CreateMesh(const std::string&, const MeshData&);
		Material*	   CreateMaterial(const std::string&);

	private:
		GraphicsResourceManager*								   m_resourceManager;
		std::unordered_map<std::string, std::unique_ptr<Texture>>  m_textures;
		std::unordered_map<std::string, std::unique_ptr<Mesh>>	   m_meshes;
		std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;

		// Model 실제 소유
		// Mesh와 Material을 비소유 포인터로 참조
	};

} // namespace My
