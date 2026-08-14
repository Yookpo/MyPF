# MyPF 작업 인계

마지막 갱신: 2026-08-14

## 이 문서의 목적

노트북과 데스크톱의 Codex 작업 맥락을 Git으로 공유하기 위한 현재 상태 문서다. 새 환경에서 작업을 시작할 때 저장소 루트의 `AGENTS.md`를 먼저 읽고, 그다음 이 문서와 실제 소스를 대조한다.

문서보다 현재 코드와 `git status`를 우선한다.

## 최종 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 걸으며 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 연출을 핵심 장면으로 삼는다.

주요 목표 기능:

- 1인칭 카메라와 WASD/마우스 조작
- 짧은 골목 탐색
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 점광원
- 조명 상태 변화 및 순차 점등
- 젖은 바닥 반사
- 그림자
- Bloom
- 안개 또는 볼륨감 있는 분위기
- 색조 보정
- ImGui 기반 씬 편집 및 디버그 UI

## 작업 방식과 사용자 의도

- 사용자가 모든 C++와 HLSL 코드를 직접 작성한다.
- Codex는 구조와 구현 순서를 안내하고, 사용자가 작성한 코드를 리뷰하며, 빌드와 실행으로 검증한다.
- 사용자의 명시적인 요청이 없으면 Codex가 C++/HLSL을 대신 작성하거나 자동 수정하지 않는다.
- 큰 기능을 한 번에 넣지 않고, 작게 동작하는 기능 하나씩 확장한다.
- ImGui 기반 씬 편집 환경을 먼저 만들고, 그다음 플레이 모드의 키보드/마우스 입력을 넣는다.
- Editor와 Play가 완전히 별개의 Scene 구조를 가지기보다 같은 Scene 데이터를 활용하는 방향이다.
- 핵심 설계 원칙은 “하위 시스템은 상위 시스템을 전혀 모른다”이다.

## 저장소 정보

- 로컬 저장소: `C:/Users/Diguedman/source/repos/MyPF`
- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 현재 기준 브랜치: `main`
- 솔루션: `MyPF/MyPF.sln`
- 기본 빌드 구성: `Debug | x64`
- 프로젝트 작업 디렉터리: `MyPF/`
- Shader 경로: `MyPF/Shaders/`
- HLSL은 Shader Model 5.0으로 런타임 컴파일한다.

## 현재 완료된 기반 기능

### 플랫폼과 DirectX 11

- Win32 윈도우 생성과 메시지 루프
- DX11 Device, DeviceContext, SwapChain
- Render Target View와 Depth Stencil View/State
- Rasterizer State
- `WM_SIZE`에 따른 SwapChain Buffer, RTV, Depth Buffer 재생성
- Scene 영역에 맞춘 Viewport 설정
- Vertex/Pixel Shader 런타임 컴파일과 Input Layout 생성
- Model/View/Projection을 사용하는 기본 HLSL
- Indexed Drawing
- `GameTimer`

### Scene과 오브젝트

- `Scene`이 `std::vector<std::unique_ptr<GameObject>>`로 GameObject를 소유
- `GameObject`가 이름, `Transform`, `MeshComponent`를 보유
- `Transform`이 Position/Rotation/Scale과 World Matrix 계산을 담당
- Scene에 Cube와 Triangle 두 GameObject 생성
- 두 오브젝트에 서로 다른 Mesh를 연결하여 렌더링

### Mesh와 렌더 제출 구조

- `MeshData.h`로 CPU 측 `Vertex`, `MeshData` 분리
- `ShaderConstants.h`로 셰이더 상수 구조 분리
- `Mesh`가 Vertex/Index Buffer와 Index Count를 소유
- `GeometryGenerator`가 Cube와 Triangle의 `MeshData` 생성
- `MeshComponent`가 사용할 `Mesh`를 비소유 `const Mesh*`로 참조
- `RenderItem`이 오브젝트 렌더 제출 경계 역할 수행
- `FrameRenderData`가 프레임의 View/Projection 전달 경계 역할 수행
- `Renderer::DrawRenderItem`이 특정 Cube가 아닌 전달된 Mesh를 범용 렌더링

### Camera와 ImGui 편집

- `Camera` 클래스 분리
- Camera가 Position/Forward/Up과 Perspective Projection 값을 보유
- Camera가 View/Projection 행렬을 계산
- Scene View의 폭과 높이에 맞춰 Camera Aspect Ratio를 매 프레임 갱신
- Renderer는 Camera를 직접 알지 않고 `FrameRenderData`만 받음
- ImGui 배경색 편집
- Scene GameObject 목록과 선택 상태 표시
- 선택된 GameObject의 Position/Rotation/Scale 편집
- ImGui에서 Camera Position 편집

## 현재 소유 관계

