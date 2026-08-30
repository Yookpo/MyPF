# MyPF 작업 인계

마지막 갱신: 2026-08-30

## 시작 절차

노트북과 데스크톱에서 Git으로 공유하는 MyPF의 실제 구현 상태와 다음 작업을 기록한다.

1. 저장소 루트의 AGENTS.md를 전부 읽는다.
2. 이 문서를 전부 읽는다.
3. 실제 소스, git status와 최근 commit을 확인한다.
4. 문서와 코드가 다르면 실제 코드와 최신 commit을 우선한다.
5. 사용자의 명시적 요청 없이 소스 수정, 빌드, 실행, commit 또는 push를 하지 않는다.

## 최종 목표와 현재 진행률

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 장면이 핵심 연출이다.

현재 전체 진행률은 약 **50%**다.

| 영역 | 진행 | 현재 상태 |
|---|---:|---|
| Win32/DX11 기반 | 약 85% | 기본 렌더링, Resize와 제출 경계 완료. 다중 패스는 남음 |
| GPU Resource/Asset 구조 | 약 75% | Buffer/Texture Handle, Asset 캐시와 Model 업로드 완료 |
| Model 파이프라인 | 약 65% | FBX/OBJ/glTF BaseColor 로드 완료. aiNode/PBR은 남음 |
| Scene/ImGui 편집 | 약 60% | Greybox Scene, 선택과 Transform/Material 편집 완료 |
| 1인칭 입력/카메라 | 약 90% | Editor/Play, WASD/마우스/ESC/focus와 단발 키 입력 완료 |
| 조명/Material | 약 25% | Directional Diffuse/Ambient와 BaseColor만 완료 |
| 실제 골목/상호작용 | 약 40% | 1m Greybox, 전원 스위치, 거리/시선/E 입력과 On 상태 검증 완료 |
| 고급 렌더링/연출 | 약 5% | Shadow/Wet/HDR/Bloom/Fog/Rain은 없음 |

## 협업 방식

- 사용자가 모든 C++와 HLSL을 직접 작성한다.
- Codex는 기능이 필요한 이유와 최종 데모에 주는 기여를 먼저 설명한다.
- 책임, 설계 질문, 첫 힌트, 완료 조건 순서로 사용자가 스스로 작성하도록 유도한다.
- 사용자가 더 구체적인 도움을 원할 때만 다음 힌트를 제공한다.
- 사용자가 막혔다고 명시하고 요청하기 전에는 완성 코드를 바로 제시하지 않는다.
- 사용자가 수정 완료라고 하면 실제 파일을 다시 읽고 리뷰한다.
- 빌드와 실행은 사용자가 명시적으로 요청할 때만 수행한다.
- 하위 시스템은 상위 시스템을 전혀 모른다는 원칙을 유지한다.
- 현재는 최종 기능을 우선하며 대규모 Unreal식 이름 변경은 보류한다.

## Git 체크포인트

- 노트북 저장소: C:/Users/Diguedman/source/repos/MyPF
- 데스크톱 저장소: C:/Users/tls15/source/repos/MyPF
- 원격 저장소: https://github.com/Yookpo/MyPF.git
- 브랜치: main
- 문서 갱신 기준 HEAD: fefdb2f — E키를 누르면 스위치 On Text로 확인
- 문서 갱신 시 작업 트리: MyPF/imgui.ini만 수정됨
- imgui.ini는 런타임 UI 배치 파일이므로 기능 commit에서 제외한다.
- 이번 요청에서 Codex가 수정한 파일: AGENTS.md, MyPF/docs/CODEX_HANDOFF.md
- 솔루션: MyPF/MyPF.sln
- 기본 구성: Debug | x64
- 실행 작업 디렉터리: MyPF/
- C++20, HLSL Shader Model 5.0 런타임 컴파일
- Assimp는 양쪽 PC의 사용자 vcpkg 환경에 설치돼 있다.

최근 기능 commit:

~~~text
fefdb2f E키를 누르면 스위치 On Text로 확인
942a04e 스위치 거리 기반 판정 완료
aa666d5 전원 스위치 오브젝트 구성
20229d8 큐브 재조정 및 위치 설정
9cf9b37 GreyBox 완성
48ab8b9 골목 씬 구성하기
4dee100 Codex Update
03bf197 전체적으로 매끄러운 1인칭 카메라 완료
2221f01 Play 종료 로직
8ba9e3a ESC로 Play 모드 종료
96cce52 마우스 이동을 통해 1인칭 자유시점 카메라 구현
56c0af2 WSAD 테스트 완료
7e45783 Mode 분리 및 입력 상태 도입
~~~

## 핵심 아키텍처와 소유 구조

