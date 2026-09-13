# MyPF 진행 기록 및 작업 인계

마지막 갱신: 2026-09-13

노트북과 데스크톱에서 Git으로 공유하는 MyPF의 실제 구현 상태와 다음 작업을 기록한다.

**이 문서의 역할**: 프로젝트 규칙·아키텍처 원칙·협업 방식·프레임 흐름은 저장소 루트의 `AGENTS.md`가 소유한다. 이 문서는 그와 중복되지 않는 **진행 상태, 완료 기록, 수치 상세, 설계 결정 근거**만 담는다. 자동 로드 대상이 아니며 필요할 때 읽는다.

**시작 절차**

1. 저장소 루트의 `AGENTS.md`를 전부 읽는다.
2. 이 문서에서 필요한 절을 읽는다.
3. 실제 소스, `git status`와 최근 commit을 확인한다.
4. 문서와 코드가 다르면 **실제 코드와 최신 commit을 우선**한다.

---

## 1. 진행률

현재 전체 진행률은 약 **65%**다.

| 영역 | 진행 | 현재 상태 |
|---|---:|---|
| Win32/DX11 기반 | 약 85% | 기본 렌더링, Resize와 제출 경계 완료. 다중 패스는 남음 |
| GPU Resource/Asset 구조 | 약 75% | Buffer/Texture Handle, Asset 캐시와 Model 업로드 완료 |
| Model 파이프라인 | 약 65% | FBX/OBJ/glTF BaseColor 로드 완료. aiNode/PBR은 남음 |
| Scene/ImGui 편집 | 약 65% | Greybox Scene, 선택과 Transform/Material 편집, Point Light 소유 완료 |
| 1인칭 입력/카메라 | 약 90% | Editor/Play, WASD/마우스/ESC/focus와 단발 키 입력 완료 |
| 조명/Material | 약 72% | Directional/Ambient, 최대 8 Point Light, 거리 감쇠, Emissive CPU→GPU→HLSL 경로 완료 |
| 실제 골목/상호작용 | 약 80% | PowerSwitch와 Ray 기반 거리/E 입력, 스위치 피드백, 순차 점등 완료 |
| 고급 렌더링/연출 | 약 15% | Entry 기반 순차 점등 완료. Shadow/Wet/HDR/Bloom/Fog/Rain은 없음 |

---

## 2. Git 체크포인트

- 문서 갱신 기준 HEAD: `1622173` — clangformat 적용 + neonSign팩토리
- 현재 작업 트리: 브랜치 `WORK_CLAUDE`. `NeonSign.h`(인코딩/오타 수정), `Shaders/simpleVertexShader.hlsl`·`simplePixelShader.hlsl`·`Lighting.hlsli`(BOM 제거), `MyPF.vcxproj`/`.filters`가 커밋되지 않은 상태로 남아 있다. commit/push는 사용자가 요청할 때만 한다.
- `MyPF/imgui.ini` 변경은 런타임 UI 배치이므로 기능 commit에서 제외한다.

최근 기능 commit:

```text
5a5f463 소스 인코딩을 UTF-8 BOM으로 통일
c526ebd ImGui 수정
ba24811 Init
001722a Ray 도입
75aa8d9 Editor UI 도입
3d4a428 Runtime Multiplier 도입
4e2610e 포인트라이트를 컴포넌트로 리팩토링
2d26b75 Editor 모드에서 우클릭하면 카메라 이동
f20e429 CODEX_UPDATE
421a58f Entry 기반 점등 소등 구현
```

---

## 3. 현재 완료된 기능

### 플랫폼, Scene과 Asset

- Win32 Window, 메시지 루프, Resize
- DX11 Device/Context/SwapChain, BackBuffer RTV, Depth Buffer, Indexed Drawing
- ImGui 패널을 제외한 Scene Viewport
- `GameTimer`, deltaTime과 FPS 표시
- `Scene`의 `vector<unique_ptr<GameObject>>` 소유
- `GameObject`의 `Transform` / `MeshComponent` / `ModelComponent`
- 여러 GameObject 선택과 독립 Transform 편집
- `FrameRenderData`와 `RenderItem` 제출 경계
- `GraphicsDevice`, `GraphicsResourceManager`, `BufferHandle`, `TextureHandle`
- `AssetManager`의 Mesh/Texture/Material/Model 소유와 이름 캐시
- Assimp `ModelLoader`와 독립 CPU `ModelData`
- Zelda FBX, Pikachu OBJ, Dragonite glTF 다중 Mesh/Material/Texture 렌더링 확인

### Editor / Play와 입력

- Editor/Play 상태 전환, ImGui Play/Stop
- `EditorUI`의 환경·Directional Light·Camera·Scene Hierarchy·Inspector 패널 분리
- Editor 모드는 마우스 우클릭을 누르고 있는 동안만 `FirstPersonCameraController`가 동작(`AppBase::UpdateEditorCamera`), Play 모드는 항상 동작
- 선택 GameObject의 Transform / Point Light / Material 편집 UI
- `InputSystem`의 key down state와 누적 MouseDelta
- `WasKeyPressed`와 `EndFrame`을 통한 키 단발 입력
- deltaTime 기반 WASD 평면 이동과 대각선 정규화
- 마우스 Yaw/Pitch와 Pitch ±89도 제한
- `Camera` Position, +Z Forward, FOV, Aspect
- Play 커서 숨김과 Scene View 중앙 복귀
- ESC / Stop 버튼 / focus 상실 시 Play 종료
- Play 종료 시 Editor Camera Snapshot 복원

