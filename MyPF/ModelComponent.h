#pragma once

namespace My
{
	class Model;

	class ModelComponent
	{
	public:
		ModelComponent()
			: m_model(nullptr) {}
		void		 SetModel(const Model* newModel) { m_model = newModel; }
		const Model* GetModel() const { return m_model; }
		bool		 HasModel() const { return m_model; }

	private:
		const Model* m_model; // 오브젝트가 사용할 모델
	};
} // namespace My
