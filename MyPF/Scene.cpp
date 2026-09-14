#include "Scene.h"

namespace My
{

	GameObject& Scene::CreateGameObject(const std::string& name)
	{
		m_gameObjects.emplace_back(std::make_unique<GameObject>(name));

		return *m_gameObjects.back();
	}

	GameObject& Scene::CreatePointLightObject(const std::string& name)
	{
		GameObject& pointLightObject = this->CreateGameObject(name);
		pointLightObject.AddPointLightComponent();
		return pointLightObject;
	}

	std::size_t Scene::GatherPointLights(PointLight* outPointLights, std::size_t capacity) const
	{
		if (!outPointLights || capacity == 0)
		{
			return 0;
		}

		// 존재하는 조명 개수
		std::size_t gatheredLightCount{ 0 };

		for (std::size_t i = 0; i < m_gameObjects.size(); i++)
		{
			if (gatheredLightCount >= capacity)
			{
				break;
			}

			if (!m_gameObjects[i]->HasPointLightComponent())
			{
				continue;
			}

			const Transform&		   transform = m_gameObjects[i]->GetTransform();
			const PointLightComponent& pointLightComponent = m_gameObjects[i]->GetPointLightComponent();

			outPointLights[gatheredLightCount].position = transform.GetPosition();
			outPointLights[gatheredLightCount].color = pointLightComponent.GetColor();
			outPointLights[gatheredLightCount].range = pointLightComponent.GetRange();
			outPointLights[gatheredLightCount].intensity = pointLightComponent.GetIntensity();
			outPointLights[gatheredLightCount].isEnabled = pointLightComponent.IsEnabled();

			gatheredLightCount += 1;
		}

		return gatheredLightCount;
	}

	std::vector<DirectX::BoundingBox> Scene::GatherBoxColliders() const
	{
		// Scene의 GameObjects를 순회하며
		// HasBoxCollisionComponent()가 true인 것만
		// BoundingBox(Vector3(0),Vector3(0.5))를 GetWorldMatrix()로 변환해서 벡터에 담기
		std::vector<DirectX::BoundingBox> boxColliders;

		for (const auto& gameObject : m_gameObjects)
		{
			if (!gameObject->HasBoxCollisionComponent())
			{
				continue;
			}

			DirectX::BoundingBox box(Vector3(0.0f), Vector3(0.5f));
			box.Transform(box, gameObject->GetTransform().GetWorldMatrix());

			boxColliders.push_back(box);
		}

		return boxColliders;
	}

} // namespace My
