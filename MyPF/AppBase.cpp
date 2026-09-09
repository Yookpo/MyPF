#include <algorithm>
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

		m_firstPersonCameraController.Initialize(m_camera, m_inputSystem);
		m_editorUI.Initialize(m_scene,
			m_camera,
			m_firstPersonCameraController,
			m_directionalLight,
			m_backgroundColor);
		m_pointLightSequence.Initialize();

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

	bool AppBase::IsCursorInSceneView() const
	{
		const auto mousePos = ImGui::GetMousePos();
		float	   sceneViewWidth = m_screenWidth - m_guiWidth;

		if (sceneViewWidth <= 0 || m_screenHeight <= 0)
		{
			return false;
		}

		if (mousePos.x < m_guiWidth || mousePos.x >= m_screenWidth)
		{
			return false;
		}

		if (mousePos.y < 0 || mousePos.y >= m_screenHeight)
		{
			return false;
		}

		return true;
	}

	void AppBase::BeginEditorCameraNavigation()
	{
		if (m_isEditorCameraNavigating)
		{
			return;
		}

		m_isEditorCameraNavigating = true;
		ShowCursor(FALSE);
		CenterCursorInSceneView();
	}

	void AppBase::UpdateEditorCamera(float dt)
	{
		// 현재 내비게이션 중인데 오른쪽 마우스버튼을 놨다면
		if (m_isEditorCameraNavigating && !m_inputSystem.IsRightMouseButtonDown())
		{
			EndEditorCameraNavigation();
			return;
		}

		// 아직 내비게이션 중이 아니라면
		if (!m_isEditorCameraNavigating)
		{
			if (!m_inputSystem.IsRightMouseButtonDown() || !IsCursorInSceneView()
				|| ImGui::GetIO().WantCaptureMouse)
			{
				return;
			}
			BeginEditorCameraNavigation();
		}

		m_firstPersonCameraController.Update(dt);
		CenterCursorInSceneView();
	}

	void AppBase::EndEditorCameraNavigation()
	{
		if (!m_isEditorCameraNavigating)
		{
			return;
		}

		m_isEditorCameraNavigating = false;
		ShowCursor(TRUE);
		m_inputSystem.ResetMouseTracking();
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

		// Neon test material
		auto neonMat1 = m_assetManager.CreateMaterial("neonMat1");

		if (!neonMat1)
		{
			return false;
		}

		neonMat1->SetAlbedoTexture(greyBoxTex);
		neonMat1->SetBaseColor(Vector3(0.1f, 0.1f, 0.1f));
		neonMat1->SetEmissiveColor(Vector3(1.0f, 0.05f, 0.65f));
		neonMat1->SetEmissiveIntensity(3.0f);

		auto neonMat2 = m_assetManager.CreateMaterial("neonMat2");

		if (!neonMat2)
		{
			return false;
		}

		neonMat2->SetAlbedoTexture(greyBoxTex);
		neonMat2->SetBaseColor(Vector3(0.1f, 0.1f, 0.1f));
		neonMat2->SetEmissiveColor(Vector3(0.37f, 0.86f, 1.00f));
		neonMat2->SetEmissiveIntensity(8.0f);

		auto neonMat3 = m_assetManager.CreateMaterial("neonMat3");

		if (!neonMat3)
		{
			return false;
		}

		neonMat3->SetAlbedoTexture(greyBoxTex);
		neonMat3->SetBaseColor(Vector3(0.1f, 0.1f, 0.1f));
		neonMat3->SetEmissiveColor(Vector3(1.0f, 0.35f, 0.03f));
		neonMat3->SetEmissiveIntensity(5.f);

		// Create Point Light

		// 약하게 항상 켜져 있는 환경 보조광
		GameObject& EnvFillLight = m_scene.CreatePointLightObject("EnvironmentFillLight");
		EnvFillLight.GetTransform().SetPosition(Vector3{ 0.0f, 2.5f, 10.0f });
		EnvFillLight.GetPointLightComponent().SetColor(Vector3{ 0.55f, 0.10f, 1.0f });
		EnvFillLight.GetPointLightComponent().SetRange(6.0f);
		EnvFillLight.GetPointLightComponent().SetIntensity(1.58f);
		EnvFillLight.GetPointLightComponent().SetEnabled(true);

		// 첫 번째 네온 전용 조명
		GameObject& pinkNeonLight = m_scene.CreatePointLightObject("PinkNeonLight");
		pinkNeonLight.GetTransform().SetPosition(Vector3{ -1.2f, 2.3f, 5.0f });
		pinkNeonLight.GetPointLightComponent().SetColor(Vector3{ 1.0f, 0.05f, 0.65f });
		pinkNeonLight.GetPointLightComponent().SetRange(4.5f);
		pinkNeonLight.GetPointLightComponent().SetIntensity(2.0f);
		pinkNeonLight.GetPointLightComponent().SetEnabled(false);
		if (!m_pointLightSequence.AddSequenceEntry(pinkNeonLight, neonMat1))
		{
			return false;
		}

		// 두 번째 네온 전용 조명
		GameObject& cyanNeonLight = m_scene.CreatePointLightObject("CyanNeonLight");
		cyanNeonLight.GetTransform().SetPosition(Vector3{ 1.2f, 2.3f, 15.0f });
		cyanNeonLight.GetPointLightComponent().SetColor(Vector3{ 0.37f, 0.86f, 1.00f });
		cyanNeonLight.GetPointLightComponent().SetRange(4.5f);
		cyanNeonLight.GetPointLightComponent().SetIntensity(2.0f);
		cyanNeonLight.GetPointLightComponent().SetEnabled(false);
		if (!m_pointLightSequence.AddSequenceEntry(cyanNeonLight, neonMat2))
		{
			return false;
		}

		// 골목 끝에서 마지막에 켜지는 주요 네온 조명
		GameObject& orangeNeonLight = m_scene.CreatePointLightObject("OrangeNeonLight");
		orangeNeonLight.GetTransform().SetPosition(Vector3{ 0.0f, 3.0f, 20.0f });
		orangeNeonLight.GetPointLightComponent().SetColor(Vector3{ 1.0f, 0.35f, 0.03f });
		orangeNeonLight.GetPointLightComponent().SetRange(7.5f);
		orangeNeonLight.GetPointLightComponent().SetIntensity(5.0f);
		orangeNeonLight.GetPointLightComponent().SetEnabled(false);
		if (!m_pointLightSequence.AddSequenceEntry(orangeNeonLight, neonMat3))
		{
			return false;
		}

		// Create floor
		GameObject* floor = &m_scene.CreateGameObject("floor");

		floor->GetTransform().SetPosition(Vector3(0.0f, -0.1f, 10.0f));
		floor->GetTransform().SetScale(Vector3(4.0f, 0.2f, 20.0f));

		m_editorUI.SetSelectedObject(floor);

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

		// Neon Test Object
		auto neonTestObject0 = &m_scene.CreateGameObject("neonTestObject0");
		neonTestObject0->GetTransform().SetPosition(Vector3(-1.95f, 2.4f, 6.0f));
		neonTestObject0->GetTransform().SetScale(Vector3(0.1f, 0.6f, 2.0f));
		neonTestObject0->GetMeshComponent().SetMesh(greyBoxMesh);
		neonTestObject0->GetMeshComponent().SetMaterial(neonMat1);

		auto neonTestObject1 = &m_scene.CreateGameObject("neonTestObject1");
		neonTestObject1->GetTransform().SetPosition(Vector3(1.95f, 2.4f, 15.0f));
		neonTestObject1->GetTransform().SetScale(Vector3(0.1f, 0.8f, 1.46f));
		neonTestObject1->GetMeshComponent().SetMesh(greyBoxMesh);
		neonTestObject1->GetMeshComponent().SetMaterial(neonMat2);

		auto neonTestObject2 = &m_scene.CreateGameObject("neonTestObject2");
		neonTestObject2->GetTransform().SetPosition(Vector3(0.0f, 3.0f, 19.95f));
		neonTestObject2->GetTransform().SetScale(Vector3(0.1f, 0.3f, 1.0f));
		neonTestObject2->GetMeshComponent().SetMesh(greyBoxMesh);
		neonTestObject2->GetMeshComponent().SetMaterial(neonMat3);

		return true;
	}

	AppBase::AppBase()
		: m_screenWidth(1280), m_screenHeight(720), m_mainWindow(nullptr), m_appMode{ AppMode::Editor }, m_graphicsDevice{}, m_renderer{}, m_backgroundColor{ 0.047f, 0.031f, 0.125f, 1.0f }
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
		// Editor Camera Update
		if (m_appMode == AppMode::Editor)
		{
			UpdateEditorCamera(dt);
			return;
		}

		// ESC로 Play 모드 종료
		if (m_inputSystem.IsKeyDown(VK_ESCAPE))
		{
			ExitPlayMode();
			return;
		}

		m_firstPersonCameraController.Update(dt);

		// E키를 눌러 조명을 키거나 끈다
		if (m_inputSystem.WasKeyPressed('E'))
		{
			if (m_powerSwitch.CanInteract(m_camera.GetPosition(), m_camera.GetForward()))
			{
				if (m_powerSwitch.Toggle())
				{
					// 시퀀스 목표 설정
					m_pointLightSequence.Start(m_powerSwitch.IsPowerOn());
				}
			}
		}

		CenterCursorInSceneView();

		m_pointLightSequence.Update(dt);
	}

	void AppBase::Render()
	{
		FrameRenderData frameRenderData{};

		frameRenderData.view = m_camera.GetViewMatrix();
		frameRenderData.projection = m_camera.GetProjectionMatrix();
		frameRenderData.directionalLight = m_directionalLight;
		frameRenderData.pointLightCount = m_scene.GatherPointLights(frameRenderData.pointLights.data(), frameRenderData.pointLights.size());

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
		if (m_appMode == AppMode::Editor)
		{
			if (m_editorUI.Draw(static_cast<float>(m_screenHeight)))
			{
				EnterPlayMode();
			}

			m_guiWidth = m_editorUI.GetPanelWidth();
			return;
		}

		DrawPlayPanel();
	}

	void AppBase::DrawPlayPanel()
	{
		constexpr float			   panelWidth = 360.0f;
		const float				   panelHeight = m_screenHeight > 0 ? static_cast<float>(m_screenHeight) : 1.0f;
		constexpr ImGuiWindowFlags panelFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

		if (ImGui::Begin("Play Panel", nullptr, panelFlags))
		{
			m_guiWidth = ImGui::GetWindowSize().x;

			ImGui::TextUnformatted("Play");
			ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
				1000.0f / ImGui::GetIO().Framerate,
				ImGui::GetIO().Framerate);

			if (ImGui::Button("Stop", ImVec2(-1.0f, 0.0f)))
			{
				ExitPlayMode();
			}

			ImGui::Separator();
			if (m_powerSwitch.CanInteract(m_camera.GetPosition(), m_camera.GetForward()))
			{
				ImGui::TextUnformatted("Interaction available");
			}
			else
			{
				ImGui::TextUnformatted("Interaction unavailable");
			}

			ImGui::Text("Power: %s", m_powerSwitch.IsPowerOn() ? "On" : "Off");

			ImGui::Separator();
			ImGui::Text("W: %s", m_inputSystem.IsKeyDown('W') ? "Down" : "Up");
			ImGui::Text("S: %s", m_inputSystem.IsKeyDown('S') ? "Down" : "Up");
			ImGui::Text("A: %s", m_inputSystem.IsKeyDown('A') ? "Down" : "Up");
			ImGui::Text("D: %s", m_inputSystem.IsKeyDown('D') ? "Down" : "Up");
		}

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
