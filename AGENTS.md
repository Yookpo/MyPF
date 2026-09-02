# MyPF Codex 작업 지침

마지막 갱신: 2026-09-03

## 프로젝트 목표와 현재 위치

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 짧은 데모가 목표다.

최종 목표 기능:

- 1인칭 카메라와 WASD/마우스 조작
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 Point Light
- 전원 상태 변화와 순차 점등
- 그림자와 젖은 바닥 반사
- HDR Scene Target, Bloom과 Tone Mapping
- 안개, 비와 색조 보정
- ImGui 기반 씬 편집 및 디버그 UI

최종 결과물은 범용 엔진 자체가 아니라 `DX11 렌더링 기술 + 확장 가능한 구조 + 짧지만 완결된 플레이 경험`을 보여주는 포트폴리오다.

현재 전체 진행률은 약 60%다. 기반 렌더링, GPU Resource 소유, Asset/Model 파이프라인, Editor/Play와 1인칭 조작은 완료했다. 1m 단위 Greybox 골목, 컴포지션 기반 PowerSwitch, 스위치 On/Off 시각 변화, 최대 8개의 Point Light 제출과 거리 감쇠, 전원 상태에 따른 순차 점등까지 실행 확인했다. 다음 핵심 작업은 Emissive Material과 네온 간판 표현이다.

## Codex와 사용자의 역할

- 모든 C++와 HLSL 코드는 사용자가 직접 작성한다.
- Codex는 사용자의 명시적인 요청 없이 프로젝트 소스를 생성하거나 수정하지 않는다.
- 문서, 작업 인계 파일, 빌드 설정처럼 사용자가 직접 수정을 요청한 파일만 Codex가 수정할 수 있다.
- Codex는 새 기능의 목적, 배경지식, 책임 경계와 완료 조건을 먼저 설명한다.
- 기능을 구현할 때마다 왜 필요한지와 최종 데모에 어떤 기여를 하는지 먼저 설명한다.
- 사용자가 스스로 설계와 코드를 생각할 수 있도록 첫 힌트부터 단계적으로 제공한다.
- 사용자가 더 구체적인 도움이 필요하다고 할 때만 다음 힌트를 제공하고, 완성 코드는 사용자가 명시적으로 요청할 때만 보여준다.
- 작업은 한 줄씩 지나치게 잘게 나누지 않고 검토 가능한 하나의 기능 책임 단위로 안내한다.
- 코드가 잘못됐거나 불완전해도 즉시 완성 코드를 제시하지 않는다. 먼저 놓친 개념과 조건을 설명하고 사용자가 다시 작성하도록 유도한다.
- 사용자가 막혔다고 명시하고 완성 형태를 요청할 때만 직접적인 완성 코드를 제공한다.
- 사용자가 수정을 완료하면 실제 작업 트리를 다시 읽고 리뷰한다.
- 오류가 발견돼도 Codex가 임의로 고치지 않고 원인, 영향과 수정 방향을 설명한다.
- 빌드와 실행은 사용자가 명시적으로 요청할 때만 수행한다. 기본 검증 구성은 `Debug | x64`다.
- 현재는 최종 기능 목표를 우선한다. 대규모 Unreal식 타입/변수 이름 마이그레이션과 전면 스타일 정리는 보류한다.
- 새로 작성하거나 직접 수정하는 코드에는 저장소의 `.clang-format`을 적용하되 외부 코드를 일괄 포맷하지 않는다.

## 핵심 아키텍처 원칙

- 하위 시스템은 상위 시스템을 알지 않는다.
- `Renderer`는 `Scene`, `GameObject`, `Transform`, `Camera`, `Model`과 ImGui를 모른다.
- `Scene`과 `GameObject`는 DirectX 11 렌더링 과정이나 ImGui를 모른다.
- `Transform`은 GPU/HLSL의 행렬 전치와 Constant Buffer 저장 규칙을 모른다.
- `GraphicsDevice`와 `GraphicsResourceManager`는 Renderer, AssetManager, Scene과 AppBase를 모른다.
- `ModelLoader`는 GPU 계층을 모르며 Assimp 데이터를 독립 CPU `ModelData`로 변환한다.
- `AppBase`가 시스템 초기화, ImGui 편집, Play 입력과 Scene→RenderItem 변환을 조정한다.
- 오브젝트 단위 제출 경계는 `RenderItem`, 프레임 단위 제출 경계는 `FrameRenderData`를 사용한다.
- 리소스의 실제 소유권과 비소유 참조를 명확히 구분한다.
- 범용 ECS, 과도한 인터페이스, 사용처가 없는 추상화를 미리 만들지 않는다.
- 목표 기능을 진행한 뒤 실제 중복과 변경 압력이 확인될 때 구조를 확장한다.

