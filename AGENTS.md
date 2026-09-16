# MyPF 작업 지침

마지막 갱신: 2026-09-17

이 문서가 MyPF 프로젝트의 단일 진실 원본이다. 사용하는 도구(Claude Code, Codex 등)와 무관하게 적용된다.

완료 기능 목록, 진행률, 작업 로그, 설계 결정 상세는 `MyPF/docs/CODEX_HANDOFF.md`에 있다. 그 문서는 자동 로드하지 않으며 필요할 때 읽는다.

## 1. 프로젝트 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하다 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜진다.

최종 결과물은 범용 엔진 자체가 아니라 `DX11 렌더링 기술 + 확장 가능한 구조 + 짧지만 완결된 플레이 경험`을 보여주는 포트폴리오다.

남은 목표 기능: 그림자, 젖은 바닥 반사(Specular·Fresnel·Cube Map·IBL), 안개·비·색조 보정, 나머지 이동 제한.

현재 전체 진행률은 약 73%다. 기반 렌더링, GPU Resource 소유, Asset/Model 파이프라인, Editor/Play와 1인칭 조작, Greybox 골목과 플레이어-벽 충돌, PowerSwitch, 다중 Point Light와 Emissive 순차 점등, Rim Lighting, HDR 씬 타깃·Exposure·톤 매핑(Reinhard/ACES)·선형 색공간, Bloom(밝은 부분 추출 → 분리형 블러 → 합성)까지 완료했다.

**바로 다음 기능 책임:** 룩 재튜닝(Exposure·Threshold·Strength·Iterations·Ambient·네온 세기를 맞추고 코드 초기값에 반영)으로 로드맵 11번을 닫는다. 그다음 로드맵 10번(젖은 바닥 반사), 9번(Shadow Mapping) 순서다. 상세 진행 방향은 CODEX_HANDOFF.md 참고.

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

**색 공간 규칙:** 조명과 후처리 계산은 **선형 공간**에서 한다. 파일 텍스처는 sRGB 포맷으로 읽고, 사람이 고르는 색 상수(Material/Light/배경색)는 sRGB 값으로 두되 `Renderer`가 GPU에 올릴 때 선형으로 바꾼다. 모니터용 감마 인코드는 마지막 후처리(톤 매핑) 셰이더만 한다.

리소스의 **실제 소유권과 비소유 참조를 항상 명확히 구분한다.**

범용 ECS, 과도한 인터페이스, 사용처가 없는 추상화를 미리 만들지 않는다. 목표 기능을 진행한 뒤 실제 중복과 변경 압력이 확인될 때 구조를 확장한다.

단, `BufferHandle` / `TextureHandle` / `RenderItem` / `FrameRenderData`는 얇아 보여도 **의도된 계층 경계**다. "단일 사용 추상화"로 판단해 걷어내지 않는다.

