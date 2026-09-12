# MyPF 작업 지침

마지막 갱신: 2026-09-13

이 문서가 MyPF 프로젝트의 단일 진실 원본이다. 사용하는 도구(Claude Code, Codex 등)와 무관하게 적용된다.

완료 기능 목록, 진행률, 작업 로그, 설계 결정 상세는 `MyPF/docs/CODEX_HANDOFF.md`에 있다. 그 문서는 자동 로드하지 않으며 필요할 때 읽는다.

## 1. 프로젝트 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하다 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜진다.

최종 결과물은 범용 엔진 자체가 아니라 `DX11 렌더링 기술 + 확장 가능한 구조 + 짧지만 완결된 플레이 경험`을 보여주는 포트폴리오다.

남은 목표 기능: 그림자, 젖은 바닥 반사, HDR Scene Target·Bloom·Tone Mapping, 안개·비·색조 보정, 충돌과 이동 제한.

현재 전체 진행률은 약 65%다. 기반 렌더링, GPU Resource 소유, Asset/Model 파이프라인, Editor/Play와 1인칭 조작, Greybox 골목, PowerSwitch, 다중 Point Light와 Emissive 순차 점등까지 완료했다.

**바로 다음 기능 책임:** 남은 Point Light의 역할 분류(네온 연동 조명 / 환경 조명)와 실제 네온 배치 정리. 그다음이 Shadow Mapping이다. 상세 진행 방향은 CODEX_HANDOFF.md 참고.

## 2. 에이전트의 역할 — 가장 중요한 규칙

**이 프로젝트의 목적은 사용자가 DX11 렌더러를 직접 만드는 것이다. 에이전트가 코드를 대신 작성하면 프로젝트의 의미가 사라진다.**

### 하는 일

- 솔루션을 읽고 현재 구조와 책임 분포를 분석한다.
- 목표 씬에 필요한 기능을 **한 번에 검증 가능한 단위로 쪼갠다.**
- 각 단계마다 아래 형식으로 설계 가이드를 제공한다.
- 사용자가 "수정 완료"라고 하면 실제 작업 트리를 다시 읽고 리뷰한다.

### 하지 않는 일

- 사용자의 명시적 요청 없이 `.cpp` / `.h` / `.hlsl`을 생성하거나 수정하지 않는다.
- 사용자가 막혔다고 명시하고 요청하기 전에는 완성 코드 블록을 제시하지 않는다.
- 코드가 잘못됐거나 불완전해도 즉시 고치지 않는다. 놓친 개념·조건·영향·수정 방향만 설명하고 사용자가 다시 작성하도록 유도한다.
- 사용자가 요청할 때만 빌드하고 실행한다. commit, push, pull, branch 변경은 하지 않는다.

문서, 작업 인계 파일, 빌드 설정은 사용자가 수정을 요청한 경우 에이전트가 직접 수정할 수 있다.

### 기능 하나를 안내할 때의 형식

1. **왜 필요한가** — 최종 데모에 어떤 기여를 하는지. 이 기능이 없으면 무엇을 표현할 수 없는지.
2. **배경 개념** — 이 기능이 기대는 그래픽스/C++ 개념을 먼저 설명한다. 용어만 던지지 않는다.
3. **책임 경계** — 어느 클래스가 무엇을 알아야 하고 무엇을 몰라야 하는지. §3의 계층 원칙을 어디에 적용하는지.
4. **단계 분할** — 각 단계는 독립적으로 빌드되고 화면이나 값으로 확인 가능해야 한다.
5. **완료 조건** — 단계마다 "화면에 무엇이 보이면 / 어떤 값이 나오면 성공인가"를 문장으로 정의한다.
6. **첫 힌트 하나** — 사용자가 스스로 설계할 수 있는 수준까지만. 다음 힌트는 사용자가 요청할 때 제공한다.
7. **스스로 디버깅하는 법** — 이 단계에서 틀렸을 때 어디를 먼저 의심하고 무엇으로 확인하는가. (RenderDoc / VS Graphics Debugger 캡처, `OutputDebugStringW`, 상수 버퍼 값 덤프, 셰이더 출력을 색으로 시각화 등 이 단계에 맞는 수단을 지정한다.)

작업은 한 줄씩 지나치게 잘게 나누지 않고, 리뷰 가능한 **하나의 기능 책임 단위**로 안내한다.

## 3. 핵심 아키텍처 원칙

**하위 시스템은 상위 시스템을 알지 않는다.** 이 규칙이 이 프로젝트 구조의 핵심이다.

- `Renderer`는 `Scene`, `GameObject`, `Transform`, `Camera`, `Model`과 ImGui를 모른다.
- `Scene`과 `GameObject`는 DirectX 11 렌더링 과정이나 ImGui를 모른다.
- `Transform`은 GPU/HLSL의 행렬 전치와 Constant Buffer 저장 규칙을 모른다.
- `GraphicsDevice`와 `GraphicsResourceManager`는 `Renderer`, `AssetManager`, `Scene`, `AppBase`를 모른다.
- `ModelLoader`는 GPU 계층을 모르며 Assimp 데이터를 독립 CPU `ModelData`로 변환한다.
- `AppBase`가 시스템 초기화, Play 입력과 Scene→RenderItem 변환을 조정하고, `EditorUI`가 Editor ImGui 패널을 담당한다.

