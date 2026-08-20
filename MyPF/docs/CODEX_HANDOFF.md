# MyPF 작업 인계

마지막 갱신: 2026-08-20

## 이 문서의 목적

노트북과 데스크톱의 Codex 작업 맥락을 Git으로 공유하기 위한 현재 체크포인트다.

새 환경에서는 다음 순서로 상태를 확인한다.

1. 저장소 루트의 `AGENTS.md`를 읽는다.
2. 이 문서를 읽는다.
3. 실제 소스, `git status`와 최근 commit을 대조한다.
4. 문서와 코드가 다르면 실제 코드를 우선한다.
5. 사용자의 명시적 요청 없이 빌드, 실행 또는 소스 수정을 하지 않는다.

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
- Codex는 기능의 목적과 배경지식, 책임 경계와 완료 조건을 먼저 설명한다.
- 사용자가 직접 판단하고 구현할 수 있도록 검토 가능한 작은 기능 단위로 진행한다.
- 사용자가 막힌 문법이나 API를 구체적으로 질문하면 더 직접적으로 설명한다.
- 사용자가 `수정 완료`라고 하면 실제 파일을 다시 읽어 리뷰한다.
- 빌드, 컴파일, 실행은 사용자가 명시적으로 요청할 때만 한다. 요청 시 기본 검증 구성은 `Debug | x64`다.
- ImGui 기반 편집 환경을 먼저 확장하고 플레이 입력과 상호작용은 이후 추가한다.
- 핵심 원칙은 “하위 시스템은 상위 시스템을 전혀 모른다”이다.

## 저장소 상태

