#pragma once
#include <cstddef>

namespace My
{
	class Scene;

	class PointLightSequence
	{
	public:
		void Initialize(Scene& scene);
		void Start(bool shouldEnable);
		void Update(float deltaTime);
		bool IsPlaying() const { return m_enabledLightCount != m_targetEnabledLightCount; }

	private:
		Scene*		m_scene{ nullptr };
		float		m_stepInterval{ 1.4f };			// 점등 간격
		float		m_elapsedTime{ 0.0f };			// 누적 시간
		std::size_t m_enabledLightCount{ 0 };		// 현재 실제로 켜진 조명 개수
		std::size_t m_targetEnabledLightCount{ 0 }; // 최종적으로 켜져야 하는 개수
	};
} // namespace My
