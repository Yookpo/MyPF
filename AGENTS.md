# MyPF Codex 작업 지침

마지막 갱신: 2026-08-27

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

최종 결과물은 범용 엔진 자체가 아니라 `DX11 렌더링 기술 + 확장 가능한 구조 + 짧지만 완성된 플레이 경험`을 보여주는 포트폴리오다.

## Codex와 사용자의 역할

- 모든 C++와 HLSL 코드는 사용자가 직접 작성한다.
- Codex는 사용자의 명시적인 요청 없이 프로젝트 소스 코드를 생성하거나 수정하지 않는다.
- 문서, 작업 인계 파일, 빌드 설정처럼 사용자가 직접 수정을 요청한 파일만 Codex가 수정할 수 있다.
- Codex는 새 기능의 목적과 책임을 먼저 설명하고, 기능을 작고 응집된 단계로 나누어 안내한다.
- 사용자가 수동으로 코드를 받아 적지 않도록 각 단계에서 필요한 배경지식, 판단 기준과 질문을 먼저 제공한다.
- 사용자의 코드가 잘못됐거나 불완전해도 곧바로 완성 코드를 제시하지 않는다. 먼저 놓친 개념과 조건을 설명하고 사용자가 다시 작성하도록 유도하며, 사용자가 막혔다고 명시하고 완성 형태를 요청할 때만 직접적인 완성 코드를 제공한다.
- 사용자가 막힌 문법이나 API를 구체적으로 질문하면 해당 문법, API 사용법과 실패 원인을 더 직접적으로 설명한다.
- 사용자가 수정을 완료하면 실제 작업 트리를 다시 읽고 코드 리뷰를 수행한다.
- 오류가 발견돼도 Codex가 임의로 고치지 않고 원인, 영향과 사용자가 수정할 방향을 먼저 설명한다.
- 빌드, 컴파일, 실행은 사용자가 명시적으로 요청할 때만 수행한다. 요청 시 우선 `Debug | x64` 컴파일 단계로 확인하며 실행 검증은 별도 요청이 있을 때 수행한다.
- ImGui 기반 편집 환경을 먼저 확장하고, 플레이 입력과 상호작용은 이후 추가한다.
- 한 번에 지나치게 작은 문법 한 줄보다 사용자가 검토할 수 있는 작은 기능 단위로 진행한다.

## 핵심 아키텍처 원칙

- 하위 시스템은 상위 시스템을 알지 않는다.
- `Renderer`는 `Scene`, `GameObject`, `Transform`, `Camera`, ImGui를 포함하거나 참조하지 않는다.
- `Scene`과 `GameObject`는 DirectX 11 렌더링 과정이나 ImGui를 알지 않는다.
- `Transform`은 GPU/HLSL의 행렬 전치 및 Constant Buffer 저장 규칙을 알지 않는다.
- `GraphicsDevice`와 `GraphicsResourceManager`는 Renderer, Scene, AppBase를 알지 않는다.
- `AppBase`가 상위 조정 계층으로서 시스템 초기화, ImGui 편집과 Scene→RenderItem 변환을 담당한다.
- 오브젝트 단위 제출 경계는 `RenderItem`, 프레임 단위 제출 경계는 `FrameRenderData`를 사용한다.
- 리소스의 실제 소유권과 비소유 참조를 명확히 구분한다.
- 최종 범용 ECS, 과도한 인터페이스 계층, 사용처가 없는 추상화를 미리 만들지 않는다.
- 현재 목표 기능을 향해 진행한 뒤 실제 중복과 변경 압력이 확인될 때 구조를 확장한다.

## 현재 그래픽 계층의 책임

### GraphicsDevice

- `ID3D11Device`, `ID3D11DeviceContext`, SwapChain을 소유한다.
- 기본 BackBuffer RTV와 기본 Depth Texture/DSV를 소유한다.
- Window 크기 변경에 따른 SwapChain Buffer, RTV와 Depth Buffer 재생성을 담당한다.
- Present를 담당한다.
- 현재는 Device와 Window Surface 책임을 함께 가진 과도기 구조다.
- Shadow Map, HDR, Bloom 단계에서 필요성이 분명해지면 `SwapChainSurface` 또는 `WindowRenderSurface` 분리를 검토하되 지금 미리 분리하지 않는다.