- 로컬 저장소: `C:/Users/tls15/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 현재 브랜치: `main`
- 문서 갱신 전 확인 HEAD: `2c26322` (`Vertex/Index Buffer 이관`)
- 문서 갱신 시작 시 확인 작업 트리: clean
- 이번 작업에서 Codex가 변경한 파일: `AGENTS.md`, `MyPF/docs/CODEX_HANDOFF.md`
- 최종 확인 시 `MyPF/AppBase.cpp`에 Mesh GPU Buffer 생성 설명 주석 1줄이 별도로 추가돼 있다. Codex가 만든 변경이 아니므로 그대로 보존했다.
- 솔루션: `MyPF/MyPF.sln`
- 기본 구성: `Debug | x64`
- 프로젝트 작업 디렉터리: `MyPF/`
- Shader 경로: `MyPF/Shaders/`
- HLSL은 Shader Model 5.0으로 런타임 컴파일한다.

최근 관련 commit:

```text
2c26322 Vertex/Index Buffer 이관
71e0e1f enderer Constant Buffer 이관 완료
8a9242f Renderer Constant Buffer 이관
1c2299f 리소스 매니저 작성완료
d016aef 리소스매니저 시작
```

사용자는 최신 코드에서 빌드 성공, 화면 정상 출력과 현재 기능의 정상 동작을 확인했다고 보고했다. 이번 문서 갱신 후에는 소스가 바뀌지 않았으므로 Codex가 별도 빌드나 실행을 하지 않았다.

## 현재 완료된 기능

### 플랫폼과 기본 렌더링

- Win32 Window와 메시지 루프
- DX11 Device, DeviceContext와 SwapChain
- BackBuffer RTV, Depth Texture/DSV와 DepthStencil State
- Rasterizer State와 Sampler State
- `WM_SIZE`에 따른 SwapChain/RTV/Depth 재생성
- ImGui 패널 폭을 제외한 Scene Viewport
- Vertex/Pixel Shader 런타임 컴파일
- Input Layout과 Indexed Drawing
- `GameTimer`, Delta Time과 ImGui FPS 표시

### Scene과 제출 경계

- `Scene`이 `vector<unique_ptr<GameObject>>`로 GameObject 소유
- `GameObject`가 이름, Transform과 MeshComponent 보유
- `MeshComponent`가 `const Mesh*`와 `Material*`를 비소유 참조
- `RenderItem`이 Mesh, Material과 World Matrix를 Renderer로 전달
- `FrameRenderData`가 View, Projection과 DirectionalLight를 Renderer로 전달
- AppBase가 Scene을 순회하고 RenderItem을 구성
- Renderer는 Scene, GameObject, Transform, Camera와 ImGui를 모름

### Camera와 ImGui

- Camera Position과 View Matrix
- +Z Forward 기준 Yaw/Pitch 회전
- Perspective Projection과 FOV
- Scene View 크기에 따른 Aspect Ratio 갱신
- Scene GameObject 목록과 선택
- 선택 오브젝트의 Transform 독립 편집
- 선택 오브젝트의 Material BaseColor 편집
- 배경색과 Directional Light 값 편집

### Mesh, 조명, Texture와 Material

- Cube와 Triangle Mesh
- Vertex Position/Color/Normal/UV
- Object/Camera Constant Buffer 분리
- Directional Light와 Light Constant Buffer
- 기본 Normal 기반 Diffuse 조명
- Texture2D/SRV와 Sampler
- Material과 Material Constant Buffer
- 같은 Texture 공유 및 오브젝트별 BaseColor
- 오브젝트별 Mesh/Material 연결

### GraphicsDevice 분리

- `GraphicsDevice`가 Device, Context, SwapChain, 기본 RTV와 기본 Depth Texture/DSV 소유
- `GraphicsDevice::Initialize`, `Resize`, `Present`
- AppBase가 GraphicsDevice를 Renderer보다 먼저 소유하고 초기화
- Renderer가 GraphicsDevice를 비소유 포인터로 참조
- BeginFrame이 GraphicsDevice의 Context/RTV/DSV 사용
- EndFrame이 `GraphicsDevice::Present()`에 위임
- WM_SIZE가 양수 크기와 Device 상태를 확인한 뒤 Resize 요청

### BufferHandle과 GraphicsResourceManager

- `BufferHandle`은 무효 상태를 표현하는 index 기반 값 타입
- `GraphicsResourceManager`가 `GraphicsDevice`를 비소유 포인터로 참조
- 내부 `BufferResource`가 실제 `ComPtr<ID3D11Buffer>`, ByteWidth와 CPU Write 가능 여부 보유
- Handle 유효성, 배열 범위와 Buffer 유효성을 검사하는 `GetBuffer`
- Immutable Vertex/Index Buffer 생성
- Dynamic Constant Buffer 생성
- Map/WriteDiscard/Unmap 기반 Buffer 업데이트
- Camera/Light/Object/Material Constant Buffer의 `ComPtr`를 BufferHandle로 이관
- Mesh Vertex/Index Buffer의 `ComPtr`를 BufferHandle로 이관
- Renderer가 ResourceManager에서 raw Buffer를 즉시 대여해 바인딩

## 현재 소유 및 의존 구조

```text
AppBase
├─ GraphicsDevice
│  ├─ ID3D11Device
│  ├─ ID3D11DeviceContext
│  ├─ IDXGISwapChain
│  ├─ 기본 BackBuffer RTV
│  ├─ 기본 Depth Texture / DSV
│  ├─ Resize
│  └─ Present
├─ GraphicsResourceManager --비소유--> GraphicsDevice
│  └─ BufferResource[]
│     └─ Constant / Vertex / Index Buffer ComPtr
├─ Renderer --비소유--> GraphicsDevice, GraphicsResourceManager
│  ├─ Shader / InputLayout
│  ├─ Rasterizer / DepthStencil / Sampler State
│  ├─ Camera / Light BufferHandle
│  └─ Object / Material BufferHandle
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     └─ MeshComponent
│        ├─ 비소유 const Mesh*
│        └─ 비소유 Material*
├─ Camera
├─ Mesh m_cubeMesh / m_triangleMesh
│  └─ Vertex / Index BufferHandle
├─ Texture m_texture
│  └─ Texture2D / SRV ComPtr 직접 소유
└─ Material m_cubeMaterial / m_triangleMaterial
   ├─ BaseColor
   └─ 비소유 const Texture*

