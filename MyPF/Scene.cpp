#include "Scene.h"

namespace My
{

	GameObject& Scene::CreateGameObject(const std::string& name)
	{
		m_gameObjects.emplace_back(std::make_unique<GameObject>(name));

		return *m_gameObjects.back();
	}

	PointLight& Scene::CreatePointLight()
	{
		m_pointLights.emplace_back();

		return m_pointLights.back();
	}

	bool Scene::SetPointLightEnabled(std::size_t index, bool isEnabled)
	{
		if (index >= m_pointLights.size())
		{
			return false;
		}

		m_pointLights[index].isEnabled = isEnabled;

		return true;
	}

	void Scene::SetAllPointLightsEnabled(bool isEnabled)
	{
		for (auto& pointLight : m_pointLights)
		{
			pointLight.isEnabled = isEnabled;
		}
	}

} // namespace My
