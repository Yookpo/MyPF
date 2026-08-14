#include "Scene.h"

namespace My
{
	GameObject& Scene::CreateGameObject(const std::string& name)
	{
		m_gameObjects.emplace_back(std::make_unique<GameObject>(name));

		return *m_gameObjects.back();
	}
}




