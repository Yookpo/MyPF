#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cstddef>
#include "GameObject.h"
#include "PointLight.h"

namespace My
{
	class Scene
	{
	public:
		GameObject&										CreateGameObject(const std::string& name);
		const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }

		PointLight&					   CreatePointLight();
		const std::vector<PointLight>& GetPointLights() const { return m_pointLights; }
		bool						   SetPointLightEnabled(std::size_t index, bool isEnabled);
		void						   SetAllPointLightsEnabled(bool isEnabled);
		std::size_t					   GetPointLightCount() const { return m_pointLights.size(); }

	private:
		std::vector<std::unique_ptr<GameObject>> m_gameObjects;
		std::vector<PointLight>					 m_pointLights;
	};

} // namespace My
