# MyPF Codex 작업 지침

## 프로젝트 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 짧은 실시간 데모를 목표로 한다.

최종 목표 기능:

- 1인칭 카메라와 WASD/마우스 조작
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 점광원
- 전원 상태에 따른 조명 변화와 순차 점등 연출
- 젖은 바닥의 반사 표현
- 그림자
- Bloom
- 안개 또는 볼륨감 있는 분위기 표현
- 색조 보정
- ImGui 기반 씬 편집 및 디버그 UI

## Codex와 사용자의 역할

- 모든 C++와 HLSL 코드는 사용자가 직접 작성한다.
- Codex는 사용자의 명시적인 요청 없이 프로젝트 소스 코드를 생성하거나 수정하지 않는다.
- 문서, 작업 인계 파일, 빌드 설정처럼 사용자가 직접 수정을 요청한 파일만 Codex가 수정할 수 있다.
- Codex는 새 기능의 목적과 책임을 먼저 설명하고, 기능을 작고 응집된 단계로 나누어 안내한다.
- 사용자가 수동으로 코드를 받아 적는 방식이 되지 않도록, 각 단계에서 필요한 판단 기준과 질문을 제시해 사용자가 직접 구현을 생각하도록 유도한다.
- 사용자가 막힌 부분을 구체적으로 질문하면 해당 문법, API 사용법, 실패 원인을 더 직접적으로 설명한다.
- 사용자가 수정을 완료하면 현재 작업 트리를 다시 읽고 코드 리뷰를 수행한다.
- 오류가 발견돼도 Codex가 임의로 고치지 않고 원인, 영향, 사용자가 수정할 방향을 먼저 설명한다.
- 빌드, 컴파일, 실행은 사용자가 명시적으로 요청할 때만 수행한다. 요청 시에는 우선 `Debug | x64`의 컴파일 단계로 코드 문제를 확인하고, 실행 검증은 별도 요청이 있을 때 수행한다.
- 설명은 초보자가 직접 구현할 수 있도록 구체적으로 하되, 완성 코드를 대신 작성하지 않는다.

## 핵심 아키텍처 원칙

- 하위 시스템은 상위 시스템을 알지 않는다.
- `Renderer`는 `Scene`, `GameObject`, `Transform`, `Camera`, ImGui를 포함하거나 참조하지 않는다.
- `Scene`과 `GameObject`는 DirectX 11 렌더링 과정이나 ImGui를 알지 않는다.
- `Transform`은 GPU/HLSL의 행렬 전치 및 상수 버퍼 저장 규칙을 알지 않는다.
- `AppBase`가 상위 조정 계층으로서 시스템 초기화와 Scene→RenderItem 변환을 담당한다.
- 오브젝트 단위 제출 경계는 `RenderItem`, 프레임 단위 경계는 `FrameRenderData`를 사용한다.
- 리소스의 실제 소유권과 비소유 참조를 명확히 구분한다.
- 최종 범용 ECS, 과도한 인터페이스 계층, 사용처가 없는 추상화를 미리 만들지 않는다.

### 그래픽 계층의 현재 책임

- `GraphicsDevice`는 현재 `ID3D11Device`, `ID3D11DeviceContext`, SwapChain, 기본 BackBuffer RTV, 기본 Depth Texture/DSV를 소유하며 Resize와 Present를 담당한다.
- `Renderer`는 `GraphicsDevice`를 비소유 포인터로 참조하고, 셰이더 슬롯, 갱신 시점, Draw 순서 같은 렌더링 의미와 정책을 담당한다.
- `Renderer`는 Rasterizer/DepthStencil/Sampler State, Shader/InputLayout, 네 종류의 Constant Buffer를 아직 직접 소유한다.
- 향후 `GraphicsResourceManager`가 일반 GPU Buffer/Texture/View를 실제 `ComPtr`로 소유하고 타입별 Handle을 발급한다.
- 기본 BackBuffer RTV와 기본 Depth Texture/DSV는 창 크기 및 SwapChain 수명에 묶여 있으므로, 일반 리소스와 섞지 않고 나중에 `SwapChainSurface` 또는 `WindowRenderSurface`로 분리한다.
- Shadow Map, Bloom, HDR Scene Texture 같은 오프스크린 리소스는 향후 `GraphicsResourceManager`가 관리한다.
- `AssetManager`는 이후 Mesh/Texture/Material 같은 논리 에셋을 관리하며 GPU 리소스 Handle을 보관한다. 렌더 타깃과 Constant Buffer는 Asset이 아니다.

## 현재 의존성과 소유 구조

```text
AppBase
├─ GraphicsDevice
│  ├─ Device / Context
│  ├─ SwapChain
│  ├─ 기본 RTV / Depth Texture / DSV
│  ├─ Resize
│  └─ Present
├─ Renderer --비소유 참조--> GraphicsDevice
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     └─ MeshComponent
│        ├─ 비소유 const Mesh*
│        └─ 비소유 Material*
├─ Camera
├─ Mesh 리소스들
├─ Texture
└─ Material 리소스들

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
```

