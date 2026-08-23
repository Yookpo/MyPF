# MyPF 작업 인계

마지막 갱신: 2026-08-23

## 이 문서의 목적

노트북과 데스크톱의 Codex 작업 맥락을 Git으로 공유하기 위한 현재 체크포인트다.

새 환경에서는 다음 순서로 시작한다.

1. 저장소 루트의 `AGENTS.md`를 읽는다.
2. 이 문서를 읽는다.
3. 실제 소스, `git status`와 최근 commit을 대조한다.
4. 문서와 코드가 다르면 실제 코드를 우선한다.
5. 사용자의 명시적 요청 없이 빌드, 실행 또는 프로젝트 소스 수정을 하지 않는다.

## 최종 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 연출을 핵심 장면으로 삼는다.

주요 목표 기능:

- 1인칭 카메라와 WASD/마우스 조작
- 짧은 골목 탐색
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 점광원
- 조명 상태 변화와 순차 점등
- 젖은 바닥 반사
- 그림자
- Bloom
- 안개 또는 볼륨감 있는 분위기
- 색조 보정
- ImGui 기반 씬 편집 및 디버그 UI

최종 결과물은 범용 엔진이 아니라 `DX11 렌더링 기술 + 확장 가능한 구조 + 짧지만 완성된 플레이 경험`을 보여주는 포트폴리오다.

## 사용자와 Codex의 작업 방식

- 사용자가 모든 C++와 HLSL 코드를 직접 작성한다.
- Codex는 사용자의 명시적 요청 없이 프로젝트 소스를 생성하거나 자동 수정하지 않는다.
- Codex는 기능의 목적, 기대 효과, 책임 경계와 완료 조건을 먼저 설명한다.
- 한 기능을 검토 가능한 작은 단계로 나누고, 사용자가 스스로 판단하도록 질문과 설계 기준을 먼저 제시한다.
- 사용자가 구체적인 구현 도움을 요청하면 필요한 API와 코드 구조를 직접 설명한다.
- 사용자가 `수정 완료`라고 하면 실제 작업 트리를 다시 읽고 리뷰한다.
- 빌드, 컴파일, 실행은 사용자가 명시적으로 요청할 때만 한다. 요청 시 기본 검증 구성은 `Debug | x64`다.
- 핵심 원칙은 “하위 시스템은 상위 시스템을 전혀 모른다”이다.

## 저장소 체크포인트