### 조명과 Material

- Directional Light의 Direction/Color/Intensity/Ambient Strength CPU→GPU→HLSL→ImGui 연결
- Texture Albedo, Material BaseColor, 기본 Diffuse/Ambient
- `GameObject`가 `PointLightComponent`(Range/Color/Intensity/Enabled, Position은 `Transform`)로 Point Light 데이터를 소유하고, `Scene::GatherPointLights`가 매 프레임 순회해서 모은다
- `FrameRenderData`가 고정 배열과 유효 개수로 최대 8개 Point Light 제출
- CPU `bool`을 GPU `uint32_t`로 변환하고 C++/HLSL 상수 버퍼 레이아웃 정렬
- `Lighting.hlsli`의 범위 기반 거리 감쇠와 Pixel Shader의 다중 Point Light 누적
- 서로 다른 색상의 Point Light 4개가 골목을 비추는 결과 확인
- Material의 Emissive Color / Intensity 추가, 기본 Intensity 0
- 32바이트 Material Constant Buffer의 C++/HLSL 일치와 `Renderer` 전달
- Pixel Shader가 조명 결과와 별도로 Emissive를 최종 색에 더함

### Greybox 골목과 상호작용

- `GeometryGenerator::MakeCube()` 정점 범위를 -0.5 ~ +0.5로 바꿔 기본 크기 1m 통일
- 폭 4m, 높이 4m, 길이 20m Greybox 골목
- 바닥과 좌우/끝 벽이 공용 Cube Mesh와 Greybox Material 공유
- 끝 벽의 전원 스위치 GameObject와 독립 Material
- 상호작용 거리 2.0m 이내의 Ray-BoundingBox 교차 판정(`PowerSwitch::CanInteract`, `DirectX::SimpleMath::Ray`/`BoundingBox::Intersects` 사용)
- `WasKeyPressed('E')`로 단발 상호작용 입력 처리
- `PowerSwitch`가 Scene 소유 GameObject를 비소유 참조하는 컴포지션 구조
- 전원 On/Off 토글과 스위치 Material의 빨강/초록 시각 변화
- `PointLightSequence`가 1.4초 간격으로 등록된 Entry 순서대로 켜고 역순으로 끔
- 점등 중 E키 재입력 시 현재 개수에서 목표 방향 전환
- `SequenceEntry`가 Point Light GameObject와 비소유 Emissive Material 포인터를 묶음
- 독립 Material과 다른 Emissive 색을 가진 네온 세 개(Pink/Cyan/Orange)의 순차 On/Off 실행 확인
- `NeonSignFactory::Create`가 네온 1개(Material + 발광 GameObject + Point Light GameObject + 시퀀스 등록)를 `NeonSignDesc` 하나로 생성하는 절차를 통합해, `InitGreyBoxScene`의 반복 코드를 제거함(Pink/Cyan/Orange 3개 모두 적용 완료, 오브젝트/Material 이름도 `"PinkNeon_Light"`/`"PinkNeon_Glow"`처럼 의미 있는 이름으로 정리됨)
- 네온 전용 민무늬 텍스처(`neonFlat.jpg`, 64x64 흰색)를 추가해 네온 Material의 Albedo를 `wall.jpg`(벽돌 사진 텍스처)에서 분리함 — `surfaceColor = albedo * baseColor` 계산에서 벽돌 무늬가 섞여 Emissive로 포화되지 않는 채널(G/B)에 얼룩으로 비치던 문제를 해결
- `NeonSignFactory`가 `BaseColor = desc.color * 0.12f`로 꺼진 상태의 유리관 색조를 자동 계산, Cyan의 Emissive Color를 `(0.37,0.86,1.0)`→`(0.0,0.86,1.0)`로 조정해 파스텔톤 대신 채도 높은 시안으로 변경, 세 네온의 위치/크기를 좌·우·끝벽 실제 배치 감각에 맞게 재조정(Orange는 끝벽에 평평하게 붙도록 방향 자체를 수정)

### Greybox 배치 수치

| 오브젝트 | Position | Scale |
|---|---|---|
| Floor | (0, -0.1, 10) | (4, 0.2, 20) |
| LeftWall | (-2.1, 2, 10) | (0.2, 4, 20) |
| RightWall | (2.1, 2, 10) | (0.2, 4, 20) |
| EndWall | (0, 2, 20.1) | (4, 4, 0.2) |
| PowerSwitch | (0, 1.2, 19.95) | (0.4, 0.6, 0.1) |
| Camera 시작점 | (0, 1.6, 0) | +Z 방향 |

---

## 4. 완료된 설계 방향

### PowerSwitch 컴포지션

전원 스위치는 개념적으로 GameObject지만 현재 `GameObject`는 상속 기반 다형 타입으로 설계되지 않았다.

- `GameObject`에 virtual destructor나 virtual `Update`/`Interact`가 없다.
- Scene Factory는 `make_unique<GameObject>`만 생성한다.
- 현재 `GameObject`는 Transform/MeshComponent/ModelComponent를 조합하는 방향이다.
- 상속을 도입하면 Scene Factory, 파괴, 타입 접근과 행동 인터페이스까지 함께 바뀐다.

