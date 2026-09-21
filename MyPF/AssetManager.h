#pragma once
#include <unordered_map>
#include <utility>
#include <functional>
#include <memory>
#include <string>
#include "TextureType.h"

namespace My
{
	using TextureKey = std::pair<std::string, TextureType>;

	class GraphicsResourceManager;
	class Texture;
	class Mesh;
	class Material;
	struct MeshData;
	class Model;

	class AssetManager
	{
		struct TextureKeyHash
		{
			size_t operator()(const TextureKey& texKey) const
			{
				// 경로와, 용도의 해시를 각각 구함
				size_t seed = std::hash<std::string>{}(texKey.first);	   // 경로
				size_t typeHash = std::hash<TextureType>{}(texKey.second); // 용도

				// 해시를 합친다
				seed ^= typeHash + 0x9e3779b9 + (seed << 6) + (seed >> 2);

				return seed;
			}
		};

	public:
		AssetManager();
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;
		~AssetManager();

		bool		   Initialize(GraphicsResourceManager&);
		const Texture* LoadTexture(const std::string&, TextureType);
		const Mesh*	   CreateMesh(const std::string&, const MeshData&);
		Material*	   CreateMaterial(const std::string&);
		const Model*   LoadModel(const std::string&);

	private:
		GraphicsResourceManager*												 m_resourceManager;
		std::unordered_map<TextureKey, std::unique_ptr<Texture>, TextureKeyHash> m_textures;
		std::unordered_map<std::string, std::unique_ptr<Mesh>>					 m_meshes;
		std::unordered_map<std::string, std::unique_ptr<Material>>				 m_materials;
		std::unordered_map<std::string, std::unique_ptr<Model>>					 m_models;
	};

} // namespace My
