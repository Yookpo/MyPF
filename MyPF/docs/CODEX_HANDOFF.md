# MyPF 작업 인계

마지막 갱신: 2026-09-03

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

현재 전체 진행률은 약 **65%**다.

| 영역 | 진행 | 현재 상태 |
|---|---:|---|
| Win32/DX11 기반 | 약 85% | 기본 렌더링, Resize와 제출 경계 완료. 다중 패스는 남음 |
| GPU Resource/Asset 구조 | 약 75% | Buffer/Texture Handle, Asset 캐시와 Model 업로드 완료 |
| Model 파이프라인 | 약 65% | FBX/OBJ/glTF BaseColor 로드 완료. aiNode/PBR은 남음 |
| Scene/ImGui 편집 | 약 65% | Greybox Scene, 선택과 Transform/Material 편집, Point Light 소유 완료 |
| 1인칭 입력/카메라 | 약 90% | Editor/Play, WASD/마우스/ESC/focus와 단발 키 입력 완료 |
| 조명/Material | 약 72% | Directional/Ambient, 최대 8 Point Light, 거리 감쇠, Emissive CPU→GPU→HLSL 경로와 네온 두 개 완료 |
| 실제 골목/상호작용 | 약 80% | PowerSwitch와 거리/시선/E 입력, 스위치 피드백, Point Light·Emissive 순차 점등 완료 |
| 고급 렌더링/연출 | 약 15% | 두 네온의 Entry 기반 순차 점등 완료. Shadow/Wet/HDR/Bloom/Fog/Rain은 없음 |

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
- 문서 갱신 기준 HEAD: 421a58f — Entry 기반 점등 소등 구현
- 문서 갱신 직전 작업 트리: MyPF/AppBase.cpp에 두 번째 네온 Material/Object/Entry 추가 변경이 있으며 실행 테스트 완료
- imgui.ini는 런타임 UI 배치 파일이므로 기능 commit에서 제외한다.
- 이번 요청에서 Codex가 수정한 파일: AGENTS.md, MyPF/docs/CODEX_HANDOFF.md
- 솔루션: MyPF/MyPF.sln
- 기본 구성: Debug | x64
- 실행 작업 디렉터리: MyPF/
- C++20, HLSL Shader Model 5.0 런타임 컴파일
- Assimp는 양쪽 PC의 사용자 vcpkg 환경에 설치돼 있다.

최근 기능 commit:

~~~text
421a58f Entry 기반 점등 소등 구현
92cccc3 CODEX_UPDATE
ca6d15c 순차점등시퀀스 적용 완료
f9fb2be 조명4개 스위치로 on/off 확인
597d1f5 점광 배치 확인
0ef4013 Point Light 적용중
bdd60b2 E키를 누르면 스위치의 색깔 변화확인
1d1afd5 PowerSwitch 생성 및 소유
46878a4 Codex Update
fefdb2f E키를 누르면 스위치 On Text로 확인
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
├─ PowerSwitch --비소유--> Scene 소유 GameObject / Material
├─ PointLightSequence --비소유--> Scene / 등록된 Material
├─ Scene 소유 PointLight[]
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