따라서 `PowerSwitch`가 Scene 소유 GameObject를 **비소유로 연결하는 컴포지션**을 사용한다.

`PowerSwitch`의 책임: 연결된 GameObject 비소유 참조, 전원 상태, Ray-BoundingBox 교차 기반 상호작용 거리 판정(`DirectX::SimpleMath::Ray`/`BoundingBox`), 양방향 토글, 전원 상태를 스위치 Material에 반영.

`PowerSwitch`가 몰라야 하는 것: `AppBase`, `InputSystem`, `Camera` 클래스 자체, `Renderer`, ImGui, `Scene`.

`AppBase`에는 E 단발 입력 확인, Camera 위치/전방 전달, 활성화 요청만 남긴다. 범용 ECS나 Component Registry는 아직 만들지 않는다.

### NeonSignFactory

`InitGreyBoxScene`에 네온 1개당 생성 절차(Material 생성 → 발광 GameObject 배치 → Point Light GameObject 생성 → `AddSequenceEntry` 등록)가 Pink/Cyan/Orange 3번 그대로 반복되던 것을 제거하기 위해 도입했다.

- `NeonSignDesc`: 이름, 발광 메쉬 위치/크기, Point Light 위치/범위/세기, 공용 `color`(Emissive Color와 Point Light Color가 항상 같아야 하므로 필드 하나로 공유), Emissive Intensity를 담는 순수 입력 데이터. 런타임 상태 없음.
- `NeonSignFactory::Create`: `Scene`/`AssetManager`/`PointLightSequence`의 공개 API만 알고 `AppBase`/`Renderer`/`InputSystem`/ImGui는 모른다.
- 런타임 상태가 없어 인스턴스를 만드는 클래스 대신 **정적 함수**로 구현했다 — 생성 시점 배선만 책임지는 데 인스턴스는 불필요한 추상화라고 판단.
- `PointLightComponent`에 별도 역할(enum) 태그를 추가하지 않았다 — `PointLightSequence`에 등록됐는지 여부가 이미 "네온 연동 vs 환경 조명" 분류 그 자체이기 때문에, 지금 쓰지 않을 태그를 미리 만들지 않았다.
- Pink/Cyan/Orange 3개 네온 모두 이 팩토리로 교체 완료. `neonMat1`/`neonTestObject0`처럼 남아 있던 테스트용 이름은 `desc.name` 기반 이름(`"PinkNeon_Mat"`, `"PinkNeon_Light"`, `"PinkNeon_Glow"` 등)으로 자동 정리됐다.
- `Create` 내부에서 `BaseColor`를 `desc.color * 0.12f`로 계산해, 꺼진 상태에서도 각 네온 고유의 어두운 색조가 비치게 했다(모두 동일한 회색이던 것을 개선).
- Albedo에 `wall.jpg`(벽돌 텍스처) 대신 전용 `neonFlat.jpg`(흰색 민무늬)를 쓴다 — 네온은 발광이 핵심이라 벽 재질과 텍스처를 공유하면 안 된다는 게 이번에 확인된 설계 원칙이다.

### PointLightSequence와 SequenceEntry

- `AppBase`가 값으로 소유하고 Scene 오브젝트를 비소유 참조한다.
- `PowerSwitch`, `InputSystem`, `Camera`, `Renderer`, ImGui를 직접 알지 않는다.
- `SequenceEntry`는 Point Light를 가진 GameObject 포인터와 비소유 Material 포인터를 묶는다.
- `AddSequenceEntry`는 중복 등록을 검사하고 Point Light와 Material을 Off 상태로 맞춘 뒤 등록한다.
- `Start(bool)`은 실제 대상을 즉시 바꾸지 않고 **등록된 Entry 개수를 기준으로 목표만** 설정한다.
- `Update(deltaTime)`은 등록 순서로 켜고, `m_enabledLightCount - 1` 위치를 사용해 역순으로 끈다.
- 진행 도중 목표가 바뀌면 현재 켜진 개수에서 방향을 전환한다.
- Material의 `m_emissiveIntensity`(에디터 값)와 `m_runtimeEmissiveMultiplier`(시퀀스 값)를 분리해, 시퀀스가 꺼도 에디터 설정값이 파괴되지 않는다.

---

## 5. 현재 알려진 문제와 보류 항목

### 구조

- `AppBase`가 초기화, Greybox 구성, Play UI, Play 입력, RenderItem 조립, 기능 객체 조율까지 담당해 여전히 크다. Editor UI는 `EditorUI`로 분리했으며, 실제 변경 압력이 확인되는 책임부터 추가 분리한다.
- `AppBase`의 멤버가 전부 `public`이고 전역 `g_appBase`로 접근 가능하다.
- 씬 정의가 `InitGreyBoxScene()`에 하드코딩돼 있어 변경 시 재컴파일이 필요하다.
- `GameObject`의 컴포넌트가 선택적이지 않다. `PointLightComponent`만 `Has` 플래그를 쓰고 나머지는 포인터 null 검사로 판단해 규칙이 일관되지 않다.

### 렌더링