- 노트북 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 데스크톱 저장소: `C:/Users/tls15/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 현재 브랜치: `main`
- 문서 갱신 직전 확인 HEAD: `ee953a2` (`AssetManager mesh 소유`)
- 문서 갱신 직전 작업 트리: clean
- 이번 요청에서 Codex가 수정한 파일: `AGENTS.md`, `MyPF/docs/CODEX_HANDOFF.md`
- 솔루션: `MyPF/MyPF.sln`
- 기본 구성: `Debug | x64`
- 프로젝트 작업 디렉터리: `MyPF/`
- Shader 경로: `MyPF/Shaders/`
- HLSL은 Shader Model 5.0으로 런타임 컴파일한다.

최근 관련 commit:

```text
ee953a2 AssetManager mesh 소유
06371d8 캐싱되는것 확인
42cb164 AssetManager 도입
f4d7850 TextureHandle 렌더링 까지 연결
c2b2a2e TextureHandle 추가 및 D3D11Utils로 책임 인가
b5e9719 08_20 Codex Update
2c26322 Vertex/Index Buffer 이관
71e0e1f enderer Constant Buffer 이관 완료
8a9242f Renderer Constant Buffer 이관
1c2299f 리소스 매니저 작성완료
```

2026-08-23의 Buffer/Texture/AssetManager 변경 묶음에 대해 Codex는 빌드나 실행을 수행하지 않았다. 사용자가 요청한 경우에만 컴파일한다.

## 핵심 아키텍처 원칙

- Renderer는 Scene, GameObject, Transform, Camera와 ImGui를 모른다.
- Scene과 GameObject는 DX11 렌더링 과정과 ImGui를 모른다.
- GraphicsDevice와 GraphicsResourceManager는 AssetManager, Renderer, Scene과 AppBase를 모른다.
- AssetManager는 하위 GraphicsResourceManager를 비소유 참조해 논리 에셋을 생성한다.
- AppBase가 시스템 초기화, 테스트 콘텐츠 구성, ImGui 편집과 Scene→RenderItem 변환을 조정한다.
- GPU 리소스의 실제 소유와 논리 에셋의 소유, Scene의 비소유 참조를 구분한다.
- 현재 사용처가 없는 ECS, generation, free list, shared_ptr/weak_ptr 구조를 미리 도입하지 않는다.

## 현재 완료된 주요 기능

### 플랫폼과 렌더링 기반

- Win32 Window와 메시지 루프
- DX11 Device, DeviceContext와 SwapChain
- 기본 BackBuffer RTV와 Depth Texture/DSV
- `WM_SIZE`에 따른 SwapChain/RTV/Depth 재생성
- Rasterizer/DepthStencil/Sampler State
- Vertex/Pixel Shader 런타임 컴파일
- Input Layout과 Indexed Drawing
- ImGui 패널 폭을 제외한 Scene Viewport
- GameTimer, Delta Time과 ImGui FPS 표시

### Scene, Camera와 제출 경계

- `Scene`의 `vector<unique_ptr<GameObject>>` 소유
- GameObject의 이름, Transform과 MeshComponent
- MeshComponent의 비소유 `const Mesh*`, `Material*`
- `RenderItem`의 Mesh, Material과 World Matrix 제출
- `FrameRenderData`의 View, Projection과 DirectionalLight 제출
- Camera Position, +Z Forward Yaw/Pitch, Perspective FOV와 Aspect Ratio
- Scene 오브젝트 선택과 독립 Transform 편집
- Material BaseColor, 배경색과 Directional Light ImGui 편집

### 기본 표현

- Cube/Triangle Mesh와 Indexed Drawing
- Vertex Position/Color/Normal/UV
- 기본 Normal 기반 Directional Diffuse 조명
- Texture2D/SRV와 Sampler
- Material BaseColor와 같은 Texture 공유
- 오브젝트별 Mesh/Material 연결

### GraphicsDevice와 GraphicsResourceManager

- GraphicsDevice가 Device/Context/SwapChain/기본 RTV/Depth/Resize/Present 담당
- BufferHandle과 TextureHandle의 index 기반 무효 상태
- GraphicsResourceManager가 Constant/Vertex/Index Buffer `ComPtr` 단독 소유
- GraphicsResourceManager가 일반 Texture2D/SRV `ComPtr` 단독 소유
- Buffer 생성·조회·Dynamic 갱신과 Texture 생성·SRV 조회
- Camera/Light/Object/Material Constant Buffer의 BufferHandle 이관
- Mesh Vertex/Index Buffer의 BufferHandle 이관
- Texture의 직접 ComPtr 제거와 TextureHandle 이관
- Renderer가 Handle을 ResourceManager의 raw Buffer/SRV로 해석해 즉시 바인딩

### D3D11Utils 책임 정리

- `CreateImmutableBuffer()`가 Vertex/Index Buffer의 공통 DX11 생성 절차 담당
- 기존 `CreateVertexBuffer()`와 `CreateIndexBuffer()`가 공통 Immutable 생성 함수에 위임
- Constant Buffer는 타입 기반 템플릿 오버로드와 `const void* + byteWidth` 비템플릿 오버로드로 분리
- Dynamic Buffer Update도 타입 기반 템플릿 오버로드와 비템플릿 오버로드로 분리
- 템플릿 오버로드는 타입 크기 추출과 컴파일 시 정렬 검사를 담당
- 비템플릿 오버로드는 `CreateBuffer`, `Map`, `memcpy`, `Unmap` 같은 실제 DX11 절차 담당
- ResourceManager는 생성 방법을 직접 구현하지 않고 검증, 소유, 메타데이터와 Handle 발급 담당

### AssetManager 최소 구현

- AssetManager가 GraphicsResourceManager를 비소유 포인터로 참조
- 문자열 경로 기반 `unique_ptr<Texture>` 캐시
- 문자열 key 기반 `unique_ptr<Mesh>` 캐시
- `LoadTexture(path)`는 같은 path 요청 시 기존 `const Texture*` 반환
- `CreateMesh(key, MeshData)`는 같은 key 요청 시 기존 `const Mesh*` 반환
- 생성 실패한 Texture/Mesh는 캐시에 넣지 않음
- AppBase의 직접 Texture와 Mesh 값 멤버 제거
- Cube/Triangle이 AssetManager의 Mesh를 비소유 참조
- Cube/Triangle이 동일한 `wall.jpg`를 두 번 요청해 같은 Texture 주소가 반환되는 조건 확인
- Visual Studio의 `Assets` 필터에 AssetManager 등록

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
│  │  └─ Constant / Vertex / Index Buffer ComPtr
│  └─ TextureResource[]
│     └─ Texture2D / SRV ComPtr
├─ AssetManager --비소유--> GraphicsResourceManager
│  ├─ string → unique_ptr<Mesh>
│  │  └─ Vertex / Index BufferHandle
│  └─ string → unique_ptr<Texture>
│     └─ TextureHandle
├─ Renderer --비소유--> GraphicsDevice, GraphicsResourceManager
│  ├─ Shader / InputLayout
│  ├─ Rasterizer / DepthStencil / Sampler State
│  └─ Camera / Light / Object / Material BufferHandle
├─ Material m_cubeMaterial / m_triangleMaterial
│  ├─ BaseColor
│  └─ 비소유 const Texture*
└─ Scene
   └─ GameObject[]
      ├─ Transform
      └─ MeshComponent
         ├─ 비소유 const Mesh*
         └─ 비소유 Material*

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
Renderer / Mesh --BufferHandle--> GraphicsResourceManager
Renderer / Texture --TextureHandle--> GraphicsResourceManager
AssetManager --생성 요청--> GraphicsResourceManager
```