제출 경계는 두 개뿐이다. 오브젝트 단위는 `RenderItem`, 프레임 단위는 `FrameRenderData`.

리소스의 **실제 소유권과 비소유 참조를 항상 명확히 구분한다.**

범용 ECS, 과도한 인터페이스, 사용처가 없는 추상화를 미리 만들지 않는다. 목표 기능을 진행한 뒤 실제 중복과 변경 압력이 확인될 때 구조를 확장한다.

단, `BufferHandle` / `TextureHandle` / `RenderItem` / `FrameRenderData`는 얇아 보여도 **의도된 계층 경계**다. "단일 사용 추상화"로 판단해 걷어내지 않는다.

## 4. 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice                          Device/Context/SwapChain/RTV/DSV 소유
├─ GraphicsResourceManager --비소유--> GraphicsDevice
│  ├─ BufferResource[]  → ID3D11Buffer ComPtr
│  └─ TextureResource[] → Texture2D / SRV ComPtr
├─ AssetManager --비소유--> GraphicsResourceManager
│  └─ unique_ptr<Mesh / Texture / Material / Model> + 이름 캐시
├─ Renderer --비소유--> GraphicsDevice / GraphicsResourceManager
├─ Scene
│  └─ vector<unique_ptr<GameObject>>
│     ├─ Transform
│     ├─ MeshComponent        --비소유--> Mesh / Material
│     ├─ ModelComponent       --비소유--> Model
│     └─ PointLightComponent  (Has 플래그로 선택적)
├─ InputSystem
├─ Camera / EditorCameraSnapshot / FirstPersonCameraController
├─ PowerSwitch        --비소유--> Scene 소유 GameObject / Material
├─ PointLightSequence --비소유--> Scene 소유 GameObject / 등록된 Material
├─ DirectionalLight
└─ EditorUI --비소유--> Scene / Camera / CameraController / DirectionalLight / BackgroundColor

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
```

`AppBase` 멤버 선언의 **역순으로 파괴**되므로 Scene/Renderer가 먼저 소멸하고 AssetManager → GraphicsResourceManager → GraphicsDevice가 뒤에 소멸한다. 참조하는 쪽이 항상 먼저 죽는다. **멤버 선언 순서를 바꾸지 않는다.**

## 5. 시스템 책임

| 시스템 | 소유하는 것 | 모르는 것 |
|---|---|---|
| `GraphicsDevice` | Device, Context, SwapChain, 기본 RTV/Depth Texture/DSV. Resize와 Present | 그 위 전부 |
| `GraphicsResourceManager` | 실제 Buffer/Texture2D/SRV ComPtr. 생성·조회·업데이트 | Camera/Light/Material의 의미, Draw 순서 |
| `Renderer` | Shader/InputLayout, Rasterizer/DepthStencil/Sampler State, 4개 Constant Buffer Handle | Scene, GameObject, Camera 클래스, ImGui |
| `AssetManager` | Mesh/Texture/Material/Model의 `unique_ptr`와 이름 캐시 | Scene, Renderer |
| `Scene` | `vector<unique_ptr<GameObject>>` | DX11, ImGui |
| `InputSystem` | key state, 단발 입력(`WasKeyPressed`), 누적 MouseDelta | Camera, Scene |
| `Camera` | Position, +Z Forward, Up, Yaw/Pitch, FOV, Aspect, View/Projection | 입력 장치, 렌더링 |
| `EditorUI` | Editor ImGui 패널 구성과 선택 상태 | Renderer, GPU |

외부에는 `BufferHandle` / `TextureHandle`과 즉시 바인딩용 raw pointer만 **비소유 대여**한다.

## 6. 현재 프레임 흐름

1. Win32 메시지를 `InputSystem`과 `AppBase::MsgProc`가 처리한다. Resize와 focus 상실을 반영한다.
2. `GameTimer::Tick()`으로 deltaTime을 계산한다.
3. ImGui 프레임을 시작하고 `UpdateGui()`를 호출한다. Editor 모드는 `EditorUI::Draw`, Play 모드는 `AppBase::DrawPlayPanel`.
4. Scene View 크기로 Camera Aspect와 Renderer Viewport를 갱신한다.
5. Editor 모드는 마우스 우클릭을 누르고 있는 동안만 `FirstPersonCameraController`가 WASD 이동과 MouseDelta 회전을 적용한다(`AppBase::UpdateEditorCamera`). Play 모드는 ESC 확인 후 매 프레임 항상 적용한다.
6. `WasKeyPressed('E')`가 참이면 `PowerSwitch`가 Ray-BoundingBox 교차로 상호작용 가능 여부를 판정하고 전원 상태와 스위치 색을 반전한다.
7. `PointLightSequence`가 새 목표를 받고, 누적 시간에 따라 등록된 `SequenceEntry`의 Point Light와 Emissive Material을 한 단계씩 함께 켜거나 역순으로 끈다.
8. 커서를 Scene View 중앙으로 되돌린다.
9. `AppBase`가 Camera, DirectionalLight, `Scene::GatherPointLights` 결과로 `FrameRenderData`를 만든다.
10. `Renderer::BeginFrame`이 RTV/DSV를 Clear하고 CPU 조명을 GPU 상수 데이터로 변환해 Camera/Light Constant Buffer를 갱신한다.
11. `AppBase`가 Scene을 순회해 `ModelPart` 또는 `MeshComponent`를 `RenderItem`으로 변환하고, `Renderer::DrawRenderItem`이 Object/Material Buffer와 Mesh/Texture를 바인딩해 `DrawIndexed`한다.
12. ImGui DrawData를 렌더링하고 `Renderer::EndFrame`이 Present한다. `InputSystem::EndFrame`이 단발 키 상태를 초기화한다.

## 7. 코드를 읽어도 알 수 없는 함정

새 기능을 안내하기 전에 이 목록을 확인한다. (전체 목록은 CODEX_HANDOFF.md)

- **인코딩**: 과거 일부 파일의 한글 주석이 CP949로 저장돼 깨져 있었으나, 전체 소스를 UTF-8 BOM으로 통일하는 커밋(`5a5f463`)으로 해결됐다. 새로 쓰는 파일도 **UTF-8 with BOM**으로 저장한다.
- **셰이더 경로**: `L"Shaders\\simpleVertexShader.hlsl"` 상대 경로라 **작업 디렉터리가 `MyPF/`여야** 실행된다. exe를 직접 실행하면 실패한다.
- **LDR 클리핑**: Pixel Shader 마지막 `saturate` 때문에 1을 넘는 Emissive Intensity가 잘린다. 현재 값 3, 8은 화면에서 구분되지 않는다. HDR Scene Target 단계에서 해결한다.
- **에러 정책**: `Renderer::DrawRenderItem`은 Albedo Texture가 없으면 `false`를 반환하고, `AppBase::Render`가 이를 받아 `PostQuitMessage(-1)`로 앱을 종료한다. 기본 Material/Texture 정책이 없다.
- **`PointLightSequence` 전제**: 자신만 조명 활성 상태를 바꾼다고 가정한다. 다른 코드가 직접 `SetEnabled`를 호출하면 내부 개수와 실제 상태가 어긋난다.
- **`SequenceEntry` 수명**: Material을 비소유 포인터로 참조한다. 현재는 `AssetManager`가 수명을 보장한다는 전제를 쓴다.
- **`GraphicsResourceManager`**: append-only다. 개별 삭제, 슬롯 재사용, generation이 없다. 삭제를 도입하는 날 핸들에 generation을 같이 넣어야 한다.
- **단위 혼용**: `Camera`는 Yaw/Pitch를 도(degree)로 다루는데 `Transform`의 회전과 Inspector 라벨은 라디안이다.
- **`invTranspose` 계산**: `Renderer::DrawRenderItem`의 `Translation(Vector3(0))` 호출은 이미 전치된 행렬에서 아무 효과가 없다. 결과는 우연히 맞지만 의도와 코드가 다르다.
- **Vertex Color**: `Vertex`에 존재하고 VS가 PS로 넘기지만 최종 색에 사용되지 않는다.
- **테스트 없음**: 이 프로젝트에는 테스트 프레임워크가 없다. 검증은 `Debug | x64` 빌드 후 실행해 화면으로 확인하는 것뿐이다. "테스트를 써서 통과시켜라" 방식은 적용되지 않는다.

## 8. 빌드 및 실행 정보

- 노트북 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 데스크톱 저장소: `C:/Users/tls15/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git` / 브랜치 `main`
- 솔루션: `MyPF/MyPF.sln`
- 기본 검증 구성: `Debug | x64`
- 실행 작업 디렉터리: `MyPF/`
- C++20, HLSL Shader Model 5.0 런타임 컴파일
- Assimp는 양쪽 PC의 사용자 vcpkg 환경에 설치돼 있다
- `imgui.ini`, `.vs`, `x64` 등 로컬 산출물은 commit에서 제외한다

## 9. 코드 스타일

- 새로 작성하거나 직접 수정하는 코드에는 저장소의 `.clang-format`을 적용한다.
- 외부 코드(ImGui, stb, SimpleMath, Assimp)를 일괄 포맷하거나 수정하지 않는다.
- 대규모 Unreal식 타입/변수 이름 마이그레이션과 전면 스타일 정리는 기능 우선 결정으로 보류한다.

## 10. 변경 안전성

- 사용자 변경을 보존하고 요청받지 않은 소스를 수정하지 않는다.
- 빌드/실행을 위해 기존 산출물 폴더를 지우지 않는다.
- 코드와 문서가 다르면 **코드를 기준으로 문서를 고친다.**
- 기능이나 구조가 달라졌다면 이 파일과 `MyPF/docs/CODEX_HANDOFF.md`를 갱신한다.
- 외부 캐릭터 에셋은 테스트용이며 공개 전 라이선스를 확인한다.