### Greybox, 상호작용과 Point Light 연출

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
- PowerSwitch가 Scene 소유 GameObject를 비소유 참조하며 거리·시선 판정과 상태를 캡슐화
- `Toggle()`이 전원 상태를 On/Off로 반전하고 독립 Material을 빨강/초록으로 변경
- `WasKeyPressed('E')`와 상호작용 판정이 모두 참일 때만 상태 전환
- Scene이 Position, Range, Color, Intensity, Enabled 상태의 Point Light 목록을 소유
- FrameRenderData가 최대 8개의 Point Light를 고정 배열과 유효 개수로 Renderer에 제출
- C++ bool을 GPU `uint32_t`로 변환하고 Point Light 상수 버퍼를 HLSL 레이아웃과 일치시킴
- `Lighting.hlsli`가 범위 기반 거리 감쇠와 Point Light Diffuse를 계산
- 청록·자홍·보라 계열 Point Light 4개가 골목을 비추는 결과 확인
- PointLightSequence가 1.4초 간격으로 등록된 Entry 순서대로 켜고 역순으로 끔
- 점등 도중 다시 E를 누르면 현재 켜진 개수에서 목표 방향을 전환
- Material의 Emissive Color/Intensity와 발광하지 않는 기본값
- 32바이트 Material Constant Buffer의 C++/HLSL 일치와 Renderer 전달
- Pixel Shader에서 조명 결과와 별도로 Emissive를 더하는 표면 자체 발광
- Point Light 인덱스, 비소유 Material 포인터와 목표 밝기를 묶는 SequenceEntry
- Scene 전체 조명 수 대신 등록된 Entry 수와 등록 순서를 사용하는 PointLightSequence
- 같은 Entry의 Point Light와 Emissive Material을 함께 켜고 역순으로 함께 끄는 연출
- 독립 Material과 다른 Emissive 색을 가진 네온 두 개의 순차 On/Off 실행 확인

## 현재 프레임 흐름

1. Win32 메시지를 InputSystem과 AppBase가 처리한다.
2. Resize와 focus 상실을 처리한다.
3. GameTimer와 ImGui 프레임을 시작한다.
4. Editor/Play UI, Camera Aspect와 Scene Viewport를 갱신한다.
5. Play이면 ESC, WASD와 MouseDelta로 Camera를 갱신한다.
6. E가 이번 프레임에 눌렸다면 PowerSwitch가 거리·시선을 판정하고 전원 상태와 색상을 반전한다.
7. PointLightSequence가 전원 상태를 목표로 받고 `deltaTime`을 누적해 등록된 Entry의 Point Light와 Emissive Material을 한 단계씩 함께 갱신한다.
8. Scene의 Point Light 목록을 포함한 FrameRenderData를 Renderer::BeginFrame에 전달한다.
9. Renderer가 CPU Point Light를 GPU 상수 데이터로 변환하고 Pixel Shader가 조명 기여를 누적한다.
10. AppBase가 Scene을 RenderItem으로 변환하고 Renderer가 DrawIndexed한다.
11. ImGui를 렌더링하고 Present한다.
12. InputSystem::EndFrame이 단발 키 상태를 초기화한다.

## 현재 알려진 문제와 보류 항목

- AppBase가 초기화, Greybox 구성, UI, Play 입력, RenderItem 조립과 기능 객체 조율까지 담당해 방대하다. 실제 변경 압력이 확인되는 책임부터 분리한다.
- Camera 충돌이 없어 벽과 스위치를 통과할 수 있다.
- 스위치의 아주 작은 거리 조건은 현재 상호작용 불가능으로 처리한다.
- 골목 이동 경로는 약 20m로 최종 20~40초 탐색 동선보다 짧다.
- 실제 골목 에셋, 네온 간판 위치와 Scene 저장/Prefab은 없다.
- PointLightSequence는 자신만 조명 활성 상태를 변경한다는 전제를 사용하므로 다른 코드의 직접 변경과 함께 사용하면 내부 개수가 어긋날 수 있다.
- Point Light 점등 순서는 Scene 생성 순서가 아니라 SequenceEntry 등록 순서에 의존한다.
- SequenceEntry는 Material을 비소유 포인터로 참조한다. 현재 AssetManager가 시퀀스보다 오래 Material을 소유한다는 수명 전제를 사용한다.
- 시퀀스는 한 프레임에 한 단계만 처리하므로 큰 프레임 지연 후 누적 시간을 한 번에 따라잡지 않는다.
- Renderer Material 경로는 유효한 Albedo Texture를 전제로 한다.
- GeometryGenerator의 평행 배열은 데이터 불일치 가능성이 있다.
- MakeCube의 지역 scale = 1.0f는 현재 중복 표현이다.
- GraphicsResourceManager는 append-only이며 삭제/재사용/generation이 없다.
- AssetManager key/path 정규화가 없다.
- ModelLoader는 aiNode Transform/Instance를 반영하지 않는다.
- Material은 BaseColor/Diffuse/Emissive를 지원하며 Normal/Metallic/Roughness는 아직 없다.
- Texture sRGB/Gamma, MipMap과 UV Tiling 정책이 없다.
- Pixel Shader의 최종 `saturate` 때문에 1을 넘는 Emissive 값은 LDR에서 잘린다. HDR/Bloom 단계에서 HDR Scene Target으로 옮겨야 한다.
- 일부 한글 주석 인코딩이 깨져 있다.
- 외부 캐릭터 에셋은 테스트용이며 공개 전 라이선스를 확인해야 한다.

