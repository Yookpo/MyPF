# MyPF 작업 인계

마지막 갱신: 2026-08-11

## 최종 목표

DirectX 11 기반의 1~2분 분량 실시간 사이버펑크 골목 렌더링 데모를 제작한다.

주요 기능:

- 1인칭 카메라
- WASD와 마우스 조작
- 네온 간판과 다수의 점광원
- 젖은 바닥 반사
- 그림자
- Bloom
- 안개
- 색조 보정
- ImGui 디버그 UI
- 전원 스위치 상호작용
- 네온과 조명의 순차 점등

## 작업 방식

작업 전에 저장소 루트의 `AGENTS.md`를 먼저 읽는다.

사용자가 모든 C++와 HLSL 코드를 직접 작성한다. Codex는 다음 기능 하나를 설명하고, 사용자가 작성한 코드를 리뷰하며, 빌드와 실행을 검증한다.

## 저장소 정보

- 원격 저장소: `https://github.com/Yookpo/MyPF.git`
- 브랜치: `main`
- 솔루션: `MyPF/MyPF.sln`
- 빌드 구성: `Debug | x64`
- Shader 경로: `MyPF/Shaders`

## 현재 완료된 기능

- Win32 윈도우와 메시지 루프
- DX11 Device, Context, SwapChain
- Render Target과 Depth Buffer
- Rasterizer State
- Vertex/Index Buffer
- HLSL 런타임 컴파일
- Vertex/Pixel Shader
- Model/View/Projection Constant Buffer
- 3D Cube 렌더링
- GameTimer
- ImGui 연동
- ImGui 배경색 및 Transform 편집
- Transform 클래스
- GameObject 클래스
- Scene 클래스
- Scene에서 여러 GameObject 소유
- Scene의 GameObject 순회
- GameObject의 World Matrix를 Renderer에 전달
- 선택된 GameObject의 Transform을 ImGui에서 수정

## 현재 구조

소유 관계:

AppBase
- Renderer
- Scene
  - GameObject 목록
    - Transform
- 선택된 GameObject를 가리키는 비소유 포인터

의존성:

Main → AppBase  
AppBase → Scene  
Scene → GameObject  
GameObject → Transform  

AppBase → Renderer  
Renderer → D3D11Utils  
Renderer → GeometryGenerator  

Renderer는 Scene, GameObject, Transform을 알지 않는다.

## 현재 렌더링 흐름

1. AppBase가 Renderer의 BeginFrame 호출
2. Scene의 GameObject 목록 순회
3. 각 GameObject의 Transform에서 World Matrix 계산
4. World Matrix를 Renderer::DrawCube에 전달
5. Constant Buffer 갱신
6. Vertex/Index Buffer와 Shader 바인딩
7. DrawIndexed 호출
8. ImGui 렌더링
9. Present

현재 Scene에 여러 GameObject를 추가하면 모두 동일한 Renderer 내부 Cube Mesh로 렌더링된다.

## 현재 Renderer에 남아 있는 책임

- DX11 Device와 Context
- SwapChain
- RTV와 DSV
- Rasterizer State
- Shader와 Input Layout
- Cube Vertex/Index Buffer
- Index Count
- Model/View/Projection Constant Buffer
- 고정 Camera 데이터

Renderer가 아직 Cube Mesh와 Camera를 직접 소유하므로 범용 Renderer는 아니다.

## 최근 검증 결과

- Debug x64 빌드 성공
- 오류 0개
- C4244 경고 3개
- 실행 후 3초 이상 정상 유지
- ImGui로 선택된 Cube의 Transform 편집 가능

## 알려진 문제

- WM_SIZE 이후 Viewport와 Aspect Ratio가 항상 갱신되지 않음
- Renderer가 Cube Mesh를 직접 생성하고 소유함
- DrawCube가 Cube에 종속됨
- Camera가 Renderer 내부에 있음
- Model/View/Projection이 하나의 Constant Buffer에 있음
- GeometryGenerator.h에 Vertex, MeshData, Constant Data가 섞여 있음
- Cube Normal을 생성하지만 Vertex에 저장하지 않음
- Vertex에 UV와 Tangent가 없음
- GameObject 삭제 기능을 만들면 selectedObject 무효화 처리가 필요함

## 현재 진행 단계

Renderer에서 Cube 전용 데이터와 GPU Mesh 리소스를 분리하는 단계다.

## 바로 다음 작업

`MeshData.h`를 생성한다.

이동할 타입:

- Vector3 타입 별칭
- Vertex
- MeshData

`GeometryGenerator.h`는 `MeshData.h`를 포함하도록 변경한다.

이번 작업에서는 다음을 변경하지 않는다.

- Renderer 로직
- Cube 생성 로직
- HLSL
- Constant Buffer
- Vertex 멤버

완료 조건:

- Vertex와 MeshData 정의가 MeshData.h에만 존재
- GeometryGenerator의 Cube 생성 동작 유지
- Debug x64 빌드 성공
- 기존 Cube가 동일하게 표시

## 다음 작업 순서

1. MeshData.h 분리
2. ShaderConstants.h 분리
3. Mesh 클래스 생성
4. Vertex/Index Buffer를 Mesh로 이동
5. DrawCube를 범용 Mesh Draw로 변경
6. MeshRenderer 성격의 컴포넌트 추가
7. RenderItem 추가
8. Camera 분리
9. Object/Camera Constant Buffer 분리
10. 여러 Mesh와 여러 GameObject 렌더링 검증

## 전체 진행률

- 최종 데모 기준: 약 20%
- 렌더링 기반 기준: 약 70%
- 구조 작업 완료 예상 시 최종 데모 기준: 약 25~30%