```text
AppBase
├─ Renderer
├─ Scene
│  └─ vector<unique_ptr<GameObject>>
│     └─ GameObject
│        ├─ Transform
│        └─ MeshComponent --비소유 const Mesh*--> Mesh
├─ Camera
├─ Mesh m_cubeMesh
├─ Mesh m_triangleMesh
└─ GameObject* m_selectedObject  (비소유 선택 포인터)
```

현재 Mesh 수명은 `AppBase` 수명과 같으므로 `MeshComponent`의 비소유 포인터가 유효하다. 여러 모델과 텍스처를 로딩하는 단계 전까지 ResourceManager와 `shared_ptr` 도입은 보류했다.

GameObject 삭제 기능을 추가할 때는 `m_selectedObject`가 댕글링 포인터가 되지 않도록 선택 해제 또는 안정적인 ID 체계가 필요하다.

## 현재 의존성 경계

```text
Main → AppBase

AppBase → Scene → GameObject → Transform
                           └→ MeshComponent

AppBase → Camera
AppBase → FrameRenderData → Renderer
AppBase → RenderItem       → Renderer → Mesh/D3D11Utils
```

지켜진 핵심 규칙:

- Renderer는 Scene, GameObject, Transform, Camera, ImGui를 모른다.
- Scene/GameObject/Transform은 Renderer와 DX11 렌더링 절차를 모른다.
- AppBase가 상위 조정 계층에서 Scene 데이터를 `RenderItem`으로 변환한다.
- Camera 데이터는 Camera 객체가 아닌 `FrameRenderData`로 Renderer에 전달된다.
- 행렬 전치는 Renderer가 상수 버퍼에 기록하기 직전에 수행한다.

## 현재 프레임 및 렌더링 흐름

1. Win32 메시지를 처리하고 `GameTimer`를 갱신한다.
2. ImGui 새 프레임을 시작하고 `UpdateUI()`에서 편집 값을 Scene/Camera에 반영한다.
3. ImGui가 차지한 폭을 제외해 Scene View의 폭, 높이, Aspect Ratio를 계산한다.
4. Camera Aspect Ratio와 Renderer Viewport를 갱신한다.
5. `AppBase::Render()`가 Camera의 View/Projection으로 `FrameRenderData`를 만든다.
6. `Renderer::BeginFrame()`이 Render Target/Depth를 지우고 View/Projection을 보관한다.
7. AppBase가 Scene의 GameObject를 순회한다.
8. Mesh가 없는 GameObject는 건너뛴다.
9. 각 GameObject의 Mesh 포인터와 World Matrix로 `RenderItem`을 만든다.
10. `Renderer::DrawRenderItem()`이 Model 행렬을 넣고 Constant Buffer를 갱신한다.
11. 해당 Mesh의 Vertex/Index Buffer, Shader, Constant Buffer를 바인딩하고 `DrawIndexed()`를 호출한다.
12. ImGui DrawData를 렌더링한다.
13. `Renderer::EndFrame()`에서 Present한다.

## 현재 상수 버퍼 상태

`BasicVertexConstantData` 하나에 Model/View/Projection이 함께 들어 있다.

- View/Projection 값은 `BeginFrame()`에서 프레임당 한 번 설정한다.
- 실제 GPU Constant Buffer 업로드는 `DrawRenderItem()`에서 오브젝트마다 수행한다.
- 따라서 오브젝트별 Model 데이터와 프레임 공통 Camera 데이터를 논리적으로 전달하는 경계는 나뉘었지만, GPU Constant Buffer 자체는 아직 분리되지 않았다.

향후 Object Constant Buffer와 Camera/Frame Constant Buffer를 서로 다른 슬롯으로 분리하면 프레임 공통 데이터를 오브젝트마다 다시 업로드하지 않아도 되고, 이후 Material/Light Buffer 확장도 명확해진다.

## 현재 확인된 화면 동작

- 왼쪽 Cube와 오른쪽 Triangle이 서로 다른 Mesh로 표시된다.
- Depth Buffer를 사용한다.
- ImGui에서 배경색을 바꿀 수 있다.
- Scene 목록에서 오브젝트를 선택하고 Transform을 수정할 수 있다.
- ImGui에서 Camera Position을 바꿔 시점을 이동할 수 있다.
- 창 크기와 ImGui 패널 폭에 따라 Scene Viewport와 Camera Aspect Ratio가 맞춰진다.

## 최근 검증 결과

2026-08-14 기준 최근 검증:

- `Debug | x64` 빌드 성공
- 경고 0개
- 오류 0개
- 실행 후 3초 동안 프로세스 정상 유지
- 검증 후 작업 트리 clean 확인

새 환경에서는 Git pull 이후 다시 빌드하고, Shader 상대 경로를 위해 실행 작업 디렉터리가 `MyPF/`인지 확인한다.

## 남아 있는 기술 부채와 주의점