## 4. 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice                          Device/Context/SwapChain/RTV/DSV 소유
├─ GraphicsResourceManager --비소유--> GraphicsDevice
│  ├─ BufferResource[]  → ID3D11Buffer ComPtr
│  └─ TextureResource[] → Texture2D / SRV / RTV ComPtr (+렌더 타깃 포맷)
├─ AssetManager --비소유--> GraphicsResourceManager
│  └─ unique_ptr<Mesh / Texture / Material / Model> + 이름 캐시
├─ Renderer --비소유--> GraphicsDevice / GraphicsResourceManager
│  └─ 렌더 타깃 TextureHandle 4개(HDR 씬 + Bloom Bright/BlurX/BlurY), Constant Buffer Handle 5개,
│     셰이더(씬·전체 화면 VS·복사·톤 매핑·밝은 부분 추출·BlurX·BlurY), 샘플러 2개(WRAP/CLAMP)
├─ Scene
│  └─ vector<unique_ptr<GameObject>>
│     ├─ Transform
│     ├─ MeshComponent        --비소유--> Mesh / Material
│     ├─ ModelComponent       --비소유--> Model
│     ├─ PointLightComponent  (Has 플래그로 선택적)
│     └─ BoxCollisionComponent (Has 플래그로 선택적, 모양은 Transform에서 유도)
├─ InputSystem
├─ Camera / EditorCameraSnapshot / FirstPersonCameraController
├─ PowerSwitch        --비소유--> Scene 소유 GameObject / Material
├─ PointLightSequence --비소유--> Scene 소유 GameObject / 등록된 Material
├─ DirectionalLight / PostProcessSettings (Exposure, ToneMapper)
└─ EditorUI --비소유--> Scene / Camera / CameraController / DirectionalLight / BackgroundColor / PostProcessSettings

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
AppBase --EndScene()-------> Renderer (HDR 씬 타깃 → 톤 매핑 → 백버퍼)
AppBase --Resize(w, h)-----> GraphicsDevice → Renderer (HDR 씬 타깃 재생성)
PlayerCollision / NeonSignFactory: 상태 없는 정적 함수, AppBase가 호출
```

`AppBase` 멤버 선언의 **역순으로 파괴**되므로 Scene/Renderer가 먼저 소멸하고 AssetManager → GraphicsResourceManager → GraphicsDevice가 뒤에 소멸한다. 참조하는 쪽이 항상 먼저 죽는다. **멤버 선언 순서를 바꾸지 않는다.**

## 5. 시스템 책임

| 시스템 | 소유하는 것 | 모르는 것 |
|---|---|---|
| `GraphicsDevice` | Device, Context, SwapChain, 기본 RTV/Depth Texture/DSV. Resize와 Present | 그 위 전부 |
| `GraphicsResourceManager` | 실제 Buffer/Texture2D/SRV/RTV ComPtr. 생성·조회·업데이트, 렌더 타깃 생성과 같은 슬롯 재생성 | Camera/Light/Material의 의미, Draw 순서, 렌더 타깃의 용도 |
| `Renderer` | Shader/InputLayout, Rasterizer/DepthStencil/Sampler State(WRAP·CLAMP), 5개 Constant Buffer Handle, 렌더 타깃 Handle 4개(포맷·크기 결정, Bloom은 화면 절반), 후처리 패스 체인(`EndScene`, `DrawFullScreenPass`), 색 상수 sRGB→선형 변환 | Scene, GameObject, Camera 클래스, ImGui |
| `AssetManager` | Mesh/Texture/Material/Model의 `unique_ptr`와 이름 캐시 | Scene, Renderer |
| `Scene` | `vector<unique_ptr<GameObject>>` | DX11, ImGui |
| `InputSystem` | key state, 단발 입력(`WasKeyPressed`), 누적 MouseDelta | Camera, Scene |
| `Camera` | Position, +Z Forward, Up, Yaw/Pitch, FOV, Aspect, View/Projection | 입력 장치, 렌더링 |
| `EditorUI` | Editor ImGui 패널 구성과 선택 상태 | Renderer, GPU |

외부에는 `BufferHandle` / `TextureHandle`과 즉시 바인딩용 raw pointer만 **비소유 대여**한다.

## 6. 현재 프레임 흐름

1. Win32 메시지를 `InputSystem`과 `AppBase::MsgProc`가 처리한다. Resize는 `GraphicsDevice::Resize` → `Renderer::Resize`(HDR 씬 타깃을 같은 슬롯에서 재생성) 순서로 반영하고, focus 상실도 반영한다.
2. `GameTimer::Tick()`으로 deltaTime을 계산한다.
3. ImGui 프레임을 시작하고 `UpdateGui()`를 호출한다. Editor 모드는 `EditorUI::Draw`, Play 모드는 `AppBase::DrawPlayPanel`.
4. Scene View 크기로 Camera Aspect와 Renderer Viewport를 갱신한다.
5. Editor 모드는 마우스 우클릭을 누르고 있는 동안만 `FirstPersonCameraController`가 WASD 이동과 MouseDelta 회전을 적용한다(`AppBase::UpdateEditorCamera`). Play 모드는 ESC 확인 후 매 프레임 항상 적용하고, `PlayerCollision::Resolve`가 `Scene::GatherBoxColliders` 결과로 카메라 위치를 보정한다.
6. Play 모드에서 `PowerSwitch::CanInteract`(Ray-BoundingBox 교차)가 참이면 스위치 Material의 Rim을 강조하고, `WasKeyPressed('E')`까지 참이면 전원 상태와 스위치 색을 반전한다.
7. `PointLightSequence`가 새 목표를 받고, 누적 시간에 따라 등록된 `SequenceEntry`의 Point Light와 Emissive Material을 한 단계씩 함께 켜거나 역순으로 끈다.
8. 커서를 Scene View 중앙으로 되돌린다.
9. `AppBase`가 Camera, DirectionalLight, `Scene::GatherPointLights` 결과, `PostProcessSettings`(Exposure, ToneMapper)로 `FrameRenderData`를 만든다.
10. `Renderer::BeginFrame`이 HDR 씬 타깃(`R16G16B16A16_FLOAT`)과 DSV를 Clear·바인딩하고, 색 상수를 선형으로 바꿔 Camera/Light/PostProcess Constant Buffer를 갱신한다. Light 버퍼는 매 프레임 PS `b0`에 다시 건다.
11. `AppBase`가 Scene을 순회해 `ModelPart` 또는 `MeshComponent`를 `RenderItem`으로 변환하고, `Renderer::DrawRenderItem`이 Object/Material Buffer와 Mesh/Texture를 바인딩해 `DrawIndexed`한다. 씬 셰이더는 1을 넘는 선형 HDR 값을 그대로 기록한다.
12. `Renderer::EndScene`이 후처리 패스를 순서대로 실행한다. 뷰포트를 Bloom 타깃 크기(화면 절반)로 바꿔 ① 밝은 부분 추출(HDR → Bright) ② BlurX·BlurY를 반복 횟수만큼(첫 회차만 Bright에서 읽고 이후 직전 결과에서 누적)을 돌린 뒤 화면 뷰포트로 되돌리고, ③ 백버퍼를 깊이 버퍼 없이 출력으로 바꿔 톤 매핑 셰이더가 HDR(`t0`)과 Bloom(`t1`)을 `hdr * exposure + bloom * strength`로 합친 뒤 Reinhard/ACES → `1/2.2` 감마 인코드를 적용한다. 디버그 뷰가 Final이 아니면 마지막 패스만 복사 셰이더로 해당 중간 타깃을 보여준다. 각 패스는 출력 교체 → 입력 바인딩 → `Draw(3, 0)` → 입력 해제 순서를 지킨다.
13. ImGui DrawData를 백버퍼에 렌더링하고 `Renderer::EndFrame`이 Present한다. `InputSystem::EndFrame`이 단발 키 상태를 초기화한다.

## 7. 코드를 읽어도 알 수 없는 함정

새 기능을 안내하기 전에 이 목록을 확인한다. (전체 목록은 CODEX_HANDOFF.md)

- **인코딩**: 과거 일부 파일의 한글 주석이 CP949로 저장돼 깨져 있었으나, 전체 소스를 UTF-8 BOM으로 통일하는 커밋(`5a5f463`)으로 해결됐다. 새로 쓰는 `.cpp`/`.h`도 **UTF-8 with BOM**으로 저장한다. 저장소 루트 `.editorconfig`가 VS에서 저장할 때 `.cpp`/`.h`는 UTF-8 with BOM, `.hlsl`/`.hlsli`는 BOM 없는 UTF-8을 강제한다. 이 설정이 없을 때 새 파일이 CP949로 저장되는 문제가 반복됐고, 이미 있는 파일은 저장할 때 바뀐다.
- **HLSL은 BOM 없는 UTF-8**: `5a5f463`이 `.hlsl`/`.hlsli`까지 UTF-8 BOM으로 바꿔버렸는데, HLSL 컴파일러(`fxc`/`D3DCompiler`)는 UTF-8 BOM을 인식하지 못하고 `error X3000: Illegal character in shader file`로 컴파일이 실패한다(2026-09-13 `fxc.exe`로 직접 재현·확인, `Shaders/*.hlsl`·`*.hlsli`를 BOM 없는 UTF-8로 다시 저장해 해결). `.cpp`/`.h`와 달리 `.hlsl`/`.hlsli`는 **BOM 없는 UTF-8**로 저장한다.
- **셰이더 경로**: `L"Shaders\\simpleVertexShader.hlsl"` 상대 경로라 **작업 디렉터리가 `MyPF/`여야** 실행된다. exe를 직접 실행하면 실패한다.
- **HDR 규칙**: 씬 셰이더는 1을 넘는 선형 값을 float HDR 씬 타깃에 그대로 쓴다. 씬 셰이더에 `saturate`를 다시 넣거나, 셰이더에서 텍스처·색 상수에 `pow(2.2)`를 추가하면 안 된다(이중 처리). 모니터용 감마 인코드는 톤 매핑 셰이더에서만 한다.
- **sRGB 텍스처 전제**: `D3D11Utils::CreateTexture`는 모든 파일 텍스처를 `R8G8B8A8_UNORM_SRGB`로 만든다. 노멀/러프니스 같은 데이터 텍스처를 추가하기 전에 sRGB 여부를 고르는 인자가 먼저 필요하다. 색 상수는 `Renderer::SrgbToLinear`를 거치고, 세기·거리·Exposure 같은 배율은 변환하지 않는다.
- **에러 정책**: `Renderer::DrawRenderItem`은 Albedo Texture가 없으면 `false`를 반환하고, `AppBase::Render`가 이를 받아 `PostQuitMessage(-1)`로 앱을 종료한다. 기본 Material/Texture 정책이 없다.
- **`PointLightSequence` 전제**: 자신만 조명 활성 상태를 바꾼다고 가정한다. 다른 코드가 직접 `SetEnabled`를 호출하면 내부 개수와 실제 상태가 어긋난다.
- **`SequenceEntry` 수명**: Material을 비소유 포인터로 참조한다. 현재는 `AssetManager`가 수명을 보장한다는 전제를 쓴다.
- **`GraphicsResourceManager`**: append-only다. 개별 삭제, 슬롯 재사용, generation이 없다. 삭제를 도입하는 날 핸들에 generation을 같이 넣어야 한다.
- **렌더 타깃 raw pointer**: `GetRTV`/`GetSRV`가 돌려준 포인터는 `Renderer::Resize` 후 무효다. 멤버에 저장하지 말고 바인딩 직전에 핸들로 조회한다. 렌더 타깃은 리사이즈 시 append가 아니라 같은 슬롯에서 재생성된다.
- **바인딩 규칙**: 같은 텍스처를 RTV와 SRV로 동시에 걸 수 없으므로 패스는 출력 교체 → 입력 바인딩 → Draw → 입력 해제 순서를 지킨다. 파이프라인은 상태 기계라 각 패스는 자기가 쓰는 샘플러·Input Layout·상수 버퍼를 직접 설정한다. 후처리가 PS `b0`를 쓰므로 `BeginFrame`의 Light 버퍼 `b0` 재바인딩을 지우면 두 번째 프레임부터 조명이 사라진다.
- **Rim Power 0 금지**: `pow(1 - N·V, rimPower)`에서 지수가 0이면 정면 픽셀이 `pow(0, 0)`(NaN)이 된다. Rim을 끌 때는 Intensity만 0으로 한다.
- **`Renderer::SetViewPort`는 `m_screenViewport` 멤버를 덮어쓴다.** 후처리 패스에서 이 함수를 부르면 복구할 화면 뷰포트가 사라져 이후 패스가 화면 일부에만 그려진다. Bloom처럼 타깃 크기가 다른 패스는 **지역 `D3D11_VIEWPORT` + `RSSetViewports`**를 쓰고, 구간이 끝나면 `m_screenViewport`로 되돌린다.
- **`ComPtr::operator&`는 Release한다.** `PSSetSamplers(0, 1, &m_clampSamplerState)`처럼 쓰면 들고 있던 객체가 해제된다(WRL이 `&`를 출력 파라미터용으로 설계했기 때문). 이미 들고 있는 객체를 넘길 때는 `.GetAddressOf()`, 함수에 전달할 때는 `.Get()`을 쓴다. 컴파일도 경고도 통과하므로 증상으로 찾기 어렵다.
- **중간 렌더 타깃은 Clear하지 않는다.** 전체 화면 패스가 매번 전부 덮어쓰기 때문인데, 그래서 **패스 순서가 틀리면 이전 프레임 내용을 읽는다.** 반복 블러의 첫 회차가 BlurY에서 읽으면 프레임 간 되먹임(잔상)이 생긴다. "이번 프레임에 이 텍스처를 누가 먼저 쓰는가"를 항상 확인한다.
- **바인딩한 슬롯 수만큼 해제한다.** 합성 패스가 `t0`·`t1` 두 장을 걸므로 해제도 두 슬롯이어야 한다. `t1`이 걸린 채 다음 프레임이 시작되면 그 텍스처를 출력으로 걸 때 충돌한다.
- **RenderDoc은 빌드하지 않는다**: RenderDoc Launch는 지정한 exe를 그대로 실행하므로 C++ 수정 후 빌드를 잊으면 옛 코드가 캡처된다. 셰이더만 바꿨다면 앱 재실행으로 충분하다.
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
- 그래픽스 디버거는 RenderDoc을 쓴다. Launch 설정은 Executable `MyPF/x64/Debug/MyPF.exe`, Working Directory `MyPF/`이다

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
