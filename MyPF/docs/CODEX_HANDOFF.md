# MyPF 작업 인계

마지막 갱신: 2026-08-19

## 이 문서의 목적

노트북과 데스크톱의 Codex 작업 맥락을 Git으로 공유하기 위한 현재 상태 문서다. 새 환경에서는 저장소 루트의 `AGENTS.md`를 먼저 읽고, 이 문서와 실제 소스 및 `git status`를 대조한다.

문서와 코드가 다르면 코드를 우선한다.

## 최종 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 연출을 핵심 장면으로 삼는다.

주요 목표 기능:

- 1인칭 카메라와 WASD/마우스 조작
- 짧은 골목 탐색
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 점광원
- 조명 상태 변화 및 순차 점등
- 젖은 바닥 반사
- 그림자
- Bloom
- 안개 또는 볼륨감 있는 분위기
- 색조 보정
- ImGui 기반 씬 편집 및 디버그 UI

최종 결과물은 범용 엔진 자체가 아니라 `DX11 렌더링 기술 + 확장 가능한 구조 + 짧지만 완성된 플레이 경험`을 보여주는 포트폴리오다.

## 사용자와 Codex의 작업 방식

- 사용자가 모든 C++와 HLSL 코드를 직접 작성한다.
- Codex는 사용자의 명시적 요청 없이 프로젝트 소스를 생성하거나 자동 수정하지 않는다.
- Codex는 기능의 목적과 책임을 먼저 설명하고, 사용자가 직접 판단해 구현할 수 있도록 작은 기능 단위와 완료 조건을 제시한다.
- 사용자가 수동적으로 코드를 받아 적지 않도록 가능한 경우 설계 질문과 판단 기준을 먼저 준다.
- 사용자가 막힌 문법이나 API를 구체적으로 질문하면 더 직접적으로 설명한다.
- 사용자가 `수정 완료`라고 하면 실제 파일을 다시 읽어 리뷰한다.
- 빌드, 컴파일, 실행은 사용자가 명시적으로 요청할 때만 한다. 요청 시 우선 `Debug | x64` 컴파일 단계로 확인한다.
- ImGui 기반 편집 환경을 먼저 확장하고, 플레이 입력과 상호작용은 이후 추가한다.
- 핵심 원칙은 “하위 시스템은 상위 시스템을 전혀 모른다”이다.

## 저장소 상태

- 로컬 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 현재 브랜치: `main`
- 2026-08-19 확인 HEAD: `3393d06` (`버퍼 핸들`)
- 2026-08-19 문서 갱신 직전 작업 트리: clean
- 솔루션: `MyPF/MyPF.sln`
- 기본 구성: `Debug | x64`
- 프로젝트 작업 디렉터리: `MyPF/`
- Shader 경로: `MyPF/Shaders/`
- HLSL은 Shader Model 5.0으로 런타임 컴파일한다.

`GraphicsDevice.h/.cpp`는 `.vcxproj`와 `.filters`에 등록돼 있다. `GraphicsResourceHandle.h`는 Git에는 추적되지만 2026-08-19 확인 시 프로젝트 파일에는 아직 등록되지 않았다. `GraphicsResourceManager.h/.cpp`는 아직 작성하지 않았다.

이번 GraphicsDevice/BufferHandle 변경 이후에는 사용자의 요청에 따라 컴파일이나 실행을 하지 않았다. 데스크톱에서 이어갈 때도 사용자가 검증을 요청하기 전에는 빌드하지 않는다.

## 현재 완료된 기능

### 플랫폼과 기본 렌더링

- Win32 Window와 메시지 루프
- DX11 Device, DeviceContext, SwapChain
- BackBuffer RTV, Depth Texture/DSV, DepthStencil State
- Rasterizer State와 Sampler State
- `WM_SIZE`에 따른 SwapChain/RTV/Depth 재생성
- ImGui 패널 폭을 제외한 Scene Viewport
- Vertex/Pixel Shader 런타임 컴파일
- Input Layout과 Indexed Drawing
- `GameTimer`

### Scene과 제출 경계

- `Scene`이 `vector<unique_ptr<GameObject>>`로 GameObject 소유
- `GameObject`가 이름, Transform, MeshComponent 보유
- `MeshComponent`가 `const Mesh*`와 `Material*`를 비소유 참조
- `RenderItem`이 Mesh, Material, World Matrix를 Renderer로 전달
- `FrameRenderData`가 View/Projection/DirectionalLight를 Renderer로 전달
- AppBase가 Scene을 순회하고 RenderItem을 구성
- Renderer는 Scene, GameObject, Transform, Camera, ImGui를 모름

### Camera와 ImGui

