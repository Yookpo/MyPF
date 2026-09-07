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

		GameObject& CreatePointLightObject(const std::string& name);
		std::size_t GatherPointLights(PointLight* outPointLights, std::size_t capacity) const;

	private:
		std::vector<std::unique_ptr<GameObject>> m_gameObjects;
	};

} // namespace My
