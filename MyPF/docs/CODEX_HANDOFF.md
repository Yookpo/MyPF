# MyPF 작업 인계

마지막 갱신: 2026-08-27

## 이 문서의 목적

노트북과 데스크톱에서 Git으로 공유하는 MyPF의 실제 구현 상태와 다음 작업을 기록한다.

새 환경에서는 다음 순서로 시작한다.

1. 저장소 루트의 `AGENTS.md`를 전부 읽는다.
2. 이 문서를 전부 읽는다.
3. 실제 소스, `git status`와 최근 commit을 대조한다.
4. 문서와 코드가 다르면 실제 코드를 우선한다.
5. 사용자의 명시적 요청 없이 소스 수정, 빌드, 실행, commit 또는 push를 하지 않는다.

## 최종 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 장면을 핵심 연출로 삼는다.

주요 목표 기능:

- ImGui 기반 씬 편집과 디버그 UI
- Editor/Play 상태와 1인칭 WASD/마우스 조작
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 Point Light
- 조명 상태 변화와 순차 점등
- 그림자, 젖은 바닥과 반사
- HDR, Bloom, Tone Mapping과 색조 보정
- 안개, 비와 볼륨감 있는 분위기

범용 엔진을 만드는 것이 아니라 `DX11 렌더링 기술 + 확장 가능한 책임 구조 + 짧지만 완결된 플레이 경험`을 보여주는 포트폴리오가 목표다.

전체 로드맵 진행률은 약 42%로 본다. 기본 렌더링, GPU Resource 소유, Asset/Model 파이프라인은 마련됐지만 플레이 입력, 실제 골목, 다수 조명과 고급 렌더링은 남아 있다.

## 사용자와 Codex의 작업 방식

- 사용자가 모든 C++와 HLSL 코드를 직접 작성한다.
- Codex는 목적, 배경지식, 책임 경계와 완료 조건을 먼저 설명한다.
- 사용자의 코드가 잘못됐거나 불완전해도 즉시 완성 코드를 주지 않는다. 먼저 놓친 개념과 조건을 설명하고 다시 작성하도록 유도한다.
- 사용자가 막혔다고 명시하고 구현 형태를 요청할 때만 직접적인 완성 코드를 보여준다.
- 사용자가 `수정 완료`라고 하면 실제 파일을 다시 읽고 리뷰한다.
- 사용자가 명시적으로 요청할 때만 빌드한다. 기본 구성은 `Debug | x64`이며 실행은 별도 요청이 있을 때만 한다.
- 한 줄씩 지나치게 잘게 나누기보다 검토 가능한 작은 기능 책임 단위로 진행한다.
- 핵심 원칙은 “하위 시스템은 상위 시스템을 전혀 모른다”이다.

## 저장소 체크포인트