- Camera 회전 모델이 아직 없다. 현재 Forward/Up을 직접 설정할 수 있어 잘못된 벡터 조합을 막지 못한다.
- Model/View/Projection이 하나의 GPU Constant Buffer에 들어 있다.
- Vertex는 Position/Color만 가지고 Normal/UV/Tangent가 없다.
- `GeometryGenerator::MakeCube()`가 Normal 배열을 만들지만 현재 Vertex에 저장하지 않는다.
- Material, Texture, Sampler, Light 구조가 없다.
- AppBase가 Scene 생성, Mesh 생성, UI, 제출을 모두 조정하므로 기능 증가에 따라 역할 분리가 필요하다.
- `m_selectedObject`는 비소유 raw pointer이므로 GameObject 삭제 기능 도입 시 수명 처리가 필요하다.
- MeshComponent의 raw pointer는 현재 AppBase 소유 Mesh의 수명이 더 길다는 전제에 의존한다.
- Shader 파일 경로가 상대 경로라 실행 작업 디렉터리에 의존한다.
- 현재 Scene View는 SwapChain Back Buffer의 일부 Viewport이며, 별도의 Texture를 ImGui 창에 표시하는 완전한 에디터 Viewport는 아니다.

## 현재 진행 단계

기존의 “Renderer 내부 단일 Cube 렌더링” 구조를 벗어나 다음 단계까지 완료했다.

- Mesh GPU 리소스 분리
- GameObject의 MeshComponent 도입
- 여러 Mesh/여러 GameObject 렌더링
- RenderItem을 통한 Scene→Renderer 제출 경계
- Camera 분리
- FrameRenderData를 통한 Camera→Renderer 데이터 경계
- ImGui Camera Position 편집

초기 구조 개선 작업 10개 중 Object/Camera GPU Constant Buffer 분리를 제외한 주요 구조와 여러 Mesh 검증이 완료된 상태다.

## 바로 다음 작업

`Camera`의 안전한 Yaw/Pitch 회전을 추가하고 ImGui에서 편집한다.

이 작업이 먼저 필요한 이유:

- 현재는 Camera Position만 조절할 수 있어 씬을 원하는 방향에서 볼 수 없다.
- Forward 벡터를 ImGui에서 직접 수정하면 영벡터, 정규화되지 않은 벡터, Up과 평행한 벡터가 만들어질 수 있다.
- 이후 마우스 시점 회전도 동일한 Yaw/Pitch 로직을 재사용할 수 있다.

권장 설계 방향:

- Camera 내부에 Yaw와 Pitch를 둔다.
- `yaw = 0`, `pitch = 0`일 때 현재 기본 Forward `(0, 0, 1)`과 일치시킨다.
- Pitch는 약 `-89도 ~ +89도` 범위로 제한한다.
- Yaw/Pitch가 바뀔 때 Forward를 계산하고 정규화한다.
- 회전 규칙과 벡터 유효성은 Camera가 책임지고, ImGui는 값만 편집한다.
- 기존 `SetForward()`를 계속 둘 경우 Yaw/Pitch와 Forward가 서로 어긋나지 않도록 하나를 기준 데이터로 정해야 한다.

완료 조건:

- ImGui에서 Yaw/Pitch를 변경하면 Camera 시점이 안정적으로 회전한다.
- 기본 각도에서 기존 화면 방향이 유지된다.
- 위아래 끝까지 조절해도 View Matrix가 깨지지 않는다.
- Renderer와 Scene의 의존성 경계가 변하지 않는다.
- `Debug | x64` 빌드 및 짧은 실행이 성공한다.

이번 단계에서는 하지 않을 것:

- WASD 이동
- 마우스 입력
- Camera를 GameObject 컴포넌트로 변경
- Constant Buffer 분리
- Material/Light 추가

## 이후 작업 순서

1. Camera Yaw/Pitch와 ImGui 회전 편집
2. Camera FOV/Near/Far 편집 및 값 검증
3. Object Constant Buffer와 Camera/Frame Constant Buffer 분리
4. Vertex에 Normal/UV 추가
5. Directional Light 또는 단일 Point Light로 기본 조명 구현
6. Material, Texture, Sampler 도입
7. 모델 로딩 및 필요 시 ResourceManager 도입
8. 에디터 Scene View, 오브젝트 배치/선택 기능 확장
9. WASD/마우스 플레이 카메라와 Editor/Play 상태 전환
10. 전원 스위치 상호작용과 조명 상태 변화
11. 그림자, 젖은 바닥 반사, Bloom, 안개, 색조 보정
12. 사이버펑크 골목 콘텐츠 제작과 1~2분 연출 완성

## 진행률 추정

- 최종 데모 전체 기준: 약 25~30%
- 렌더링/Scene 구조 기반 기준: 약 85%
- 카메라 기반: 위치와 투영까지 완료, 회전과 입력은 미완료
- 고급 렌더링, 콘텐츠, 상호작용: 대부분 미착수

이 수치는 기능 개수보다 최종 데모 제작에 필요한 작업량을 기준으로 한 대략적인 추정이다.
