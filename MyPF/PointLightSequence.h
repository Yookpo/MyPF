#pragma once
#include <cstddef>
#include <vector>

namespace My
{
	class Material;
	class Scene;

	class PointLightSequence
	{
		struct SequenceEntry
		{
			std::size_t index{ 0 }; // Scene의 Point Light 인덱스
			Material*	emissiveMat{ nullptr };
			float		emissiveIntensity{ 0.0f };
		};

	public:
		void Initialize(Scene& scene);
		void Start(bool shouldEnable);
		void Update(float deltaTime);
		bool IsPlaying() const { return m_enabledLightCount != m_targetEnabledLightCount; }

		bool AddSequenceEntry(std::size_t pointLightIndex, Material* mat, float intensity);

	private:
		Scene*		m_scene{ nullptr };
		float		m_stepInterval{ 1.4f };			// 점등 간격
		float		m_elapsedTime{ 0.0f };			// 누적 시간
		std::size_t m_enabledLightCount{ 0 };		// 현재 실제로 켜진 조명 개수
		std::size_t m_targetEnabledLightCount{ 0 }; // 최종적으로 켜져야 하는 개수

		std::vector<SequenceEntry> m_pointLightSequenceEntries;
	};
} // namespace My