- 노트북 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 데스크톱 저장소: `C:/Users/tls15/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 현재 브랜치: `main`
- 문서 갱신 직전 확인 HEAD: `6ec22d3` (`피카츄, 망나뇽 로드`)
- 문서 갱신 직전 소스 변경: 없음
- 최종 검증 시 작업 트리: 사용자가 별도로 수정한 `.gitignore`, 이번 요청의 `AGENTS.md`와 `MyPF/docs/CODEX_HANDOFF.md`
- 이번 요청에서 Codex가 수정한 파일: `AGENTS.md`, `MyPF/docs/CODEX_HANDOFF.md`
- 솔루션: `MyPF/MyPF.sln`
- 기본 구성: `Debug | x64`
- 실행 작업 디렉터리: `MyPF/`
- HLSL: `MyPF/Shaders/`, Shader Model 5.0 런타임 컴파일

최근 관련 commit:

```text
6ec22d3 피카츄, 망나뇽 로드
ad29527 GameObject 하나가 Model 하나를 참조
aeff863 젤다 모델 로드 성공
c1fd4e0 AssetManager에 Model 로딩 및 캐싱 추가
251080e Model Data 정의 및 Clang-Format
64d77c6 ModelLoader에 Material 텍스처 경로 추출 추가
dce67ab ModelLoade -> ModelData
d8db30d ModelLoader 도입
5a72ec7 AssetManager Material 소유
4feaee4 08_23 Codex Update (from NoteBook)
ee953a2 AssetManager mesh 소유
```

## 핵심 아키텍처 원칙

- Renderer는 Scene, GameObject, Transform, Camera, Model과 ImGui를 모른다.
- Scene과 GameObject는 DX11 렌더링 과정이나 ImGui를 모른다.
- GraphicsDevice와 GraphicsResourceManager는 AssetManager, Renderer, Scene과 AppBase를 모른다.
- ModelLoader는 AssetManager, GraphicsResourceManager와 Renderer를 모르며 Assimp 데이터를 독립 CPU 데이터로 변환한다.
- AssetManager는 GraphicsResourceManager를 비소유 참조하여 논리 에셋을 만들고 캐싱한다.
- AppBase가 초기화, 테스트 콘텐츠, ImGui 편집과 Scene→RenderItem 변환을 조정한다.
- GPU 객체 소유, 논리 Asset 소유와 Scene의 비소유 참조를 명확히 구분한다.
- 현재 사용처가 없는 ECS, shared_ptr/weak_ptr, generation, free list와 과도한 인터페이스 계층을 미리 도입하지 않는다.

## 현재 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice
│  ├─ Device / Context / SwapChain
│  ├─ 기본 BackBuffer RTV / Depth Texture / DSV
│  ├─ Resize
│  └─ Present
├─ GraphicsResourceManager --비소유--> GraphicsDevice
│  ├─ BufferResource[] → Constant / Vertex / Index Buffer ComPtr
│  └─ TextureResource[] → Texture2D / SRV ComPtr
├─ AssetManager --비소유--> GraphicsResourceManager
│  ├─ string key → unique_ptr<Mesh>
│  ├─ string path → unique_ptr<Texture>
│  ├─ string key → unique_ptr<Material>
│  └─ model path → unique_ptr<Model>
│     └─ ModelPart[] --비소유--> Mesh / Material
├─ Renderer --비소유--> GraphicsDevice / GraphicsResourceManager
│  ├─ Shader / InputLayout / Pipeline State
│  └─ Camera / Light / Object / Material BufferHandle
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     ├─ MeshComponent --비소유--> Mesh / Material
│     └─ ModelComponent --비소유--> Model
├─ Camera
└─ DirectionalLight

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
Renderer / Mesh --BufferHandle--> GraphicsResourceManager
Renderer / Texture --TextureHandle--> GraphicsResourceManager
```

AppBase 멤버는 `GraphicsDevice → GraphicsResourceManager → AssetManager → Renderer → Scene` 순서로 선언돼 있다. C++ 멤버는 역순으로 파괴되므로 Scene과 Renderer가 먼저 소멸하고 AssetManager, ResourceManager와 GraphicsDevice가 뒤에 소멸한다. Scene이 가진 비소유 Asset 포인터의 수명은 현재 안전하다.

## 현재 완료된 주요 기능

### 플랫폼과 렌더링 기반

- Win32 Window와 메시지 루프
- DX11 Device, Context, SwapChain, BackBuffer RTV와 Depth Buffer
- `WM_SIZE`에 따른 SwapChain/RTV/Depth 재생성
- Shader Model 5.0 Vertex/Pixel Shader 런타임 컴파일
- Input Layout, Rasterizer/DepthStencil/Sampler State와 Indexed Drawing
- ImGui 패널 폭을 제외한 Scene Viewport
- GameTimer, Delta Time과 ImGui FPS
- TensorWorks UE-Clang-Format 기반 `.clang-format`

### Scene, Camera와 제출 경계

- Scene의 `vector<unique_ptr<GameObject>>` 소유
- GameObject의 이름, Transform, MeshComponent와 ModelComponent
- Camera Position, +Z Forward Yaw/Pitch, FOV와 Aspect Ratio
- Scene Object 선택과 독립 Transform 편집
- `FrameRenderData`의 View/Projection/DirectionalLight 제출
- `RenderItem`의 Mesh/Material/World Matrix 제출
- Renderer가 Scene, GameObject, Camera, Model과 ImGui를 모르는 경계 유지

### GraphicsResourceManager와 AssetManager

- BufferHandle과 TextureHandle의 index 기반 무효 상태
- GraphicsResourceManager가 Buffer와 Texture2D/SRV ComPtr를 단독 소유
- Constant/Vertex/Index Buffer 생성·조회·Dynamic Update
- Camera/Light/Object/Material Constant Buffer의 Handle 이관
- Mesh Vertex/Index Buffer와 Texture SRV의 Handle 이관
- AssetManager의 Mesh, Texture, mutable Material과 Model `unique_ptr` 캐시
- 같은 key/path 재요청 시 기존 논리 Asset 주소 반환
- AppBase의 직접 Mesh/Texture/Material 값 소유 제거

### 외부 Model 파이프라인