### GraphicsResourceManager

- 일반 GPU Buffer의 실제 `ComPtr<ID3D11Buffer>`와 일반 Texture2D/SRV를 단독 소유한다.
- Constant/Vertex/Index Buffer 생성·조회·업데이트와 파일 Texture 생성·SRV 조회를 담당한다.
- 외부에는 `BufferHandle`과 `TextureHandle`을 발급하고, 즉시 바인딩용 raw pointer를 비소유 대여한다.
- Camera, Light, Material 의미, 셰이더 슬롯과 Draw 순서를 모른다.
- 초기 구현은 개별 삭제, 슬롯 재사용과 generation을 지원하지 않는다.
- `D3D11Utils`가 실제 DX11 Buffer/Texture 생성과 Dynamic Buffer Map/Unmap 절차를 수행하고, ResourceManager는 검증·소유·메타데이터·Handle 발급을 담당한다.

### Renderer

- `GraphicsDevice`와 `GraphicsResourceManager`를 비소유 포인터로 참조한다.
- Shader/InputLayout, Rasterizer/DepthStencil/Sampler State를 소유한다.
- Camera/Light/Object/Material Constant Buffer의 `BufferHandle`을 가진다.
- Constant Buffer의 갱신 시점, Shader Slot, Pipeline Binding과 DrawIndexed 순서를 결정한다.
- `BeginFrame`은 Camera/Light처럼 프레임 단위 데이터를 처리한다.
- `DrawRenderItem`은 Object/Material처럼 Draw 단위 데이터를 처리한다.
- `EndFrame`은 GraphicsDevice에 Present를 요청한다.

### Scene과 Asset 계층

- `Scene`은 `vector<unique_ptr<GameObject>>`로 GameObject를 소유한다.
- `GameObject`는 이름, Transform, MeshComponent와 ModelComponent를 값으로 소유한다.
- `MeshComponent`는 Mesh와 Material을 비소유 포인터로 참조한다.
- `ModelComponent`는 AssetManager가 소유한 Model을 `const Model*`로 비소유 참조한다.
- `Mesh`는 Vertex/Index `BufferHandle`, Vertex Stride와 Index Count를 가진다.
- `Texture`는 Texture2D/SRV를 직접 소유하지 않고 `TextureHandle`만 가진다.
- `Material`은 Texture를 비소유 참조하고 BaseColor를 가진다.
- `Model`은 여러 `ModelPart`를 소유하며 각 Part는 Mesh와 Material을 비소유 참조한다.
- `ModelLoader`는 Assimp 데이터를 프로젝트 독립 CPU 데이터인 `ModelData`로 변환하고 GPU 자원을 생성하지 않는다.
- `AssetManager`는 문자열 key 또는 파일 경로 기반으로 `Mesh`, `Texture`, `Material`, `Model` 논리 에셋을 `unique_ptr`로 소유하고 캐싱한다.
- `AssetManager::LoadModel()`은 ModelData를 받아 Mesh/Material/Texture를 생성하고 ModelPart를 조립한다.
- 같은 Texture 경로, Mesh/Material key 또는 Model 파일 경로를 다시 요청하면 기존 객체 주소를 반환한다. 현재 경로 정규화는 지원하지 않는다.
- Constant Buffer, Shadow Map, Bloom Render Target은 논리 에셋이 아니라 그래픽 런타임 리소스다.

