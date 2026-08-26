#include "AppBase.h"
#include "GeometryGenerator.h"
#include "RenderItem.h"
#include "FrameRenderData.h"
#include "Material.h"

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

		GameObject* cube1 = &m_scene.CreateGameObject("cube1");
		GameObject* triangle1 = &m_scene.CreateGameObject("triangle1");

		cube1->GetTransform().SetPosition(Vector3(-0.6f, 0.0f, 0.0f));
		cube1->GetTransform().SetScale(Vector3(0.4f, 0.4f, 0.4f));

		triangle1->GetTransform().SetPosition(Vector3(0.6f, 0.0f, 0.0f));
		triangle1->GetTransform().SetScale(Vector3(0.4f, 0.4f, 0.4f));

		m_selectedObject = cube1;

		MeshData cubeData = GeometryGenerator::MakeCube();
		MeshData triangleData = GeometryGenerator::MakeTriangle();

		auto cubeMesh = m_assetManager.CreateMesh("cube", cubeData);
		auto triangleMesh = m_assetManager.CreateMesh("triangle", triangleData);

		if (!cubeMesh || !triangleMesh)
		{
			return false;
		}

		const Texture* cubeTex = m_assetManager.LoadTexture("wall.jpg");
		const Texture* triangleTex = m_assetManager.LoadTexture("wall.jpg");

		if (!cubeTex || !triangleTex || (cubeTex != triangleTex))
		{
			return false;
		}

		auto cubeMat = m_assetManager.CreateMaterial("cubeMat");
		auto triangleMat = m_assetManager.CreateMaterial("triangleMat");

		if (!cubeMat || !triangleMat)
		{
			return false;
		}

		cubeMat->SetAlbedoTexture(cubeTex);
		triangleMat->SetAlbedoTexture(triangleTex);

		cubeMat->SetBaseColor(Vector3(0.5f, 0.5f, 0.5f));
		triangleMat->SetBaseColor(Vector3(0.2f, 0.64f, 0.18f));

		cube1->GetMeshComponent().SetMesh(cubeMesh);
		triangle1->GetMeshComponent().SetMesh(triangleMesh);

		cube1->GetMeshComponent().SetMaterial(cubeMat);
		triangle1->GetMeshComponent().SetMaterial(triangleMat);

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
			L"CyberPunk",
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
			wc.lpszClassName, L"CyberPunk",
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

	AppBase::AppBase()
		: m_screenWidth(1280), m_screenHeight(720), m_mainWindow(nullptr), m_graphicsDevice{}, m_renderer{}, m_backgroundColor{ 0.047f, 0.031f, 0.125f, 1.0f }, m_selectedObject{ nullptr }
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

	void AppBase::Update(float dt) {}
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
			const Transform&	 tr = obj->GetTransform();
			const Matrix		 world = tr.GetWorldMatrix();
			const MeshComponent& meshComponent = obj->GetMeshComponent();

			// 추후에 mesh가 없을 때만 그리기 생략을 함
			// 추후에 기본 머터리얼 & 기본 텍스처를 도입
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
		ImGui::Begin("Test Window");
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

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
			if (ImGui::DragFloat3("Move", &pos.x, 0.01f, -1.0f, 1.0f))
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
			if (ImGui::SliderFloat3("Scaling", &scale.x, 0.1f, 2.0f))
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
			}
		}

		return static_cast<int>(msg.wParam);
	}

} // namespace My