- Camera Position, View, Perspective Projection
- Yaw/Pitch 기반 회전과 ImGui 편집
- FOV 설정
- Scene View 크기에 따른 Aspect Ratio 갱신
- Scene GameObject 선택 및 Transform 편집
- 선택된 오브젝트의 Material 편집
- 배경색과 Directional Light 값 편집

### Mesh, 조명, Texture, Material

- Cube와 Triangle Mesh
- Vertex Position/Color/Normal/UV
- Object/Camera Constant Buffer 분리
- Directional Light와 Light Constant Buffer
- 기본 Normal 기반 조명
- Texture2D/SRV와 Sampler
- Material과 Material Constant Buffer
- 같은 Texture를 공유하면서 오브젝트별로 다른 BaseColor 적용
- 오브젝트별 Material 선택 및 수정

### GraphicsDevice 분리

- `GraphicsDevice`가 Device, Context, SwapChain, 기본 RTV, 기본 Depth Texture/DSV 소유
- `GraphicsDevice::Initialize`, `Resize`, `Present`
- AppBase가 GraphicsDevice를 Renderer보다 먼저 소유하고 초기화
- Renderer가 GraphicsDevice를 비소유 포인터로 참조
- Renderer의 Device/Context/SwapChain/RTV/DSV 직접 소유 제거
- BeginFrame이 GraphicsDevice의 Context/RTV/DSV 사용
- EndFrame이 `GraphicsDevice::Present()`에 위임
- Mesh, Texture, ImGui가 AppBase의 GraphicsDevice에서 Device/Context를 직접 받음
- WM_SIZE는 Device 존재 및 양수 크기를 확인한 뒤 GraphicsDevice Resize 호출