- Renderer는 Scene, GameObject, Transform, Camera, Model과 ImGui를 모른다.
- Scene과 GameObject는 DX11 렌더링 과정이나 ImGui를 모른다.
- GraphicsDevice와 GraphicsResourceManager는 AssetManager, Renderer, Scene과 AppBase를 모른다.
- ModelLoader는 GPU 계층을 모르며 Assimp 데이터를 CPU ModelData로 변환한다.
- AssetManager는 GraphicsResourceManager를 비소유 참조하고 논리 Asset을 소유·캐싱한다.
- AppBase가 초기화, Greybox 구성, ImGui, Play 입력, 상호작용과 Scene→RenderItem 변환을 조정한다.

~~~text
AppBase
├─ GraphicsDevice
├─ GraphicsResourceManager --비소유--> GraphicsDevice
├─ AssetManager --비소유--> GraphicsResourceManager
├─ Renderer --비소유--> GraphicsDevice / GraphicsResourceManager
├─ Scene
│  └─ vector<unique_ptr<GameObject>>
│     ├─ Transform
│     ├─ MeshComponent --비소유--> Mesh / Material
│     └─ ModelComponent --비소유--> Model
├─ InputSystem
├─ Camera / EditorCameraSnapshot
├─ 전원 스위치 GameObject 비소유 포인터
├─ 전원 상태와 거리/시선 설정
└─ DirectionalLight

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
~~~

## 현재 완료된 기능

### 플랫폼, Scene과 Asset

- Win32 Window, 메시지 루프, Resize와 DX11 Device/Context/SwapChain
- BackBuffer RTV, Depth Buffer, Shader와 Indexed Drawing
- ImGui Scene Viewport, GameTimer, deltaTime과 FPS
- Scene의 vector<unique_ptr<GameObject>> 소유
- GameObject의 Transform, MeshComponent와 ModelComponent
- FrameRenderData와 RenderItem 제출 경계
- BufferHandle/TextureHandle과 GraphicsResourceManager
- AssetManager의 Mesh/Texture/Material/Model 소유와 캐시
- Assimp ModelLoader와 독립 CPU ModelData
- Zelda FBX, Pikachu OBJ, Dragonite glTF 출력 확인

### Editor/Play와 입력

- Editor/Play 상태, ImGui Play/Stop
- InputSystem의 key down state와 누적 MouseDelta
- WasKeyPressed와 EndFrame을 통한 키 단발 입력
- deltaTime 기반 WASD 평면 이동과 대각선 정규화
- 마우스 Yaw/Pitch와 Pitch 제한
- Play 커서 숨김/중앙 복귀
- ESC, Stop, focus 상실 시 Play 종료
- Play 종료 시 Editor Camera Snapshot 복원

### Greybox와 최소 상호작용

- GeometryGenerator::MakeCube 정점 범위를 -0.5~+0.5로 바꿔 기본 크기 1m로 통일
- Floor: Position (0, -0.1, 10), Scale (4, 0.2, 20)
- LeftWall: Position (-2.1, 2, 10), Scale (0.2, 4, 20)
- RightWall: Position (2.1, 2, 10), Scale (0.2, 4, 20)
- EndWall: Position (0, 2, 20.1), Scale (4, 4, 0.2)
- Camera 시작점 (0, 1.6, 0), +Z 방향
- 구조물은 공용 Cube Mesh와 Greybox Material을 공유
- PowerSwitch: Position (0, 1.2, 19.95), Scale (0.4, 0.6, 0.1)
- PowerSwitch는 공용 Cube Mesh와 독립 Material을 사용
- 상호작용 거리 2.0m, 시선 내적 임계값 0.8
- CanInteractWithPowerSwitch가 널, 거리와 시선 조건을 읽기 전용으로 판정
- ActivatePower가 전원 상태를 false → true로 한 번만 변경
- WasKeyPressed('E')와 상호작용 판정이 모두 참일 때만 활성화
- ImGui에서 Interaction available/unavailable와 Power On/Off를 실행 확인

## 현재 프레임 흐름

1. Win32 메시지를 InputSystem과 AppBase가 처리한다.
2. Resize와 focus 상실을 처리한다.
3. GameTimer와 ImGui 프레임을 시작한다.
4. Editor/Play UI, Camera Aspect와 Scene Viewport를 갱신한다.
5. Play이면 ESC, WASD와 MouseDelta로 Camera를 갱신한다.
6. E가 이번 프레임에 눌렸다면 거리·시선을 판정하고 전원을 활성화한다.
7. FrameRenderData를 Renderer::BeginFrame에 전달한다.
8. AppBase가 Scene을 RenderItem으로 변환하고 Renderer가 DrawIndexed한다.
9. ImGui를 렌더링하고 Present한다.
10. InputSystem::EndFrame이 단발 키 상태를 초기화한다.

## 현재 알려진 문제와 보류 항목