- Assimp를 vcpkg로 데스크톱과 노트북 환경에 구성
- `ModelData`와 `ImportedMeshData`로 독립 CPU 데이터 정의
- ModelLoader가 Position, Normal, UV와 삼각형 Index를 복사
- `aiProcess_Triangulate`, `aiProcess_ConvertToLeftHanded`, `aiProcess_GenSmoothNormals` 사용
- 모델 디렉터리 기준 상대 Diffuse/BaseColor Texture 경로 해석
- AssetManager가 각 Imported Mesh를 Mesh/Material/Texture로 GPU 업로드
- Model이 ModelPart 배열을 소유하고 각 Part가 Mesh/Material을 비소유 참조
- ModelComponent가 AssetManager 소유 Model을 `const Model*`로 비소유 참조
- AppBase가 Model의 각 Part를 동일 World Matrix의 RenderItem으로 변환
- FBX Zelda, OBJ Pikachu, glTF Dragonite 출력 확인
- 여러 파트의 BaseColor Texture와 GameObject 단위 Transform/Scale 확인

현재 AppBase 테스트 장면은 Pikachu와 Dragonite를 활성화해 두었고 Zelda 및 Cube/Triangle 관련 코드는 일부 주석 처리돼 있다. 이는 임시 검증 콘텐츠이며 Renderer 구조가 아니다.

### 현재 기본 조명

- Directional Light 방향, 색과 세기
- Normal 기반 Diffuse 조명
- Texture Albedo와 Material BaseColor 결합
- Ambient와 Diffuse를 분리해 빛을 등진 면의 Texture가 완전히 사라지지 않게 함
- Ambient Strength는 현재 Pixel Shader 내부 `0.4f` 하드코딩 상태

사용자가 Pikachu와 Dragonite에서 Texture, Diffuse와 Ambient 결과를 실행 확인했다. 이번 문서 갱신에서 Codex는 빌드나 실행을 수행하지 않았다.

## 현재 초기화와 프레임 흐름

### 초기화

1. AppBase가 Window를 생성한다.
2. GraphicsDevice가 Device/Context/SwapChain/RTV/Depth를 만든다.
3. GraphicsResourceManager가 GraphicsDevice를 비소유 참조한다.
4. AssetManager가 GraphicsResourceManager를 비소유 참조한다.
5. Renderer가 GraphicsDevice와 ResourceManager를 비소유 참조하고 Pipeline/Shader/Constant Buffer Handle을 만든다.
6. ImGui Win32/DX11 Backend를 초기화한다.
7. AssetManager가 ModelLoader의 ModelData를 받아 Mesh/Material/Texture/Model을 생성하고 캐싱한다.
8. Scene의 GameObject ModelComponent에 Model 포인터를 연결한다.

### 프레임

1. Win32 메시지와 Resize를 처리하고 GameTimer를 Tick한다.
2. ImGui에서 Scene, Camera, Light, Transform과 Material을 편집한다.
3. Scene View 크기로 Camera Aspect Ratio와 Renderer Viewport를 갱신한다.
4. AppBase가 FrameRenderData를 만들어 Renderer::BeginFrame에 전달한다.
5. Renderer가 Camera/Light Constant Buffer를 갱신한다.
6. AppBase가 Scene GameObject를 순회한다.
7. ModelComponent가 있으면 모든 ModelPart를, 그렇지 않으면 MeshComponent를 RenderItem으로 변환한다.
8. Renderer::DrawRenderItem이 Object/Material Constant Buffer를 갱신한다.
9. Renderer가 ResourceManager에서 Buffer/SRV raw pointer를 즉시 대여해 바인딩한다.
10. DrawIndexed 후 ImGui를 렌더링하고 Present한다.

## 현재 규칙과 보류 항목

- BufferHandle과 TextureHandle은 리소스를 소유하지 않는 index 기반 값 타입이다.
- ResourceManager 조회 시 Handle의 배열 범위를 다시 검사한다.
- raw DirectX 포인터는 즉시 바인딩에만 사용하고 외부에서 Release하거나 장기간 보관하지 않는다.
- ResourceManager는 append-only이며 개별 삭제, 슬롯 재사용과 generation을 지원하지 않는다.
- AssetManager의 key/path는 입력 문자열 그대로 사용한다. 경로 정규화는 아직 없다.
- 같은 Mesh/Material key에 다른 데이터를 전달해도 기존 에셋을 반환한다. key가 정체성이다.
- ModelLoader는 `pScene->mMeshes`를 평면 순회한다. aiNode 계층 Transform과 Mesh Instance를 아직 반영하지 않는다.
- Material과 ModelLoader는 BaseColor/Diffuse Texture만 사용한다. Normal Map, Metallic/Roughness와 glTF PBR 데이터는 아직 렌더링하지 않는다.
- Model 원본 단위가 서로 다르므로 현재 GameObject의 균일 Scale로 배치 크기를 맞춘다. `1 unit = 1m`와 ImportScale 정책은 실제 골목 에셋 단계에서 정리한다.
- Texture sRGB/Gamma 정책, 기본 Material/Texture와 Material 없는 RenderItem 정책은 아직 명시적으로 정리하지 않았다.
- 일부 한글 주석 인코딩이 깨져 있다. 기능 변경과 분리해 정리한다.
- 외부 캐릭터 에셋은 테스트용이다. 공개 포트폴리오나 저장소 배포 전 원저작자, 라이선스와 재배포 허용 범위를 확인하고 라이선스 파일을 보존한다.
- `imgui.ini`는 실행 중 변경되는 로컬 산출물이므로 기능 commit에서 제외한다.

