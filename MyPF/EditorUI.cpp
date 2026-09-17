#include "EditorUI.h"

#include "Camera.h"
#include "DirectionalLight.h"
#include "FirstPersonCameraController.h"
#include "GameObject.h"
#include "ImGui/imgui.h"
#include "Material.h"
#include "MeshComponent.h"
#include "PointLightComponent.h"
#include "PostProcessSettings.h"
#include "Scene.h"
#include "Transform.h"

namespace My
{
	namespace
	{
		constexpr float kEditorPanelWidth = 360.0f;
	}

	void EditorUI::Initialize(Scene& scene, Camera& camera, FirstPersonCameraController& cameraController,
		DirectionalLight& directionalLight, std::array<float, 4>& backgroundColor,
		PostProcessSettings& postProcessSettings)
	{
		m_scene = &scene;
		m_camera = &camera;
		m_cameraController = &cameraController;
		m_directionalLight = &directionalLight;
		m_backgroundColor = &backgroundColor;
		m_postProcessSettings = &postProcessSettings;
	}

	bool EditorUI::Draw(float screenHeight)
	{
		if (m_scene == nullptr || m_camera == nullptr || m_cameraController == nullptr || m_directionalLight == nullptr
			|| m_backgroundColor == nullptr || m_postProcessSettings == nullptr)
		{
			return false;
		}

		const float				   initialHeight = screenHeight > 32.0f ? screenHeight - 32.0f : 688.0f;
		constexpr ImGuiWindowFlags panelFlags = ImGuiWindowFlags_None;

		ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(kEditorPanelWidth, initialHeight), ImGuiCond_FirstUseEver);

		bool playRequested = false;
		if (ImGui::Begin("Editor Panel", nullptr, panelFlags))
		{
			playRequested = DrawEditorPanel();
		}
		ImGui::End();