## 현재 시스템 책임

### GraphicsDevice

- Device/Context/SwapChain과 기본 RTV/Depth Texture/DSV를 소유한다.
- Window Resize에 따른 SwapChain Buffer, RTV와 Depth 재생성을 담당한다.
- Present를 담당한다.

### GraphicsResourceManager

- 실제 Buffer와 Texture2D/SRV ComPtr를 단독 소유한다.
- Constant/Vertex/Index Buffer 생성·조회·업데이트와 파일 Texture 생성을 담당한다.
- 외부에는 BufferHandle/TextureHandle과 즉시 바인딩용 raw pointer를 비소유 대여한다.
- Camera, Light, Material 의미와 Draw 순서를 모른다.

### Renderer

- GraphicsDevice와 GraphicsResourceManager를 비소유 참조한다.
- Shader/InputLayout, Rasterizer/DepthStencil/Sampler State를 소유한다.
- Camera/Light/Object/Material Constant Buffer Handle을 가진다.
- BeginFrame은 Camera/DirectionalLight를, DrawRenderItem은 Object/Material을 처리한다.
- EndFrame은 GraphicsDevice에 Present를 요청한다.

### Scene과 Asset

- Scene은 `vector<unique_ptr<GameObject>>`로 GameObject를 소유한다.
- GameObject는 이름, Transform, MeshComponent와 ModelComponent를 값으로 소유한다.
- MeshComponent와 ModelComponent는 AssetManager 소유 Asset을 비소유 참조한다.
- Mesh/Texture는 실제 GPU 객체 대신 Handle을 가진다.
- AssetManager는 Mesh, Texture, Material과 Model을 unique_ptr로 소유하고 캐싱한다.
- Model은 ModelPart 배열을 소유하고 각 Part는 Mesh/Material을 비소유 참조한다.

### InputSystem과 Camera

- InputSystem은 Win32 메시지를 key state와 누적 MouseDelta로 변환한다.
- InputSystem은 `WasKeyPressed`와 프레임 종료 초기화로 키의 단발 입력을 제공한다.
- focus 상실, Play 진입과 종료에서 입력 상태를 Reset한다.
- Camera는 Position, +Z Forward, Up, Yaw/Pitch, FOV, Aspect와 View/Projection 계산을 담당한다.
- AppBase는 Play에서 deltaTime 기반 평면 WASD 이동과 마우스 회전을 적용한다.

