#pragma once
#include <string>
#include <vector>
#include <memory>
#include "GameObject.h"

namespace My
{
	class Scene
	{
	public:
		GameObject& CreateGameObject(const std::string& name);
		const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return m_gameObjects; }

	private:
		std::vector<std::unique_ptr<GameObject>> m_gameObjects;
	};

}