## 현재 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice
│  ├─ Device / Context / SwapChain
│  ├─ 기본 RTV / Depth Texture / DSV
│  ├─ Resize
│  └─ Present
├─ GraphicsResourceManager --비소유--> GraphicsDevice
│  ├─ BufferResource[]
│  │  └─ 실제 Constant / Vertex / Index Buffer ComPtr
│  └─ TextureResource[]
│     └─ 실제 Texture2D / SRV ComPtr
├─ AssetManager --비소유--> GraphicsResourceManager
│  ├─ 문자열 key → unique_ptr<Mesh>
│  ├─ 문자열 경로 → unique_ptr<Texture>
│  ├─ 문자열 key → unique_ptr<Material>
│  └─ 파일 경로 → unique_ptr<Model>
│     └─ ModelPart[] --비소유--> Mesh / Material
├─ Renderer --비소유--> GraphicsDevice, GraphicsResourceManager
│  ├─ Shader / InputLayout
│  ├─ Rasterizer / DepthStencil / Sampler State
│  └─ Camera / Light / Object / Material BufferHandle
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     ├─ MeshComponent --비소유--> Mesh / Material
│     └─ ModelComponent --비소유--> Model
└─ Camera / DirectionalLight

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
Renderer / Mesh --BufferHandle--> GraphicsResourceManager
Renderer / Texture --TextureHandle--> GraphicsResourceManager
```

AppBase 멤버는 `GraphicsDevice → GraphicsResourceManager → AssetManager → Renderer` 순서로 선언돼 있다. C++ 멤버는 역순으로 파괴되므로 Renderer와 AssetManager가 먼저 소멸하고 GraphicsResourceManager와 GraphicsDevice가 뒤에 소멸한다.

## BufferHandle과 ResourceManager 규칙

- `BufferHandle`은 실제 DirectX 포인터나 `ComPtr`를 담지 않는 값 타입 식별자다.
- 기본 생성 Handle은 무효 인덱스를 가지며 `IsValid()`가 false다.
- 정상 Handle은 ResourceManager 내부 Buffer 저장소의 인덱스를 가진다.
- Handle은 생성, 업데이트, 삭제, 바인딩과 소유권 관리를 수행하지 않는다.
- ResourceManager가 실제 Buffer의 생성, 소유, 조회, 업데이트와 소멸을 담당한다.
- ResourceManager가 반환한 raw DirectX 포인터는 즉시 바인딩할 때만 사용하며 외부에서 `Release()`하거나 장기간 보관하지 않는다.
- Handle이 `IsValid()`여도 임의로 큰 인덱스일 수 있으므로 ResourceManager 조회 시 배열 범위를 반드시 검사한다.
- `TextureHandle`도 같은 index 기반 값 타입 규칙을 따르며, 실제 Texture2D/SRV를 소유하지 않는다.
- 초기 구현에서는 리소스를 배열에서 개별 삭제하지 않으므로 index만 사용한다. 삭제 및 슬롯 재사용이 필요해질 때 `index + generation`을 도입한다.
- 초기 구현에 `shared_ptr`, `weak_ptr`, free list나 범용 리소스 계층을 미리 넣지 않는다.

## 현재 프레임 흐름

1. Win32 메시지를 처리하고 `WM_SIZE`에서 GraphicsDevice Resize를 요청한다.
2. GameTimer를 Tick한다.
3. ImGui 프레임을 시작하고 Scene/Camera/Light/Transform/Material을 편집한다.
4. ImGui 폭을 제외한 Scene View 크기로 Camera Aspect Ratio와 Renderer Viewport를 갱신한다.
5. AppBase가 Camera와 DirectionalLight로 `FrameRenderData`를 만든다.
6. `Renderer::BeginFrame()`이 RTV/DSV를 Clear하고 Camera/Light Constant Buffer를 갱신한다.
7. AppBase가 Scene의 GameObject를 순회한다. ModelComponent가 있으면 각 ModelPart를, 아니면 MeshComponent를 `RenderItem`으로 변환한다.
8. `Renderer::DrawRenderItem()`이 Object/Material Constant Buffer를 갱신한다.
9. Renderer가 ResourceManager에서 Vertex/Index/Constant Buffer raw pointer를 대여한다.
10. Mesh Buffer, Texture SRV, Shader, Constant Buffer와 Sampler를 바인딩하고 `DrawIndexed()`를 호출한다.
11. ImGui DrawData를 렌더링한다.
12. `Renderer::EndFrame()`이 GraphicsDevice에 Present를 요청한다.

## 현재 완료된 주요 기능

- Win32 Window와 메시지 루프
- DX11 Device/Context/SwapChain과 기본 RTV/DSV
- Resize 시 BackBuffer/Depth 재생성
- ImGui 패널 폭을 제외한 Scene Viewport
- GameTimer와 FPS 표시
- Cube/Triangle Indexed Drawing
- Scene/GameObject/Transform/MeshComponent
- 여러 GameObject 선택 및 독립 Transform 편집
- Camera Position, Yaw/Pitch, FOV, Aspect Ratio
- Directional Light와 기본 Normal 조명
- Texture2D/SRV, Sampler와 Material BaseColor
- `FrameRenderData`와 `RenderItem` 제출 경계
- GraphicsDevice 분리
- BufferHandle과 GraphicsResourceManager
- Camera/Light/Object/Material Constant Buffer의 Handle 이관
- Mesh Vertex/Index Buffer의 Handle 이관
- D3D11Utils의 Immutable/Constant Buffer 생성과 Dynamic Buffer 갱신 공통화
- TextureHandle과 GraphicsResourceManager의 Texture2D/SRV 소유
- Texture의 직접 ComPtr 제거와 Handle 전환
- Renderer의 TextureHandle → ResourceManager::GetSRV 바인딩
- AssetManager의 Texture 경로 캐시와 Mesh key 캐시
- AssetManager의 mutable Material key 캐시
- Assimp 기반 ModelLoader와 독립 CPU `ModelData`
- `Model`, `ModelPart`, `ModelComponent`와 Model 파일 경로 캐시
- FBX Zelda, OBJ Pikachu, glTF Dragonite의 다중 Mesh/Material/BaseColor Texture 렌더링
- GameObject Transform 하나로 Model의 모든 Part를 함께 배치하고 조절하는 흐름
- TensorWorks UE-Clang-Format 기반 `.clang-format`
- AppBase의 직접 Mesh/Texture 소유 제거

## 현재 알려진 점검 항목

- AppBase Render 순회는 Mesh와 Material이 모두 있는 오브젝트만 RenderItem으로 변환한다. 추후 기본 Material/Texture 정책이 필요해지면 별도로 도입한다.
- Renderer의 메서드는 GraphicsDevice뿐 아니라 ResourceManager 유효 조건도 명시적으로 확인하는 방향을 검토한다.
- Input Layout Offset의 숫자 계산은 `Vertex` 멤버 위치를 직접 표현하는 방식으로 개선할 수 있다.
- GeometryGenerator의 Position/Color/Normal/UV 평행 배열은 Vertex 직접 구성 또는 면 추가 helper로 중복과 불일치 위험을 줄일 수 있다.
- Vertex Color는 현재 Shader 입출력을 통과하지만 최종 Pixel Color 계산에는 사용되지 않는다. 실제 사용하거나 제거할지 결정한다.
- GraphicsResourceManager의 공개 `UpdateBuffer`는 이름은 범용이지만 현재 16-byte 정렬을 강제해 사실상 Constant Buffer 갱신 정책이다. 실제로 다른 Dynamic Buffer가 추가될 때 API 이름과 범위를 재검토한다.
- D3D11Utils의 타입 기반 Buffer 편의 오버로드는 모든 생성 호출이 ResourceManager를 거치게 되면 공개 필요성을 실제 호출 기준으로 재검토한다.
- AssetManager의 key와 Model/Texture 경로는 현재 입력 문자열 그대로 사용한다. 동일 파일도 경로 표기가 다르면 별도 에셋이 될 수 있다.
- ModelLoader는 현재 `pScene->mMeshes`를 평면 순회하므로 `aiNode` 계층 Transform과 Mesh Instance를 반영하지 않는다. 실제 골목 에셋에서 필요성이 확인될 때 확장한다.
- Material과 ModelLoader는 현재 BaseColor/Diffuse Texture만 처리한다. Normal Map, Metallic/Roughness와 glTF PBR 확장은 젖은 바닥과 Material 확장 단계에서 진행한다.
- Pixel Shader의 Ambient Strength는 현재 `0.4f`로 하드코딩돼 있다. 정식 조명 데이터와 ImGui로 연결하는 것이 바로 다음 작업이다.
- ImGui 부분 초기화 실패와 Shutdown 상태 추적은 기능 우선순위에 따라 나중에 보완한다.
- 일부 한글 주석의 문자 인코딩이 깨져 있으므로 기능 변경과 분리해 UTF-8 정책을 정리한다.

## 바로 다음 우선 작업

다음 기능 단위는 Pixel Shader에 하드코딩된 Ambient Strength를 정식 조명 데이터로 연결하는 것이다.

진행 방향:

1. `DirectionalLight`에 `ambientStrength`를 추가한다.
2. CPU `LightConstantData`의 기존 `pad`를 `ambientStrength`로 교체해 32바이트 배치를 유지한다.
3. `Renderer::BeginFrame()`에서 FrameRenderData의 값을 Light Constant Buffer에 복사한다.
4. HLSL cbuffer의 `pad`를 `ambientStrength`로 바꾸고 지역 하드코딩 값을 제거한다.
5. ImGui Light 영역에 Ambient Strength 슬라이더를 추가한다.
6. Pikachu와 Dragonite에서 직접광과 주변광이 독립적으로 조절되는지 사용자가 실행 확인한다.

이 작업이 끝나면 STEP 3의 Editor/Play 상태와 1인칭 WASD/마우스 입력으로 진입한다. ModelLoader의 aiNode/PBR 확장은 당장 진행하지 않는다.

## 이후 주요 로드맵

1. DirectionalLight Ambient Strength 데이터/ImGui 연결
2. Editor/Play 상태와 1인칭 WASD/마우스 입력
3. ImGui Scene Hierarchy/Inspector와 배치 기능 확장
4. 사이버펑크 골목 Greybox와 최소 상호작용 흐름
5. 다수 Point Light와 네온 조명
6. 실제 골목 콘텐츠 구성
7. Shadow Mapping
8. 젖은 바닥 재질과 반사
9. HDR Scene Target과 Bloom
10. 안개와 색조 보정
11. 문/전원 스위치 상호작용과 순차 점등
12. 디버그 UI, 최적화와 1~2분 최종 연출

## 작업 절차

1. 저장소 루트의 이 파일과 `MyPF/docs/CODEX_HANDOFF.md`를 먼저 읽는다.
2. 현재 소스, `git status`와 최근 변경을 확인하며 문서보다 실제 코드를 우선한다.
3. 이번 단계의 목표, 필요한 이유, 책임 경계와 완료 조건을 먼저 설명한다.
4. 사용자가 직접 C++/HLSL을 구현한다.
5. Codex가 실제 변경 파일과 의존성 경계를 다시 읽고 리뷰한다.
6. 빌드/컴파일/실행은 사용자가 요청한 경우에만 수행한다.
7. 결과와 남은 문제를 정리한 뒤 다음 기능 단위를 제안한다.
8. 기능 또는 구조가 의미 있게 달라졌다면 `MyPF/docs/CODEX_HANDOFF.md`를 갱신한다.

## 빌드 및 실행 정보

- 노트북 저장소 루트: `C:/Users/Diguedman/source/repos/MyPF`
- 데스크톱 저장소 루트: `C:/Users/tls15/source/repos/MyPF`
- 솔루션: `MyPF/MyPF.sln`
- 프로젝트 디렉터리: `MyPF/`
- 기본 검증 구성: `Debug | x64`
- HLSL 파일: `MyPF/Shaders/`
- 셰이더는 `D3DCompileFromFile`로 런타임 컴파일한다.
- 실행 시 `Shaders\\simpleVertexShader.hlsl` 같은 상대 경로가 해석되므로 작업 디렉터리를 `MyPF/`로 맞춘다.

## 변경 안전성

- 사용자가 작성한 기존 변경사항을 보존한다.
- 요청받지 않은 소스 파일을 수정하거나 삭제하지 않는다.
- `.vs`, `x64`, `CodexBuild*`, `CodexCompile*`, 실행 중 변경된 `imgui.ini` 같은 로컬 산출물을 커밋하지 않는다.
- 사용자가 요청하지 않는 한 Git commit, push, pull, branch 변경을 수행하지 않는다.
- 빌드나 실행을 위해 사용자의 기존 산출물 폴더를 지우지 않는다.
- 코드와 문서가 다르면 코드를 기준으로 문서를 고친다.