## 바로 다음 작업: Ambient Strength 데이터 연결

### 필요한 이유

현재 Ambient 계산은 효과가 확인됐지만 Pixel Shader의 `0.4f`에 고정돼 있다. 조명 설정을 CPU 데이터로 관리하고 ImGui에서 편집하려면 DirectionalLight부터 HLSL Constant Buffer까지 동일한 값을 전달해야 한다.

### 작업 순서

1. `DirectionalLight`에 `ambientStrength`를 추가하고 기본값을 현재 결과와 같은 `0.4f`로 둔다.
2. CPU `LightConstantData`의 마지막 `pad`를 `ambientStrength`로 바꾼다. 구조체 크기는 32바이트로 유지한다.
3. Renderer 초기값과 `BeginFrame()` 복사에 ambientStrength를 연결한다.
4. HLSL LightConstantData의 `pad`를 `ambientStrength`로 바꾼다.
5. Pixel Shader의 지역 하드코딩 변수를 제거하고 cbuffer 값을 사용한다.
6. ImGui Light 영역에 `0.0f~1.0f` Ambient Strength 슬라이더를 추가한다.
7. Pikachu와 Dragonite에서 Light Intensity와 Ambient Strength가 독립적으로 동작하는지 확인한다.

### 완료 조건

- Ambient Strength가 0이면 빛을 등진 면이 어두워진다.
- 값을 올리면 최소 밝기만 증가한다.
- Light Intensity는 직접광에만 영향을 준다.
- CPU와 HLSL LightConstantData의 필드 순서와 32바이트 크기가 일치한다.
- 사용자가 실행 화면에서 여러 Model에 동일한 조명 값이 적용되는 것을 확인한다.

## Ambient 완료 후 진행 순서

1. STEP 3-1 Editor/Play 상태 정의
2. ImGui Play/Stop 버튼과 현재 상태 표시
3. Play 입력과 ImGui 편집 입력 분리
4. deltaTime 기반 WASD 이동
5. 마우스 Yaw/Pitch, Pitch 제한, 커서 캡처와 ESC 해제
6. STEP 4 골목 Greybox와 최소 상호작용 흐름
7. 다수 Point Light와 네온 Emissive Material
8. Shadow Mapping
9. 젖은 바닥, Roughness/Normal Map과 반사
10. HDR Scene Target, Bloom, Tone Mapping
11. 안개, 비, 전원 장치와 순차 점등
12. 디버그, 최적화와 1~2분 최종 연출

ModelLoader의 aiNode/PBR 확장은 당장 다음 작업이 아니다. 실제 골목 에셋에서 파트 배치가 깨지거나 Normal/PBR Material이 필요해질 때 다시 연다.

## 새 환경에서 바로 확인할 체크리스트

- HEAD가 최소 `6ec22d3` 이상인지 확인한다.
- `AGENTS.md`와 이 문서의 마지막 갱신일이 2026-08-27인지 확인한다.
- `git status`에서 문서 변경과 사용자의 별도 `.gitignore` 변경을 구분한다.
- AssetManager가 Mesh, Texture, Material과 Model을 소유하는지 확인한다.
- ModelLoader가 ModelData만 만들고 GPU 자원을 생성하지 않는지 확인한다.
- ModelComponent가 `const Model*`를 비소유 참조하는지 확인한다.
- AppBase가 ModelPart를 RenderItem으로 변환하고 Renderer가 Model을 모르는지 확인한다.
- Pixel Shader의 Ambient Strength가 아직 `0.4f` 하드코딩인지 확인한다.
- 바로 다음 작업은 Ambient Strength의 CPU→GPU→HLSL→ImGui 연결이다.
- 사용자가 요청하지 않으면 빌드나 실행을 하지 않는다.