AppBase --FrameRenderData--> Renderer::BeginFrame
AppBase --RenderItem-------> Renderer::DrawRenderItem
Renderer / Mesh --BufferHandle--> GraphicsResourceManager
Renderer --Texture::GetShaderResourceView()--> SRV
```

AppBase 멤버 선언 순서는 `GraphicsDevice → GraphicsResourceManager → Renderer`다. C++ 멤버는 역순으로 파괴되므로 Renderer가 먼저 소멸하고 ResourceManager와 GraphicsDevice가 뒤에 소멸한다.

Scene은 AppBase가 소유한 Mesh, Material과 Texture보다 먼저 파괴되므로 MeshComponent의 비소유 포인터 수명도 현재 선언 순서에서는 안전하다.

## 현재 초기화 흐름

1. `Main.cpp`가 AppBase를 생성한다.
2. AppBase가 Win32 Window를 생성한다.
3. GraphicsDevice가 Device/Context/SwapChain/RTV/Depth를 생성한다.
4. GraphicsResourceManager가 GraphicsDevice 주소를 비소유 저장한다.
5. Renderer가 GraphicsDevice와 ResourceManager 주소를 비소유 저장한다.
6. Renderer가 Pipeline State, Shader/InputLayout과 네 Constant Buffer Handle을 만든다.
7. ImGui Win32/DX11 Backend를 초기화한다.
8. Scene에 Cube와 Triangle GameObject를 만든다.
9. GeometryGenerator가 MeshData를 생성한다.
10. Mesh가 ResourceManager를 통해 Vertex/Index Buffer Handle을 만든다.
11. Texture가 아직 GraphicsDevice의 Device와 D3D11Utils를 통해 Texture2D/SRV를 직접 생성한다.
12. Material과 Mesh를 각 GameObject의 MeshComponent에 연결한다.

## 현재 프레임 흐름

1. Win32 메시지와 `WM_SIZE`를 처리한다.
2. GameTimer를 Tick한다.
3. ImGui 프레임을 시작한다.
4. Scene/Camera/Light/Transform/Material 값을 편집한다.
5. ImGui Render Data를 확정한다.
6. ImGui 폭을 제외한 Scene View 크기를 계산한다.
7. Camera Aspect Ratio와 Renderer Viewport를 갱신한다.
8. AppBase가 Camera와 DirectionalLight로 `FrameRenderData`를 만든다.
9. `Renderer::BeginFrame()`이 GraphicsDevice에서 Context/RTV/DSV를 빌려 Clear하고 바인딩한다.
10. ResourceManager를 통해 Camera/Light Constant Buffer를 갱신한다.
11. AppBase가 Scene의 GameObject를 순회하고 `RenderItem`을 만든다.
12. `Renderer::DrawRenderItem()`이 Object/Material Constant Buffer를 갱신한다.
13. Renderer가 Mesh의 Vertex/Index BufferHandle을 받는다.
14. ResourceManager에서 실제 Vertex/Index/Constant Buffer raw pointer를 즉시 대여한다.
15. Texture에서 SRV를 받아 Shader, Buffer, SRV, Sampler를 바인딩한다.
16. `DrawIndexed()`를 호출한다.
17. ImGui DrawData를 렌더링한다.
18. `Renderer::EndFrame()`이 GraphicsDevice에 Present를 요청한다.

## 주요 클래스의 현재 역할

### AppBase

- Window, 시스템 초기화와 메인 루프 조정
- 현재 테스트 Scene과 Asset 직접 소유
- ImGui 편집
- Scene을 Renderer 제출 데이터로 변환
- 현재는 플랫폼 Application과 데모 콘텐츠 책임이 함께 있는 과도기 클래스

`AppBase`를 당장 대규모 분리하지 않는다. UI와 콘텐츠가 더 커질 때 private 메서드 분리 또는 실제 Demo Application 파생/분리를 검토한다.

### GraphicsDevice

- DX11 실행 환경과 기본 Window Surface 소유
- Resize와 Present
- 상위 Renderer/Scene을 모름

### GraphicsResourceManager

- 일반 GPU Buffer의 유일한 소유자
- Buffer 생성, 조회와 업데이트
- 데이터의 Camera/Light/Material 의미나 Shader Slot을 모름

### Renderer

- 렌더링 순서와 DX11 Pipeline 정책 담당
- Frame 단위 Camera/Light, Draw 단위 Object/Material 갱신
- ResourceManager Handle을 실제 Buffer로 해석해 즉시 바인딩
- Texture/SRV는 아직 Texture에서 직접 가져오는 과도기

### Scene / GameObject

- Scene은 GameObject 수명과 목록을 관리
- GameObject는 논리 상태를 보유
- DirectX와 ImGui를 모름

### Mesh / Texture / Material

- Mesh는 GPU Buffer Handle과 Draw에 필요한 Stride/Index Count 보유
- Texture는 아직 Texture2D/SRV ComPtr 직접 소유
- Material은 BaseColor와 Texture 비소유 참조 보유

## BufferHandle과 ResourceManager 현재 규칙

- Handle은 리소스를 식별하고 무효 상태를 표현할 뿐이다.
- Handle에 DirectX 포인터나 `ComPtr`를 넣지 않는다.
- ResourceManager가 실제 Buffer `ComPtr`의 유일한 소유자다.
- `GetBuffer()`의 raw pointer는 즉시 바인딩할 때만 빌려 쓴다.
- raw pointer를 외부에서 `Release()`하거나 멤버로 장기간 저장하지 않는다.
- Handle의 `IsValid()`만 믿지 않고 배열 범위도 검사한다.
- 초기 구현은 리소스를 삭제하지 않으므로 index만 사용한다.
- 개별 삭제와 슬롯 재사용 요구가 생길 때 generation을 추가한다.
- 처음부터 `shared_ptr`, `weak_ptr`, free list나 범용 리소스 상속 구조를 넣지 않는다.

## 현재 코드 리뷰에서 확인한 개선 후보

### 다음 작업 전에 확인할 안전성

- AppBase는 Render 순회에서 Mesh만 검사한다. Material이 없는 GameObject는 Renderer 실패와 PostQuit로 이어질 수 있으므로 Renderable 판정 또는 기본 Material 정책이 필요하다.
- Renderer는 GraphicsDevice뿐 아니라 ResourceManager가 유효하다는 조건도 명시적으로 표현하는 편이 좋다.
- WM_SIZE에서 `GraphicsDevice::Resize()` 반환값을 현재 사용하지 않는다. 실패 기록 정책을 도입할 때 함께 보완한다.
- ImGui 부분 초기화 실패와 Shutdown 상태 추적은 이전 결정대로 기능 우선순위 뒤에 둔다.

### 중복과 가독성

- Renderer Input Layout Offset의 `4 * 3` 계산은 현재 Vertex 구조에서는 맞지만 `offsetof(Vertex, member)` 형태가 구조 변경에 더 안전하다.
- GeometryGenerator는 Position/Color/Normal/UV 평행 배열을 사용하므로 Vertex 직접 구성 또는 Cube 면 추가 helper를 검토한다.
- Renderer의 Camera/Light/Object/Material Constant Buffer 데이터와 Handle은 단순 배열보다 Frame 단위와 Draw 단위의 의미 있는 그룹이 적합하다.
- Renderer `Initialize()`가 커지면 같은 파일 안에서 Pipeline State, Constant Buffer, Shader 생성 private 메서드로 나눈다. 별도 클래스를 미리 추가하지 않는다.
- AppBase `UpdateGui()`가 더 커지면 Performance, Light, Camera, Scene Hierarchy, Inspector 단위 private 메서드로 나눈다.
- Vertex Color는 Shader 입출력을 통과하지만 최종 Pixel Color 계산에는 사용되지 않는다. 실제 사용하거나 제거한다.
- D3D11Utils에는 Buffer/Depth 관련 이전 helper가 남아 있다. Texture 이관 뒤 실제 호출되지 않는 함수만 제거한다.
- 일부 한글 주석이 인코딩 문제로 깨져 있다. 기능 변경과 섞지 않고 별도 정리한다.

### 현재 허용하는 과도기 상태

- Texture가 Texture2D/SRV를 직접 소유한다.
- AppBase가 테스트용 Mesh, Texture와 Material을 고정 멤버로 가진다.
- AppBase가 플랫폼 처리, ImGui와 테스트 콘텐츠 설정을 함께 담당한다.
- ResourceManager는 개별 삭제와 generation을 지원하지 않는다.
- Camera의 Orthographic 분기는 전환 API가 없어 아직 사용되지 않는다.

이 항목들은 현재 기능을 막지 않으므로 한 번에 모두 리팩토링하지 않는다.

## 바로 다음 작업

다음 큰 기능 단위는 `Texture2D/SRV의 GraphicsResourceManager 이관`이다.

### 이 작업이 필요한 이유

- Buffer만 Manager가 소유하고 Texture는 논리 객체가 직접 GPU ComPtr를 소유하는 현재 불균형을 해소한다.
- 이후 외부 Texture 로딩, AssetManager, Material 공유와 SRV 재사용의 기반이 된다.
- Shadow Map, HDR Scene Texture와 Bloom Texture 같은 GPU Texture 리소스 관리로 확장할 수 있다.
- Renderer가 Texture 구현이 아니라 Handle과 ResourceManager 경계를 통해 SRV를 받게 할 수 있다.

### 권장 세부 단계

1. Texture Resource의 최소 구성과 수명을 먼저 정의한다.
2. `TextureHandle`이 BufferHandle과 마찬가지로 index와 무효 상태만 갖게 한다.
3. ResourceManager 내부에 Texture2D/SRV를 함께 소유하는 Resource를 추가한다.
4. Texture 파일 생성 API와 SRV 조회 API를 추가한다.
5. `Texture::Initialize`가 Device가 아니라 ResourceManager를 사용하도록 바꾼다.
6. Texture의 직접 `ComPtr`를 TextureHandle로 교체한다.
7. Renderer가 ResourceManager에서 SRV를 조회해 즉시 바인딩한다.
8. Cube/Triangle의 기존 Texture와 BaseColor 결과가 동일한지 확인한다.
9. 이관 후 D3D11Utils의 미사용 Texture/Buffer helper를 호출 기준으로 정리한다.

Material이 Texture 논리 객체를 계속 참조할지 TextureHandle을 직접 보관할지는 이관 단계에서 실제 책임을 비교해 결정한다. AssetManager까지 동시에 도입하지 않는다.

## 이후 주요 로드맵

1. Texture/SRV Handle과 GraphicsResourceManager 이관
2. AssetManager와 외부 Mesh/Texture 로딩
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

최종 데모 전체 작업량 기준 진행률은 대략 30~35%로 본다. 기본 렌더링, Scene 제출 경계와 Buffer 리소스 소유 기반은 마련됐지만, 외부 콘텐츠, 플레이 입력, 다수 조명, 고급 렌더링과 최종 연출은 대부분 남아 있다.

## 다음 환경에서 바로 확인할 체크리스트

- `AGENTS.md`의 마지막 갱신일이 2026-08-20인지 확인
- 이 문서와 실제 HEAD/작업 트리 대조
- `GraphicsResourceManager`가 Constant/Vertex/Index Buffer를 소유하는지 확인
- Renderer의 네 Constant Buffer가 `BufferHandle`인지 확인
- Mesh의 Vertex/Index Buffer가 `BufferHandle`인지 확인
- Texture가 아직 Texture2D/SRV `ComPtr`를 직접 소유하는지 확인
- 사용자에게 다음 작업을 시작할지 확인한 뒤 Texture 이관을 작은 기능 단위로 안내
- 사용자가 요청하지 않으면 빌드나 실행을 하지 않음