## 완료된 설계 방향: PowerSwitch 컴포지션

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
- 양방향 전원 On/Off 토글
- 전원 상태를 스위치 Material에 반영

PowerSwitch가 몰라야 하는 상위 개념:

- AppBase
- InputSystem
- Camera 클래스 자체
- Renderer
- ImGui
- Scene

AppBase에는 E 단발 입력 확인, Camera 위치/전방 전달과 활성화 요청만 남긴다. 범용 ECS나 Component Registry는 아직 만들지 않는다.

## 완료된 설계 방향: PointLightSequence와 SequenceEntry

- AppBase가 값으로 소유하고 Scene을 비소유 참조한다.
- PowerSwitch, InputSystem, Camera, Renderer와 ImGui를 직접 알지 않는다.
- `SequenceEntry`는 Scene Point Light 인덱스, 비소유 Material 포인터와 켜졌을 때의 Emissive Intensity를 묶는다.
- `AddSequenceEntry`는 Scene/Material/인덱스/밝기와 중복 등록을 검사하고 Point Light와 Material을 Off 상태로 맞춘 뒤 등록한다.
- `Start(bool)`은 실제 대상을 즉시 바꾸지 않고 등록된 Entry 개수를 기준으로 목표와 진행 방향만 설정한다.
- `Update(deltaTime)`은 등록 순서로 Point Light와 Material을 함께 켜고, `m_enabledLightCount - 1` 위치를 사용해 역순으로 함께 끈다.
- 진행 도중 목표가 바뀌면 현재 켜진 Entry 개수에서 방향을 전환한다.
- Material 포인터는 비소유이며 AssetManager가 수명을 보장한다.

## 바로 다음 작업

남은 Point Light의 역할을 분류하고 실제 네온 배치를 정리한다.

1. 미등록 상태인 pointLight1과 pointLight4를 네온 연동 조명 또는 환경 조명으로 분류한다.
2. 네온과 연결된 Point Light가 같은 광원이라면 두 색상을 의도적으로 맞춘다.
3. 네온 연동 대상에는 독립 Material/GameObject를 만들고 원하는 순서로 SequenceEntry에 등록한다.
4. 환경 조명은 Sequence 밖에서 초기 활성 상태를 명확히 관리한다.
5. 현재 테스트 네온의 이름과 위치를 정리하고 임시 골목 레이아웃을 확정한다.
6. 이후 Shadow Mapping으로 진행한다.

Emissive 완료 후 순서:

1. 실제 골목 에셋 배치와 Scene 편집 보강
2. Shadow Mapping
3. Normal/Roughness와 젖은 바닥 반사
4. HDR Scene Target, Bloom과 Tone Mapping
5. 안개, 비와 색조 보정
6. Point Light와 Emissive를 함께 다루는 최종 전원 연출 및 깜빡임
7. 충돌/이동 제한, 디버그 UI와 최적화
8. 에셋 라이선스와 1~2분 최종 연출 정리

## 2026-09-03 작업 완료 기록

### PowerSwitch 컴포지션과 시각 피드백 ✅