- `Renderer` Material 경로는 유효한 Albedo Texture를 전제로 하며 기본 Material/Texture 정책이 없다. 없으면 앱이 종료된다.
- 드로우콜마다 InputLayout, Shader, Sampler를 다시 바인딩한다.
- 컬링이 없다. `CullMode`가 `D3D11_CULL_NONE`이고 프러스텀 컬링도 없다.
- Pixel Shader 최종 `saturate` 때문에 1을 넘는 Emissive가 LDR에서 잘린다. 현재 Intensity 3과 8은 구분되지 않는다.
- Texture가 `R8G8B8A8_UNORM`(sRGB 아님)이고 MipMap과 UV Tiling 정책이 없다.
- Material에 Normal / Metallic / Roughness가 없다.
- 스페큘러 항이 없다. 젖은 바닥 반사의 전제가 빠져 있다.
- Vertex Color가 최종 Pixel Color에 사용되지 않는다.
- 셰이더가 런타임 컴파일인데 핫 리로드가 없다.
- `D3D11Utils::CreateDepthBuffer`는 아무도 호출하지 않는 죽은 코드다.

### 리소스와 데이터

- `GraphicsResourceManager`는 append-only이며 개별 삭제, 슬롯 재사용, generation이 없다.
- `AssetManager`의 key/path 정규화가 없다.
- `ModelLoader`는 aiNode Transform/Instance를 반영하지 않는다.
- `GeometryGenerator`의 평행 배열은 데이터 불일치 위험이 있다.
- `MakeCube`의 지역 `scale = 1.0f`는 현재 중복 표현이다.

### 씬과 플레이

- Camera 충돌이 없어 벽과 스위치를 통과할 수 있다. `PlayerCollision` 설계 완료, 구현 착수 전(§6 참고).
- 플레이어 높이와 월드 단위 정책이 명문화돼 있지 않다.
- 골목 이동 경로가 약 20m로 최종 20~40초 탐색 동선보다 짧다.
- 실제 골목 에셋, Scene 저장/Prefab이 없다. 네온 배치·색상은 1차 정리를 마쳤다(2026-09-13, §8 참고).
- ImGui Transform의 Position `-50~50`, Scale `0.01~50` 범위는 조정 여지가 있다.
- 활성 Scene은 Greybox 골목이며 외부 모델 파이프라인은 유지하되 현재 장면에 배치하지 않는다.

### 기타

- ImGui 부분 초기화 실패와 Shutdown 상태 추적이 없다.
- `Main.cpp`가 `WinMain`인데 `std::cerr`로 출력해 아무 데도 보이지 않는다.
- `MsgProc`에서 `InputSystem::ProcessMessage`가 ImGui 핸들러보다 먼저 호출된다.
- 전면 Epic 명명 마이그레이션은 기능 우선 결정으로 보류했다.
- 외부 캐릭터 에셋은 테스트용이며 공개 전 라이선스를 확인한다.

---

## 6. 바로 다음 작업

**`PlayerCollision`(가칭) 설계 합의 완료 — 구현 전. 그다음은 Shadow Mapping.**

로드맵 8번(에셋 배치) vs 9번(Shadow Mapping) 중 어디로 갈지 논의한 결과, **Shadow Mapping을 다음 렌더링 작업으로 확정**했다(이유는 §8 참고). 다만 Shadow Mapping을 시작하기 직전, "Camera 충돌이 없어 벽을 통과한다"는 §5의 알려진 문제를 먼저 잡기로 했다 — 원래 로드맵 13번 항목의 일부(플레이어 충돌)를 앞당기는 것이다.

**합의된 설계(초안, 아직 코드로 옮기지 않음):**

- 새 클래스 `PlayerCollision`: `GameObject`(Transform)만 알고 `Camera`/`InputSystem`/`FirstPersonCameraController`/`Renderer`/ImGui는 모른다. `Vector3 Resolve(const Vector3& desiredPosition) const` — 입력도 출력도 Vector3뿐인 순수 함수형 API.
- `FirstPersonCameraController`는 손대지 않는다 — 벽의 존재를 몰라야 한다는 §3 원칙 유지. 대신 `AppBase::Update`가 `m_firstPersonCameraController.Update(dt)` 직후 `PlayerCollision::Resolve`를 호출해 `Camera` 위치를 보정한다(`PowerSwitch`/`PointLightSequence`와 같은 조율 위치).
- 이 씬은 좌/우/끝벽이 정확히 `Floor`의 가장자리와 겹치므로, **"플레이어 위치를 Floor 범위 안으로 clamp"** 하나로 "벽 통과 방지"와 "골목 이탈 방지"를 동시에 해결한다. 별도 처리가 필요한 건 통로 중간의 PowerSwitch 박스(원-사각형 밀어내기)뿐이다.
- Y축 충돌은 다루지 않는다 — 점프/중력이 없어 카메라 Y가 항상 1.6으로 고정이므로 XZ 평면 충돌로 충분하다고 판단했다.

**진행 순서 (Step 1도 아직 시작 전):**

1. `PlayerCollision.h/.cpp` 골격 + `Floor` 범위로 clamp하는 `Resolve` 구현 (아직 `AppBase`에서 호출 안 함) ← 다음에 여기부터 이어간다.
2. `AppBase::Update`의 Play 분기에 연결. 완료 조건: 좌우 벽·끝벽·입구 밖으로 못 나감.
3. PowerSwitch 박스 원-사각형 밀어내기 추가. 완료 조건: PowerSwitch를 통과 못 함.
4. 이후 Shadow Mapping 착수(별도 설계 가이드 이미 대화로 제공됨 — Depth-only 패스, Shadow Map 텍스처(`GraphicsResourceManager`에 GPU 전용 Depth+SRV 텍스처 생성 기능 추가 필요), 광원 View/Projection, PCF 순).

