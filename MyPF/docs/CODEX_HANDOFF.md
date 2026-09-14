# MyPF 진행 기록 및 작업 인계

마지막 갱신: 2026-09-14

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

- 문서 갱신 기준 HEAD: `2835951` — RimLight 기능 추가
- 현재 작업 트리: 브랜치 `WORK_CLAUDE`, `git status` 기준 clean. commit/push는 사용자가 요청할 때만 한다.
- `MyPF/imgui.ini`와 `MyPF/ImGui/imgui.ini`는 `.gitignore`에 등록하고 `git rm --cached`로 인덱스에서 제거했다(로컬 파일은 유지) — 이제부터는 변경돼도 `git status`에 아예 안 잡힌다(2026-09-14).

최근 기능 commit:

```text
2835951 RimLight 기능 추가
f329bc1 Update CODEX_HANDOFF.md
9eb727d imgui.ini 추적안하기
74e0c46 BoxCollisionComponent + PlayerCollision 충돌 시스템 구현
be2cdf5 Update CODEX_HANDOFF.md
82aae2a 네온 텍스처 및 위치 수정
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
- Rim Lighting: `Material`에 `rimColor`/`rimIntensity`(기본 0)/`rimPower` 추가, Pixel Shader가 처음으로 카메라 월드 위치를 받아(`CameraConstantData`→`register(b2)`) `pow(1-saturate(dot(normal,viewDir)), rimPower)` 기반 가장자리 발광을 `finalColor`에 가산. ImGui Material Inspector에 Rim 슬라이더 3종 추가. 설계 배경은 §4 "Rim Lighting과 카메라 위치 전달" 참고

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

### BoxCollisionComponent와 PlayerCollision

플레이어(카메라)가 벽·스위치를 통과하는 문제를 해결하기 위해 도입했다. 처음에는 `PlayerCollision`이 `Floor`/`PowerSwitch` GameObject를 직접 참조하는 방식으로 설계했으나, "Unreal처럼 컴포넌트로 만드는 게 낫지 않나"는 논의 끝에 이 프로젝트에 이미 있는 `PointLightComponent` 패턴(Has 플래그 + `Scene::Gather*`)을 재사용하는 방향으로 바꿨다.

- `GameObject`가 `PointLightComponent`와 완전히 같은 패턴으로 `BoxCollisionComponent`를 선택적으로 가진다(`AddBoxCollisionComponent`/`GetBoxCollisionComponent`/`HasBoxCollisionComponent`). `BoxCollisionComponent` 자체는 필드 없는 마커 클래스다 — 충돌 모양은 항상 `Transform`(Cube 기준)에서 유도되므로 별도 데이터가 필요 없다.
- `Scene::GatherBoxColliders()`가 `GatherPointLights`와 같은 위치의 책임으로, 플래그가 켜진 오브젝트만 `PowerSwitch::CanInteract`와 동일한 방식(`BoundingBox(Vector3(0),Vector3(0.5))`를 `WorldMatrix`로 변환)으로 모은다. `GatherPointLights`와 달리 GPU Constant Buffer 제약이 없어서 고정 배열이 아니라 `std::vector<DirectX::BoundingBox>`를 그냥 반환한다.
- **이름을 `GatherColliders`가 아니라 `GatherBoxColliders`로, `CollisionComponent`가 아니라 `BoxCollisionComponent`로 정직하게 지었다** — 지금은 씬에 Cube 모양(`GeometryGenerator::MakeCube`)뿐이라 Box만 지원하지만, 나중에 실제 에셋(로드맵 8번)이 들어와 다른 모양이 필요해지면 `SphereCollisionComponent`/`GatherSphereColliders`를 **나란히 추가**하면 된다 — 기존 이름을 바꾸거나 호출부를 고칠 필요가 없다. Unreal도 `UBoxComponent`/`USphereComponent`를 하나의 다형 클래스가 아니라 형제 클래스로 분리해 놓은 것과 같은 방향이다.
- **`Floor`는 충돌체에서 제외했다** — 이 시스템은 XZ 평면만 다루는데(Y는 항상 1.6 고정, 점프/중력 없음), `Floor`의 XZ 풋프린트가 걸을 수 있는 영역 전체와 정확히 일치해서 "장애물"로 취급하면 플레이어가 항상 그 박스 안에 있는 깨진 경우(거리 0, 방향 미정의)가 된다. `Floor`는 Y축(수직 지지) 개념이라 지금 만든 XZ 전용 벽 충돌과는 다른 문제 — 나중에 중력을 넣을 때 별도 로직(예: 바닥 높이 비교)으로 처리할 대상이다.
- `PlayerCollision::Resolve`/`PushOutOfBox`는 상태가 없어서(더 이상 `Floor`/`PowerSwitch`를 직접 참조하지 않음) `NeonSignFactory`와 같은 이유로 정적 함수로 구현했다. 원-박스 최근접점(clamp) → 거리 계산 → `radius`보다 가까우면 정규화한 방향으로 밀어내는 3단계로, 콜라이더 목록을 순서대로 누적 적용한다.
- 좌/우/끝벽 + 입구를 막는 새 `startWall`(Mesh/Material 없이 Transform+`BoxCollisionComponent`만 있는 보이지 않는 벽) + `PowerSwitch`, 총 5개 오브젝트가 충돌체로 등록돼 있다.

### PointLightSequence와 SequenceEntry

- `AppBase`가 값으로 소유하고 Scene 오브젝트를 비소유 참조한다.
- `PowerSwitch`, `InputSystem`, `Camera`, `Renderer`, ImGui를 직접 알지 않는다.
- `SequenceEntry`는 Point Light를 가진 GameObject 포인터와 비소유 Material 포인터를 묶는다.
- `AddSequenceEntry`는 중복 등록을 검사하고 Point Light와 Material을 Off 상태로 맞춘 뒤 등록한다.
- `Start(bool)`은 실제 대상을 즉시 바꾸지 않고 **등록된 Entry 개수를 기준으로 목표만** 설정한다.
- `Update(deltaTime)`은 등록 순서로 켜고, `m_enabledLightCount - 1` 위치를 사용해 역순으로 끈다.
- 진행 도중 목표가 바뀌면 현재 켜진 개수에서 방향을 전환한다.
- Material의 `m_emissiveIntensity`(에디터 값)와 `m_runtimeEmissiveMultiplier`(시퀀스 값)를 분리해, 시퀀스가 꺼도 에디터 설정값이 파괴되지 않는다.

### Rim Lighting과 카메라 위치 전달

그래픽스 강의에서 배운 Rim Lighting을 기존 조명·Material 파이프라인 위에 얹는 형태로 구현했다.

- **Rim 데이터는 새 컴포넌트가 아니라 `Material`의 필드로 저장한다**(`rimColor`/`rimIntensity`/`rimPower`, 기본 `rimIntensity = 0`). Rim은 표면 셰이딩 파라미터라 이미 `baseColor`/`emissiveColor`를 들고 있는 `Material`의 책임 범위에 속한다고 판단했고, 기본값을 0으로 둬서 Emissive와 같은 "기본 꺼짐, 오브젝트별 opt-in" 패턴을 그대로 재사용했다 — `BoxCollisionComponent` 같은 Has 플래그 컴포넌트가 필요 없다.
- **Pixel Shader가 처음으로 카메라 월드 위치를 받도록 Constant Buffer 경로를 새로 텄다.** 기존에는 `view`/`projection`을 Vertex Shader만 알았다. Rim 계산(`viewDir = normalize(cameraPosition - posWorld)`)은 픽셀별 월드 위치 기준으로 Pixel Shader에서 계산해야 해서, `CameraConstantData`에 `cameraPosition`(+정렬용 `pad`, 총 144바이트)을 추가하고 같은 버퍼를 `simplePixelShader.hlsl`의 `register(b2)`에도 바인딩했다.
- `Renderer::DrawRenderItem`의 Pixel Shader Constant Buffer 바인딩(`PSSetConstantBuffers(1, 2, { materialConstantBuffer, cameraConstantBuffer })`)은 기존 Vertex Shader 바인딩(`VSSetConstantBuffers(0, 2, ...)`)과 동일한 형태를 그대로 따라, 셰이더 슬롯 규칙(PS의 b0=Light, b1=Material, b2=Camera)을 일관되게 유지했다.

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

**Rim Lighting 완료 ✅ — 다음은 HDR Scene Target + Bloom + Tone Mapping (로드맵 11번, Shadow Mapping보다 먼저).**

`Material`에 Rim Color/Intensity/Power를 추가하고 Pixel Shader가 처음으로 카메라 월드 위치를 받도록 Constant Buffer 경로를 새로 터서 Rim 발광을 구현했다. PowerSwitch Material에 테스트 값을 적용해 화면 가장자리가 시야각에 따라 밝아지는 것을 빌드·실행으로 확인했다. 설계 배경과 이유는 §4 "Rim Lighting과 카메라 위치 전달" 참고.

**진행 순서 변경**: 그래픽스 강의(홍정모)에서 Rim, Cube Mapping, Environment Mapping, IBL+CMFT, Fresnel, Bloom을 배운 시점에 맞춰, 기존 로드맵의 9→10→11 순서 대신 **Rim(완료) → 11번(HDR+Bloom) → 10번(Normal/Roughness+Fresnel+Cube Mapping+IBL, 묶어서 진행) → 9번(Shadow Mapping)** 순으로 진행하기로 했다. 이유는 §8 "Rim Lighting 구현 완료" 설계 결정 참고. Shadow Mapping은 다른 항목에 의존하지 않아 뒤로 미뤄도 손해가 없다.

**다음 작업 — HDR Scene Target + Bloom + Tone Mapping (로드맵 11번)**

아직 설계 가이드를 시작하지 않았다(다음 세션에서 왜 필요한가/배경 개념부터 안내 예정). 핵심 동기: `simplePixelShader.hlsl`의 최종 `saturate`가 1을 넘는 색을 그대로 잘라버려서, 지금까지 넣은 Emissive Intensity(3/5/8)와 Rim Intensity 차이가 화면에서 전혀 구분되지 않는다 — HDR Scene Target(부동소수점 렌더 타깃) + Tone Mapping으로 이 클리핑 자체를 없애고, Bloom으로 밝은 픽셀(네온/Rim)이 주변으로 번지는 효과까지 더하는 것이 목표다.

**참고 — Shadow Mapping (로드맵 9번, 순서상 마지막으로 미룸)**: 이미 대화로 설계 가이드까지 나온 상태이니 재개 시 아래 순서로 이어가면 된다.

1. `GraphicsResourceManager`에 GPU 전용 Depth+SRV 겸용 텍스처 생성 기능 추가(`DXGI_FORMAT_R32_TYPELESS`로 만들어 DSV는 `D32_FLOAT`, SRV는 `R32_FLOAT`) — 지금은 파일 로드 텍스처만 지원해서 이 기능이 없다.
2. Directional Light 시점 View/Projection(Orthographic) 계산 — Point Light 그림자는 Cube Map이 필요해 훨씬 복잡하므로 이번엔 제외.
3. Depth-only Shadow Pass 셰이더로 Shadow Map에 렌더(RenderDoc/Graphics Debugger로 캡처해서 확인 — 화면엔 안 보이는 단계).
4. 메인 패스 Pixel Shader에서 Shadow Map 샘플링 + 그림자 판정(여기서 처음 화면에 그림자가 보임).
5. Shadow Acne/Peter Panning 보정(Depth Bias).
6. PCF(`SamplerComparisonState`+`SampleCmp`)로 그림자 경계 부드럽게.

---

## 7. 로드맵

1. ✅ 골목 Greybox와 월드 스케일 확정
2. ✅ 최소 상호작용 기반과 PowerSwitch 책임 분리
3. ✅ 전원 상태에 따른 스위치 시각 피드백
4. ✅ 다수 Point Light와 Renderer 제출
5. ✅ Emissive Material과 네온 표면 표현
6. ✅ 전원 상태와 Point Light·Emissive Material 순차 점등 연동
7. ✅ 조명 역할 분류(`NeonSignFactory`), 네온 배치·색상 정리
7-1. ✅ Rim Lighting (그래픽스 강의 연계, 원래 로드맵에 없던 항목 — 2026-09-14 완료. Pixel Shader가 카메라 월드 위치를 처음 받도록 Constant Buffer를 확장했다. 상세는 §4 "Rim Lighting과 카메라 위치 전달" 참고)
8. 실제 골목 에셋 배치와 Scene 편집 보강 (보류 — 9번 이후 재판단)
9. Shadow Mapping (그래픽스 강의 연계로 10·11번 다음 순서로 미룸 — §6 참고)
10. Normal/Roughness Material과 젖은 바닥 반사 (Fresnel·Cube Mapping·IBL+CMFT를 여기 묶어서 진행 — 11번 다음 순서)
11. HDR Scene Target, Bloom과 Tone Mapping ← 다음 (그래픽스 강의 연계로 9·10번보다 먼저 진행. LDR saturate 클리핑을 없애 Rim/Emissive 밝기 차이를 실제로 보이게 하는 것이 목표)
12. 안개, 비와 색조 보정
13. 충돌/이동 제한, 디버그 UI와 최적화 (기본 플레이어-벽 충돌은 9번보다 먼저 앞당겨 완료 ✅ — `BoxCollisionComponent`/`PlayerCollision`. 이동 제한 나머지와 디버그 UI·최적화는 그대로 보류)
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

### 2026-09-14 — 플레이어-벽 충돌 구현 완료 ✅

- 완료한 작업: `BoxCollisionComponent`(빈 마커, `GameObject`에 `PointLightComponent`와 같은 Has 패턴으로 추가), `Scene::GatherBoxColliders()`(`GatherPointLights`와 같은 위치의 책임, `std::vector<DirectX::BoundingBox>` 반환), `PlayerCollision::Resolve`/`PushOutOfBox`(원-박스 최근접점 계산 후 밀어내기, 정적 함수)를 구현했다. `InitGreyBoxScene`에서 좌/우/끝벽 + 새로 만든 입구 벽(`startWall`, Mesh 없이 충돌만) + `PowerSwitch`에 `AddBoxCollisionComponent()`를 호출해 총 5개를 충돌체로 등록했고, `AppBase::Update`(Play 분기)에서 `FirstPersonCameraController::Update` 직후 `PlayerCollision::Resolve` 결과로 `Camera` 위치를 보정하도록 연결했다.
- 확인한 결과: 매 단계 코드 리뷰로 확인했고, 실제로 두 차례 버그를 잡았다 — ① `PushOutOfBox`에서 최근접점 Y를 `0.0f`로 고정해 `diff.y`에 카메라 높이(1.6)가 그대로 남는 바람에 `diffLength`가 항상 `radius`보다 커져서 충돌 판정이 절대 안 걸리던 버그(`0.0f` → `position.y`로 수정), ② `Resolve`가 루프만 있고 `PushOutOfBox` 호출과 `return`이 없어 미완성 상태였던 것. 빌드·실행해서 좌/우/끝벽·입구·PowerSwitch를 통과할 수 없고 트인 공간은 평소처럼 움직인다는 것을 사용자가 직접 확인했다.
- 남아 있는 문제: `Floor`는 걷는 영역과 충돌체가 XZ에서 겹치는 특성상 의도적으로 충돌체 목록에서 제외했다(§4 참고) — 나중에 중력/점프가 생기면 별도의 Y축 전용 로직이 필요하다. `simplePixelShader.hlsl`의 LDR `saturate` 클리핑은 여전히 미해결(HDR 단계 대기).
- 다음에 이어서 할 작업: 로드맵 9번 Shadow Mapping 착수(§6 참고, 설계 가이드는 이미 대화로 제공됨).
- 중요한 설계 결정과 이유:
  - **`PlayerCollision`이 `GameObject`를 직접 참조하던 원래 설계를 버리고 `BoxCollisionComponent` + `Scene::GatherBoxColliders` 컴포넌트 패턴으로 바꿈**: 사용자가 "Unreal처럼 컴포넌트로 만드는 게 낫지 않나"라고 제안했고, 이 프로젝트에 이미 있는 `PointLightComponent`(Has 플래그 + `Scene::Gather*`) 패턴을 재사용하는 것이라 §3 원칙(범용 추상화를 미리 안 만든다)에 어긋나지 않는다고 판단해 채택했다. 부수 효과로 "Floor 범위로 clamp"라는 특수 케이스가 사라지고, 벽 4개+스위치를 전부 동일한 원-박스 로직으로 처리하는 더 단순한 구조가 됐다.
  - **`BoundingBox`만 지원하고 `BoundingSphere` 등은 지금 만들지 않음**: 사용자가 "다양한 모양의 Mesh가 있을 텐데"라고 물었으나, 현재 씬에는 Cube뿐이고 실제로 다른 모양이 필요한 시점이 아니라서(로드맵 8번은 보류 중) §3 원칙대로 지금은 만들지 않기로 했다. 대신 `GatherColliders`가 아니라 `GatherBoxColliders`, `CollisionComponent`가 아니라 `BoxCollisionComponent`로 **정직하게 이름 지어서**, 나중에 `SphereCollisionComponent`/`GatherSphereColliders`를 형제로 추가할 때 기존 이름을 바꾸거나 호출부를 고칠 필요가 없게 했다(Unreal의 `UBoxComponent`/`USphereComponent`가 형제 클래스인 것과 같은 방향).
  - **`Floor`를 충돌체에서 제외함**: 이 시스템은 XZ 평면만 다루는데 `Floor`의 XZ 풋프린트가 걸을 수 있는 영역 전체와 일치해서, 장애물로 취급하면 플레이어가 항상 그 박스 안에 있는 깨진 경우(거리 0, 방향 미정의)가 된다. `Floor`는 "수직으로 받쳐주는" Y축 개념이라 지금의 XZ 전용 벽 충돌과는 다른 문제라는 걸 확인했다.
  - **`PlayerCollision`을 정적 함수로 구현**: `Floor`/`PowerSwitch` 참조를 없애면서 완전히 무상태가 돼, `NeonSignFactory`와 같은 이유로 인스턴스 없이 정적 함수로 충분하다고 판단했다.
  - **`MyPF/imgui.ini`, `MyPF/ImGui/imgui.ini` 두 파일을 `.gitignore`에 등록하고 `git rm --cached`로 인덱스에서 제거함**: 전자는 원래도 `.gitignore`에 있었지만 이미 추적 중이던 파일이라 규칙이 무효했다(추가만 막을 뿐 기존 추적은 안 끊음). 후자는 ImGui 벤더 폴더 안에 있던 스트레이 파일(작업 디렉터리가 잘못 잡혔을 때 생긴 것으로 추정)로, 이번에 발견해서 같이 정리했다. 둘 다 로컬 파일은 삭제하지 않고 인덱스에서만 뺐다.

### 2026-09-14 — Rim Lighting 구현 완료 ✅

- 완료한 작업: Rim Lighting을 Step 1(카메라 월드 위치를 Pixel Shader까지 전달하는 Constant Buffer 확장) → Step 2(Rim 계산: `Material`에 `rimColor`/`rimIntensity`/`rimPower` 추가, HLSL에서 `pow(1-saturate(dot(normal,viewDir)), rimPower)` 계산해 `finalColor`에 가산) 순서로 구현했다. `ShaderConstants.h`의 `CameraConstantData`에 `cameraPosition`(+정렬용 `pad`)을 추가하고 `FrameRenderData`/`Renderer::BeginFrame`을 거쳐 채웠으며, `simplePixelShader.hlsl`에 `register(b2)` 카메라 Constant Buffer를 새로 선언했다. `MaterialConstantData`를 64바이트로 확장해 Rim 세 필드를 추가하고 `EditorUI::DrawMaterialInspector`에 Rim Color/Intensity/Power 슬라이더를 추가했다.
- 확인한 결과: 매 단계 코드 리뷰로 확인했고, 두 차례 버그를 잡았다 — ① `CameraConstantData`가 `Matrix view + Matrix projection + Vector3 cameraPosition` = 140바이트로 16의 배수가 아니어서 `CreateConstantBuffer<T>`의 `static_assert`를 어길 뻔한 것(`float pad` 추가로 144바이트로 수정), ② `Renderer::DrawRenderItem`에서 Pixel Shader용 상수 버퍼 배열이 `{ materialConstantBuffer }` 하나만 들어있는 채로 slot 2(카메라가 기대하는 slot)에 바인딩되고 있어서 slot 1(Material이 기대하는 slot)은 비고 카메라 버퍼는 Pixel Shader에 아예 바인딩된 적이 없던 문제(`{ materialConstantBuffer, cameraConstantBuffer }`로 고치고 `PSSetConstantBuffers(1, 2, ...)`로 수정) — Rim이 Pixel Shader가 카메라 데이터를 받는 첫 사례라서 처음 드러난 버그다. PowerSwitch Material에 테스트 값(Rim Color 청록, Intensity 2.5, Power 4.0)을 적용해 화면 가장자리가 시야각에 따라 밝아지는 것을 사용자가 직접 확인했다.
- 남아 있는 문제: LDR `saturate` 클리핑이 여전히 해결되지 않아 Rim Intensity 값 간 밝기 차이도 Emissive와 마찬가지로 화면에서 잘 구분되지 않는다(HDR 단계에서 함께 해결 예정). `Renderer::Initialize()`가 `m_materialConstantData`의 Rim 필드를 버퍼 생성 시점에 명시적으로 0으로 초기화하지 않는다(첫 `DrawRenderItem` 호출 전에 항상 덮어써지므로 실질적 위험은 없음 — 선택적 정리 항목으로 남겨둠).
- 다음에 이어서 할 작업: 로드맵 11번 HDR Scene Target + Bloom + Tone Mapping(§6/§7 참고 — 그래픽스 강의 연계로 9번 Shadow Mapping보다 먼저 진행하기로 함).
- 중요한 설계 결정과 이유:
  - **Rim 데이터를 새 컴포넌트가 아니라 `Material`의 필드로 저장함**: `PlayerCollision`/`BoxCollisionComponent`와 달리 Rim은 표면 셰이딩 파라미터라 이미 `baseColor`/`emissiveColor`/`emissiveIntensity`를 들고 있는 `Material`의 책임 범위에 자연스럽게 속한다고 판단했다. 기본값을 `rimIntensity = 0`으로 둬서 Emissive와 같은 "기본 꺼짐, 오브젝트별로 opt-in" 패턴을 재사용했다 — 새 컴포넌트나 Has 플래그가 필요 없다.
  - **Pixel Shader가 카메라 월드 위치를 받도록 새 Constant Buffer 경로를 텄음**: 기존에는 Vertex Shader만 `view`/`projection`을 알았고 Pixel Shader는 몰랐다. Rim 계산(`viewDir = normalize(cameraPosition - posWorld)`)은 반드시 Pixel Shader에서 픽셀별 월드 위치가 필요해, `CameraConstantData`에 `cameraPosition`을 추가하고 같은 버퍼를 Pixel Shader의 `register(b2)`에도 바인딩하는 방식을 택했다 — 이 프로젝트에서 Pixel Shader가 카메라 데이터를 받는 첫 사례다.
  - **그래픽스 강의 진도(Rim → HDR/Bloom → Fresnel/Cube Mapping/IBL+CMFT)를 기존 로드맵 순서보다 우선함**: 강의에서 막 배운 개념을 바로 포트폴리오에 적용하는 게 학습 정착에도 낫고, HDR을 Shadow Mapping보다 먼저 하면 그동안 LDR `saturate`에 가려져 있던 Emissive/Rim 밝기 차이가 실제로 보이게 되는 이득도 있다고 판단해 §6/§7을 이 순서로 갱신했다. Shadow Mapping 자체는 다른 항목에 의존하지 않으므로 순서를 미뤄도 손해가 없다.

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

- `git pull` 후 HEAD가 최소 `2835951`인지 확인한다.
- Play에서 PowerSwitch를 여러 각도에서 바라보며 가장자리가 시야각에 따라 청록색으로 밝아지는지(Rim Lighting) 확인한다.
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