AppBase 멤버 선언 순서는 `GraphicsDevice → GraphicsResourceManager → AssetManager → Renderer`다. C++ 멤버는 역순으로 소멸하므로 Renderer와 AssetManager가 ResourceManager와 GraphicsDevice보다 먼저 소멸한다. Scene과 Material은 AssetManager보다 먼저 소멸하므로 비소유 Mesh/Texture 포인터 수명도 현재 순서에서는 안전하다.

## 현재 초기화 흐름

1. AppBase가 Win32 Window를 생성한다.
2. GraphicsDevice가 Device/Context/SwapChain/RTV/Depth를 생성한다.
3. GraphicsResourceManager가 GraphicsDevice 주소를 비소유 저장한다.
4. AssetManager가 GraphicsResourceManager 주소를 비소유 저장한다.
5. Renderer가 GraphicsDevice와 ResourceManager 주소를 비소유 저장하고 Pipeline State, Shader와 Constant Buffer Handle을 만든다.
6. ImGui Win32/DX11 Backend를 초기화한다.
7. Scene에 Cube와 Triangle GameObject를 만든다.
8. GeometryGenerator가 Cube/Triangle MeshData를 만든다.
9. AssetManager가 서로 다른 key로 Mesh를 생성·소유하고 Mesh는 BufferHandle을 저장한다.
10. AssetManager가 `wall.jpg` Texture를 최초 생성한 뒤 같은 경로의 두 번째 요청에는 기존 Texture를 반환한다.
11. Texture는 TextureHandle만 저장하고 실제 Texture2D/SRV는 GraphicsResourceManager가 소유한다.
12. AppBase가 소유한 두 Material에 Texture와 BaseColor를 설정한다.
13. Mesh와 Material을 각 GameObject의 MeshComponent에 연결한다.

## 현재 프레임 흐름

