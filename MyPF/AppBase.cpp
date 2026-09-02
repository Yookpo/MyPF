#include "AppBase.h"
#include "GeometryGenerator.h"
#include "RenderItem.h"
#include "FrameRenderData.h"
#include "Material.h"
#include "Model.h"

namespace My
{
	using namespace std;

	// RegisterClassEx()에서 멤버 함수를 직접 등록할 수가 없기 때문에
	// 클래스의 멤버 함수에서 간접적으로 메시지를 처리할 수 있도록 도와줍니다.
	AppBase* g_appBase = nullptr;

	// RegisterClassEx()에서 실제로 등록될 콜백 함수
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// g_appBase를 이용해서 간접적으로 멤버 함수 호출
		if (g_appBase == nullptr)
		{
			return DefWindowProcW(hWnd, msg, wParam, lParam);
		}

		return g_appBase->MsgProc(hWnd, msg, wParam, lParam);
	}

	// 윈도우 프로시저
	// Windows가 전달하는 창 이벤트 처리
	LRESULT AppBase::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		m_inputSystem.ProcessMessage(msg, wParam, lParam);

		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		{
			return true;
		}

		switch (msg)
		{
			case WM_SIZE:
				m_screenWidth = int(LOWORD(lParam));
				m_screenHeight = int(HIWORD(lParam));

				if (m_graphicsDevice.GetDevice() && (m_screenWidth > 0 && m_screenHeight > 0))
				{
					m_graphicsDevice.Resize(m_screenWidth, m_screenHeight);
				}

				break;

			case WM_KILLFOCUS:
				if (m_appMode == AppMode::Play)
				{
					ExitPlayMode();
				}
				break;

			case WM_DESTROY:
				m_mainWindow = nullptr;
				::PostQuitMessage(0);
				return 0;
		}

		return ::DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	bool AppBase::Initialize()
	{
		if (!InitMainWindow())
		{
			return false;
		}

		if (!m_graphicsDevice.Initialize(m_mainWindow, m_screenWidth, m_screenHeight))
		{
			return false;
		}

		if (!m_resourceManager.Initialize(m_graphicsDevice))
		{
			return false;
		}

		if (!m_assetManager.Initialize(m_resourceManager))
		{
			return false;
		}

		if (!m_renderer.Initialize(m_graphicsDevice, m_resourceManager, m_screenWidth, m_screenHeight))
		{
			return false;
		}

		if (!InitGUI())
		{
			return false;
		}

		// Init GeryBox Scene
		if (!InitGreyBoxScene())
		{
			return false;
		}

		return true;
	}

	bool AppBase::InitMainWindow()
	{
		// 창 클래스 등록
		WNDCLASSEX wc = {
			sizeof(WNDCLASSEX), CS_CLASSDC,
			WndProc,
			0L, 0L,
			GetModuleHandle(NULL),
			NULL,
			LoadCursor(nullptr, IDC_ARROW),
			NULL,
			NULL,
			L"TEST",
			NULL
		};

		if (!RegisterClassEx(&wc))
		{
			std::cerr << "RegisterClassEx() failed." << endl;
			return false;
		}

		RECT wr = { 0, 0, m_screenWidth, m_screenHeight };

		AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);

		m_mainWindow = CreateWindow(
			wc.lpszClassName, L"TEST",
			WS_OVERLAPPEDWINDOW,
			100,				// 윈도우 좌측 상단의 x 좌표
			100,				// 윈도우 좌측 상단의 y 좌표
			wr.right - wr.left, // 윈도우 가로 방향 해상도
			wr.bottom - wr.top, // 윈도우 세로 방향 해상도
			NULL, NULL, wc.hInstance, NULL);

		if (!m_mainWindow)
		{
			std::cerr << "CreateWindow() failed.\n";
			return false;
		}

		ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
		UpdateWindow(m_mainWindow);

		return true;
	}
	bool AppBase::InitGUI()
	{
		// ImGui 생성 및 초기화
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		// Setup Platform/Renderer backends
		if (!ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice(), m_graphicsDevice.GetContext()))
		{
			return false;
		}

		if (!ImGui_ImplWin32_Init((void*)m_mainWindow))
		{
			return false;
		}

		return true;
	}

	float AppBase::GetAspectRatio(float sceneViewWidth, float sceneViewHeight) const
	{
		return (sceneViewWidth / sceneViewHeight);
	}

	void AppBase::EnterPlayMode()
	{
		// 커서 숨김은 전환 시 한 번 호출
		ShowCursor(FALSE);

		m_editorCameraSnapshot = m_camera;
		m_inputSystem.Reset();
		m_appMode = AppMode::Play;

		return;
	}

	void AppBase::ExitPlayMode()
	{
		ShowCursor(TRUE);

		m_inputSystem.Reset();
		m_camera = m_editorCameraSnapshot;
		m_appMode = AppMode::Editor;

		return;
	}

	void AppBase::CenterCursorInSceneView()
	{
		if (!m_mainWindow || (m_screenWidth - m_guiWidth) <= 0 || m_screenHeight <= 0)
		{
			return;
		}

		float centerClientX = m_guiWidth + (m_screenWidth - m_guiWidth) * 0.5f;
		float centerClientY = m_screenHeight * 0.5f;

		POINT sceneViewPos{ static_cast<LONG>(centerClientX), static_cast<LONG>(centerClientY) };

		if (!ClientToScreen(m_mainWindow, &sceneViewPos))
		{
			return;
		}

		if (!SetCursorPos(sceneViewPos.x, sceneViewPos.y))
		{
			return;
		}

		m_inputSystem.SetMouseReferencePosition(centerClientX, centerClientY);
	}

	bool AppBase::InitGreyBoxScene()
	{
		// Setting for Mesh, Material
		MeshData greyBoxData = GeometryGenerator::MakeCube();
		auto	 greyBoxMesh = m_assetManager.CreateMesh("greybox", greyBoxData);

		if (!greyBoxMesh)
		{
			return false;
		}

		const Texture* greyBoxTex = m_assetManager.LoadTexture("wall.jpg");

		if (!greyBoxTex)
		{
			return false;
		}

		auto greyBoxMat = m_assetManager.CreateMaterial("greyBoxMat");

		if (!greyBoxMat)
		{
			return false;
		}

		greyBoxMat->SetAlbedoTexture(greyBoxTex);
		greyBoxMat->SetBaseColor(Vector3(0.5f, 0.5f, 0.5f));

		// Create floor
		GameObject* floor = &m_scene.CreateGameObject("floor");

		floor->GetTransform().SetPosition(Vector3(0.0f, -0.1f, 10.0f));
		floor->GetTransform().SetScale(Vector3(4.0f, 0.2f, 20.0f));

		m_selectedObject = floor;

		floor->GetMeshComponent().SetMesh(greyBoxMesh);
		floor->GetMeshComponent().SetMaterial(greyBoxMat);

		// Create Wall
		GameObject* leftWall = &m_scene.CreateGameObject("leftWall");
		GameObject* rightWall = &m_scene.CreateGameObject("rightWall");
		GameObject* endWall = &m_scene.CreateGameObject("endWall");

		leftWall->GetTransform().SetScale(Vector3(0.2f, 4.0f, 20.0f));
		rightWall->GetTransform().SetScale(Vector3(0.2f, 4.0f, 20.0f));
		endWall->GetTransform().SetScale(Vector3(4.0f, 4.0f, 0.2f));

		leftWall->GetTransform().SetPosition(Vector3(-2.1f, 2.0f, 10.0f));
		rightWall->GetTransform().SetPosition(Vector3(2.1f, 2.0f, 10.0f));
		endWall->GetTransform().SetPosition(Vector3(0.0f, 2.0f, 20.1f));

		leftWall->GetMeshComponent().SetMesh(greyBoxMesh);
		rightWall->GetMeshComponent().SetMesh(greyBoxMesh);
		endWall->GetMeshComponent().SetMesh(greyBoxMesh);

		leftWall->GetMeshComponent().SetMaterial(greyBoxMat);
		rightWall->GetMeshComponent().SetMaterial(greyBoxMat);
		endWall->GetMeshComponent().SetMaterial(greyBoxMat);

		// Set Camera Pos
		m_camera.SetPosition(Vector3(0.0f, 1.6f, 0.0f));

		// Create Power Switch
		auto powerSwitchObject = &m_scene.CreateGameObject("powerSwitch");
		powerSwitchObject->GetTransform().SetPosition(Vector3(0.0f, 1.2f, 19.95f));
		powerSwitchObject->GetTransform().SetScale(Vector3(0.4f, 0.6f, 0.1f));

		// Switch Mat
		auto powerSwitchMat = m_assetManager.CreateMaterial("powerSwitchMat");

		if (!powerSwitchMat)
		{
			return false;
		}

		powerSwitchMat->SetAlbedoTexture(greyBoxTex);

		powerSwitchObject->GetMeshComponent().SetMesh(greyBoxMesh);
		powerSwitchObject->GetMeshComponent().SetMaterial(powerSwitchMat);

		m_powerSwitch.Initialize(*powerSwitchObject);

		return true;
	}

	AppBase::AppBase()
		: m_screenWidth(1280), m_screenHeight(720), m_mainWindow(nullptr), m_cameraSpeed{ 4.0f }, m_mouseSensitivity{ 0.1f }, m_appMode{ AppMode::Editor }, m_graphicsDevice{}, m_renderer{}, m_backgroundColor{ 0.047f, 0.031f, 0.125f, 1.0f }, m_selectedObject{ nullptr }
	{
		g_appBase = this;
	}

	AppBase::~AppBase()
	{
		if (m_mainWindow)
		{
			DestroyWindow(m_mainWindow);
		}

		g_appBase = nullptr;

		// ImGui 소멸
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void AppBase::Update(float dt)
	{
		if (m_appMode == AppMode::Editor)
		{
			return;
		}

		// ESC로 Play 모드 종료
		if (m_inputSystem.IsKeyDown(VK_ESCAPE))
		{
			ExitPlayMode();
			return;
		}

		MouseDelta delta = m_inputSystem.ConsumeMouseDelta();
		float	   cameraYaw = m_camera.GetYaw();
		float	   cameraPitch = m_camera.GetPitch();

		cameraYaw += (delta.deltaX * m_mouseSensitivity);
		cameraPitch -= (delta.deltaY * m_mouseSensitivity);

		m_camera.SetYawPitch(cameraYaw, cameraPitch);

		Vector3 cameraPos = m_camera.GetPosition();
		Vector3 cameraForward = m_camera.GetForward();
		Vector3 cameraUp = m_camera.GetUp();
		cameraForward.y = 0.0f;

		if (cameraForward.LengthSquared() > 0.00001f)
		{
			cameraForward.Normalize();
		}

		Vector3 cameraRight = cameraUp.Cross(cameraForward);
		if (cameraRight.LengthSquared() > 0.00001f)
		{
			cameraRight.Normalize();
		}

		Vector3 moveDirection{};

		if (m_inputSystem.IsKeyDown('W'))
		{
			moveDirection += cameraForward;
		}

		if (m_inputSystem.IsKeyDown('S'))
		{
			moveDirection -= cameraForward;
		}

		if (m_inputSystem.IsKeyDown('A'))
		{
			moveDirection -= cameraRight;
		}

		if (m_inputSystem.IsKeyDown('D'))
		{
			moveDirection += cameraRight;
		}

		if (moveDirection.LengthSquared() > 0.00001f)
		{
			moveDirection.Normalize();
		}

		cameraPos += moveDirection * m_cameraSpeed * dt;

		m_camera.SetPosition(cameraPos);

		if (m_inputSystem.WasKeyPressed('E'))
		{
			if (m_powerSwitch.CanInteract(m_camera.GetPosition(), m_camera.GetForward()))
			{
				m_powerSwitch.Activate();
			}
		}

		CenterCursorInSceneView();
	}

	void AppBase::Render()
	{
		FrameRenderData frameRenderData;

		frameRenderData.view = m_camera.GetViewMatrix();
		frameRenderData.projection = m_camera.GetProjectionMatrix();
		frameRenderData.directionalLight = m_directionalLight;

		if (!m_renderer.BeginFrame(frameRenderData, m_backgroundColor))
		{
			OutputDebugStringW(L"Draw camera failed, Program shutting down");
			PostQuitMessage(-1);
			return;
		}

		// 해당 Scene의 오브젝트들을 순회
		const auto& sceneObjects = m_scene.GetGameObjects();
		for (const auto& obj : sceneObjects)
		{
			const Transform&	  tr = obj->GetTransform();
			const Matrix		  world = tr.GetWorldMatrix();
			const MeshComponent&  meshComponent = obj->GetMeshComponent();
			const ModelComponent& modelComponent = obj->GetModelComponent();

			if (modelComponent.HasModel())
			{
				// Model 가져오기
				const auto*					  model = modelComponent.GetModel();
				const std::vector<ModelPart>& parts = model->GetParts();

				// 모든 ModelPart 순회
				// ModelPart -> RenderItem으로 변환
				for (size_t i = 0; i < parts.size(); i++)
				{
					// 각 Part의 Mesh/Material로 RenderItem 생성
					RenderItem renderItem{
						parts[i].mesh, parts[i].material, world
					};

					if (!m_renderer.DrawRenderItem(renderItem))
					{
						OutputDebugStringW(L"Draw RenderItem failed, Program shutting down");
						PostQuitMessage(-1);
						return;
					}
				}
			}

			// 모델이 없으면 기존 Mesh 처리
			else
			{
				if (!meshComponent.HasMesh() || !meshComponent.HasMaterial())
				{
					continue;
				}

				RenderItem renderItem{ meshComponent.GetMesh(), meshComponent.GetMaterial(), world };

				if (!m_renderer.DrawRenderItem(renderItem))
				{
					OutputDebugStringW(L"Draw RenderItem failed, Program shutting down");
					PostQuitMessage(-1);
					return;
				}
			}
		}

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (!m_renderer.EndFrame())
		{
			OutputDebugStringW(L"Rendering failed, Program shutting down");
			PostQuitMessage(-1);
		}
	}

	void AppBase::UpdateGui()
	{
		// ImGui 로직
		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::Begin("Control Panel");

		// 현재 Play 모드인가?
		if (m_appMode == AppMode::Play)
		{
			ImGui::Text("Mode: Play");
			ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			if (ImGui::SliderFloat("Camera Speed", &m_cameraSpeed, 0.0f, 10.0f))
			{
			}

			if (ImGui::SliderFloat("Mouse Sensitivity", &m_mouseSensitivity, 0.01f, 1.0f))
			{
			}

			if (ImGui::Button("Stop"))
			{
				ExitPlayMode();
			}

			if (m_powerSwitch.CanInteract(m_camera.GetPosition(), m_camera.GetForward()))
			{
				ImGui::Text("Interaction available");
			}
			else
			{
				ImGui::Text("Interaction unavailable");
			}

			if (m_powerSwitch.IsPowerOn())
			{
				ImGui::Text("Power: On");
			}
			else
			{
				ImGui::Text("Power: Off");
			}

			// Test
			if (m_inputSystem.IsKeyDown('W'))
			{
				ImGui::Text("W : Down");
			}
			else
			{
				ImGui::Text("W : Up");
			}
			if (m_inputSystem.IsKeyDown('S'))
			{
				ImGui::Text("S : Down");
			}
			else
			{
				ImGui::Text("S : Up");
			}
			if (m_inputSystem.IsKeyDown('A'))
			{
				ImGui::Text("A : Down");
			}
			else
			{
				ImGui::Text("A : Up");
			}
			if (m_inputSystem.IsKeyDown('D'))
			{
				ImGui::Text("D : Down");
			}
			else
			{
				ImGui::Text("D : Up");
			}
		}

		// 현재 Edit 모드인가?
		else
		{
			ImGui::Text("Mode: Editor");

			if (ImGui::Button("Play"))
			{
				EnterPlayMode();
			}

			if (ImGui::ColorEdit4("Control BackColor", m_backgroundColor.data(), 0))
			{
			}

			ImGui::Separator();
			ImGui::Text("Light");
			Vector3 lightDir = m_directionalLight.direction;
			if (ImGui::DragFloat3("Light Direction", &lightDir.x, 0.01f, -1.0f, 1.0f))
			{
				if (lightDir.LengthSquared() > 0.00001f)
				{
					lightDir.Normalize();
					m_directionalLight.direction = lightDir;
				}
			}

			Vector3 lightColor = m_directionalLight.color;
			if (ImGui::ColorEdit3("Light Color", &lightColor.x))
			{
				m_directionalLight.color = lightColor;
			}
			float lightIntensity = m_directionalLight.intensity;
			if (ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 5.0f))
			{
				m_directionalLight.intensity = lightIntensity;
			}
			float ambientStrength = m_directionalLight.ambientStrength;
			if (ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.0f, 1.0f))
			{
				m_directionalLight.ambientStrength = ambientStrength;
			}

			ImGui::Separator();
			ImGui::Text("Camera");
			Vector3 cameraPos = m_camera.GetPosition();
			if (ImGui::DragFloat3("Camera Move", &cameraPos.x, 0.01f, -10.0f, 10.0f))
			{
				m_camera.SetPosition(cameraPos);
			}

			float cameraYaw = m_camera.GetYaw();
			float cameraPitch = m_camera.GetPitch();
			float cameraFov = m_camera.GetFovAngleY();

			bool yawChanged = ImGui::DragFloat("Yaw Slider", &cameraYaw, 0.1f, -180.0f, 180.0f);
			bool pitchChanged = ImGui::DragFloat("Pitch Slider", &cameraPitch, 0.1f, -89.0f, 89.0f);
			bool fovChanged = ImGui::SliderFloat("Fov Slider", &cameraFov, 30.0f, 120.0f);

			if (yawChanged || pitchChanged)
			{
				m_camera.SetYawPitch(cameraYaw, cameraPitch);
			}

			if (fovChanged)
			{
				m_camera.SetFovAngleY(cameraFov);
			}

			ImGui::Separator();
			ImGui::Text("Scene Objects");

			const auto& sceneObjects = m_scene.GetGameObjects();

			for (const auto& obj : sceneObjects)
			{
				GameObject* gameObject = obj.get();
				const bool	isSelected = (m_selectedObject == gameObject);

				ImGui::PushID(gameObject);

				if (ImGui::Selectable(gameObject->GetName().c_str(), isSelected))
				{
					m_selectedObject = gameObject;
				}

				ImGui::PopID();
			}

			ImGui::Separator();

			if (m_selectedObject)
			{
				ImGui::Text("Selected: %s", m_selectedObject->GetName().c_str());
				MeshComponent& comp = m_selectedObject->GetMeshComponent();
				Transform&	   tr = m_selectedObject->GetTransform();
				Material*	   mat = comp.GetMaterial();

				// 위치 수정
				Vector3 pos = tr.GetPosition();
				if (ImGui::DragFloat3("Move", &pos.x, 0.01f, -50.0f, 50.0f))
				{
					tr.SetPosition(pos);
				}
				// 회전 수정
				Vector3 rot = tr.GetRotation();
				if (ImGui::SliderFloat3("Rotate(Rad)", &rot.x, -3.14f, 3.14f))
				{
					tr.SetRotation(rot);
				}
				// 스케일 수정
				Vector3 scale = tr.GetScale();
				if (ImGui::SliderFloat3("Scaling", &scale.x, 0.01f, 50.0f))
				{
					tr.SetScale(scale);
				}

				if (mat)
				{
					Vector3 matBaseColor = mat->GetBaseColor();
					if (ImGui::SliderFloat3("Base Color", &matBaseColor.x, 0.0f, 1.0f))
					{
						mat->SetBaseColor(matBaseColor);
					}
				}
			}
		}

		m_guiWidth = ImGui::GetWindowSize().x;

		ImGui::End();
	}

	int AppBase::Run()
	{
		// Main Message loop
		MSG msg = { 0 };
		m_gameTimer.Reset();

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				m_gameTimer.Tick();

				// ImGui 프레임 시작
				// 백버퍼 렌더링 호출 전에 ImGui 렌더링 준비, 컨트롤 설정, 렌더링 요청 함수 호출
				ImGui_ImplWin32_NewFrame();
				ImGui_ImplDX11_NewFrame();
				ImGui::NewFrame();

				UpdateGui();

				ImGui::Render();

				if (m_screenWidth - m_guiWidth > 0 && m_screenHeight > 0)
				{
					float sceneViewWidth = static_cast<float>(m_screenWidth - m_guiWidth);
					float sceneViewHeight = static_cast<float>(m_screenHeight);
					float sceneViewRatio = this->GetAspectRatio(sceneViewWidth, sceneViewHeight);

					m_camera.SetAspectRatio(sceneViewRatio);
					m_renderer.SetSceneViewport(m_guiWidth, 0, sceneViewWidth, sceneViewHeight);
				}

				Update(m_gameTimer.GetDeltaTime());

				Render();

				m_inputSystem.EndFrame();
			}
		}

		return static_cast<int>(msg.wParam);
	}

} // namespace My