- `Camera`는 View/Projection 행렬과 Yaw/Pitch/FOV 규칙을 담당하지만 Renderer를 알지 않는다.
- `Renderer`는 Camera 객체 대신 `FrameRenderData`만 받는다.
- `MeshComponent`는 Mesh와 Material을 참조할 뿐 직접 렌더링하지 않는다.
- `RenderItem`은 현재 Mesh, Material, World Matrix를 Renderer로 전달한다.
- 행렬 전치는 Renderer가 Constant Buffer에 기록하기 직전에 수행한다.
- 현재 `Mesh`는 Vertex/Index Buffer를, `Texture`는 Texture2D/SRV를 직접 소유한다. ResourceManager 이관 전까지의 과도기 구조다.

## BufferHandle과 ResourceManager 규칙

- `BufferHandle`은 실제 `ID3D11Buffer`나 `ComPtr`를 담지 않는 값 타입 식별자다.
- 기본 생성 Handle은 무효 인덱스를 가지며 `IsValid()`가 `false`여야 한다.
- 정상 Handle은 ResourceManager 내부 Buffer 저장소의 인덱스를 가진다.
- Handle은 생성, 업데이트, 해제, 바인딩을 수행하지 않는다.
- `GraphicsResourceManager`가 실제 리소스의 생성, 소유, 조회, 업데이트, 소멸을 담당한다.
- Renderer는 Camera/Light/Material 의미와 업데이트 시점, 셰이더 바인딩 슬롯을 결정한다.
- ResourceManager가 반환한 raw DirectX 포인터는 즉시 바인딩하기 위한 비소유 대여 값이며 외부에서 `Release()`하거나 장기간 저장하지 않는다.
- 초기 구현은 리소스 개별 삭제와 슬롯 재사용을 지원하지 않는다. 필요해질 때 `index + generation`으로 확장한다.
- ResourceManager의 초기 구현에 `shared_ptr` 또는 `weak_ptr`를 사용하지 않는다. Manager가 단독 소유하고 외부는 Handle을 사용한다.

## 작업 방식

1. 저장소 루트의 이 파일과 `MyPF/docs/CODEX_HANDOFF.md`를 먼저 읽는다.
2. 현재 소스, `git status`, 최근 변경을 확인하며 문서보다 코드를 우선한다.
3. 이번 단계의 목표, 필요한 이유, 책임 경계와 완료 조건을 먼저 설명한다.
4. 한 번에 지나치게 작은 문법 한 줄이 아니라 검토 가능한 작은 기능 단위로 진행한다.
5. 사용자가 직접 C++/HLSL을 구현한다.
6. Codex가 변경 파일과 의존성 경계를 리뷰한다.
7. 빌드/컴파일/실행은 사용자가 요청한 경우에만 수행한다.
8. 결과와 남은 문제를 정리한 뒤 다음 기능 단위를 제안한다.
9. 기능 또는 구조가 의미 있게 달라졌다면 `MyPF/docs/CODEX_HANDOFF.md`를 갱신한다.

## 빌드 및 실행 정보

- 저장소 루트: `C:/Users/Diguedman/source/repos/MyPF`
- 솔루션: `MyPF/MyPF.sln`
- 프로젝트 디렉터리: `MyPF/`
- 기본 검증 구성: `Debug | x64`
- HLSL 파일: `MyPF/Shaders/`
- 셰이더는 `D3DCompileFromFile`로 런타임 컴파일한다.
- 실행 시 `Shaders\\simpleVertexShader.hlsl` 같은 상대 경로가 해석되므로 작업 디렉터리를 `MyPF/`로 맞춘다.
- 검증용 임시 빌드 폴더는 저장소 내부의 명확한 경로를 사용하고, 삭제 전 절대 경로가 저장소 내부인지 확인한다.

## 변경 안전성

- 사용자가 작성한 기존 변경사항을 보존한다.
- 요청받지 않은 소스 파일을 수정하거나 삭제하지 않는다.
- `.vs`, `x64`, `CodexBuild*`, `CodexCompile*`, 실행 중 변경된 `imgui.ini` 같은 로컬 산출물을 커밋하지 않는다.
- 사용자가 요청하지 않는 한 Git commit, push, pull, branch 변경을 수행하지 않는다.
- 빌드나 실행을 위해 사용자의 기존 산출물 폴더를 지우지 않는다.
- 코드와 문서가 다르면 코드를 기준으로 문서를 고친다.

## 현재 우선순위

`GraphicsDevice` 분리와 AppBase/Renderer 연결은 완료됐다. `BufferHandle`도 작성됐다.

바로 다음 단계는 `GraphicsResourceManager`의 Buffer 저장소와 조회 기반을 만드는 것이다.

1. `GraphicsResourceManager.h/.cpp` 생성
2. 비소유 `GraphicsDevice*`와 내부 `BufferResource` 배열 구성
3. `Initialize(GraphicsDevice&)`와 `GetBuffer(BufferHandle) const` 구현
4. Constant Buffer 생성/업데이트 API 추가
5. AppBase가 ResourceManager를 소유하고 Renderer가 비소유 참조
6. Renderer의 Object/Camera/Light/Material Constant Buffer `ComPtr`를 `BufferHandle`로 교체
7. 이후 Mesh Vertex/Index Buffer와 일반 Texture/SRV를 단계적으로 이관

`GraphicsResourceHandle.h`는 Git에 추적되어 있지만 2026-08-19 확인 시 `.vcxproj/.filters`에는 아직 등록되지 않았다. 사용자가 요청하기 전까지 프로젝트 등록이나 컴파일을 임의로 수행하지 않는다.