1. Win32 메시지와 `WM_SIZE`를 처리하고 GameTimer를 Tick한다.
2. ImGui에서 Scene/Camera/Light/Transform/Material 값을 편집한다.
3. Scene View 크기로 Camera Aspect Ratio와 Renderer Viewport를 갱신한다.
4. AppBase가 `FrameRenderData`를 만들어 `Renderer::BeginFrame()`에 전달한다.
5. Renderer가 Camera/Light Constant Buffer를 ResourceManager를 통해 갱신한다.
6. AppBase가 Scene의 GameObject를 순회한다.
7. Mesh와 Material이 모두 있는 오브젝트만 `RenderItem`으로 변환한다.
8. `Renderer::DrawRenderItem()`이 Object/Material Constant Buffer를 갱신한다.
9. Renderer가 Mesh의 BufferHandle과 Texture의 TextureHandle을 얻는다.
10. ResourceManager에서 실제 Buffer/SRV raw pointer를 즉시 대여한다.
11. Shader, Buffer, SRV, Sampler를 바인딩하고 `DrawIndexed()`를 호출한다.
12. ImGui DrawData를 렌더링하고 `Renderer::EndFrame()`이 Present를 요청한다.

## 현재 규칙과 과도기 상태

- BufferHandle과 TextureHandle은 리소스를 소유하지 않는 index 기반 값 타입이다.
- Handle의 `IsValid()`만 믿지 않고 ResourceManager 조회에서 배열 범위를 다시 검사한다.
- ResourceManager가 반환한 raw DirectX 포인터는 즉시 바인딩에만 사용하며 외부에서 `Release()`하거나 장기간 보관하지 않는다.
- ResourceManager는 append-only 저장소이며 개별 삭제, 슬롯 재사용과 generation을 지원하지 않는다.
- AssetManager의 Texture key는 전달된 문자열 그대로 사용한다. `wall.jpg`와 `.\\wall.jpg`는 다른 key다.
- 같은 Mesh key에 다른 MeshData를 전달해도 기존 Mesh를 반환한다. key가 에셋 정체성이다.
- AssetManager가 논리 에셋 객체를 소유하고 GraphicsResourceManager가 실제 GPU 객체를 소유한다.
- Material은 편집 가능한 값이므로 현재 AppBase가 직접 소유한다. 다음 단계에서 mutable `Material*`을 반환하는 AssetManager 캐시로 이전한다.
- 외부 Mesh 파일 importer는 아직 없다. 현재 Mesh 캐시는 GeometryGenerator가 만든 MeshData를 받는다.

## 바로 다음 작업: Material AssetManager 소유 이전

2026-08-23 마지막 대화에서 다음 단계로 Material 캐시를 시작하기로 했다. 아직 코드는 작성하지 않았다.

### 필요한 이유

- AppBase의 `m_cubeMaterial`, `m_triangleMaterial` 같은 고정 멤버 증가를 막는다.
- 골목 건물, 바닥, 문, 간판용 Material의 명명, 공유와 수명을 중앙에서 관리한다.
- Mesh/Texture와 Material을 같은 논리 Asset 계층에 모은다.
- ImGui에서 Material을 편집할 수 있어야 하므로 읽기 전용 Mesh/Texture와 다른 mutable 정책을 명확히 한다.

### 첫 단계

`AssetManager.h`만 다음 방향으로 확장한다.

1. `class Material;` 전방 선언
2. `Material* CreateMaterial(const std::string& key);` 선언
3. `unordered_map<string, unique_ptr<Material>> m_materials;` 추가

반환형은 `const Material*`이 아니라 `Material*`이다. Material은 ImGui에서 BaseColor와 Texture를 변경해야 하기 때문이다.

### 이후 연결 순서

1. `AssetManager.cpp`에서 `Material.h`를 포함한다.
2. 초기화 여부와 빈 key를 검사한다.
3. 같은 key가 있으면 기존 mutable Material 포인터를 반환한다.
4. 없으면 기본 Material을 `unique_ptr`로 생성하고 캐시에 저장한다.
5. AppBase의 두 Material 값 멤버를 제거한다.
6. `cube_material`, `triangle_material`처럼 서로 다른 key로 Material을 만든다.
7. 반환된 Material에 Texture와 서로 다른 BaseColor를 설정한다.
8. MeshComponent에 AssetManager가 소유한 Material 포인터를 연결한다.
9. 같은 key 공유 시 하나를 편집하면 모든 사용처에 반영되는 정책을 확인한다.
10. 인터페이스 변경 묶음이 끝나면 사용자가 원할 때 `Debug | x64` 컴파일로 확인한다.

