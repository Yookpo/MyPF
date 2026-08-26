#pragma once
#include <unordered_map>
#include <memory>
#include <string>

namespace My
{
	class GraphicsResourceManager;
	class Texture;
	class Mesh;
	class Material;
	struct MeshData;
	class Model;

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
		const Model*   LoadModel(const std::string&);

	private:
		GraphicsResourceManager*								   m_resourceManager;
		std::unordered_map<std::string, std::unique_ptr<Texture>>  m_textures;
		std::unordered_map<std::string, std::unique_ptr<Mesh>>	   m_meshes;
		std::unordered_map<std::string, std::unique_ptr<Material>> m_materials;
		std::unordered_map<std::string, std::unique_ptr<Model>>	   m_models;
	};

} // namespace My