**참고**: `simplePixelShader.hlsl`의 LDR `saturate` 클리핑은 여전히 미해결이다 — Emissive Intensity 3/5/8의 밝기 차이가 화면에서 구분되지 않는 문제는 HDR Scene Target 단계(로드맵 11번)에서 해결 예정.

---

## 7. 로드맵

1. ✅ 골목 Greybox와 월드 스케일 확정
2. ✅ 최소 상호작용 기반과 PowerSwitch 책임 분리
3. ✅ 전원 상태에 따른 스위치 시각 피드백
4. ✅ 다수 Point Light와 Renderer 제출
5. ✅ Emissive Material과 네온 표면 표현
6. ✅ 전원 상태와 Point Light·Emissive Material 순차 점등 연동
7. ✅ 조명 역할 분류(`NeonSignFactory`), 네온 배치·색상 정리
8. 실제 골목 에셋 배치와 Scene 편집 보강 (보류 — 9번 이후 재판단)
9. Shadow Mapping ← 확정, 착수 직전
10. Normal/Roughness Material과 젖은 바닥 반사
11. HDR Scene Target, Bloom과 Tone Mapping
12. 안개, 비와 색조 보정
13. 충돌/이동 제한, 디버그 UI와 최적화 (기본 플레이어-벽 충돌은 9번보다 먼저 앞당겨 처리 ← 현재, `PlayerCollision` 설계 완료·구현 전)
14. 라이선스 정리와 1~2분 최종 연출

---

## 8. 작업 완료 기록

### 2026-09-03 — PowerSwitch 컴포지션과 시각 피드백 ✅

- Scene이 소유한 스위치 GameObject를 `PowerSwitch`가 비소유로 참조하도록 분리했다.
- 거리·시선 판정, 전원 상태와 On/Off 토글을 `PowerSwitch` 책임으로 옮겼다.
- 전원 Off는 빨강, On은 초록 BaseColor로 즉시 반영되도록 했다.
- 상호작용 범위에서 E키를 반복 입력해 상태와 색상이 양방향 전환되는 것을 확인했다.

### 2026-09-03 — 다중 Point Light와 GPU 제출 ✅

- Scene에 Point Light 목록과 생성·조회·활성 상태 변경 경로를 추가했다.
- 최대 8개의 Point Light를 `FrameRenderData`와 Light Constant Buffer로 전달한다.
- CPU 의미 데이터와 GPU 정렬 데이터를 분리하고 `bool`을 `uint32_t`로 변환한다.
- `Lighting.hlsli`에서 범위 기반 감쇠와 Diffuse를 계산하고 Pixel Shader에서 모든 유효 조명을 누적한다.
- 서로 다른 위치와 색상의 Point Light 4개가 골목을 비추는 결과를 확인했다.

### 2026-09-03 — 전원 기반 순차 점등 ✅

- `PointLightSequence`가 현재 개수, 목표 개수, 누적 시간과 간격을 관리한다.
- 전원 On이면 등록된 Entry 순서대로 켜고 Off이면 역순으로 끈다.
- 진행 중 방향 전환, 목표 도달 정지와 스위치 On/Off 연동을 실행 확인했다.
- 현재 간격은 1.4초다.

### 2026-09-03 — Emissive Material과 Entry 기반 네온 점등 ✅

- Material에 Emissive Color와 Intensity를 추가하고 기본 밝기를 0으로 설정했다.
- Material Constant Buffer의 C++/HLSL 구조를 32바이트로 맞추고 `Renderer`가 값을 전달한다.
- Pixel Shader에서 조명과 별도로 Emissive를 더해 표면 자체 발광을 확인했다.
- `SequenceEntry`를 도입해 Point Light와 네온 Material을 하나의 연출 단위로 등록한다.
- Scene 전체 조명이 아니라 등록된 Entry 목록을 기준으로 목표와 순서를 계산한다.
- 독립 Material을 쓰는 네온 두 개를 Point Light와 연결해 순서 점등과 역순 소등을 확인했다.
- 현재 Emissive Intensity 3과 8은 LDR `saturate`에서 잘리므로 실제 밝기 차이와 번짐은 HDR/Bloom에서 확인한다.

### 2026-09-10 — Editor UI 분리 ✅

- Editor ImGui 패널을 `AppBase`에서 `EditorUI.h/.cpp`로 분리했다.
- 환경, Directional Light, Camera, Scene Hierarchy, Inspector 패널을 각각의 함수로 나눴다.
- 커밋 완료(`75aa8d9 Editor UI 도입`), 이후 커밋들이 이 구조 위에서 계속 진행된다.

### 2026-09-07 ~ 2026-09-12 — Point Light 컴포넌트화, Editor 카메라, Ray 판정, 인코딩 통일 ✅