- AppBase가 초기화, Greybox 구성, UI, Play 입력, 전원 상호작용과 RenderItem 조립까지 담당해 방대해졌다.
- Camera 충돌이 없어 벽과 스위치를 통과할 수 있다.
- 전원 상태는 ImGui Text로만 확인되며 스위치 Material과 조명에는 아직 반영되지 않는다.
- 스위치의 아주 작은 거리 조건은 현재 상호작용 불가능으로 처리한다.
- 골목 이동 경로는 약 20m로 최종 20~40초 탐색 동선보다 짧다.
- 실제 골목 에셋, 네온 간판 위치와 Scene 저장/Prefab은 없다.
- Renderer Material 경로는 유효한 Albedo Texture를 전제로 한다.
- GeometryGenerator의 평행 배열은 데이터 불일치 가능성이 있다.
- MakeCube의 지역 scale = 1.0f는 현재 중복 표현이다.
- GraphicsResourceManager는 append-only이며 삭제/재사용/generation이 없다.
- AssetManager key/path 정규화가 없다.
- ModelLoader는 aiNode Transform/Instance를 반영하지 않는다.
- Material은 BaseColor/Diffuse만 지원하며 Normal/Metallic/Roughness/Emissive가 없다.
- Texture sRGB/Gamma, MipMap과 UV Tiling 정책이 없다.
- Pixel Shader의 최종 saturate는 HDR/Bloom 전에 HDR 경로로 옮겨야 한다.
- 일부 한글 주석 인코딩이 깨져 있다.
- 외부 캐릭터 에셋은 테스트용이며 공개 전 라이선스를 확인해야 한다.

## 결정된 설계 방향: PowerSwitch 컴포지션

전원 스위치는 개념적으로 GameObject지만 현재 GameObject는 상속 기반 다형 타입으로 설계되지 않았다.

- GameObject에는 virtual destructor나 virtual Update/Interact가 없다.
- Scene Factory는 make_unique<GameObject>만 생성한다.
- 현재 GameObject는 Transform/MeshComponent/ModelComponent를 조합하는 방향이다.
- 상속을 도입하면 Scene Factory, 파괴, 타입 접근과 행동 인터페이스까지 함께 바뀐다.

따라서 지금은 PowerSwitch가 Scene 소유 GameObject를 비소유로 연결하는 컴포지션을 사용한다.

PowerSwitch가 가질 책임:

- 연결된 GameObject 비소유 참조
- 전원 상태
- 상호작용 거리와 시선 임계값
- 위치/전방 데이터를 이용한 상호작용 판정
- 단방향 전원 활성화
- 전원 상태를 스위치 Material에 반영

PowerSwitch가 몰라야 하는 상위 개념:

- AppBase
- InputSystem
- Camera 클래스 자체
- Renderer
- ImGui
- Scene

AppBase에는 E 단발 입력 확인, Camera 위치/전방 전달과 활성화 요청만 남긴다. 범용 ECS나 Component Registry는 아직 만들지 않는다.

## 바로 다음 작업

PowerSwitch 책임을 AppBase에서 안전하게 분리한다.

1. PowerSwitch의 책임과 최소 공개 인터페이스를 정의한다.
2. Scene이 소유한 GameObject를 PowerSwitch에 비소유로 연결한다.
3. m_isPowerOn, m_interactionRange, m_interactionFacingThreshold를 이동한다.
4. 거리·시선 판정과 단방향 활성화를 이동한다.
5. AppBase에는 E 단발 입력, Camera 데이터 전달과 활성화 요청만 남긴다.
6. ImGui 상태 표시는 PowerSwitch의 질의 API만 사용한다.
7. 기존 Interaction/Power 실행 결과가 유지되는지 확인한다.

PowerSwitch 분리 완료 후 순서:

1. 전원 상태에 따른 스위치 Material 시각 변화
2. 다수 Point Light 데이터와 Renderer 제출
3. Emissive Material과 네온 간판
4. 전원 상태와 순차 점등 연출
5. 실제 골목 에셋 배치와 Scene 편집 보강
6. Shadow Mapping
7. Normal/Roughness와 젖은 바닥 반사
8. HDR Scene Target, Bloom과 Tone Mapping
9. 안개, 비와 색조 보정
10. 충돌/이동 제한, 디버그 UI와 최적화
11. 에셋 라이선스와 1~2분 최종 연출 정리

## 노트북에서 바로 확인할 체크리스트

- git pull 후 HEAD가 최소 fefdb2f인지 확인한다.
- AGENTS.md와 이 문서의 갱신일이 2026-08-30인지 확인한다.
- git status에서 사용자 변경과 로컬 MyPF/imgui.ini 변경을 구분한다.
- GeometryGenerator::MakeCube가 1m 단위인지 확인한다.
- Greybox 바닥/벽과 PowerSwitch Transform이 위 수치와 같은지 확인한다.
- InputSystem::WasKeyPressed와 EndFrame이 연결됐는지 확인한다.
- Play에서 거리와 시선 조건이 모두 맞을 때 E로 Power가 On 되는지 확인한다.
- 바로 다음 구조 책임은 PowerSwitch 컴포지션 분리다.
- 대규모 Unreal식 명명 마이그레이션이나 범용 ECS를 시작하지 않는다.
- 사용자가 요청하지 않으면 빌드, 실행, commit 또는 push하지 않는다.
