#pragma once
#include <cstddef>
#include <vector>

namespace My
{
	class Material;
	class GameObject;

	class PointLightSequence
	{
		struct SequenceEntry
		{
			GameObject* pointLightObject{ nullptr };
			Material*	emissiveMat{ nullptr };
			float		emissiveIntensity{ 0.0f };
		};

	public:
		void Initialize();
		void Start(bool shouldEnable);
		void Update(float deltaTime);
		bool IsPlaying() const { return m_enabledLightCount != m_targetEnabledLightCount; }

		bool AddSequenceEntry(GameObject& pointLightObject, Material* mat, float intensity);

	private:
		float		m_stepInterval{ 1.4f };			// 점등 간격
		float		m_elapsedTime{ 0.0f };			// 경과 시간
		std::size_t m_enabledLightCount{ 0 };		// 현재 실제로 켜진 조명 개수
		std::size_t m_targetEnabledLightCount{ 0 }; // 최종적으로 켜져야 하는 개수

		std::vector<SequenceEntry> m_pointLightSequenceEntries;
	};
} // namespace My