- (`2026-09-07`) Point Light를 `Scene`이 배열로 직접 소유하던 구조에서 `GameObject` + `PointLightComponent` 조합으로 리팩토링했다(`4e2610e`). `Scene::GatherPointLights`가 매 프레임 순회해서 모으는 방식은 유지된다.
- (`2026-09-07`) Editor 모드에서 마우스 우클릭을 누르고 있는 동안에만 `FirstPersonCameraController`가 동작하도록 `AppBase::UpdateEditorCamera`를 추가했다(`2d26b75`).
- (`2026-09-10`) Material에 `m_runtimeEmissiveMultiplier`를 도입해 에디터 설정값(`m_emissiveIntensity`)과 `PointLightSequence`가 조절하는 런타임 배율을 분리했다(`3d4a428`).
- (`2026-09-11`) `PowerSwitch::CanInteract`의 판정 방식을 거리+시선 내적에서 `DirectX::SimpleMath::Ray`와 `BoundingBox::Intersects` 기반 Ray-Box 교차로 변경했다(`001722a`).
- (`2026-09-12`) 전체 소스 파일의 한글 주석 인코딩을 UTF-8 BOM으로 통일했다(`5a5f463`).

### 2026-09-12 — 문서 구조 정리 ✅

- `AGENTS.md`를 도구 중립적으로 바꾸고 규칙·아키텍처·프레임 흐름만 남겨 압축했다.
- 완료 기능 목록, 진행률, 로드맵, 작업 기록을 이 문서로 이동해 두 문서의 중복을 제거했다.
- `CLAUDE.md`가 `@AGENTS.md`를 import하고 MyPF 우선 규칙을 선언하도록 구성했다.

### 2026-09-13 — NeonSignFactory 구현 완료 ✅

- 완료한 작업: `NeonSign.h`/`.cpp`에 `NeonSignDesc`(입력 데이터)와 `NeonSignFactory::Create`(정적 팩토리 함수)를 구현했다. Step 1(뼈대) → Step 2(Pink 교체) → Step 3(Cyan/Orange 교체) 순서로 진행했고, 매 단계 코드 리뷰로 확인했다.
- 확인한 결과: `InitGreyBoxScene`의 네온 1개당 반복 생성 코드(Material/Point Light GameObject/발광 GameObject/시퀀스 등록, 총 30줄 안팎 x 3)가 `NeonSignDesc` 3개 + `NeonSignFactory::Create` 호출 3번으로 정리됐다. 각 desc 값을 원본 리터럴과 대조해 위치·색·범위·세기가 전부 일치함을 확인했고, 옛 이름(`neonMat1`, `pinkNeonLight`, `neonTestObject0` 등) 참조가 코드에 전혀 남아있지 않음을 grep으로 확인했다. 빌드·실행해서 3개 네온이 이전과 동일한 위치·색으로 보이고 Pink→Cyan→Orange 점등 순서가 유지됨을 사용자가 직접 확인했다.
- 리뷰 중 두 차례 발견·수정된 문제: ① `Scene::CreateGameObject`가 반환하는 `GameObject&`를 `auto`(참조 아님)로 받아 복사본을 수정하던 버그 — 컴파일은 되지만 화면에 아무것도 안 보이는 조용한 실패였다. ② `desc` 필드 중 일부(발광 오브젝트 위치/크기)가 하드코딩으로 남아있던 것. ③ 새 파일 저장 시 인코딩이 CP949로 깨지는 문제가 반복 발생 — 매번 UTF-8 BOM으로 재저장해 해결.
- 설계 규칙: `NeonSignFactory`가 왜 정적 함수인지, `PointLightComponent`에 역할 enum을 안 만든 이유는 §4 "NeonSignFactory" 참고.
- 다음에 이어서 할 작업: 실제 골목 디자인 관점에서 네온 위치/크기/방향을 재구상한다(대화로 방향 논의 후 `NeonSignDesc` 숫자만 조정). 그다음은 로드맵 8번(실제 골목 에셋 배치)이나 9번(Shadow Mapping).

### 2026-09-13 — 다음 로드맵 방향 결정 + PlayerCollision 설계 (구현 전)

