#pragma once

#include <array>

namespace My
{
	class Camera;
	class FirstPersonCameraController;
	class GameObject;
	class Scene;
	struct DirectionalLight;

	class EditorUI
	{
	public:
		void Initialize(Scene&			 scene,
			Camera&						 camera,
			FirstPersonCameraController& cameraController,
			DirectionalLight&			 directionalLight,
			std::array<float, 4>&		 backgroundColor);

		bool Draw(float screenHeight);

		void SetSelectedObject(GameObject* selectedObject) { m_selectedObject = selectedObject; }

	private:
		bool DrawEditorPanel();

		void DrawEnvironmentPanel();
		void DrawDirectionalLightPanel();
		void DrawEditorCameraPanel();
		void DrawSceneHierarchyPanel();
		void DrawSelectedObjectInspector();
		void DrawTransformInspector(GameObject& gameObject);
		void DrawPointLightInspector(GameObject& gameObject);
		void DrawMaterialInspector(GameObject& gameObject);

	private:
		Scene*						 m_scene{ nullptr };
		Camera*						 m_camera{ nullptr };
		FirstPersonCameraController* m_cameraController{ nullptr };
		DirectionalLight*			 m_directionalLight{ nullptr };
		std::array<float, 4>*		 m_backgroundColor{ nullptr };
		GameObject*					 m_selectedObject{ nullptr };
	};
} // namespace My