## 현재 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice
│  ├─ ID3D11Device
│  ├─ ID3D11DeviceContext
│  ├─ IDXGISwapChain
│  ├─ 기본 BackBuffer RTV
│  ├─ 기본 Depth Texture / DSV
│  ├─ Resize
│  └─ Present
├─ Renderer --비소유--> GraphicsDevice
│  ├─ Shader / InputLayout
│  ├─ Rasterizer / DepthStencil / Sampler State
│  └─ Object / Camera / Light / Material Constant Buffer
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     └─ MeshComponent
│        ├─ 비소유 const Mesh*
│        └─ 비소유 Material*
├─ Camera
├─ Mesh m_cubeMesh / m_triangleMesh
├─ Texture m_texture
└─ Material m_cubeMaterial / m_triangleMaterial
```

AppBase에서 멤버 선언 순서는 GraphicsDevice가 Renderer보다 앞이다. C++ 멤버는 역순으로 파괴되므로 Renderer가 먼저 소멸하고 GraphicsDevice가 나중에 소멸한다.

## 현재 프레임 흐름

1. Win32 메시지와 `WM_SIZE`를 처리한다.
2. ImGui 프레임을 시작하고 Scene/Camera/Light/Material 값을 편집한다.
3. Scene View 크기로 Camera Aspect Ratio와 Renderer Viewport를 갱신한다.
4. AppBase가 Camera와 DirectionalLight로 `FrameRenderData`를 만든다.
5. `Renderer::BeginFrame()`이 GraphicsDevice에서 Context/RTV/DSV를 빌려 Clear 및 바인딩한다.
6. Camera/Light Constant Buffer를 프레임 단위로 갱신한다.
7. AppBase가 Scene의 GameObject를 순회하고 `RenderItem`을 만든다.
8. `Renderer::DrawRenderItem()`이 Object/Material Constant Buffer를 갱신한다.
9. Mesh Buffer, Texture SRV, Shader, Constant Buffer를 바인딩하고 `DrawIndexed()`를 호출한다.
10. ImGui DrawData를 렌더링한다.
11. `Renderer::EndFrame()`이 GraphicsDevice에 Present를 요청한다.

## 그래픽 계층에 대한 최신 결정

### GraphicsDevice

현재는 DX11 기반 환경과 기본 Window Surface를 함께 가진 과도기 클래스다.

- Device/Context는 최종적으로도 GraphicsDevice 책임이다.
- SwapChain, 기본 RTV, 기본 Depth Texture/DSV는 향후 `SwapChainSurface` 또는 `WindowRenderSurface`로 분리한다.
- 지금 즉시 다시 분리하지 않는다. 방금 끝낸 GraphicsDevice 연결을 안정화하고 ResourceManager를 먼저 진행한다.

### GraphicsResourceManager

일반 GPU 리소스의 실제 소유자다.

- Constant/Vertex/Index Buffer
- 일반 Texture2D와 SRV
- 향후 Shadow Map, HDR Scene Texture, Bloom용 RTV/SRV/DSV
- 생성, 조회, 업데이트, 수명 종료

ResourceManager는 Camera, Light, Material의 의미나 셰이더 슬롯을 모른다. Renderer가 업데이트 시점과 바인딩 정책을 담당한다.

### AssetManager

향후 Mesh, Texture, Material 같은 논리 에셋을 관리하고 GPU Handle을 보관한다. Constant Buffer, Shadow Map, Bloom Render Target은 논리 에셋이 아니라 그래픽 런타임 리소스다.

## BufferHandle 현재 상태

`MyPF/GraphicsResourceHandle.h`에 `BufferHandle`을 작성했다.

현재 의도:

- 실제 `ID3D11Buffer*`나 `ComPtr`를 보관하지 않음
- `uint32_t m_index` 하나만 객체 상태로 보관
- `static constexpr` 무효 인덱스는 `uint32_t` 최댓값
- 기본 생성은 무효 Handle
- `explicit BufferHandle(uint32_t index)`로 정상 Handle 생성
- `GetIndex() const`
- `IsValid() const`는 인덱스가 무효 값과 다를 때 true

Handle의 책임은 ResourceManager 내부 리소스를 식별하고 무효 상태를 표현하는 것뿐이다. 생성, 업데이트, 해제, 바인딩, 소유권 관리는 하지 않는다.

초기 구현에서는 리소스를 배열에서 삭제하지 않으므로 index만 사용한다. 개별 삭제 및 슬롯 재사용이 필요해지면 generation을 추가한다.

## 바로 다음 작업

`GraphicsResourceManager`의 Buffer 저장소와 조회 기반을 만든다.

작성할 파일:

- `MyPF/GraphicsResourceManager.h`
- `MyPF/GraphicsResourceManager.cpp`

첫 구현 범위:

1. `GraphicsDevice`를 비소유 포인터로 보관한다.
2. 복사 생성자와 복사 대입을 삭제한다.
3. private `BufferResource`가 `ComPtr<ID3D11Buffer>`와 `uint32_t byteWidth`를 가진다.
4. `vector<BufferResource>`가 실제 Buffer를 소유한다.
5. `Initialize(GraphicsDevice&)`가 Device와 Context 유효성을 확인하고 주소를 저장한다.
6. `GetBuffer(BufferHandle) const`가 Handle 유효성, 배열 범위, ComPtr 유효성을 검사한 뒤 raw pointer를 반환한다.

이 첫 범위에서는 아직 Buffer 생성/업데이트, Renderer 변경, 프로젝트 등록, 컴파일을 하지 않는다.

그 다음 순서:

1. 타입 템플릿 공개 API와 비템플릿 내부 API로 Constant Buffer 생성/업데이트 구현
2. AppBase가 GraphicsResourceManager를 GraphicsDevice 다음, Renderer 이전에 소유
3. Renderer가 GraphicsDevice와 GraphicsResourceManager를 비소유 참조
4. Renderer의 네 Constant Buffer `ComPtr`를 `BufferHandle`로 교체
5. Vertex/Index Buffer를 ResourceManager로 이관
6. 일반 Texture/SRV를 ResourceManager로 이관
7. 이후 AssetManager 도입

## 다음 작업에서 지킬 점

- `BufferHandle`에 DirectX 포인터나 ComPtr를 넣지 않는다.
- ResourceManager가 실제 ComPtr의 유일한 소유자다.
- `GetBuffer()`의 raw pointer는 즉시 바인딩할 때만 빌려 쓴다. 외부에서 Release하거나 장기간 보관하지 않는다.
- Handle이 `IsValid()`여도 임의로 큰 인덱스일 수 있으므로 배열 범위를 반드시 검사한다.
- 처음부터 `shared_ptr`, `weak_ptr`, 리소스 삭제, free list, generation을 넣지 않는다.
- 하위 GraphicsDevice/ResourceManager는 Renderer, Scene, AppBase를 알지 않는다.
- Renderer는 ResourceManager를 사용해도 Scene/GameObject/Camera를 직접 알지 않는다.

## 남은 주요 로드맵

1. GraphicsResourceManager와 Handle 기반 GPU 리소스 소유
2. AssetManager와 외부 Mesh/Texture 로딩
3. ImGui Scene Hierarchy/Inspector와 배치 기능 확장
4. 1인칭 WASD/마우스 입력과 Editor/Play 상태
5. 다수 Point Light와 네온 조명
6. 사이버펑크 골목 기본 콘텐츠 구성
7. Shadow Mapping
8. 젖은 바닥 재질과 반사
9. HDR Scene Target과 Bloom
10. 안개와 색조 보정
11. 문/전원 스위치 상호작용 및 순차 점등
12. 디버그 UI, 최적화, 1~2분 최종 연출

최종 데모 전체 작업량 기준 진행률은 대략 30~35%로 본다. 기본 렌더링 및 Scene 구조는 상당 부분 마련됐지만, 고급 렌더링, 외부 콘텐츠, 상호작용과 최종 연출은 대부분 남아 있다.