- 완료한 작업: 코드 변경은 없고, 다음 작업 방향을 정하는 논의를 진행했다. (1) 로드맵 8번(에셋 배치) vs 9번(Shadow Mapping) 중 무엇을 먼저 할지, (2) Unreal식 W/E/R 트랜스폼 기즈모 도입 여부, (3) `AppBase` 비대화 문제를 지금 리팩토링할지, (4) 플레이어-벽 충돌을 어떻게 넣을지 — 이 네 가지를 논의하고 설계까지 마쳤다.
- 확인한 결과: `git log`/`git status` 기준 HEAD는 `82aae2a`(네온 텍스처·위치 수정)이고 소스 변경 없음. `PlayerCollision` 관련 파일은 아직 생성되지 않았다(`find`로 확인) — 설계 논의만 하고 구현은 시작 전이다.
- 남아 있는 문제: §5의 "Camera 충돌 없음" 문제가 이번 논의의 핵심 대상이며 아직 미해결. LDR `saturate` 클리핑도 여전히 미해결(HDR 단계 대기).
- 다음에 이어서 할 작업: `PlayerCollision` Step 1(골격 + Floor 범위 clamp)부터 시작. 완료되면 Step 2(AppBase 연결) → Step 3(PowerSwitch 밀어내기) → Shadow Mapping 순.
- 중요한 설계 결정과 이유:
  - **Shadow Mapping을 8번(에셋 배치)보다 먼저 하기로 함**: (a) 취업 목표 포트폴리오 관점에서 렌더링 기술(그림자·HDR 등)이 면접에서 직접 검증 가능한 핵심 스킬이고, 에셋 배치는 "콘텐츠 소싱" 성격이 강해 프로그래밍 역량을 덜 보여준다. (b) 렌더링 코어 기능을 Greybox로 먼저 검증해야, 나중에 실제 에셋을 넣었을 때 문제가 생겨도 "셰이더 버그"와 "에셋 임포트 문제"를 구분해서 디버깅할 수 있다.
  - **W/E/R 트랜스폼 기즈모 도입을 보류함**: 최종 Play 데모 영상에는 전혀 노출되지 않는 에디터 툴링이라 포트폴리오 임팩트가 낮고, 요구되는 작업량(마우스 피킹 + 오버레이 렌더링 + 축별 드래그 수학)이 이 프로젝트에서 지금까지 한 것 중 가장 크다. 신입 그래픽스/엔진 포지션보다 툴 프로그래머 포지션에 더 맞는 스킬셋이라는 점도 고려했다. 데모 완성 후 시간이 남거나 다음 포폴에서 고려하기로 했다.
  - **`AppBase` 리팩토링은 지금 하지 않기로 함**: §3 원칙(실제 압력이 확인될 때 구조 확장)대로, Shadow Mapping도 에셋 배치도 `AppBase` 크기 자체가 작업을 막지 않는다고 판단했다. 다만 8번(에셋 배치)을 진행하면 `InitGreyBoxScene`이 더 커져서 `GreyboxSceneBuilder` 같은 분리가 필요해질 가능성이 높다는 점은 기록해둔다.
  - **`PlayerCollision`을 별도 클래스로 분리하고 `FirstPersonCameraController`에 충돌 지식을 넣지 않기로 함**: §3 계층 원칙(하위 시스템이 Scene을 모름) 유지를 위해 `PowerSwitch`/`PointLightSequence`와 같은 위치(`AppBase`가 조율)에 두기로 했다. Floor와 좌/우/끝벽의 경계가 겹친다는 이 씬의 기하학적 특성을 이용해 "Floor 범위 clamp" 하나로 벽 충돌과 골목 이탈 방지를 동시에 해결하도록 범위를 단순화했다.

### 2026-09-13 — 네온 배치·색상·텍스처 정리 ✅

- 완료한 작업: 대화로 골목 평면 배치를 구상해(좌→우→끝벽 리듬, 점등이 입구 쪽에서 스위치 쪽으로 다가오는 연출 유지) `NeonSignDesc` 3개의 위치/크기를 조정했다. Orange는 끝벽에 평평하게 붙도록 `glowScale`을 X축이 얇은 형태(옆벽 방식)에서 Z축이 얇은 형태(끝벽 방식)로 바꿨다.
- 확인한 결과: 점등 후 네온 색이 이상해 보인 진짜 원인은 배치가 아니라 **모든 네온 Material이 `wall.jpg`(벽돌 사진 텍스처)를 Albedo로 공유하고 있었던 것**이었다. `surfaceColor = albedo * baseColor`가 `ambient`/`diffuse`/`pointLight` 항에 쓰이는데, Emissive가 채널별로 포화 정도가 달라(예: Pink는 R·B만 포화, G는 안 됨) 포화 안 되는 채널에 벽돌 무늬가 얼룩으로 비쳤다.
- 해결: 네온 전용 흰색 민무늬 텍스처 `neonFlat.jpg`(64x64)를 새로 만들어 세 네온 Material의 Albedo를 이걸로 교체했다(`wall.jpg`는 floor/wall/switch에서만 계속 사용). `NeonSignFactory::Create`에서 `BaseColor = desc.color * 0.12f`로 계산해 꺼진 상태 색조도 네온마다 다르게 나오게 했고, Cyan의 Emissive Color를 `(0.37,0.86,1.0)`→`(0.0,0.86,1.0)`로 바꿔 채도를 높였다.
- 설계 규칙: 네온처럼 발광이 핵심인 Material은 벽 재질과 Albedo 텍스처를 공유하면 안 된다는 원칙을 확인했다 — 이후 새 발광 오브젝트를 추가할 때도 `neonFlat.jpg`(또는 같은 성격의 민무늬 텍스처)를 쓴다.
- 다음에 이어서 할 작업: 로드맵 8번(실제 골목 에셋 배치)과 9번(Shadow Mapping) 중 무엇을 다음으로 할지 결정한다.

### 2026-09-13 — 셰이더 컴파일 실패 원인 진단: HLSL + UTF-8 BOM ✅