## 현재 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice
├─ GraphicsResourceManager --비소유--> GraphicsDevice
│  ├─ BufferResource[] → Buffer ComPtr
│  └─ TextureResource[] → Texture2D / SRV ComPtr
├─ AssetManager --비소유--> GraphicsResourceManager
│  └─ unique_ptr<Mesh / Texture / Material / Model>
├─ Renderer --비소유--> GraphicsDevice / GraphicsResourceManager
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     ├─ MeshComponent --비소유--> Mesh / Material
│     └─ ModelComponent --비소유--> Model
├─ InputSystem
├─ Camera / EditorCameraSnapshot
├─ PowerSwitch --비소유--> Scene 소유 GameObject / Material
├─ PointLightSequence --비소유--> Scene
├─ Scene 소유 PointLight[]
└─ DirectionalLight

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
```

AppBase 멤버 선언과 역순 파괴에 따라 Scene/Renderer가 먼저 소멸하고 AssetManager, ResourceManager와 GraphicsDevice가 뒤에 소멸한다.

## 현재 프레임 흐름

1. Win32 메시지를 InputSystem과 AppBase가 처리한다.
2. focus를 잃으면 Play를 종료하고 입력 상태를 Reset한다.
3. GameTimer를 Tick하고 ImGui 프레임을 시작한다.
4. Editor/Play UI를 만들고 Scene View 크기로 Camera Aspect와 Renderer Viewport를 갱신한다.
5. Play이면 ESC, WASD와 MouseDelta로 Camera를 갱신한다.
6. E 단발 입력이 들어오면 PowerSwitch가 거리·시선을 검사하고 전원 상태와 스위치 색을 반전한다.
7. PointLightSequence가 새 목표를 받고, 매 프레임 누적 시간에 따라 Scene의 Point Light를 하나씩 켜거나 끈다.
8. 커서를 Scene View 중앙으로 되돌린다.
9. AppBase가 Camera, DirectionalLight와 Scene의 Point Light 목록으로 FrameRenderData를 만든다.
10. Renderer::BeginFrame이 CPU 조명을 GPU 상수 데이터로 변환해 Camera/Light Constant Buffer를 갱신한다.
11. AppBase가 Scene을 순회해 ModelPart 또는 MeshComponent를 RenderItem으로 변환한다.
12. Renderer가 Object/Material Buffer, Mesh와 Texture를 바인딩해 DrawIndexed한다.
13. ImGui DrawData를 렌더링하고 Present한다.

## 현재 완료된 주요 기능

- Win32 Window, 메시지 루프와 Resize
- DX11 Device/Context/SwapChain, RTV/DSV와 Indexed Drawing
- ImGui 패널을 제외한 Scene Viewport
- GameTimer, deltaTime과 FPS
- Scene/GameObject/Transform/MeshComponent/ModelComponent
- 여러 GameObject 선택과 독립 Transform 편집
- Camera Position, +Z Forward Yaw/Pitch, Pitch 제한, FOV와 Aspect
- Directional Light Direction/Color/Intensity/Ambient Strength의 CPU→GPU→HLSL→ImGui 연결
- Texture Albedo, Material BaseColor와 기본 Diffuse/Ambient
- FrameRenderData와 RenderItem 제출 경계
- GraphicsDevice, GraphicsResourceManager, BufferHandle과 TextureHandle
- AssetManager의 Mesh/Texture/Material/Model 소유와 캐시
- Assimp ModelLoader와 독립 CPU ModelData
- Zelda FBX, Pikachu OBJ, Dragonite glTF 다중 Mesh/Material/Texture 렌더링
- Editor/Play 상태와 ImGui Play/Stop
- InputSystem key state와 MouseDelta
- deltaTime 기반 WASD 평면 이동과 대각선 정규화
- 마우스 Yaw/Pitch, 커서 숨김/중앙 복귀
- ESC, Stop과 focus 상실 시 Play 종료
- Play 종료 시 Editor Camera Snapshot 복원
- `GeometryGenerator::MakeCube()`의 기본 크기를 1m 단위 Cube로 통일
- 폭 4m, 높이 4m, 길이 20m의 Greybox 골목과 1.6m 카메라 시작점
- 바닥과 좌우/끝 벽이 공용 Cube Mesh와 Greybox Material을 공유
- 끝 벽의 전원 스위치 GameObject와 독립 Material
- 상호작용 거리 2m와 시선 내적 임계값 0.8 판정
- `WasKeyPressed('E')`로 단발 상호작용 입력 처리
- PowerSwitch가 Scene 소유 GameObject를 비소유 참조하는 컴포지션 구조
- 전원 상태 On/Off 토글과 스위치 Material의 빨강/초록 시각 변화
- Scene이 Position, Range, Color, Intensity, Enabled 상태를 가진 Point Light 목록 소유
- FrameRenderData의 고정 배열과 개수로 최대 8개 Point Light 제출
- CPU bool을 GPU `uint32_t`로 변환하고 C++/HLSL 상수 버퍼 레이아웃 정렬
- `Lighting.hlsli`의 거리 감쇠와 Pixel Shader의 다중 Point Light 누적
- 서로 다른 색상의 Point Light 4개가 골목을 비추는 실행 결과 확인
- PointLightSequence가 1.4초 간격으로 생성 순서대로 켜고 역순으로 끄는 연출
- 점등 중 E키 재입력 시 현재 개수에서 목표 방향을 바꾸는 동작 확인

## 현재 알려진 문제와 보류 항목

- 활성 Scene은 Greybox 골목이며 외부 모델 파이프라인은 유지하지만 현재 장면에는 배치하지 않는다.
- AppBase에 Cube/Triangle 과거 테스트 코드가 주석 블록으로 남아 있다.
- 카메라 충돌과 플레이어 높이/월드 단위 정책이 없다.
- ImGui Transform 위치 `-1~1`, Scale `0.1~2` 범위는 골목 배치에 부족하다.
- AppBase가 시스템 초기화, Greybox 생성, UI, Play 입력, RenderItem 조립과 기능 객체 조율을 함께 담당해 여전히 크다. 실제 변경 압력이 생기는 책임부터 단계적으로 분리한다.
- PointLightSequence는 자신만 조명 활성 상태를 변경한다는 전제를 사용하므로 다른 코드가 `SetAllPointLightsEnabled`를 호출하면 내부 개수와 실제 상태가 어긋날 수 있다.
- Point Light 점등 순서는 별도 연출 목록이 아니라 Scene 생성 순서에 의존한다.
- 시퀀스는 한 프레임에 한 단계만 처리하므로 큰 프레임 지연 후 남은 시간을 즉시 따라잡지는 않는다.
- Renderer Material 경로는 유효한 Albedo Texture를 전제로 하며 기본 Material/Texture 정책이 없다.
- GeometryGenerator의 평행 배열은 데이터 불일치 위험이 있다.
- Vertex Color는 최종 Pixel Color에 사용되지 않는다.
- GraphicsResourceManager는 append-only이며 개별 삭제, 슬롯 재사용과 generation이 없다.
- AssetManager key/path 정규화가 없다.
- ModelLoader는 aiNode Transform/Instance를 반영하지 않는다.
- Material은 BaseColor/Diffuse만 지원하며 Emissive, Normal, Metallic/Roughness가 없다.
- Texture sRGB/Gamma, MipMap과 UV Tiling 정책이 없다.
- Pixel Shader 마지막 saturate는 HDR/Bloom 전에 HDR 경로로 옮겨야 한다.
- ImGui 부분 초기화 실패와 Shutdown 상태 추적은 아직 없다.
- 일부 한글 주석 인코딩이 깨져 있다.
- 전면 Epic 명명 마이그레이션은 기능 우선 결정으로 보류했다.
- 외부 캐릭터 에셋은 테스트용이며 공개 전 라이선스를 확인한다.
- `imgui.ini`, `.vs`, `x64` 등 로컬 산출물은 commit에서 제외한다.

## 바로 다음 우선 작업

다음 기능 책임은 `Emissive Material과 네온 표면 표현`이다.

목적:

- Point Light가 주변을 비추는 것과 별개로 네온 간판 표면 자체가 빛나는 색을 표현한다.
- Material의 발광 색과 밝기를 분리해 전원 상태와 연출에서 재사용한다.
- 이후 HDR Scene Target과 Bloom이 추출할 수 있는 밝은 발광 값을 준비한다.

진행 방향:

1. Material에 Emissive Color와 Emissive Intensity를 추가하고 기본값을 발광 없음으로 둔다.
2. MaterialConstantData의 C++/HLSL 레이아웃을 16바이트 경계에 맞춰 확장한다.
3. Renderer가 Material의 Emissive 값을 GPU 상수 버퍼로 전달한다.
4. Pixel Shader가 조명 결과에 Emissive를 더한다.
5. 테스트 네온 오브젝트를 배치해 조명이 없어도 표면이 밝게 보이는지 확인한다.
6. 전원 상태 및 PointLightSequence와 Emissive 점등을 연결할 책임 경계를 결정한다.

## 이후 주요 로드맵

1. ✅ 골목 Greybox와 월드 스케일 확정
2. ✅ 최소 상호작용 기반과 PowerSwitch 책임 분리
3. ✅ 전원 상태에 따른 스위치 시각 피드백
4. ✅ 다수 Point Light와 Renderer 제출
5. 🟡 Emissive Material과 네온 간판
6. 🟡 전원 장치 상태와 Point Light 순차 점등 완료, Emissive 연동 남음
7. 실제 골목 에셋 배치와 Scene 편집 보강
8. Shadow Mapping
9. Normal/Roughness Material과 젖은 바닥 반사
10. HDR Scene Target, Bloom과 Tone Mapping
11. 안개, 비와 색조 보정
12. 충돌/이동 제한, 디버그 UI와 최적화
13. 라이선스 정리와 1~2분 최종 연출

## 작업 절차

1. 저장소 루트의 이 파일과 `MyPF/docs/CODEX_HANDOFF.md`를 먼저 전부 읽는다.
2. 현재 소스, git status와 최근 commit을 확인하며 문서보다 실제 코드를 우선한다.
3. 목표, 필요한 이유, 책임 경계와 완료 조건을 먼저 설명한다.
4. 사용자가 직접 C++/HLSL을 구현한다.
5. Codex가 실제 변경 파일을 다시 읽고 리뷰한다.
6. 빌드/실행은 사용자가 요청한 경우에만 수행한다.
7. 기능이나 구조가 달라졌다면 이 파일과 CODEX_HANDOFF를 갱신한다.

## 빌드 및 실행 정보

- 노트북 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 데스크톱 저장소: `C:/Users/tls15/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 솔루션: `MyPF/MyPF.sln`
- 기본 검증: `Debug | x64`
- C++20, HLSL Shader Model 5.0 런타임 컴파일
- 실행 작업 디렉터리: `MyPF/`
- Assimp는 데스크톱과 노트북의 사용자 vcpkg 환경에 각각 설치돼 있다.
- 현재 문서 기준 HEAD: `ca6d15c` — `순차점등시퀀스 적용 완료`
- 작업 트리의 `MyPF/imgui.ini` 변경은 런타임 UI 배치이므로 기능 commit에서 제외한다.

## 변경 안전성

- 사용자 변경을 보존하고 요청받지 않은 소스를 수정하지 않는다.
- 사용자가 요청하지 않는 한 commit, push, pull과 branch 변경을 수행하지 않는다.
- 빌드/실행을 위해 기존 산출물 폴더를 지우지 않는다.
- 외부 ImGui, stb, SimpleMath와 Assimp 코드를 일괄 포맷하거나 수정하지 않는다.
- 코드와 문서가 다르면 코드를 기준으로 문서를 고친다.