- Scene이 소유한 스위치 GameObject를 PowerSwitch가 비소유로 참조하도록 분리했다.
- 거리·시선 판정, 전원 상태와 On/Off 토글을 PowerSwitch 책임으로 옮겼다.
- 전원 Off는 빨강, On은 초록 BaseColor로 즉시 반영되도록 했다.
- 상호작용 범위에서 E키를 반복 입력해 상태와 색상이 양방향 전환되는 것을 확인했다.

### 다중 Point Light와 GPU 제출 ✅

- Scene에 Point Light 목록과 생성·조회·활성 상태 변경 API를 추가했다.
- 최대 8개의 Point Light를 FrameRenderData와 Light Constant Buffer로 전달한다.
- CPU 의미 데이터와 GPU 정렬 데이터를 분리하고 bool을 `uint32_t`로 변환한다.
- `Lighting.hlsli`에서 범위 기반 감쇠와 Diffuse를 계산하고 Pixel Shader에서 모든 유효 조명을 누적한다.
- 서로 다른 위치와 색상의 Point Light 4개가 골목을 비추는 결과를 확인했다.

### 전원 기반 순차 점등 ✅

- PointLightSequence가 Scene을 비소유 참조하고 현재 개수, 목표 개수, 누적 시간과 간격을 관리한다.
- 전원 On이면 등록된 Entry 순서대로 켜고 Off이면 역순으로 끈다.
- 진행 중 방향 전환, 목표 도달 정지와 스위치 On/Off 연동을 실행 확인했다.
- 현재 간격은 1.4초다.

### Emissive Material과 Entry 기반 네온 점등 ✅

- Material에 Emissive Color와 Intensity를 추가하고 기본 밝기를 0으로 설정했다.
- Material Constant Buffer의 C++/HLSL 구조를 32바이트로 맞추고 Renderer가 값을 전달한다.
- Pixel Shader에서 조명과 별도로 Emissive를 더해 표면 자체 발광을 확인했다.
- PointLightSequence에 SequenceEntry를 도입해 Point Light와 네온 Material을 하나의 연출 단위로 등록한다.
- Scene 전체 조명이 아니라 등록된 Entry 목록을 기준으로 켜기/끄기 목표와 순서를 계산한다.
- 독립 Material을 쓰는 네온 두 개를 Point Light와 연결해 등록 순서 점등과 역순 소등을 실행 확인했다.
- 현재 Emissive Intensity 3과 8은 LDR `saturate`에서 잘리므로 실제 밝기 차이와 번짐은 HDR/Bloom에서 확인한다.

## 노트북에서 바로 확인할 체크리스트

- git pull 후 HEAD가 최소 421a58f인지 확인한다.
- AGENTS.md와 이 문서의 갱신일이 2026-09-03인지 확인한다.
- git status에서 사용자 변경과 로컬 MyPF/imgui.ini 변경을 구분한다.
- GeometryGenerator::MakeCube가 1m 단위인지 확인한다.
- Greybox 바닥/벽과 PowerSwitch Transform이 위 수치와 같은지 확인한다.
- InputSystem::WasKeyPressed와 EndFrame이 연결됐는지 확인한다.
- Play에서 거리와 시선 조건이 모두 맞을 때 E로 전원과 스위치 색상이 On/Off 전환되는지 확인한다.
- 등록된 네온 두 개와 연결 Point Light가 1.4초 간격으로 함께 켜지고 역순으로 꺼지는지 확인한다.
- 미등록 pointLight1/pointLight4는 현재 시퀀스에서 계속 꺼져 있는 것이 정상이다.
- 바로 다음 기능 책임은 남은 조명의 역할 분류와 네온 배치 정리다.
- 대규모 Unreal식 명명 마이그레이션이나 범용 ECS를 시작하지 않는다.
- 사용자가 요청하지 않으면 빌드, 실행, commit 또는 push하지 않는다.