		return playRequested;
	}

	bool EditorUI::DrawEditorPanel()
	{
		ImGui::TextUnformatted("Editor");

		const bool playRequested = ImGui::Button("Play", ImVec2(-1.0f, 0.0f));

		ImGui::Separator();
		DrawEnvironmentPanel();
		DrawDirectionalLightPanel();
		DrawPostProcessPanel();
		DrawEditorCameraPanel();

		ImGui::Separator();
		DrawSceneHierarchyPanel();

		ImGui::Separator();
		DrawSelectedObjectInspector();

		return playRequested;
	}

	void EditorUI::DrawEnvironmentPanel()
	{
		if (!ImGui::CollapsingHeader("Environment", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		ImGui::ColorEdit4("Background Color", m_backgroundColor->data());
	}

	void EditorUI::DrawDirectionalLightPanel()
	{
		if (!ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		Vector3 lightDirection = m_directionalLight->direction;
		if (ImGui::DragFloat3("Direction", &lightDirection.x, 0.01f, -1.0f, 1.0f)
			&& lightDirection.LengthSquared() > 0.00001f)
		{
			lightDirection.Normalize();
			m_directionalLight->direction = lightDirection;
		}

		Vector3 lightColor = m_directionalLight->color;
		if (ImGui::ColorEdit3("Directional Color", &lightColor.x))
		{
			m_directionalLight->color = lightColor;
		}

		float lightIntensity = m_directionalLight->intensity;
		if (ImGui::SliderFloat("Directional Intensity", &lightIntensity, 0.0f, 5.0f))
		{
			m_directionalLight->intensity = lightIntensity;
		}

		// 밤 골목에서 쓸모 있는 구간이 0.01~0.1이라 선형 슬라이더로는 조절이 안 된다.
		float ambientStrength = m_directionalLight->ambientStrength;
		if (ImGui::SliderFloat(
				"Ambient Strength", &ambientStrength, 0.001f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic))
		{
			m_directionalLight->ambientStrength = ambientStrength;
		}
	}

	void EditorUI::DrawPostProcessPanel()
	{
		if (!ImGui::CollapsingHeader("Post Process", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		// Order must match the ToneMapper enum values.
		const char* const toneMapperNames[] = { "Reinhard", "ACES" };
		int				  toneMapperIndex = static_cast<int>(m_postProcessSettings->toneMapper);
		if (ImGui::Combo("Tone Mapper", &toneMapperIndex, toneMapperNames, IM_ARRAYSIZE(toneMapperNames)))
		{
			m_postProcessSettings->toneMapper = static_cast<ToneMapper>(toneMapperIndex);
		}

		float exposure = m_postProcessSettings->exposure;
		if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic))
		{
			m_postProcessSettings->exposure = exposure;
		}

		ImGui::SeparatorText("Bloom");

		float threshold = m_postProcessSettings->threshold;
		if (ImGui::SliderFloat("Threshold", &threshold, 0.0f, 5.0f, "%.2f"))
		{
			m_postProcessSettings->threshold = threshold;
		}

		float bloomStrength = m_postProcessSettings->bloomStrength;
		if (ImGui::SliderFloat("Strength", &bloomStrength, 0.0f, 2.0f, "%.2f"))
		{
			m_postProcessSettings->bloomStrength = bloomStrength;
		}

		int iterations = m_postProcessSettings->bloomBlurIterations;
		if (ImGui::SliderInt("Blur Iterations", &iterations, 1, 10))
		{
			m_postProcessSettings->bloomBlurIterations = iterations;
		}

		// Order must match the PostProcessDebugView enum values.
		const char* const debugViewNames[] = { "Final", "Bright", "Blur X", "Blur" };
		int				  debugViewIndex = static_cast<int>(m_postProcessSettings->debugView);
		if (ImGui::Combo("Debug View", &debugViewIndex, debugViewNames, IM_ARRAYSIZE(debugViewNames)))
		{
			m_postProcessSettings->debugView = static_cast<PostProcessDebugView>(debugViewIndex);
		}
	}

	void EditorUI::DrawEditorCameraPanel()
	{
		if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			return;
		}

		float cameraSpeed = m_cameraController->GetMoveSpeed();
		if (ImGui::SliderFloat("Move Speed", &cameraSpeed, 0.1f, 10.0f))
		{
			m_cameraController->SetMoveSpeed(cameraSpeed);
		}

		float mouseSensitivity = m_cameraController->GetMouseSensitivity();
		if (ImGui::SliderFloat("Mouse Sensitivity", &mouseSensitivity, 0.01f, 1.0f))
		{
			m_cameraController->SetMouseSensitivity(mouseSensitivity);
		}

		Vector3 cameraPosition = m_camera->GetPosition();
		ImGui::Text("Position: X %.2f Y %.2f Z %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
		ImGui::Text("Rotation: Yaw %.2f Pitch %.2f", m_camera->GetYaw(), m_camera->GetPitch());

		float cameraFov = m_camera->GetFovAngleY();
		if (ImGui::SliderFloat("FOV", &cameraFov, 30.0f, 120.0f))
		{
			m_camera->SetFovAngleY(cameraFov);
		}
	}

	void EditorUI::DrawSceneHierarchyPanel()
	{
		ImGui::TextUnformatted("Scene Hierarchy");
		ImGui::BeginChild("Scene Hierarchy List", ImVec2(0.0f, 190.0f), true);

		const auto& sceneObjects = m_scene->GetGameObjects();
		for (const auto& object : sceneObjects)
		{
			GameObject* gameObject = object.get();
			const bool	isSelected = (m_selectedObject == gameObject);

			ImGui::PushID(gameObject);
			if (ImGui::Selectable(gameObject->GetName().c_str(), isSelected))
			{
				m_selectedObject = gameObject;
			}
			ImGui::PopID();
		}

		ImGui::EndChild();
	}

	void EditorUI::DrawSelectedObjectInspector()
	{
		ImGui::TextUnformatted("Inspector");
		ImGui::BeginChild("Inspector Content", ImVec2(0.0f, 0.0f), true);

		if (m_selectedObject == nullptr)
		{
			ImGui::TextUnformatted("No object selected");
			ImGui::EndChild();
			return;
		}

		ImGui::Text("Selected: %s", m_selectedObject->GetName().c_str());
		DrawTransformInspector(*m_selectedObject);
		DrawPointLightInspector(*m_selectedObject);
		DrawMaterialInspector(*m_selectedObject);

		ImGui::EndChild();
	}

	void EditorUI::DrawTransformInspector(GameObject& gameObject)
	{
		Transform& transform = gameObject.GetTransform();

		Vector3 position = transform.GetPosition();
		if (ImGui::DragFloat3("Position", &position.x, 0.01f, -50.0f, 50.0f))
		{
			transform.SetPosition(position);
		}

		Vector3 rotation = transform.GetRotation();
		if (ImGui::SliderFloat3("Rotation (Rad)", &rotation.x, -3.14f, 3.14f))
		{
			transform.SetRotation(rotation);
		}

		Vector3 scale = transform.GetScale();
		if (ImGui::SliderFloat3("Scale", &scale.x, 0.01f, 50.0f))
		{
			transform.SetScale(scale);
		}
	}

	void EditorUI::DrawPointLightInspector(GameObject& gameObject)
	{
		if (!gameObject.HasPointLightComponent())
		{
			return;
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Point Light");

		PointLightComponent& pointLight = gameObject.GetPointLightComponent();
		Vector3				 color = pointLight.GetColor();
		if (ImGui::ColorEdit3("Point Light Color", &color.x))
		{
			pointLight.SetColor(color);
		}

		float intensity = pointLight.GetIntensity();
		if (ImGui::SliderFloat("Point Light Intensity", &intensity, 0.0f, 20.0f))
		{
			pointLight.SetIntensity(intensity);
		}

		float range = pointLight.GetRange();
		if (ImGui::SliderFloat("Range", &range, 0.1f, 50.0f))
		{
			pointLight.SetRange(range);
		}
	}

	void EditorUI::DrawMaterialInspector(GameObject& gameObject)
	{
		Material* material = gameObject.GetMeshComponent().GetMaterial();
		if (material == nullptr)
		{
			return;
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Material");

		Vector3 baseColor = material->GetBaseColor();
		if (ImGui::ColorEdit3("Base Color", &baseColor.x))
		{
			material->SetBaseColor(baseColor);
		}

		Vector3 emissiveColor = material->GetEmissiveColor();
		if (ImGui::ColorEdit3("Emissive Color", &emissiveColor.x))
		{
			material->SetEmissiveColor(emissiveColor);
		}

		float emissiveIntensity = material->GetEmissiveIntensity();
		if (ImGui::SliderFloat("Emissive Intensity", &emissiveIntensity, 0.0f, 20.0f))
		{
			material->SetEmissiveIntensity(emissiveIntensity);
		}

		Vector3 rimColor = material->GetRimColor();
		if (ImGui::ColorEdit3("Rim Color", &rimColor.x))
		{
			material->SetRimColor(rimColor);
		}

		float rimIntensity = material->GetRimIntensity();
		if (ImGui::SliderFloat("Rim Intensity", &rimIntensity, 0.0f, 10.0f))
		{
			material->SetRimIntensity(rimIntensity);
		}

		float rimPower = material->GetRimPower();
		if (ImGui::SliderFloat("Rim Power", &rimPower, 0.5f, 10.0f))
		{
			material->SetRimPower(rimPower);
		}
	}
} // namespace My
