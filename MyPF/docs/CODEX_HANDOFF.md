# MyPF 작업 인계

마지막 갱신: 2026-08-30

## 시작 절차

노트북과 데스크톱에서 Git으로 공유하는 MyPF의 실제 구현 상태와 다음 작업을 기록한다.

새 환경에서는 다음 순서로 시작한다.

1. 저장소 루트의 `AGENTS.md`를 전부 읽는다.
2. 이 문서를 전부 읽는다.
3. 실제 소스, `git status`와 최근 commit을 확인한다.
4. 문서와 코드가 다르면 실제 코드를 우선한다.
5. 사용자의 명시적 요청 없이 소스 수정, 빌드, 실행, commit 또는 push를 하지 않는다.

## 최종 목표와 진행률

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 장면이 핵심 연출이다.

현재 전체 진행률은 약 **45%**다.

| 영역 | 진행 | 현재 상태 |
|---|---:|---|
| Win32/DX11 기반 | 약 85% | 기본 렌더링, Resize와 제출 경계 완료. 다중 패스는 남음 |
| GPU Resource/Asset 구조 | 약 75% | Buffer/Texture Handle, Asset 캐시와 Model 업로드 완료 |
| Model 파이프라인 | 약 65% | FBX/OBJ/glTF BaseColor 로드 완료. aiNode/PBR은 남음 |
| Scene/ImGui 편집 | 약 50% | 선택과 Transform 편집 가능. 생성/삭제/저장과 실제 배치는 남음 |
| 1인칭 입력/카메라 | 약 85% | Editor/Play, WASD/마우스/ESC/focus 완료. 충돌은 남음 |
| 조명/Material | 약 25% | Directional Diffuse/Ambient만 완료. Point/Emissive/PBR은 남음 |
| 실제 골목/상호작용 | 약 5% | 테스트 모델 장면만 존재. Greybox와 상호작용은 없음 |
| 고급 렌더링/연출 | 약 5% | Depth는 있으나 Shadow/Wet/HDR/Bloom/Fog/Rain은 없음 |

기반 시스템은 상당 부분 갖춰졌지만 최종 화면과 플레이 경험을 만드는 콘텐츠·조명·후처리의 비중이 크므로 절반을 넘었다고 보지는 않는다.

## 협업 방식

- 사용자가 모든 C++와 HLSL을 직접 작성한다.
- Codex는 목적, 배경지식, 책임 경계와 완료 조건을 먼저 설명한다.
- 한 줄씩 지나치게 잘게 나누지 않고 검토 가능한 기능 책임 단위로 진행한다.
- 사용자가 막혔다고 말하기 전에는 완성 코드를 바로 제시하지 않는다.
- 사용자가 수정 완료라고 하면 실제 파일을 다시 읽고 리뷰한다.
- 빌드와 실행은 사용자가 명시적으로 요청할 때만 수행한다.
- 하위 시스템은 상위 시스템을 전혀 모른다는 원칙을 유지한다.
- 현재는 최종 기능을 우선하며 대규모 Unreal식 이름 변경은 보류한다.

## 저장소 체크포인트

