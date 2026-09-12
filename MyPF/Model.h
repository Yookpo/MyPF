#pragma once
#include <vector>
#include <utility>

namespace My
{
	class Mesh;
	class Material;

	// 모델을 구성하는 하나의 렌더링 단위
	struct ModelPart
	{
		const Mesh* mesh{ nullptr };
		Material*	material{ nullptr };
	};

	class Model
	{
	public:
		explicit Model(std::vector<ModelPart> parts)
			: m_parts(std::move(parts))
		{
		}

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;

		const std::vector<ModelPart>& GetParts() const { return m_parts; }

	private:
		std::vector<ModelPart> m_parts;
	};
} // namespace My
