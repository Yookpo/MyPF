# MyPF Codex 작업 지침

## 프로젝트 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

플레이어가 비 내리는 골목을 1인칭으로 탐색하고 전원 장치를 작동시키면, 꺼져 있던 네온 간판과 조명이 순차적으로 켜지는 짧은 실시간 데모를 목표로 한다.

최종 목표 기능:

- 1인칭 카메라와 WASD/마우스 조작
- 문 또는 전원 스위치 상호작용
- 네온 간판과 다수의 점광원
- 전원 상태에 따른 조명 변화와 순차 점등 연출
- 젖은 바닥의 반사 표현
- 그림자
- Bloom
- 안개 또는 볼륨감 있는 분위기 표현
- 색조 보정
- ImGui 기반 씬 편집 및 디버그 UI

## Codex와 사용자의 역할

- 모든 C++와 HLSL 코드는 사용자가 직접 작성한다.
- Codex는 사용자의 명시적인 요청 없이 프로젝트 소스 코드를 생성하거나 수정하지 않는다.
- 문서, 작업 인계 파일, 빌드 설정처럼 사용자가 직접 수정을 요청한 파일만 Codex가 수정할 수 있다.
- Codex는 다음에 필요한 클래스, 데이터, 메서드 이름, 각 책임, 구현 순서와 완료 조건을 설명한다.
- 한 번에 작은 기능 하나씩 진행하며, 현재 기능을 검증한 뒤 다음 기능으로 넘어간다.
- 사용자가 수정을 완료하면 현재 작업 트리를 다시 읽고 코드 리뷰를 수행한다.
- 오류가 발견돼도 Codex가 임의로 고치지 않고 원인, 영향, 사용자가 수정할 방향을 먼저 설명한다.
- 필요한 경우 `Debug | x64` 빌드와 짧은 실행으로 검증한다.
- 설명은 초보자가 직접 구현할 수 있도록 구체적으로 하되, 완성 코드를 대신 작성하지 않는다.

## 핵심 아키텍처 원칙

- 하위 시스템은 상위 시스템을 알지 않는다.
- `Renderer`는 `Scene`, `GameObject`, `Transform`, ImGui를 포함하거나 참조하지 않는다.
- `Scene`과 `GameObject`는 DirectX 11 렌더링 과정이나 ImGui를 알지 않는다.
- `Transform`은 GPU/HLSL의 행렬 전치 및 상수 버퍼 저장 규칙을 알지 않는다.
- `AppBase`가 현재 상위 조정 계층으로서 Scene 데이터를 렌더링 제출 데이터로 변환한다.
- 오브젝트 단위 경계는 `RenderItem`, 프레임/카메라 단위 경계는 `FrameRenderData`를 사용한다.
- `Renderer`는 `Mesh`, 행렬, 프레임 데이터처럼 렌더링에 필요한 저수준 데이터만 받는다.
- 리소스 소유권과 비소유 참조를 명확히 구분한다.
- 현재 `AppBase`가 `Mesh`를 값으로 소유하고 `MeshComponent`와 `RenderItem`은 `const Mesh*`로 비소유 참조한다.
- 리소스 매니저와 스마트 포인터 기반 공유 소유권은 여러 외부 모델/텍스처를 다루기 시작할 때 도입한다.
- 최종 범용 ECS, 과도한 인터페이스 계층, 사용처가 없는 추상화를 미리 만들지 않는다.

## 현재 구조에서 지켜야 할 경계

```text
AppBase
├─ Scene
│  └─ GameObject[]
│     ├─ Transform
│     └─ MeshComponent --비소유 참조--> Mesh
├─ Camera
├─ Mesh 리소스들
└─ Renderer

AppBase --FrameRenderData/View·Projection--> Renderer::BeginFrame
AppBase --RenderItem/Mesh·World------------> Renderer::DrawRenderItem
```

- `Camera`는 View/Projection 행렬을 만들지만 Renderer를 알지 않는다.
- `Renderer`는 `Camera` 객체를 직접 받지 않고 `FrameRenderData`만 받는다.
- `MeshComponent`는 Mesh를 가리킬 뿐 직접 렌더링하지 않는다.
- `RenderItem`에는 현재 `const Mesh*`와 World 행렬만 둔다. Material, Object ID, Render Layer 등은 실제 기능을 구현할 때 확장한다.
- 행렬 전치는 Renderer가 상수 버퍼에 기록하기 직전에 수행한다.

## 작업 방식

1. 저장소 루트의 이 파일과 `MyPF/docs/CODEX_HANDOFF.md`를 먼저 읽는다.
2. 현재 소스, `git status`, 최근 변경을 확인해 문서보다 코드를 우선한다.
3. 이번 단계가 필요한 이유, 변경 범위, 완료 조건을 먼저 설명한다.
4. 사용자가 직접 C++/HLSL을 구현한다.
5. Codex가 변경 파일과 의존성 경계를 리뷰한다.
6. 가능한 경우 `Debug | x64` 빌드와 짧은 실행으로 회귀 여부를 확인한다.
7. 결과와 남은 문제를 정리한 뒤 다음 작은 기능 하나를 제안한다.
8. 기능 또는 구조가 의미 있게 달라졌다면 `MyPF/docs/CODEX_HANDOFF.md`도 갱신한다.

## 빌드 및 실행 정보

- 저장소 루트: `C:/Users/Diguedman/source/repos/MyPF`
- 솔루션: `MyPF/MyPF.sln`
- 프로젝트 디렉터리: `MyPF/`
- 기본 검증 구성: `Debug | x64`
- HLSL 파일: `MyPF/Shaders/`
- 셰이더는 `D3DCompileFromFile`로 런타임 컴파일한다.
- 실행 시 `Shaders\\simpleVertexShader.hlsl` 같은 상대 경로가 해석되므로 작업 디렉터리를 `MyPF/`로 맞춘다.
- 검증용 임시 빌드 폴더는 저장소 내부의 명확한 경로를 사용하고, 삭제 전 절대 경로가 저장소 내부인지 확인한다.

## 변경 안전성

- 사용자가 작성한 기존 변경사항을 보존한다.
- 요청받지 않은 소스 파일을 수정하거나 삭제하지 않는다.
- `.vs`, `x64`, `CodexBuild*`, 실행 중 변경된 `imgui.ini` 같은 로컬 산출물을 커밋하지 않는다.
- 사용자가 요청하지 않는 한 Git commit, push, pull, branch 변경을 수행하지 않는다.
- 빌드나 실행을 위해 사용자의 기존 산출물 폴더를 지우지 않는다.
- 코드와 문서가 다르면 코드를 기준으로 문서를 고친다.

## 현재 우선순위

현재는 ImGui 기반 씬 편집 환경과 범용 렌더링 구조를 먼저 완성하고, 이후 플레이 모드의 키보드/마우스 입력과 상호작용을 추가한다.

바로 다음 기능은 `Camera`의 안전한 Yaw/Pitch 회전과 ImGui 편집이다. Forward 벡터를 UI에서 직접 임의 수정하지 말고, Pitch 제한과 정규화를 포함하는 회전 규칙을 Camera 내부에 둔다.

그 이후 우선순위:

1. 카메라 회전 및 에디터 카메라 조작 완성
2. 오브젝트/카메라 상수 버퍼 분리
3. Vertex에 Normal/UV 추가 및 기본 조명
4. Material과 Texture
5. 모델 로딩과 리소스 관리
6. 에디터용 Scene View와 오브젝트 배치 기능 확장
7. WASD/마우스 플레이 카메라 및 상호작용
8. 점광원, 그림자, 젖은 바닥, Bloom, 안개, 색조 보정