- 완료한 작업: `Renderer::Initialize`가 항상 실패해 앱이 초기화 직후 종료되는 문제를 디버깅했다. `Renderer::Initialize` 안에서도 구체적으로 `D3D11Utils::CreateVertexShaderAndInputLayout`(Vertex Shader 컴파일 단계)에서 `hr = E_FAIL`, `errorBlob` 유효로 확인했고, Windows SDK 번들 `fxc.exe`로 `Shaders/simpleVertexShader.hlsl`을 직접 컴파일해 `error X3000: Illegal character in shader file`을 재현했다. 파일 앞 3바이트(UTF-8 BOM `EF BB BF`)를 제거한 사본으로 같은 파일을 다시 컴파일해 정상 컴파일됨을 확인해 원인을 확정했다.
- 확인한 결과: 원인은 `5a5f463 소스 인코딩을 UTF-8 BOM으로 통일` 커밋이 `.cpp`/`.h`뿐 아니라 `.hlsl`/`.hlsli`까지 UTF-8 BOM으로 바꿔버린 것. MSVC는 UTF-8 BOM을 허용하지만 HLSL 컴파일러(`fxc`/`D3DCompiler`)는 허용하지 않아, 그 커밋 이후 셰이더 런타임 컴파일이 계속 실패하고 있었다(그동안 실행을 안 해봐서 발견되지 않았던 것으로 보임).
- 남아 있던 문제와 해결: `Shaders/simpleVertexShader.hlsl`, `simplePixelShader.hlsl`, `Lighting.hlsli` 세 파일을 BOM 없는 UTF-8로 다시 저장해 해결했다(사용자가 직접 수정, 아직 커밋 전).
- 설계 규칙 갱신: `.hlsl`/`.hlsli`는 UTF-8 BOM 통일 규칙의 예외(BOM 없는 UTF-8)로 `AGENTS.md`/`CLAUDE.md`에 반영했다.
- 별도로 진행된 작업: 사용자가 `.clang-format`의 `ColumnLimit`을 0→120으로 바꾼 뒤 ImGui를 제외한 사용자 작성 `.h`/`.cpp` 전체에 재포맷을 적용했고, `NeonSignFactory` Step 1 뼈대(`NeonSign.h`/`.cpp`)를 작성해 `MyPF.vcxproj`에 등록했다(`1622173 clangformat 적용 + neonSign팩토리` 커밋). 리뷰에서 발견된 인코딩 문제(CP949)와 `glowSCale` 오타는 이후 수정됐다(아직 커밋 전).
- 다음에 이어서 할 작업: 셰이더가 이제 정상 컴파일되는지 실제로 빌드·실행해 화면으로 확인한다. 확인되면 §6의 NeonSignFactory Step 2(Pink 네온 교체)로 이어간다.

### 2026-09-13 — 솔루션 전체 복습, 문서 동기화, NeonSign 설계 합의 ✅ (구현 전)

- 완료한 작업: `Application/Scene/Camera/Asset/GPU Resource/Renderer/Editor/Gameplay` 전 시스템, 한 프레임 흐름, 클래스별 책임, 데이터 이동 경로(A~E), DX11 개념 매핑을 실제 소스 기준으로 복습하고 Notion 페이지에 정리했다(코드 수정 없음). 복습 중 발견한 문서-코드 불일치 4건(PowerSwitch Ray 판정, Point Light 컴포넌트화, EditorUI 분리 상태, Editor 우클릭 카메라)과 해결돼 있던 인코딩 이슈를 `AGENTS.md`/`CLAUDE.md`/`CODEX_HANDOFF.md`에 실제 코드 기준으로 반영했다.
- 확인한 결과: `git log` 기준 HEAD는 `5a5f463`로 문서 체크포인트보다 실제로 앞서 있었음을 확인했고(현재는 동기화 완료), §6 "Point Light 역할 분류"는 재점검 결과 환경 조명/네온 연동 조명 분류 로직 자체가 이미 구현돼 있음을 확인했다(`EnvFillLight` 상시 On·미등록, Pink/Cyan/Orange는 `PointLightSequence`에 등록).
- 다음 작업으로 `NeonSignFactory`(`NeonSignDesc` + 정적 `Create`) 도입을 설계했다 — `InitGreyBoxScene`에 반복되는 네온 1개당 생성 절차를 팩토리 호출 한 줄로 줄이기 위함이며, 4단계(스켈레톤 → Pink 교체 → Cyan/Orange 교체 → 이름·배치 정리)로 쪼갰다. 상세는 §6 참고.
- 남아 있는 문제: `NeonSign.h/.cpp`는 아직 생성되지 않았고 `InitGreyBoxScene`도 그대로다(소스 변경 없음). `simplePixelShader.hlsl`의 LDR `saturate` 클리핑이 미해결이라 네온 정리를 마쳐도 Emissive 밝기 차이는 화면에서 안 보인다(HDR 단계 필요). 이번 세션에서 수정한 `AGENTS.md`/`CLAUDE.md`/`CODEX_HANDOFF.md`는 아직 커밋되지 않았다.
- 다음에 이어서 할 작업: §6의 Step 1(`NeonSign.h`/`NeonSign.cpp` 뼈대 작성)부터 이어간다.

---

## 9. 다른 PC에서 확인할 체크리스트

- `git pull` 후 HEAD가 최소 `5a5f463`인지 확인한다.
- `AGENTS.md`와 이 문서의 마지막 갱신일이 같은지 확인한다.
- `git status`에서 사용자 변경과 로컬 `MyPF/imgui.ini` 변경을 구분한다.
- `GeometryGenerator::MakeCube`가 1m 단위인지 확인한다.
- Greybox 바닥/벽과 PowerSwitch Transform이 §3의 수치와 같은지 확인한다.
- `InputSystem::WasKeyPressed`와 `EndFrame`이 연결됐는지 확인한다.
- Play에서 스위치를 바라보며 상호작용 거리(Ray-BoundingBox 교차) 안에 있을 때 E로 전원과 스위치 색상이 On/Off 전환되는지 확인한다.
- 등록된 네온 두 개와 연결 Point Light가 1.4초 간격으로 함께 켜지고 역순으로 꺼지는지 확인한다.
- 미등록 Point Light는 현재 시퀀스에서 계속 꺼져 있는 것이 정상이다.
- 대규모 Unreal식 명명 마이그레이션이나 범용 ECS를 시작하지 않는다.
- 사용자가 요청하지 않으면 빌드, 실행, commit, push하지 않는다.