- 노트북 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 데스크톱 저장소: `C:/Users/tls15/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 브랜치: `main`
- 문서 갱신 직전 HEAD: `03bf197` — `전체적으로 매끄러운 1인칭 카메라 완료`
- 문서 갱신 직전 작업 트리: clean
- 이번 요청에서 Codex가 수정한 파일: `AGENTS.md`, `MyPF/docs/CODEX_HANDOFF.md`
- 솔루션: `MyPF/MyPF.sln`
- 기본 구성: `Debug | x64`
- 실행 작업 디렉터리: `MyPF/`
- C++20, HLSL Shader Model 5.0 런타임 컴파일
- Assimp는 양쪽 PC의 사용자 vcpkg 환경에 설치돼 있다.

최근 commit:

```text
03bf197 전체적으로 매끄러운 1인칭 카메라 완료
2221f01 Play 종료 로직
8ba9e3a ESC로 Play 모드 종료
96cce52 마우스 이동을 통해 1인칭 자유시점 카메라 구현
56c0af2 WSAD 테스트 완료
7e45783 Mode 분리 및 입력 상태 도입
2660d06 Merge branch 'main' of https://github.com/Yookpo/MyPF
9d91ba7 08_27 Codex Update (from Desktop)
6ec22d3 피카츄, 망나뇽 로드
ad29527 GameObject 하나가 Model 하나를 참조
aeff863 젤다 모델 로드 성공
```

## 핵심 아키텍처와 소유 구조

- Renderer는 Scene, GameObject, Transform, Camera, Model과 ImGui를 모른다.
- Scene과 GameObject는 DX11 렌더링 과정이나 ImGui를 모른다.
- GraphicsDevice와 GraphicsResourceManager는 AssetManager, Renderer, Scene과 AppBase를 모른다.
- ModelLoader는 GPU 계층을 모르며 Assimp 데이터를 CPU ModelData로 변환한다.
- AssetManager는 GraphicsResourceManager를 비소유 참조하고 논리 Asset을 소유·캐싱한다.
- AppBase가 초기화, 임시 콘텐츠, ImGui, Play 입력과 Scene→RenderItem 변환을 조정한다.

```text
AppBase
├─ GraphicsDevice
│  ├─ Device / Context / SwapChain
│  └─ BackBuffer RTV / Depth Texture / DSV
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
└─ DirectionalLight

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
```

Scene/Renderer가 먼저 파괴되고 AssetManager, GraphicsResourceManager와 GraphicsDevice가 뒤에 파괴되는 현재 선언 순서에서는 비소유 참조 수명이 안전하다.

## 현재 완료된 기능

### 플랫폼과 렌더링

- Win32 Window, 메시지 루프와 `WM_SIZE`
- DX11 Device/Context/SwapChain, BackBuffer RTV와 Depth Buffer
- Resize 시 SwapChain/RTV/Depth 재생성
- Shader Model 5.0 Vertex/Pixel Shader 런타임 컴파일
- Input Layout, Rasterizer/DepthStencil/Sampler와 Indexed Drawing
- ImGui 패널을 제외한 Scene Viewport
- GameTimer, deltaTime과 FPS

### Scene과 제출 경계

- Scene의 `vector<unique_ptr<GameObject>>` 소유
- GameObject의 이름, Transform, MeshComponent와 ModelComponent
- Scene Object 선택과 독립 Transform/Material BaseColor 편집
- FrameRenderData의 View/Projection/DirectionalLight 제출
- RenderItem의 Mesh/Material/World 제출
- Renderer가 Scene/GameObject/Camera/Model을 모르는 경계

### GPU Resource와 Asset/Model

- BufferHandle/TextureHandle과 GraphicsResourceManager의 실제 GPU 자원 소유
- Constant/Vertex/Index Buffer 생성·조회·Dynamic Update
- Mesh와 Texture의 Handle 전환
- AssetManager의 Mesh/Texture/Material/Model unique_ptr 소유와 캐시
- ModelLoader의 Position/Normal/UV/삼각형 Index 변환
- Left-Handed 변환, 삼각형화와 Smooth Normal 생성
- 모델 디렉터리 기준 상대 BaseColor/Diffuse Texture 경로 해석
- Model/ModelPart/ModelComponent와 GameObject World Transform
- Zelda FBX, Pikachu OBJ, Dragonite glTF 출력 확인

### 조명과 Material

- Directional Light Direction/Color/Intensity/Ambient Strength
- DirectionalLight → FrameRenderData → Renderer → HLSL 연결
- ImGui Light 편집
- Texture Albedo, Material BaseColor, Normal Diffuse와 Ambient

### Editor/Play와 1인칭 입력

- `AppMode::Editor`, `AppMode::Play`
- ImGui Play/Stop과 상태 표시
- InputSystem 256-key state와 누적 MouseDelta
- Play에서만 deltaTime 기반 WASD 평면 이동
- 대각선 이동 벡터 정규화
- 마우스 Yaw/Pitch와 Pitch `-89~89` 제한
- Scene View 중앙 커서 복귀와 MouseDelta 기준 재설정
- Play 진입 시 커서 숨김, Input Reset과 Camera Snapshot
- ESC, Stop과 `WM_KILLFOCUS`로 Play 종료
- 종료 시 커서 표시, Input Reset과 Editor Camera 복원

## 현재 초기화와 프레임 흐름

### 초기화

1. AppBase가 Window를 생성한다.
2. GraphicsDevice가 Device/Context/SwapChain/RTV/Depth를 만든다.
3. GraphicsResourceManager가 GraphicsDevice를 비소유 참조한다.
4. AssetManager가 GraphicsResourceManager를 비소유 참조한다.
5. Renderer가 Pipeline/Shader/Constant Buffer Handle을 만든다.
6. ImGui Win32/DX11 Backend를 초기화한다.
7. AssetManager가 ModelLoader 결과를 GPU Asset과 Model로 조립한다.
8. 현재 테스트 Scene에 Pikachu와 Dragonite를 생성한다.

### 프레임

1. Win32 메시지를 InputSystem과 AppBase가 처리한다.
2. Resize와 focus 상실을 처리한다.
3. GameTimer와 ImGui 프레임을 시작한다.
4. Editor/Play UI, Camera Aspect와 Scene Viewport를 갱신한다.
5. Play이면 ESC, WASD와 MouseDelta로 Camera를 갱신한다.
6. FrameRenderData를 Renderer::BeginFrame에 전달한다.
7. Renderer가 Camera/Light Constant Buffer를 갱신한다.
8. AppBase가 Scene을 ModelPart 또는 MeshComponent RenderItem으로 변환한다.
9. Renderer가 Buffer/Texture/Pipeline을 바인딩하고 DrawIndexed한다.
10. ImGui를 렌더링하고 Present한다.

## 현재 알려진 문제와 보류 항목

- 활성 Scene은 Pikachu/Dragonite 검증 장면이며 골목 Greybox가 없다.
- AppBase에 Cube/Triangle 과거 테스트 코드가 주석 블록으로 남아 있다.
- 카메라 이동 충돌과 플레이어 높이/월드 단위 정책이 없다.
- Transform ImGui 위치 `-1~1`, Scale `0.1~2` 범위는 골목 배치에 부족하다.
- AppBase가 초기화, 테스트 콘텐츠, UI, Play 입력과 RenderItem 조립을 모두 담당한다.
- Renderer Material 경로는 유효한 Albedo Texture를 전제로 하며 기본 Material/Texture 정책이 없다.
- GeometryGenerator의 평행 배열은 데이터 불일치 가능성이 있다.
- Vertex Color는 최종 Pixel Color에 사용되지 않는다.
- GraphicsResourceManager는 append-only이며 삭제/재사용/generation이 없다.
- AssetManager key/path 정규화가 없다.
- ModelLoader는 aiNode Transform/Instance를 반영하지 않는다.
- Material은 BaseColor/Diffuse만 지원하며 Normal/Metallic/Roughness/Emissive가 없다.
- Texture sRGB/Gamma, MipMap과 UV Tiling 정책이 없다.
- Pixel Shader의 최종 saturate는 HDR/Bloom 단계에서 HDR 경로로 옮겨야 한다.
- ImGui 부분 초기화 실패와 Shutdown 상태 추적이 없다.
- 일부 한글 주석 인코딩이 깨져 있다.
- `.clang-format`은 TensorWorks 기반 기존 설정이다. 전면 Epic 명명 마이그레이션은 기능 우선 결정으로 적용하지 않았다.
- 외부 캐릭터 에셋은 테스트용이며 공개 전 라이선스를 확인해야 한다.
- `imgui.ini`는 기능 commit에서 제외한다.

## 바로 다음 작업: 골목 Greybox 초기화

### 필요한 이유

Point Light Radius, Shadow Bias, Fog 거리, Wet Floor 크기와 Camera Speed는 Scene의 실제 단위에 의존한다. 먼저 `1 unit = 1m` 기준의 짧은 골목을 만들어 이후 수치를 일관되게 결정해야 한다.

기존 Scene/Asset/Renderer 구조가 다수 구조물에서도 유지되는지 확인하는 첫 콘텐츠 단계이기도 하다.

### 책임 경계

- AppBase가 AssetManager와 Scene을 조정해 Greybox를 만든다.
- AssetManager는 공용 Cube Mesh, Texture와 Material을 소유한다.
- Scene은 바닥/벽 GameObject를 소유한다.
- GameObject들은 같은 Mesh/Material을 비소유 참조하고 Transform만 독립적으로 가진다.
- Renderer, GraphicsResourceManager와 Scene 내부에 Greybox 전용 개념을 추가하지 않는다.
- Scene Factory, ECS 또는 Prefab 시스템을 미리 만들지 않는다.

### 구현 방향

1. AppBase에 Greybox Scene 초기화라는 하나의 책임을 가진 메서드를 둔다.
2. `GeometryGenerator::MakeCube()`와 AssetManager로 공용 Cube Mesh를 생성한다.
3. 프로젝트 루트 `wall.jpg`를 로드하고 Greybox Material을 만든다.
4. 바닥, 왼쪽 벽, 오른쪽 벽과 끝 벽을 만들고 공용 Mesh/Material을 연결한다.
5. Cube 기본 범위가 `-1~1`이므로 Transform Scale이 최종 크기의 절반임을 반영한다.
6. `1 unit = 1m`, Camera 높이 약 1.6m, 폭 약 4m, 길이 약 20m를 첫 기준으로 검증한다.
7. Pikachu/Dragonite 활성 장면을 Greybox로 교체하되 Model 파이프라인은 제거하지 않는다.
8. Transform ImGui 범위를 골목 규모에 맞게 확장한다.

첫 배치 예시:

- Floor: Position `(0, -0.1, 10)`, Scale `(2, 0.1, 10)`
- LeftWall: Position `(-2.1, 2, 10)`, Scale `(0.1, 2, 10)`
- RightWall: Position `(2.1, 2, 10)`, Scale `(0.1, 2, 10)`
- EndWall: Position `(0, 2, 20.1)`, Scale `(2, 2, 0.1)`
- Camera: Position `(0, 1.6, 0)`, +Z 방향

수치는 최종 디자인이 아니라 월드 스케일 검증용 출발점이다.

### 완료 조건

- Editor에서 바닥과 좌우 벽이 짧은 골목 형태로 보인다.
- 구조물이 하나의 Cube Mesh와 Material을 공유한다.
- 각 GameObject의 Transform은 독립적으로 편집된다.
- Play에서 WASD와 마우스로 골목을 탐색할 수 있다.
- Renderer, ResourceManager와 Scene은 Greybox라는 상위 개념을 모른다.
- 충돌, Point Light와 고급 Material은 이번 단계에 포함하지 않는다.

## Greybox 이후 순서

1. 최소 상호작용 기반: 거리 또는 Ray 판정과 상호작용 입력
2. 다수 Point Light 데이터와 Renderer 제출
3. Emissive Material과 네온 간판
4. 전원 장치 상태와 순차 점등
5. 실제 골목 에셋 배치와 Scene 편집 보강
6. Shadow Mapping
7. Normal/Roughness Material과 젖은 바닥 반사
8. HDR Scene Target, Bloom과 Tone Mapping
9. 안개, 비와 색조 보정
10. 충돌/이동 제한, 디버그 UI와 최적화
11. 에셋 라이선스 정리와 1~2분 최종 연출

ModelLoader의 aiNode/PBR 확장은 실제 골목 에셋에서 파트 배치가 깨지거나 PBR 데이터가 필요해질 때 다시 연다.

## 노트북에서 바로 확인할 체크리스트

- `git pull` 후 HEAD가 최소 `03bf197` 이상인지 확인한다.
- `AGENTS.md`와 이 문서의 갱신일이 `2026-08-30`인지 확인한다.
- `git status`에서 사용자 변경과 로컬 `imgui.ini` 변경을 구분한다.
- InputSystem.cpp/.h가 프로젝트에 포함됐는지 확인한다.
- DirectionalLight → FrameRenderData → Renderer → HLSL의 Ambient Strength 연결을 확인한다.
- AppBase의 Editor/Play, WASD/마우스, ESC/focus 종료와 Camera Snapshot 복원을 확인한다.
- 현재 활성 Scene이 Pikachu/Dragonite 테스트 장면인지 확인한다.
- 바로 다음 기능 책임은 `골목 Greybox 초기화`다.
- 대규모 Unreal식 명명 마이그레이션을 재개하지 않는다.
- 사용자가 요청하지 않으면 빌드나 실행을 하지 않는다.