## 알려진 개선 후보와 보류 항목

- AssetManager 경로 정규화와 상대/절대 경로 정책
- 외부 OBJ/glTF 등 Mesh 파일 importer 선택과 로딩 경계
- Material 없는 GameObject는 현재 RenderItem 생성을 건너뛴다. 추후 기본 Material/Texture 정책을 결정한다.
- Renderer의 ResourceManager 유효성 검사를 더 명시적으로 표현할 수 있다.
- GraphicsResourceManager의 공개 `UpdateBuffer`는 16-byte 정렬을 강제하므로 사실상 Constant Buffer 갱신 API다. 이름과 범위를 나중에 재검토한다.
- D3D11Utils의 타입 기반 Buffer 편의 오버로드는 모든 생성이 ResourceManager를 거치게 되면 공개 필요성을 다시 검토한다.
- AssetManager의 `OutputDebugStringW` 사용은 간접 include에 의존할 수 있다. 공통 로깅 정책 또는 직접 include 여부를 정리한다.
- ResourceManager의 개별 삭제와 generation은 실제 unload 요구가 생길 때 도입한다.
- Input Layout Offset을 `offsetof(Vertex, member)` 기반으로 개선할 수 있다.
- GeometryGenerator의 평행 배열 중복을 Vertex 직접 구성/helper로 줄일 수 있다.
- 일부 한글 주석의 문자 인코딩은 기능 변경과 분리해 정리한다.
- ImGui UI가 더 커지면 Performance, Light, Camera, Scene Hierarchy, Inspector 단위로 분리한다.
- AppBase는 플랫폼 처리, UI와 데모 콘텐츠 설정을 함께 담당하는 과도기 구조다.

## 이후 주요 로드맵

1. Material AssetManager 소유 이전
2. 외부 Mesh 파일 로딩과 Texture 경로 정책
3. ImGui Scene Hierarchy/Inspector와 배치 기능 확장
4. 1인칭 WASD/마우스 입력과 Editor/Play 상태
5. 다수 Point Light와 네온 조명
6. 사이버펑크 골목 기본 콘텐츠 구성
7. Shadow Mapping
8. 젖은 바닥 재질과 반사
9. HDR Scene Target과 Bloom
10. 안개와 색조 보정
11. 문/전원 스위치 상호작용과 순차 점등
12. 디버그 UI, 최적화와 1~2분 최종 연출

최종 데모 전체 작업량 기준 진행률은 약 35%로 본다. 기본 렌더링, Scene 제출 경계, GPU Resource 소유와 최소 Asset 캐시는 마련됐지만 외부 콘텐츠, 플레이 입력, 다수 조명, 고급 렌더링과 최종 연출은 대부분 남아 있다.

## 데스크톱에서 바로 확인할 체크리스트

- Git pull 후 HEAD가 최소 `ee953a2` 이상인지 확인
- `AGENTS.md`와 이 문서의 마지막 갱신일이 2026-08-23인지 확인
- `git status`로 문서 외 미커밋 변경이 없는지 확인
- GraphicsResourceManager가 Buffer와 Texture2D/SRV를 소유하는지 확인
- Texture가 TextureHandle만 보관하는지 확인
- Renderer가 `ResourceManager::GetSRV(TextureHandle)`로 SRV를 얻는지 확인
- AssetManager가 Mesh와 Texture를 `unique_ptr` 캐시로 소유하는지 확인
- AppBase에 Mesh/Texture 값 멤버가 없고 Material 값 멤버만 남아 있는지 확인
- 바로 다음 작업은 `AssetManager.h`의 Material 캐시 선언부터 시작
- 사용자가 요청하지 않으면 빌드나 실행을 하지 않음
