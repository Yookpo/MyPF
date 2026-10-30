# MyPF 진행 기록 및 작업 인계

마지막 갱신: 2026-10-29

노트북과 데스크톱에서 Git으로 공유하는 MyPF의 실제 구현 상태와 다음 작업을 기록한다.

**이 문서의 역할**: 프로젝트 규칙·아키텍처 원칙·협업 방식·프레임 흐름은 저장소 루트의 `AGENTS.md`가 소유한다. 이 문서는 그와 중복되지 않는 **진행 상태, 완료 기록, 수치 상세, 설계 결정 근거**만 담는다. 자동 로드 대상이 아니며 필요할 때 읽는다.

**시작 절차**

1. 저장소 루트의 `AGENTS.md`를 전부 읽는다.
2. 이 문서에서 필요한 절을 읽는다.
3. 실제 소스, `git status`와 최근 commit을 확인한다.
4. 문서와 코드가 다르면 **실제 코드와 최신 commit을 우선**한다.

---

## 1. 진행률

현재 전체 진행률은 약 **84%**다(2026-10-28, 10-4 환경 반사 완료 기준).

| 영역 | 진행 | 현재 상태 |
|---|---:|---|
| Win32/DX11 기반 | 약 90% | 기본 렌더링, Resize와 제출 경계, HDR 씬 타깃 → 다중 후처리 패스 → 백버퍼 구조, DDS 큐브맵 로드 완료 |
| GPU Resource/Asset 구조 | 약 80% | Buffer/Texture Handle, Asset 캐시와 Model 업로드, 렌더 타깃 생성과 같은 슬롯 재생성 완료 |
| Model 파이프라인 | 약 65% | FBX/OBJ/glTF BaseColor 로드 완료. aiNode/PBR은 남음 |
| Scene/ImGui 편집 | 약 72% | Greybox Scene, 선택과 Transform/Material 편집, Point Light 소유, Post Process 패널(Exposure/Tone Mapper/Bloom 3종/Debug View) 완료 |
| 1인칭 입력/카메라 | 약 90% | Editor/Play, WASD/마우스/ESC/focus와 단발 키 입력, 플레이어-벽 충돌 완료 |
| 조명/Material | 약 90% | Directional/Ambient, 최대 8 Point Light, Emissive, Rim, 선형 색공간(sRGB 텍스처와 색 상수 변환), 밤 골목 조명 세팅, 셰이더 빛/표면 분리, 재질별 Roughness/Specular, **GGX + Smith(Unreal 형식) + Schlick Fresnel, π 규약(diffuse = albedo / π)**, UV 타일링, roughness·노멀 맵(탄젠트, 기본 텍스처 3종), 웅덩이 마스크, **큐브맵 환경 반사(split-sum: roughness → 밉 LOD × `EnvBRDFApprox`)** 완료. diffuse IBL·metallic은 없음 |
| 실제 골목/상호작용 | 약 85% | PowerSwitch와 Ray 기반 E 입력, 스위치 피드백·대기 표시등·호박색 Rim 강조(PowerSwitch 소유), 순차 점등 완료 |
| 고급 렌더링/연출 | 약 60% | HDR 씬 타깃, Exposure, Reinhard/ACES 톤 매핑, 출력 감마, Bloom(추출·분리형 블러·합성), 룩 재튜닝, 젖은 바닥의 Point Light 반사(Specular·Fresnel)와 웅덩이 대비, HDRI Skybox(`SkyBox`), 하늘 큐브맵 반사(10-4) 완료. Shadow/평면 반사(간판이 비치는 반사)/Fog/Rain/MSAA는 없음 |

---

## 2. Git 체크포인트

- 문서 갱신 기준 HEAD: `ecdb160`(사용자: 10-4 E-3 `EnvBRDFApprox` + 10-28 문서). 그 앞이 `b6aee16`(사용자: 10-4 E-1·E-2)·`c0d5883`(10-3 문서)·`e805963`(사용자: `SkyBox` 클래스 분리). `origin/main`과 같다.
- **씬 작업 브랜치는 폐기했다(2026-10-29 사용자 결정).** `SC3_MODEL_PIPELINE`(커밋 10개: 씬 재작성 R-1·R-2, 모델 파이프라인, 건물 외관, 골목 소품, 전선, stb_image 최적화)을 로컬 태그 **`archive/sc3-model-pipeline`**(= `00d0644`)으로 남기고 로컬·원격 브랜치를 지웠다. 무엇이 들어 있고 무엇을 다시 쓸 수 있는지는 §8 "2026-10-28 ~ 29 — 씬 작업 브랜치(폐기, 태그 보관)". 태그는 push하지 않았다(이 PC에만 있다 — 노트북에서도 쓰려면 `git push origin archive/sc3-model-pipeline`).
- 현재 작업 트리 (2026-10-29): **NightSky HDRI 세 파일이 스테이징만 돼 있다**(태그에서 꺼냄 — `HDRI/CREDITS.md` 참고, 코드는 아직 hansaplatz를 읽는다), 이 문서·`AGENTS.md`·`HDRI/CREDITS.md` 갱신. 사용자가 직접 커밋한다. 브랜치에서 받은 원본 에셋 폴더(`Assets/Models/`의 모델 6종, `PBR/floor_asphalt`·`sidewalk_concrete`)가 **추적되지 않는 파일로** 디스크에 남아 있다 — 다시 받을 수 있는 원본이라 지워도 되고, 씬 단계로 돌아갈 때 그대로 쓸 수도 있다. commit/push는 사용자가 요청할 때만 한다.
- **`HDRI/hansaplatz_2k.hdr`(6.8MB)가 `26a062a`에 실수로 커밋돼 아직 추적 중이다(2026-10-29 `git ls-files`로 재확인).** `.gitignore`와 `CREDITS.md`는 "원본은 로컬에만"이라 적혀 있고, 사용자가 추적만 끊기로(B안) 정했다 — 다음 커밋에서 `git rm --cached MyPF/Assets/Textures/HDRI/hansaplatz_2k.hdr`(로컬 파일은 유지). 이미 히스토리에 들어간 6.8MB는 남는다(지우려면 히스토리 재작성이 필요해 하지 않는다).
- 10-2 Fresnel은 두 커밋이다 — `6fd8742`(사용자: `FresnelSchlick`과 `ComputeDirectLighting` 조립 + 확장 예약 목록 문서), `04fab3c`(바닥 specular 0.5 → 0.25).
- `a431f1f`는 렌더러와 무관한 학습용 정적 사이트(`study-site/`), 그 생성 규칙 스킬(`.claude/skills/study-site/SKILL.md`), 프리뷰 서버 설정(`.claude/launch.json`)을 추가했다(13개 파일). 상세는 §3 "학습 사이트".
- `e9ab696`은 10-1 E~G를 한 번에 담고 있다 — 셰이더 E-1(`viewDir` 전달)·E-2(`SurfaceData`에 roughness/specular)·E-3(Blinn-Phong)·F(정규화)와 G(`AppBase.cpp`·`NeonSign.cpp`의 재질값). 4개 파일.
- `8e4cadc`는 메시지는 카메라 이동이지만 **10-1a(Material Roughness/Specular 값 경로), R-1(`Common.hlsli` 신설), R-2(카메라 cbuffer 공유와 슬롯 재배치)**를 함께 담고 있다(11개 파일).
- **로드맵 11번 작업을 `main`에 머지했다(2026-09-17).** `--no-ff`로 머지해 "HDR + Bloom 작업 덩어리"의 경계를 히스토리에 남겼다. 33개 파일, +1416/-155, 충돌 없음.
- 로컬 기능 브랜치 `HDR_SCENE_TARGET`/`WORK_CLAUDE`는 삭제됐고 원격에는 남아 있다(`origin/HDR_SCENE_TARGET` = `3c0edda`, `origin/WORK_CLAUDE` = `9d195bd`). 둘 다 내용이 `main`에 전부 포함된 것을 `git merge-base --is-ancestor`로 확인했으므로 원격 브랜치는 지워도 잃는 것이 없다.
- `MyPF/imgui.ini`와 `MyPF/ImGui/imgui.ini`는 `.gitignore`에 등록하고 `git rm --cached`로 인덱스에서 제거했다(로컬 파일은 유지) — 이제부터는 변경돼도 `git status`에 아예 안 잡힌다(2026-09-14).
- 저장소 루트 `.editorconfig`가 VS 저장 시 `.cpp`/`.h`는 UTF-8 with BOM, `.hlsl`/`.hlsli`는 BOM 없는 UTF-8을 강제한다(2026-09-15).

최근 기능 commit:

```text
ecdb160 큐브맵 환경반사
b6aee16 환경반사 기본
c0d5883 10_27 MDs Update
e805963 스카이 박스 리팩토링
81d104c HDR 큐브맵 로드
26a062a 큐브맵 도입
fb58d71 주석 수정
01f7d38 언리얼 형식의 GGX 와 Smith 구현
891c6f1 Update MD - W-3 웅덩이 마스크 완료(10-W 닫힘), 다음 10-2' GGX
f44ea8c 에셋: 기본 검정 텍스처, 골목 바닥 웅덩이 마스크 (직접 생성)
5524d01 웅덩이 마스크 (W-3): t3 슬롯, 기본 검정, roughness/albedo/normal 보간
1c0ea97 Update MD - N-5 벽 노멀 맵 완료, 시작 벽 통과 재현, 다음 W-3
fc397d8 에셋: ambientCG Bricks096 (벽 Color, Roughness, NormalDX)
bd0272e 벽 노멀 맵 (N-5): Bricks096 텍스처 세트, 실제 크기 UV, 룩 값
70ac73f Update MD - 10-6 탄젠트·노멀 맵 완료, 다음 W-3
5014670 에셋: Asphalt023S NormalDX, 기본 평평한 노멀 텍스처
88a0eb9 탄젠트 + 노멀 맵 (10-6), 텍스처 슬롯 헬퍼
71feced 바닥 BaseColor 0.75 -> 0.55 (W-2 마무리)
b0d253f 에셋: ambientCG Asphalt023S(Color·Roughness), 기본 흰색 텍스처
9f5a1fa roughness 맵 + 기본 흰색 텍스처 (W-2), 바닥을 ambientCG 아스팔트로
ff34261 UV 타일링 (W-1) + Editor 패널 스크롤
42f7698 Update MD - 10-2 Fresnel 완료, 젖은 바닥 디테일(10-W) 앞당김
04fab3c 바닥 specular를 물리값 0.25로 (10-2c)
6fd8742 슐릭근사 함수 세팅 및 적용 (10-2a·b, 확장 예약 목록)
1a13af1 Update MD - 강의 PBR 자료 지도, 로드맵 10 보강, 에셋 정책
e1ea36e Update MD - 작업 재개 점검, 문서·스킬 설정 동기화
a431f1f 웹사이트 추가 (study-site, 렌더러 코드 변경 없음)
adee25f Update MD (10-1 Specular 완료 반영)
e9ab696 Specular 추가
b59242a 셰이더 표면 쪽 정리 - SurfaceData, ComputeDirectLighting
f0273c2 셰이더 빛 쪽 정리 - IncidentLight, GetDirectionalLight, GetPointLight
8e4cadc 카메라 cbuffer를 Common으로 옮기기 (+ Material Roughness/Specular, Common.hlsli)
f91b0e6 파일 텍스처에 밉맵 생성 (GenerateMips)
6256ed2 D3D11Utils 로그에 줄바꿈 추가
0d0eac2 텍스처 캐시 키를 (경로, 용도)로 변경
ddb29c7 텍스처 타입 생성
5794467 Merge branch 'HDR_SCENE_TARGET' - 로드맵 11번 (HDR + Tone Mapping + Bloom)
050a1d9 룩 재튜닝 - 밤 골목 조명과 후처리 초기값 확정
89f8d64 Bloom 블러 반복 횟수 조절
4df455b Step 7. Bloom 합성
80b825f Step 6. 분리형 가우시안 블러
5ddb2ab Step 5. Bloom 밝은 부분 추출 + 디버그 뷰
765b68c 스위치 Rim 강조를 PowerSwitch로 이동
10fa116 상호작용 가능할 때만 전원 스위치 Rim 강조
86f17a4 Step 3-d. ACES 톤 매핑 + Tone Mapper 전환
e17b870 Step 4. 선형 색공간(감마) 정리
d5d36d7 Step 3. 톤 매핑 + Exposure
0e75485 Step 2. 씬을 HDR 타깃에 그리고 백버퍼로 옮기기
74aebd8 Step 1. 렌더 타깃 텍스처를 만드는 기능
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
- `AssetManager`의 Mesh/Texture/Material/Model 소유와 이름 캐시 (텍스처만 `(경로, TextureType)` 키)
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
- (`8e4cadc`) `Material`에 `roughness`(기본 0.5, `[0.05, 1]`로 clamp — 0이면 Blinn-Phong 지수가 무한대)와 `specular`(기본 0.5, `[0, 1]`)를 추가했다. `MaterialConstantData`는 64바이트를 유지한 채 기존 패딩 자리(`baseColor` 뒤, `rimPower` 뒤)를 채웠다. 둘 다 배율이라 `SrgbToLinear`를 거치지 않는다. Inspector에 슬라이더가 있다.
- **셰이더 공통 파일 구조** (`8e4cadc`, `f0273c2`, `b59242a`, `e9ab696`): `Common.hlsli` = 모든 씬 셰이더가 쓰는 선언(`PI`, `VS_INPUT`/`PS_INPUT`, 카메라 cbuffer `b1`). `Lighting.hlsli` = 빛 쪽(`PointLight`, `IncidentLight`, Light cbuffer `b0`, `CalcAttenuation`, `GetDirectionalLight()`, `GetPointLight(index, posWorld)`)과 표면 쪽(`SurfaceData`, `SpecularBlinnPhong`, `ComputeDirectLighting`). Material cbuffer(`b2`)는 `simplePixelShader.hlsl`에 남는다. 상수 버퍼 슬롯은 **VS: b0=Object, b1=Camera / PS: b0=Light, b1=Camera, b2=Material**. 설계 이유는 §4 "셰이더 구조 — 빛 쪽과 표면 쪽 분리"
- **`main`의 흐름** (`b59242a`): 입력 준비(`normal`, 텍스처 × `baseColor`, `viewDir`, `SurfaceData`) → 직접광(`ComputeDirectLighting`을 방향광 결과로 시작해 점광원 루프에서 누적) → 간접광(ambient, 함수 밖에서 한 번) → 연출 항(emissive, rim) → 합성. 렌더링 방정식 `Le + Σ직접광 + 간접광`과 블록이 1:1로 대응한다
- **직접광 BRDF — GGX + Smith + Schlick, π 규약** (10-2', `01f7d38` G-1 + `fb58d71` G-3): `ComputeDirectLighting(surface, light, viewDir)` 한 곳에서 모든 빛에 적용된다. 입구에서 `roughness = max(roughness, MIN_PERCEPTUAL_ROUGHNESS 0.05)` → `NdotL ≤ 0`이면 조기 반환 → `NdotV = saturate(abs(N·V) + 1e-5)`(Unreal, 노멀 맵·뒷면의 음수 N·V를 버리지 않음) → `L + V`의 제곱 길이가 0에 가까우면 반환 → `H`, `NdotH`, `VdotH` → `F = F_Schlick(0.08 × specular, VdotH)` → `specularTerm = SpecularGGX(roughness, NdotH, NdotV, NdotL) × F` → `diffuseTerm = albedo × (1 − F) / π` → `(diffuseTerm + specularTerm) × radiance × NdotL`. `SpecularGGX`는 `a2 = roughness⁴`(Unreal `Pow4`)를 한 번 계산해 `D_GGX(a2, NdotH)` = `a2 / (π·d²)`, `d = (NdotH·a2 − NdotH)·NdotH + 1`와 `Vis_SmithJointApprox(a2, NdotV, NdotL)` = `0.5 / (NdotL(NdotV(1−a)+a) + NdotV(NdotL(1−a)+a))`의 곱(**F 제외**)을 반환한다. 함수 이름은 Unreal `BRDF.ush`, 내적 변수는 `NdotX`. specular 항에는 albedo를 곱하지 않는다. 설계 이유는 §4 "GGX + Smith와 π 규약 전환"
- **Fresnel — Schlick 근사** (`6fd8742`, 10-2'에서 이름만 `FresnelSchlick` → **`F_Schlick`**): `F_Schlick(f0, cosTheta)` = `f0 + (1 − f0) × (1 − cos)⁵`. `f0 = 0.08 × specular`(Unreal Specular 파라미터 규약). 인자 이름은 IBL(N·V)에서도 쓰므로 Unreal의 `VoH`가 아니라 `cosTheta`. 비스듬한 시선일수록 반사가 강해지고 그만큼 diffuse가 줄어든다. 설계 이유는 §4 "Fresnel — Schlick 근사"
- **직접광 세기는 π 규약 값이다** (10-2'): diffuse·specular가 모두 `/π`가 되면서 Directional·네온 3개·환경 보조광의 세기를 **`이전 값 × DirectX::XM_PI`**로 바꿨다(식 그대로 코드에 남겨 앞자리가 튜닝 값으로 읽힌다). **ambient(`ambientStrength`)·Emissive·Rim은 그대로**다 — ambient는 하늘 radiance L로 보면 `(albedo / π) × πL = albedo × L`이라 식이 같고, Emissive·Rim은 BRDF를 거치지 않는 radiance다. 결과 화면은 전환 전과 같다(`USE_GGX = false`로 확인)
- **재질별 Roughness/Specular** (`e9ab696`):

| 재질 | 위치 | BaseColor | Roughness | Specular | UV Scale |
|---|---|---:|---:|---:|---:|
| 바닥 (젖은 아스팔트, ambientCG `Asphalt023S` + roughness·노멀 맵 + 웅덩이 마스크(W-3) — 표의 값은 마른 쪽, 웅덩이 값은 아래 "웅덩이 마스크") | `AppBase::InitGreyBoxScene` `floorMat` | **0.55** (선형 약 0.27배, `71feced`. 새 아스팔트가 밝아 0.75에서 내림) | **배율 0.45** × 맵(평균 ≈ 0.65) ≈ 0.29 — 이전 평균 0.3 보존 | **0.25** (물의 F0 약 2%, `04fab3c`. 그 전 0.5는 Fresnel이 없던 때의 임시 보정) | **(3.2, 16)** — 실제 크기 1.25m 기준 |
| 옆벽 (석재 벽, ambientCG `Bricks096` + roughness·노멀 맵, N-5, 좌·우 공유) | 〃 `wallMat` | **(0.65, 0.67, 0.75)** (선형 약 0.39배, 노란 베이지 모르타르를 누르고 B를 남김) | **배율 1.0** × 맵(평균 ≈ 0.58) ≈ 0.58 — 상한 | 0.5 | **(10, 4)** — 2m × 1m 추정 기준 |
| 끝벽 (`wallMat`과 같은 룩) | 〃 `endWallMat` (`ff34261`) | (0.65, 0.67, 0.75) | 1.0 | 0.5 | **(2, 4)** |
| 전원 스위치 (도장 패널) | 〃 `powerSwitchMat` | — | 0.4 | 0.5 | (1, 1) |
| 네온 (유리관, 3개 공통) | `NeonSignFactory::Create` `neonMat` | — | 0.3 | 0.5 | (1, 1) |

UV Scale의 일반식은 **축마다 `면 크기 ÷ 텍스처 실제 크기`**다. 바닥은 1.25m라 4 ÷ 1.25, 20 ÷ 1.25 = (3.2, 16). 벽(N-5)은 Bricks096이 2:1(2048×1024)이고 실제 크기 표기가 없어 **2m × 1m로 추정**했다(근거는 `PBR/CREDITS.md`): 옆벽 20m × 4m → (10, 4), 끝벽 4m × 4m → (2, 4). 정사각 텍스처가 아니면 두 축의 나누는 수가 다르다 — 예전 "면 크기 × 0.5"는 정사각 2m 텍스처 전제의 특수한 경우다(옛 벽 텍스처의 (10, 2)를 그대로 두면 돌이 세로로 2배 늘어난다).

- **roughness 맵** (W-2, `9f5a1fa`): `Material::m_roughnessTexture`(없으면 `nullptr`) → `Renderer`가 PS `t1`에 바인딩 → PS가 `tiledUV`로 샘플해 `.r` × `roughnessFactor` → `SurfaceData.roughness`. 최종 roughness = **배율 × 맵**(glTF·Unreal 방식). cbuffer 상수는 `roughness` → **`roughnessFactor`**로 이름을 바꿨다(C++·HLSL, 자리·크기 그대로 — 예약 항목 처리). `Material::SetRoughness`는 이름을 유지하되 의미가 배율이 됐다. `SpecularBlinnPhong` 입구에서 `MIN_PERCEPTUAL_ROUGHNESS = 0.05`(Filament 이름)로 막는다
- **기본 흰색 텍스처** (W-2): `Renderer`가 `Assets/Textures/Default/white_1x1.png`를 `R8G8B8A8_UNORM`으로 만들어 `m_defaultWhiteTextureHandle`로 소유한다. 재질에 roughness 맵이 없으면 이것을 `t1`에 건다(곱해도 변하지 않는 1). 포인터는 있는데 핸들·SRV가 무효면 albedo처럼 `false`. 텍스처 슬롯은 **PS: t0 = Albedo, t1 = Roughness**. 바인딩은 `DrawRenderItem`에서 "핸들 고르기(삼항) → 핸들 검사 → `GetSRV` 한 번 → `nullptr` 검사" 순서이고, 실패 로그가 `No Albedo SRV` / `No Roughness SRV`로 구분된다. 10-6에서 이 순서를 **`Renderer::ResolveTextureSRV(texture, fallback)`** 헬퍼로 뺐다
- **탄젠트 + 노멀 맵** (10-6, `88a0eb9`): `Vertex.tangent`(모델 공간, 기본 (1, 0, 0)) → Input Layout `TANGENT`(오프셋 32, 2026-10-30 Vertex Color 삭제 전에는 44) → VS가 `model`로 변환·정규화(노멀은 `invTranspose`) → `PS_INPUT.tangentWorld` → PS가 노멀 맵을 `tiledUV`로 `Sample` → `× 2 − 1`(`normalTS`) → 그람-슈미트로 T를 정규화된 N에 수직화, `B = cross(N, T)` → `normalize(mul(normalTS, float3x3(T, B, N)))` → `SurfaceData.normal`. Rim도 같은 법선을 쓴다. `MakeCube`의 면별 탄젠트는 그 면의 u 방향(윗면·앞면 +X, 뒷면 −X, 왼쪽 면 −Z, 오른쪽 면·아랫면 +Z)
- **기본 평평한 노멀 텍스처** (10-6): `Assets/Textures/Default/flat_normal_1x1.png`(128, 128, 255) → `m_defaultFlatNormalTextureHandle`(UNORM). 바닥(`Asphalt023S`)과 벽(`Bricks096`, N-5)이 NormalDX(`TextureType::NormalMap`)를 쓰고, 스위치·네온은 평평한 기본값이다
- **웅덩이 마스크** (W-3, `5524d01`): `Material::m_puddleMaskTexture` → `Renderer`가 PS `t3`에 바인딩(없으면 **기본 검정** `Default/black_1x1.png` → `m_defaultBlackTextureHandle`, UNORM) → PS가 **반복 없는 원본 `input.uv`**로 샘플(`puddleMaskSample`) → 마른 값과 웅덩이 값을 같은 모양 `lerp(마른, 웅덩이, mask)`로 보간: ① `normalTS`를 탄젠트 공간 `(0, 0, 1)` 쪽으로(TBN 전, 뒤의 `normalize`가 길이를 맞춤) ② albedo × `PUDDLE_ALBEDO_SCALE` **0.75**(선형 배율) ③ roughness → `PUDDLE_ROUGHNESS` **0.15**. 두 상수는 Material cbuffer가 꽉 차서 PS의 `static const`다. 바닥 `floorMat`에만 `Assets/Textures/Masks/alley_floor_puddle_mask.png`(256 × 1280, 1:5, 직접 생성, `TextureType::Data`)를 지정했다. `Lighting.hlsli`는 바뀌지 않았다
- 텍스처 슬롯은 **PS: t0 = Albedo, t1 = Roughness, t2 = Normal, t3 = PuddleMask**. 기본 텍스처는 섞이는 방식별 항등원 셋 — 곱셈 흰색, 보간·덧셈 검정, 방향 평평한 노멀(Unreal `DefaultTexture`·`BlackPlaceholderTexture`·`DefaultNormal`과 같은 구성)

- **UV 타일링** (`ff34261`, W-1): `Material::m_uvScale`(기본 (1, 1), 0 이하는 무시) → `MaterialConstantData`의 마지막 패딩 자리 `uvScale`(C++ `pad2`, HLSL `pad3`이던 자리, 64바이트 유지 — **이제 꽉 찼다**) → PS가 `tiledUV = input.uv * uvScale`로 albedo를 샘플한다. 원본 `input.uv`는 반복되면 안 되는 마스크(W-3)용으로 남겨 둔다. Inspector에 `UV Scale`(`DragFloat2`, 0.1~50). 설계 이유는 §4 "UV 타일링"
- **큐브 옆면 UV 규칙** (`ff34261`): `GeometryGenerator::MakeCube`의 옆면은 **u = 그 면을 바라보는 사람의 오른쪽(`cross(+Y, 시선)`), v = 아래**다. 오른쪽 면(+X)과 뒷면(+Z)이 90° 돌아가 있던 것을 고쳤다(UV 네 줄을 한 칸 회전)
- **Editor 패널 스크롤** (`ff34261`): 패널에 세로 스크롤바를 상시 표시(`ImGuiWindowFlags_AlwaysVerticalScrollbar`)하고, Inspector 자식 창은 `ImGuiChildFlags_AutoResizeY`로 내용만큼 늘어난다. 패널을 직접 늘리지 않아도 Material 항목까지 스크롤로 내려간다. Scene Hierarchy 목록만 고정 높이(190)로 자기 스크롤을 유지한다

- 확인: 전원 Off에서 골목 가운데 채움광이 젖은 바닥에 **파란 줄무늬**로 반사되고 벽은 무광 그대로다. On에서 네온 아래 바닥에 각 색의 길쭉한 반사가 생기고 시선을 따라 미끄러진다. 깜빡이는 흰 점(스페큘러 에일리어싱)은 없다. 144 FPS 유지.

### HDR 렌더링과 선형 색공간

- `GraphicsResourceManager::CreateRenderTarget`/`GetRTV`/`ResizeRenderTarget`: 크기·포맷으로 Texture2D + RTV + SRV를 만들고, 창 크기가 바뀌면 **같은 슬롯에서** 다시 만든다(핸들 index 유지). 실제 D3D 호출은 `D3D11Utils::CreateRenderTargetTexture`
- `Renderer`가 `R16G16B16A16_FLOAT` HDR 씬 타깃(`m_hdrSceneTargetHandle`)을 소유하고 포맷·크기를 결정한다. `WM_SIZE`에서 `GraphicsDevice::Resize` 다음에 `Renderer::Resize`가 호출된다
- `BeginFrame`은 백버퍼 대신 HDR 씬 타깃을 Clear·바인딩하고, 씬 셰이더(`simplePixelShader.hlsl`)는 최종 `saturate` 없이 1을 넘는 값을 그대로 기록한다
- `Renderer::EndScene`: 후처리 패스들을 순서대로 실행한다. 각 패스는 `Renderer::DrawFullScreenPass(타깃, PS, 입력 SRV, 샘플러, 두 번째 SRV = nullptr)` 한 번이고, 전체 화면 삼각형(`fullscreenVertexShader.hlsl`, `SV_VertexID`, 정점 버퍼·Input Layout 없음)을 `Draw(3, 0)`으로 그린다. `AppBase::Render`가 씬 순회와 ImGui 사이에서 호출한다
- Exposure와 Tone Mapper(Reinhard / ACES 근사 곡선): `PostProcessSettings`(AppBase 소유) → `FrameRenderData` → `BeginFrame`이 32바이트 `PostProcessConstantData` 업로드 → 후처리 패스가 PS `b0`에 바인딩. EditorUI "Post Process" 패널에 Tone Mapper 콤보박스(기본 ACES)와 로그 스케일 Exposure 슬라이더(0.1~10)
- 선형 색공간: 파일 텍스처를 `R8G8B8A8_UNORM_SRGB`로 생성해 GPU가 자동 디코드, Material/Light/배경 색 상수는 `Renderer::SrgbToLinear`(2.2제곱)로 업로드 시 변환, 톤 매핑 셰이더 마지막에서 `1/2.2`제곱으로 출력 인코드
- 전원을 켰을 때 Emissive 3/5/8 네온의 밝기 차이가 화면에서 구분되고, 창 크기를 바꿔도 화면이 유지되는 것을 확인

### Bloom

- 타깃 세 개를 화면의 **절반 크기** `R16G16B16A16_FLOAT`로 추가했다(`m_bloomBrightTargetHandle` / `m_bloomBlurXTargetHandle` / `m_bloomBlurYTargetHandle`). HDR 타깃과 함께 `Renderer::Resize`에서 재생성하며, 절반 크기 계산은 `std::max(1, ...)`로 0을 막는다
- `brightPassPixelShader.hlsl`: HDR 씬을 샘플링해 `exposure`를 곱한 뒤 가중 휘도 `dot(color, float3(0.2126, 0.7152, 0.0722))`가 `threshold` 미만이면 0, 넘으면 원본 색을 그대로 통과시킨다(하드 컷)
- `blurXPixelShader.hlsl` / `blurYPixelShader.hlsl`: 5탭 가중치 `{ 0.0545, 0.2442, 0.4026, 0.2442, 0.0545 }`(σ=1 가우시안의 정규화 샘플, 합 1)로 가로·세로를 따로 블러한다. 샘플 간격은 상수 버퍼의 `dx`/`dy`(= 1 / 타깃 크기)
- 블러는 `m_bloomBlurIterations`만큼 반복한다. 첫 회차만 Bright에서 읽고 이후에는 직전 BlurY 결과에서 읽어 누적시킨다(BlurX/BlurY 타깃이 핑퐁, 결과는 항상 BlurY)
- 가장자리 처리를 위한 CLAMP 샘플러(`m_clampSamplerState`)를 추가했다. 블러 패스에서만 쓰고 다른 패스는 기존 WRAP 샘플러를 다시 건다
- 합성은 별도 패스 없이 `toneMappingPixelShader.hlsl`이 `t1`로 Blur 텍스처를 받아 `hdr * exposure + bloom * bloomStrength` → 톤 매핑 → `1/2.2` 감마 순서로 처리한다
- 디버그 뷰(`PostProcessDebugView`: Final / Bright / BlurX / Blur)는 GPU로 보내지 않는 CPU 전용 값이다. `EndScene`이 마지막 패스의 PS와 `t0`만 골라 바꾸고, 디버그 뷰에서는 `copyPixelShader.hlsl`이 가공 없이 복사한다
- EditorUI "Post Process" 패널의 Bloom 그룹: Threshold(0~5), Strength(0~2), Blur Iterations(1~10), Debug View
- 확인: Bright 뷰에 네온과 스위치 Rim만 남고, Blur X 뷰는 가로로만 늘어나며, Blur 뷰는 사방으로 번진다. Strength 0이면 합성 전 화면과 같고, 반복을 1→4로 올리면 번짐 폭이 약 2배가 된다

### Skybox와 HDRI 큐브맵 (10-3, `26a062a`·`81d104c`·`e805963`)

- **로드 경로**: `AppBase::InitGreyBoxScene` → `AssetManager::LoadCubeMap(경로)`(경로 키 별도 캐시 `m_cubeMaps`) → `Texture::InitializeCubeMap` → `GraphicsResourceManager::CreateCubeMapTexture`(DirectXTK `CreateDDSTextureFromFileEx`, `D3D11_RESOURCE_MISC_TEXTURECUBE`, `DDS_LOADER_DEFAULT`, 실패 시 HRESULT 로그) → `TextureHandle`. 실패하면 로그만 남기고 계속한다(`m_environmentMap = nullptr`)
- **프레임 경로**: `AppBase::Render`가 `FrameRenderData.environmentMap`에 핸들(없으면 무효 핸들)을 넣고, `Renderer::BeginFrame`이 `m_environmentMapHandle`로 복사한다. `Renderer`는 `AssetManager`·`Texture`의 출처를 모른다
- **그리기**: `Renderer::EndScene`의 검증 직후 `DrawSkybox()` — 핸들이 무효면 아무것도 안 함(배경색 Clear가 하늘). 유효하면 카메라 버퍼·환경맵 SRV·WRAP 샘플러를 `SkyBox::Render`에 넘기고, 끝나면 깊이 상태를 `m_depthStencilState`로 복구한다
- **`SkyBox` 클래스** (`e805963`, `Renderer` 멤버 `m_skyBox`): `Initialize(device, resourceManager, inputElements)`가 스카이박스 VS + 자기 Input Layout(씬과 같은 `inputElements`), PS, 깊이 상태(LESS_EQUAL + 쓰기 ZERO), `GeometryGenerator::MakeCube` 버퍼를 만든다. `Render(context, cameraBuffer, environmentSRV, sampler)`가 IA·VS `b1`·PS `t0`·`s0`·깊이 상태를 걸고 `DrawIndexed` → `t0` 해제. 깊이 상태는 복구하지 않는다
- **셰이더**: `skyboxVertexShader.hlsl`(`Common.hlsli`) — `mul(float4(pos, 0), view)`로 이동 제거 → `projection` → **`.xyww`**(깊이 1), 방향 = 큐브 로컬 위치. `skyboxPixelShader.hlsl`(`Lighting.hlsli`) — `TextureCube t0`를 방향으로 `Sample` × `environmentIntensity`
- **`environmentIntensity`**: `DirectionalLight::environmentIntensity`(기본 **0.25**) → Light cbuffer의 `pointLightCount` 다음 자리(`float environmentIntensity; float2 lightPad;`, C++ `LightConstantData`도 같은 순서) → 스카이박스 PS. EditorUI Environment 패널에 로그 슬라이더(0.01~4). 하늘·10-4 반사 밝기 배율이며 π를 곱하지 않는다(radiance 배율)
- **에셋**: `Assets/Textures/HDRI/hansaplatz_cube512.dds` — Poly Haven Hansaplatz(CC0, 밤 광장) 2K equirect를 에이전트의 Node.js 스크립트로 변환: `R16G16B16A16_FLOAT`, 면 512² × 6, 밉 10단계(2 × 2 상자 필터), 16MB, D3D 면 순서·방향(equirect 가운데 = +Z). 상세는 `HDRI/CREDITS.md`. 원본 `.hdr`는 `HDRI/.gitignore`로 제외(§2의 추적 해제 메모 참고)
- 확인: 골목 위로 어두운 하늘 띠가 보이고 물체가 하늘을 가린다. `Environment Intensity` 0이면 하늘이 검정. `SkyBox` 분리 후 화면 동일(사용자 확인)

### 큐브맵 환경 반사 (10-4, `b6aee16`·`ecdb160`)

- **바인딩**: `Renderer::DrawRenderItem`이 `m_environmentMapHandle`이 유효할 때만 `GetSRV`로 환경맵 SRV를 얻어 PS `t4`에 건다(무효면 `nullptr`). `pixelSRVs[5]`, `PSSetShaderResources(0, 5, …)`. 다른 네 슬롯과 달리 `ResolveTextureSRV`·`return false`를 쓰지 않는다(§4)
- **HLSL 선언**: `simplePixelShader.hlsl`의 `TextureCube environmentMap : register(t4)`(스카이박스 PS의 t0과 같은 이름)
- **`Lighting.hlsli` 함수** (`ComputeDirectLighting` 위, 선언 순서가 호출 순서):
  - `DielectricSpecularToF0(specular)` = `0.08 × specular` — 직접광·환경광이 함께 쓴다
  - `ComputeEnvironmentLOD(roughness, mipCount)` = `roughness × (float(mipCount) − 1)` — 선형 대응
  - `SampleEnvironmentRadiance(cubeMap, cubeSampler, direction, roughness)` — **빛 쪽**. `GetDimensions`로 밉 수 → LOD → `SampleLevel(…).rgb × environmentIntensity`
  - `EnvBRDFApprox(f0, roughness, NdotV)` — Karis 2014(Unreal `BRDF.ush`) 그대로. `F0 × A + B`
  - `ComputeEnvironmentSpecular(surface, viewDir, environmentRadiance)` — **표면 쪽**. N·V(`saturate(abs(…) + 1e-5)`), F0, `EnvBRDFApprox` → `radiance × F`
- **`main`** ("3. 간접광"): `reflectDir = reflect(−viewDir, surfaceData.normal)` → `environmentRadiance` → `environmentSpecular`, `finalColor = ambientColor + environmentSpecular + directLighting + emissive + rim`. ambient는 그대로다(`(1 − F)` 감쇠는 10-5)
- **수치** (EnvBRDFApprox, 바닥 F0 0.02, 먼 바닥 N·V 0.1): 웅덩이(roughness 0.1) 0.51 / 마른 아스팔트(약 0.4) 0.24 / 0.6에서 0.08 / 벽(1.0) 0.007. 정면은 모두 약 F0. Schlick은 각도만 봐서 전부 0.60이었다
- **LOD 수치** (512 큐브, 밉 10): 웅덩이 0.9(약 256px) / 마른 바닥 약 3.2(64px) / 벽 7~9
- 확인: E-1b 크롬 골목(임시 출력, 사용자 확인 후 삭제), E-2 웅덩이 반사와 벽 스침 번들거림, E-3 `fxc` 경고 0개. 2026-10-28 에이전트 캡처(사용자 요청, 빌드 없이 실행)의 평가는 §4 "환경 반사 — split-sum과 EnvBRDFApprox"

### 룩 재튜닝 (밤 골목 조명 세팅)

기능 검증용 임시값을 전부 버리고 사이버펑크 밤 골목 기준으로 다시 잡았다(`050a1d9`). 확정값은 아래가 전부이며 ImGui 없이도 그대로 재현된다.

| 대상 | 값 | 위치 |
|---|---|---|
| 하늘광 방향 | `(0, -0.932, 0.362)` 단위 벡터 | `DirectionalLight.h` |
| 하늘광 색/세기 | `(0.45, 0.60, 1.0)` / `0.18 × π` (10-2' π 규약) | 〃 |
| Ambient | `0.045` | 〃 |
| Exposure / Threshold | `2.0` / `1.2` | `PostProcessSettings.h` |
| Bloom Strength / Iterations | `0.8` / `5` | 〃 |
| Tone Mapper | ACES | 〃 |
| 배경(하늘) 색 | `(0.020, 0.028, 0.055)` | `AppBase` 생성자 |
| 환경 보조광 | `(0, 3.2, 10)`, 색 `(0.28, 0.48, 0.85)`, range `24`, intensity `0.55 × π` | `AppBase::InitGreyBoxScene` |

네온 세 개(`NeonSignDesc`). Point 세기는 10-2' 이후 코드에서 **표의 값 × π**다(π 규약, 화면은 같음). 아래 휘도 환산은 전환 전 값 기준이다:

| | color | Emissive | Point 세기 | Point range |
|---|---|---:|---:|---:|
| Pink | `(1.0, 0.08, 0.45)` | 18.0 | 1.4 | 6.0 |
| Cyan | `(0.10, 0.85, 1.00)` | 7.0 | 0.7 | 6.5 |
| Orange | `(1.0, 0.20, 0.05)` | 15.0 | 1.4 | 6.5 |

전원 스위치(`PowerSwitch` 생성자):

| 상태 | BaseColor | Emissive | 세기 |
|---|---|---|---:|
| Off | `(0.30, 0.12, 0.10)` | `(0.90, 0.12, 0.08)` | 0.20 |
| On | `(0.22, 0.85, 0.55)` | `(0.25, 1.00, 0.55)` | 0.30 |

Rim 색은 `(1.0, 0.75, 0.25)` 호박색, Power 4.0, 강조 시 Intensity 3.0이다.

- `EditorUI`의 Ambient 슬라이더를 로그 스케일(`0.001~1.0`, `%.3f`)로 바꿨다. 쓸모 있는 구간이 0.01~0.1인데 선형 `0~1`로는 조절 자체가 불가능했다.
- 확인: 전원 Off에서 차가운 청색 골목과 끝벽 빨간 대기 표시등이 유도등처럼 보이고, On에서 네온 심지가 번지며 벽이 은은하게 물든다. 세 번 빌드·실행하며 ① 벽이 과하게 물드는 문제(Point Light 세기 절반으로) ② 표시등이 평평하게 날아가는 문제(Emissive 0.5 → 0.20)를 고쳤다. 144 FPS 유지.

### 텍스처 색공간 선택과 밉맵 (로드맵 10번 선행 조건, 전부 완료)

`ddb29c7`(경로), `0d0eac2`(캐시 키), `f91b0e6`(밉맵)으로 커밋됐다. 호출자가 **용도**를 말하면 색공간이 따라 정해지는 경로를 뚫었다.

- `TextureType.h` — `enum class TextureType { Albedo, NormalMap, Data }`. GPU로 가지 않는 CPU 전용 값이라 숫자를 명시하지 않았다(`PostProcessDebugView`와 같은 부류, `ToneMapper`와는 반대)
- 네 계층에 정보가 흐른다. **의미는 위, 표현은 아래**이고 `Texture.cpp`가 경계다
  ```
  AppBase / ModelLoader   TextureType   (의미)
  AssetManager            TextureType   (전달)
  ──────────────────────────────────────────────
  Texture.cpp             ToDxgiFormat  ← 여기서 변환
  ──────────────────────────────────────────────
  GraphicsResourceManager DXGI_FORMAT   (표현)
  D3D11Utils              DXGI_FORMAT
  ```
- `Texture.cpp`의 파일 지역 `static ToDxgiFormat(TextureType)`이 유일한 변환 지점이다. `Albedo` → `_UNORM_SRGB`, `NormalMap`/`Data` → `_UNORM`. `switch`에 `default:`를 두지 않고 뒤에 `DXGI_FORMAT_UNKNOWN` 반환을 둬서, 값이 늘면 드러나고 잘못된 값은 `CreateTexture2D`가 실패시킨다
- 호출부 5곳(AppBase 4 + 모델 로더 1)이 전부 `TextureType::Albedo`를 명시한다. **기본 인자를 두지 않아** 새 텍스처를 추가할 때 색공간을 빠뜨릴 수 없다
- 확인: `DXGI_FORMAT` 리터럴이 `Texture.cpp`의 변환 함수와 `GraphicsDevice.cpp`의 스왑체인에만 남았다. 전체 리빌드 통과, 화면 변화 없음(순수 리팩터링)

- **캐시 키** (`0d0eac2`): `AssetManager`의 텍스처 캐시가 `TextureKey = std::pair<std::string, TextureType>`를 키로 쓴다. 같은 파일을 다른 용도로 부르면 **별개 리소스**가 된다. 해시는 `AssetManager` 안의 private 함자 `TextureKeyHash`가 `hash_combine` 관용구로 두 해시를 섞어 만든다. `LoadTexture`가 키를 지역 변수로 한 번 만들어 `find`와 `emplace`에 같이 넘긴다. Mesh/Material/Model 캐시는 여전히 이름 하나가 키다
- 확인: 임시 테스트로 **같은 경로 + 같은 용도 → 같은 포인터**(캐시 유지)와 **같은 경로 + 다른 용도 → 다른 포인터**(용도 분리)를 둘 다 확인한 뒤 테스트 코드를 제거했다
- **데이터 텍스처 실제 확인** (Step 2, 코드 변경 없음): 같은 벽 텍스처를 `Data`로 읽어 오른쪽 벽에만 붙이고 전원 Off(좌우 조명 동일)로 비교해 **`Data` 쪽이 뿌옇게 밝게** 나오는 것을 확인했다. 원본 $v$가 `Albedo`에선 화면에 $v$로, `Data`에선 $v^{1/2.2}$로 나가서 어두운 곳일수록 많이 뜬다
- **밉맵** (`f91b0e6`): 모든 파일 텍스처가 1×1까지 전체 밉 체인을 갖는다. `D3D11Utils::CreateTexture`가 `ID3D11DeviceContext`를 받아(`GraphicsResourceManager`가 내부에서 넘김) ① `USAGE_DEFAULT`, `BIND_SHADER_RESOURCE | BIND_RENDER_TARGET`, `MISC_GENERATE_MIPS`, `MipLevels = 0`으로 **빈 텍스처를 만들고** ② 생성 결과를 검사한 뒤 `UpdateSubresource`로 **레벨 0만 채우고** ③ SRV를 만든 뒤 `GenerateMips`로 나머지를 GPU가 만든다. 샘플러는 원래 `MIN_MAG_MIP_LINEAR`/`MaxLOD = FLOAT32_MAX`였다
- 확인: 골목 입구에서 먼 바닥을 본 같은 위치 스크린샷으로 비교 — **중·원거리 바닥의 날카로운 흰 점이 사라지고 부드러운 회청색이 됐고, 밝기는 유지됐다**(선형 평균). 가까운 바닥은 그대로다. RenderDoc Texture Viewer에서 밉 레벨들이 생성되고 높은 레벨이 원본보다 어둡지 않음을 확인했다
- 비용: 파일 텍스처 VRAM이 $1 + \tfrac14 + \tfrac1{16} + \cdots = \tfrac43$배, 로딩 때 밉 생성 한 번

### Greybox 골목과 상호작용

- `GeometryGenerator::MakeCube()` 정점 범위를 -0.5 ~ +0.5로 바꿔 기본 크기 1m 통일
- 폭 4m, 높이 4m, 길이 20m Greybox 골목
- 바닥과 좌우/끝 벽이 공용 Cube Mesh를 공유한다. Material은 `wallMat`(좌/우벽)·`endWallMat`(끝벽)·`floorMat`(바닥)으로 나뉜다. 바닥은 ambientCG `Asphalt023S`(W-2), 벽은 ambientCG `Bricks096`(N-5)이고, `Assets/Textures/CyberpunkAlley/` PNG 4장(2026-09-15) 중 지금 읽는 것은 네온 디퓨저·스위치 패널 두 장뿐이다
- 끝 벽의 전원 스위치 GameObject와 독립 Material
- 상호작용 거리 2.0m 이내의 Ray-BoundingBox 교차 판정(`PowerSwitch::CanInteract`, `DirectX::SimpleMath::Ray`/`BoundingBox::Intersects` 사용)
- `WasKeyPressed('E')`로 단발 상호작용 입력 처리
- `PowerSwitch`가 Scene 소유 GameObject를 비소유 참조하는 컴포지션 구조
- 전원 On/Off 토글과 스위치 Material의 빨강/초록 시각 변화
- 상호작용 가능 여부 강조를 `PowerSwitch`가 소유한다. `SetHighlighted(bool)`가 값이 바뀔 때만 `ApplyVisualState`를 호출하고, 거기서 전원 상태(BaseColor)와 강조 상태(Rim Intensity 2.5 / 0)를 함께 반영한다. Rim 색(0.3, 0.9, 1.0)과 Power(4)는 `Initialize`에서 한 번만 설정한다. `AppBase::Update`는 `CanInteract`를 프레임당 한 번 계산해 강조와 E키에 함께 쓰고, Play HUD는 `IsHighlighted()`로 같은 판정을 읽는다. `ExitPlayMode`가 강조를 끄고, `GetGameObject()`는 제거했다
- `PointLightSequence`가 1.4초 간격으로 등록된 Entry 순서대로 켜고 역순으로 끔
- 점등 중 E키 재입력 시 현재 개수에서 목표 방향 전환
- `SequenceEntry`가 Point Light GameObject와 비소유 Emissive Material 포인터를 묶음
- 독립 Material과 다른 Emissive 색을 가진 네온 세 개(Pink/Cyan/Orange)의 순차 On/Off 실행 확인
- `NeonSignFactory::Create`가 네온 1개(Material + 발광 GameObject + Point Light GameObject + 시퀀스 등록)를 `NeonSignDesc` 하나로 생성하는 절차를 통합해, `InitGreyBoxScene`의 반복 코드를 제거함(Pink/Cyan/Orange 3개 모두 적용 완료, 오브젝트/Material 이름도 `"PinkNeon_Light"`/`"PinkNeon_Glow"`처럼 의미 있는 이름으로 정리됨)
- 네온 전용 민무늬 텍스처(`neonFlat.jpg`, 64x64 흰색 — 2026-09-15에 `neon_diffuser_albedo_v1.png`로 교체됨)를 추가해 네온 Material의 Albedo를 `wall.jpg`(벽돌 사진 텍스처)에서 분리함 — `surfaceColor = albedo * baseColor` 계산에서 벽돌 무늬가 섞여 Emissive로 포화되지 않는 채널(G/B)에 얼룩으로 비치던 문제를 해결
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

### 학습 사이트 (study-site, `a431f1f`)

렌더러 기능이 아니라 **자기 코드를 다시 공부하기 위한 정적 HTML 노트**다(2026-10-05).

- 위치: 저장소 루트 `study-site/`. `index.html` + 3페이지 — **Architecture**(소유·대여 구조), **Frame Analysis**(`Run` 루프 한 바퀴의 시간축), **Rendering Pipeline**(각 `Context->…Set…` 줄이 바꾸는 파이프라인 단계). 페이지마다 예측 질문 → 다이어그램 → 코드 흐름 → 함정 → 학습 질문 → "문서와 다른 점" 순서다.
- 코드 기준: `adee25f`. 그 뒤 렌더러 코드가 바뀌지 않았으므로 **현재 코드와 줄 번호가 일치한다.** 코드를 바꾸면 줄 번호부터 어긋나므로 기능 단계를 마칠 때 갱신한다.
- 여는 법: `fetch`를 쓰지 않는 정적 페이지라 `study-site/index.html`을 브라우저로 **바로 연다.** `.claude/launch.json`의 프리뷰 서버 항목은 다른 세션의 임시 폴더에 있던 서버 스크립트를 가리키다 그 파일이 사라져 2026-10-23에 지웠다.
- 생성·갱신 규칙: `.claude/skills/study-site/SKILL.md`(`/study-site`). 소스(`.cpp/.h/.hlsl/.hlsli`)는 수정하지 않고 `study-site/` 아래에서만 작업하며, 문서가 아니라 실제 코드를 기준으로 쓴다. Architecture의 A2·A4·A5 다이어그램은 아직 자리만 있다.
- 사이트가 코드와 대조해 찾은 **"문서와 다른 점" 6개는 2026-10-23 AGENTS.md에 반영했다**(§8 참고). 사이트 HTML에는 아직 미해결로 남아 있으니 다음 사이트 갱신 때 정리한다.

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

**상속/컴포넌트 재검토 (2026-09-15)**: "PowerSwitch도 GameObject의 한 종류이니 상속이나 컴포넌트 구조가 낫지 않나"를 다시 논의했다.

- 상속은 위 이유에 더해 "상호작용 + 조명 + 충돌" 같은 조합마다 클래스가 늘어나는 문제가 있어 채택하지 않는다. Unreal도 오브젝트 종류는 Actor 상속으로, 여러 종류가 공유하는 기능은 Component로 나눈다.
- 컴포넌트가 장기적으로 맞지만, 데모 시나리오의 상호작용 오브젝트가 스위치 하나뿐이고 "상호작용 결과(행동)"를 연결하는 설계(콜백 등)가 추가로 필요해 지금은 만들지 않는다.
- **전환 조건**: 두 번째 상호작용 오브젝트(문·단말기 등)가 필요해지면 `InteractableComponent`(범위·강조 값) + 가장 가까운 대상을 고르는 상호작용 처리 + 결과 콜백 구조로 옮긴다.
- 어느 구조든 "오브젝트의 모습은 오브젝트 쪽이 정하고, AppBase는 상호작용 가능 여부만 전달한다"는 경계는 같다. 그래서 현재 AppBase에 있는 Rim 강조는 먼저 `PowerSwitch::SetHighlighted`로 옮긴다(§6).

### NeonSignFactory

`InitGreyBoxScene`에 네온 1개당 생성 절차(Material 생성 → 발광 GameObject 배치 → Point Light GameObject 생성 → `AddSequenceEntry` 등록)가 Pink/Cyan/Orange 3번 그대로 반복되던 것을 제거하기 위해 도입했다.

- `NeonSignDesc`: 이름, 발광 메쉬 위치/크기, Point Light 위치/범위/세기, 공용 `color`(Emissive Color와 Point Light Color가 항상 같아야 하므로 필드 하나로 공유), Emissive Intensity를 담는 순수 입력 데이터. 런타임 상태 없음.
- `NeonSignFactory::Create`: `Scene`/`AssetManager`/`PointLightSequence`의 공개 API만 알고 `AppBase`/`Renderer`/`InputSystem`/ImGui는 모른다.
- 런타임 상태가 없어 인스턴스를 만드는 클래스 대신 **정적 함수**로 구현했다 — 생성 시점 배선만 책임지는 데 인스턴스는 불필요한 추상화라고 판단.
- `PointLightComponent`에 별도 역할(enum) 태그를 추가하지 않았다 — `PointLightSequence`에 등록됐는지 여부가 이미 "네온 연동 vs 환경 조명" 분류 그 자체이기 때문에, 지금 쓰지 않을 태그를 미리 만들지 않았다.
- Pink/Cyan/Orange 3개 네온 모두 이 팩토리로 교체 완료. `neonMat1`/`neonTestObject0`처럼 남아 있던 테스트용 이름은 `desc.name` 기반 이름(`"PinkNeon_Mat"`, `"PinkNeon_Light"`, `"PinkNeon_Glow"` 등)으로 자동 정리됐다.
- `Create` 내부에서 `BaseColor`를 `desc.color * 0.12f`로 계산해, 꺼진 상태에서도 각 네온 고유의 어두운 색조가 비치게 했다(모두 동일한 회색이던 것을 개선).
- Albedo에 `wall.jpg`(벽돌 텍스처) 대신 전용 `neonFlat.jpg`(흰색 민무늬, 2026-09-15부터 `neon_diffuser_albedo_v1.png`)를 쓴다 — 네온은 발광이 핵심이라 벽 재질과 텍스처를 공유하면 안 된다는 게 이번에 확인된 설계 원칙이다.

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
- **(2026-09-29 변경)** 위 슬롯 배치는 더 이상 유효하지 않다. 카메라 cbuffer를 `Common.hlsli` 한 벌로 합치면서 **VS·PS 모두 b1=Camera, PS b2=Material**로 바꿨다. 아래 "셰이더 구조 — 빛 쪽과 표면 쪽 분리" 참고.

### 셰이더 구조 — 빛 쪽과 표면 쪽 분리 (로드맵 10-1 선행, 2026-09-29)

Specular를 넣기 전에 셰이더를 정리했다. diffuse 식이 방향광(`main` 인라인)과 점광원(`ComputePointLight`) 두 곳에 있어서, 그대로 Specular를 넣으면 같은 식을 두 번(스포트라이트가 생기면 세 번) 써야 했기 때문이다. 강의 Ch13이 조명 계산을 `Common`에서 분리한 것과 같은 방향이다.

- **`Common.hlsli`에 넣는 기준은 "include하는 모든 셰이더가 쓰는가"다.** 정점 구조체와 카메라는 VS·PS·앞으로의 스카이박스가 모두 쓰지만, Material은 씬 PS만 쓰고 **셰이더마다 모양이 다르다**(스카이박스는 없음, 젖은 바닥은 필드가 더 붙음). Material cbuffer는 "이 셰이더와 C++ 사이의 계약"이라 그 셰이더 파일에 둔다. 강의 Ch13도 `GlobalConstants`(카메라+조명)만 `Common`에 두고 `MaterialConstants`는 `BasicPS.hlsl`에 있다. Unity URP도 `UnityPerMaterial`은 셰이더별 `LitInput.hlsl`에 있다. **전환 조건**: 같은 Material 레이아웃을 쓰는 두 번째 셰이더가 생기면 `Common`이 아니라 조명 받는 셰이더들만 include하는 `LitInput.hlsli`로 뺀다.
- **Light cbuffer도 `Common`이 아니라 `Lighting.hlsli`다.** VS가 쓰지 않는다. 그리고 `GetDirectionalLight`/`GetPointLight`가 cbuffer 값을 직접 읽으므로 **데이터와 그 데이터를 읽는 함수가 같은 파일**에 있어야 한다. PS가 cbuffer보다 `Lighting`을 먼저 include하므로, cbuffer가 PS에 남아 있으면 `Lighting` 안의 함수에서 미선언 이름이 된다.
- **슬롯 규칙: b1 = 모든 스테이지가 공유하는 프레임 데이터(카메라), b0 = 스테이지별 데이터.** 처음에는 VS를 b2로 옮기는 안을 냈으나, 사용자가 "b1로 두면 배열 하나로 한 번에 보낼 수 있지 않나"를 제안했고 비교해 보니 변경량이 비슷하면서 **두 스테이지 모두 빈 슬롯 없이 연속 바인딩**이 된다. 강의 Ch13의 `GlobalConstants : register(b1)`과도 같다. 후처리는 PS b0만 쓰므로 b1·b2 재배치의 영향이 없다(`Renderer.cpp`의 `*SetConstantBuffers` 네 곳 확인).
- **cbuffer 멤버 이름은 셰이더 전역이다.** C++ 구조체 멤버와 달리 cbuffer는 이름공간을 만들지 않는다(그래서 `view`를 접두사 없이 읽는다). 공유 include의 cbuffer 멤버는 include하는 모든 셰이더에서 이름을 차지하므로 고유하게 짓는다 — `cameraPad`, `lightPad`, `dirLightDirection`/`dirLightColor`/`dirLightIntensity`. 반면 `struct` 멤버(`PointLight.pad`)는 범위가 구조체 안이라 겹쳐도 된다. C++ 쪽은 이름이 아니라 **순서와 크기**로 매칭되므로 HLSL 이름만 바꿔도 된다.
- **`IncidentLight { direction, radiance }` — 빛과 BRDF 사이의 계약.** 빛 종류마다 다른 것은 "표면→빛 방향"과 "도착한 빛의 양" 둘뿐이고, 그 뒤 표면 계산은 같다. 빛 M종 × 표면 항 N개를 M+N으로 줄인다. 이름은 Three.js의 `IncidentLight`와 같고, `radiance`는 강의 Ch13 `Light.radiance`와 같다. 감쇠(나중엔 그림자)는 `radiance`에 미리 곱한다 — 표면 계산은 "왜 약해졌는지"를 알 필요가 없다.
- **방향 규약은 `IncidentLight.direction` = 표면 → 빛(L), 단위 벡터.** cbuffer의 방향광 `direction`은 빛이 나아가는 방향이라 부호 반전이 필요한데, 이 반전과 `normalize`를 `GetDirectionalLight()` **한 곳**에서만 한다. 그래서 AGENTS §7의 "direction 길이가 세기 배율이 되는" 함정이 사라졌다.
- **`GetDirectionalLight()`는 인자가 없고 `GetPointLight(index, posWorld)`는 위치를 받는다.** 시그니처 차이가 곧 물리 차이다(무한히 먼 광원 vs 위치에 따라 방향·감쇠가 바뀌는 광원). `PointLight` 구조체가 아니라 `index`를 받아 `pointLights[]` 접근이 `Lighting.hlsli` 안에만 있게 했다 — 저장 방식을 StructuredBuffer나 타일 목록으로 바꿔도 `main`은 그대로다. Unity URP `GetAdditionalLight(uint i, float3 positionWS)`와 같은 시그니처다.
- **조기 반환에도 `direction`은 안전한 단위 벡터 `(0,1,0)`.** radiance가 0이어도 `NaN * 0 = NaN`이라, `direction`이 0벡터면 뒤에서 `normalize`나 `pow`를 거칠 때 NaN 픽셀이 생기고 Bloom이 번지게 한다. 결과 변수를 **맨 앞에서 안전값으로 초기화**하고, 계산은 지역 변수(`toLight`)로 하며, **모든 검사를 통과한 뒤에만** 결과 필드를 채운다.
- **표면 쪽은 `SurfaceData { normal, albedo, roughness, specular }`로 묶는다.** 필드는 쓰일 때 추가했다(D에서 `normal`·`albedo`, E-2에서 `roughness`·`specular`). 필드가 늘어도 `ComputeDirectLighting`의 시그니처와 호출부가 바뀌지 않는다. 필드 이름을 `baseColor`가 아니라 `albedo`로 하는 이유는 전역 Material 멤버 `baseColor`(배율)와 뜻이 다르기 때문이다. `roughness`/`specular`는 지금 cbuffer 값과 뜻이 같아 같은 이름을 쓴다 — roughness 맵이 들어오면 cbuffer 쪽을 `roughnessFactor`(강의 Ch13, glTF)로 바꾼다. 이름은 Unity URP `SurfaceData`.
- **시선(`viewDir`)은 `SurfaceData`가 아니라 매개변수다.** 표면의 속성이 아니라 카메라에 따라 바뀌는 값이기 때문이다(Unity URP도 `InputData`에 둔다). 방향은 L과 같이 **표면 → 바깥**이라 `H = normalize(L + V)`에 부호 반전이 없다.
- **ambient는 `ComputeDirectLighting` 밖에 둔다.** 방향이 없는 간접광이라 `IncidentLight`로 표현할 수 없고, 빛마다 부르는 함수에 넣으면 빛 개수만큼 더해진다(강의 Ch10 `Common`의 버릇). IBL이 오면 이 한 줄이 `ComputeIndirectLighting`으로 바뀐다.
- **"화면이 바뀌면 안 되는" 단계는 컴파일된 어셈블리로도 검증했다.** D-2 전후를 `fxc /Fc`로 비교해 명령어 수가 134로 같고 덧셈 순서만 다름을 확인했다. 셰이더는 입력이 같으면 출력이 정해지는 순수 함수라 이 방법이 잘 통한다.

### Specular — 정규화된 Blinn-Phong과 재질값 (로드맵 10-1, 2026-09-30)

> **10-2'(2026-10-27)에서 Blinn-Phong은 GGX로 교체됐고, 아래의 "π 없는 규약"도 정석(diffuse = albedo / π)으로 바뀌었다.** 이 절은 당시 결정의 기록이다. 재질값(roughness·specular)과 "BRDF 입구에서 roughness를 막는다"는 그대로 유효하다. 현재 구조는 §4 "GGX + Smith와 π 규약 전환".

- **Roughness는 지각적 값으로 저장하고 BRDF에서 `α = roughness²`로 바꾼다.** 제곱하지 않으면 슬라이더 절반 이상이 "거의 무광"에 몰린다. `n = 2/α² − 2`는 Blinn-Phong을 GGX와 같은 roughness 입력으로 움직이게 하는 대응식이라, 나중에 GGX로 바꿔도 Material 값을 다시 튜닝하지 않는다. roughness 1이면 n = 0이 되어 표면 전체가 번쩍이므로 `n ≥ 1` 하한을 둔다. 0이면 n이 무한대라 `Material::SetRoughness`가 0.05에서 clamp한다.
- **분포 항을 `SpecularBlinnPhong`으로 뗐다.** 정규화(F)는 이 함수 안만 고쳤고, GGX로 바꿀 때도 이 함수만 교체한다(Unreal `BRDF.ush`의 `D_Blinn`). F0는 "양"이라 함수 밖에서 곱한다.
- **정규화 계수는 `(n + 8) / (8π)`가 아니라 `(n + 8) / 8`이다.** 교과서(Real-Time Rendering)는 diffuse를 `albedo / π`로 두는 규약이다. 우리 diffuse는 처음부터 `albedo`(π 없음)이고 조명 세기도 그 기준으로 튜닝했다. specular만 π로 나누면 diffuse 대비 3.14배 어두워진다. 두 항에 같은 π를 곱한 규약(Unreal·Filament도 조명 세기에 π를 흡수)으로 `(n+8)/8`을 쓴다. **교과서 식을 보고 `/ PI`를 다시 넣지 말 것.** `Common.hlsli`의 `PI`는 IBL(적분)에서 쓰게 된다.
- **E(모양)와 F(정규화)를 나눠서 에너지 손실을 눈으로 확인했다.** 정규화 전에는 roughness 0.2에서 하이라이트가 "작아지면서 어두워졌다". 같은 에너지가 좁은 곳에 모이면 물리적으로는 밝아져야 한다. 정규화 후 roughness 0.2의 최고값은 `0.04 × 157 ≈ 6.3`으로 HDR에서 Bloom 임계값을 넘는다 — 젖은 바닥 룩의 핵심이다.
- **NaN 방지는 두 겹이다.** `NdotL ≤ 0` 조기 반환은 빛이 닿지 않는 픽셀을 건너뛰는 용도이고, `dot(L+V, L+V)` 검사는 `L = −V`일 때 `normalize(0)`을 막는다. 처음 계획은 N·L 검사만으로 `L = −V`가 원천 차단된다고 봤으나, `CullMode`가 `NONE`이라 **뒷면을 볼 때(N·V < 0)** N·L > 0이면서 `L = −V`가 될 수 있어 사용자가 넣은 두 번째 검사가 필요하다. 제곱 길이 비교로 `sqrt`를 아낀다.
- **디버그 뷰로 specular 항만 ×20 출력해 모양·움직임·roughness 반응을 확인했다.** 합쳐진 최종 화면으로는 어느 항이 틀렸는지 구분할 수 없다. 리뷰에서 "지수 n을 그대로 반환"과 "계산한 specular를 결과에 안 더함"이 **서로를 가려** 화면에 드러나지 않는 사례가 있었다. 확인용 임시 코드는 확인 후 되돌렸다. 항이 더 늘면(Fresnel, IBL) ImGui 드롭다운 + cbuffer 플래그로 "Final / Diffuse / Specular"를 고르는 디버그 뷰를 검토한다.
- **재질값: 젖은 표면은 "더 어둡고 더 매끈하게".** Lagarde가 정리하고 Unreal·Frostbite가 쓰는 모델이다. 물이 다공성 표면 틈을 채우면 흡수가 늘어 albedo가 어두워지고, 물막이 roughness를 낮추며, 반사율은 물의 F0(약 2%)를 따른다. 에이전트가 직접 빌드·실행하며 세 번 비교했다.
  - v1 (바닥 BaseColor 0.6 / roughness 0.22 / specular 0.25): 반사가 **동그란 점**이고 바닥이 너무 어두웠다. BaseColor는 sRGB라 0.6이 선형 **0.32**배다.
  - v2 (0.75 / 0.3 / 0.25): 반사가 **보는 쪽으로 길게 늘어진 줄무늬**가 됐다. roughness가 너무 낮으면 로브가 좁아 비스듬한 시선에서의 늘어남이 보이지 않는다. 분홍 반사가 흐렸다.
  - **v3 (0.75 / 0.3 / 0.5) 확정.** 물리값 0.25가 아니라 0.5로 둔 이유는 **Fresnel이 없어서**다. 실제로는 비스듬한 시선에서 반사율이 크게 늘어나는데(젖은 거리가 멀리서 거울처럼 보이는 이유) 그 증가분이 빠져 있다. **10-2 Fresnel을 넣은 뒤 0.25로 되돌려 비교한다**(코드 주석에도 적어 둠). → 2026-10-23 Fresnel 뒤 0.25로 확정했다(아래 "Fresnel" 절).
  - 바닥 roughness를 0.2 아래로 내리지 않는다. n이 수천 단위가 되면 하이라이트가 픽셀보다 좁아져 카메라가 움직일 때 깜빡이고(스페큘러 에일리어싱), Bloom이 그것을 증폭한다.
  - 네온 roughness/specular는 `NeonSignDesc` 필드가 아니라 팩토리 안의 상수다. 모든 네온이 같은 유리관이고, 네온마다 다른 값이 필요해지면 그때 `desc`로 올린다.

### Fresnel — Schlick 근사 (로드맵 10-2, 2026-10-23)

- **Schlick 근사를 쓴다.** `F = F0 + (1 − F0)(1 − cosθ)⁵`. Unreal·Unity·Filament·강의 모두 직접광에 쓰고, 정확한 Fresnel 방정식과의 차이는 1% 안팎이다. 5제곱 때문에 60°까지는 거의 F0 그대로이고 75°를 넘으며 급격히 커진다(F0 = 0.04: 75° → 0.255, 85° → 0.648). 1인칭으로 앞을 보는 데모에서는 몇 미터 앞 바닥이 바로 이 구간이라 효과가 크다.
- **직접광의 cosθ는 V·H다.** L을 V로 반사하는 건 법선이 H인 미세면이고, Fresnel은 그 미세면에 대한 입사각으로 정해진다. IBL·평면 반사처럼 특정 H가 없는 경우는 N·V를 쓴다. 그래서 인자 이름을 `VdotH`가 아니라 일반형 **`cosTheta`**로 두고, 무엇을 넘길지는 함수 위 주석과 호출부가 책임진다(강의 Ch13 `SchlickFresnel(F0, NdotH)`는 이름과 달리 V·H·N·V를 받아 혼란스럽다).
- **diffuse에 `1 − F`를 곱한다.** 표면에서 튕긴 몫은 안으로 들어가 diffuse가 되지 못한다. 없으면 스치는 각에서 받은 빛보다 많이 내보낸다. 먼 바닥은 diffuse가 줄고 그 자리를 반사가 채워 "어두운 거울"처럼 보인다.
- **`specularTerm`은 `float`, `diffuseTerm`은 `float3`다.** 비금속 반사는 빛의 색 그대로라 F가 스칼라이고, diffuse는 표면 색이라 벡터다. 리뷰 중 `diffuseTerm`을 `float`로 받아 **R 채널만 남는 버그**(X3206 implicit truncation 경고)를 잡았다 — HLSL은 벡터 → 스칼라 대입을 에러가 아니라 경고로 처리한다(AGENTS §7).
- **5제곱은 곱셈으로 풀어 써도 `pow`와 같다.** 단독 테스트 셰이더를 `fxc /Fc`로 확인하니 컴파일러가 `x² → x⁴ → x⁵` 곱셈 3번과 `mad` 하나로 줄였다. `Pow5` 공용 함수는 성능이 아니라 재사용·가독성 문제라 예약 목록(§5)에 둔다.
- **확장은 예약만 했다.** `f90` 인자(IBL diffuse·specular 0 재질), `float3`화(metallic), `Pow5`는 §5 "확장 예약 목록"에 트리거와 함께 있다. 지금 식 `f0 + (1 − f0) × w`는 `1`을 `f90`으로 바꾸기만 하면 되는 모양이라 확장에 열려 있다.
- **바닥 specular를 0.25로 확정했다.** F0 차이는 정면에서만 의미가 있고(0° 0.04 vs 0.02), 스치는 각에서는 둘 다 1로 수렴한다(85° 0.648 vs 0.641). 그래서 물리값으로 되돌려도 먼 거울 반사는 유지되고 발밑만 차분해진다. 기능이 들어온 뒤에도 남은 **임시 보정은 이중 보정**이 되므로 걷어낸다.

### GGX + Smith와 π 규약 전환 (10-2', 2026-10-27)

- **GGX로 바꾼 이유**: ① 하이라이트가 "밝은 중심 + 긴 꼬리"가 되어 젖은 아스팔트의 번지는 반사와 실측 재질에 맞는다(W-3의 매끈한 웅덩이가 가장 잘 보이는 무대) ② 10-5 IBL의 BRDF LUT가 GGX + Smith로 적분된 표라 직접광이 Blinn-Phong이면 직접광·간접광 하이라이트 모양이 어긋난다 ③ Unreal·Filament·glTF의 표준.
- **π 규약을 정석으로 바꿨다(diffuse = albedo / π).** 10-1 때 π 없는 규약은 그 시점에 맞았다(BRDF가 하나, 외부 데이터 없음). GGX(외부 식)와 HDRI(10-3, 물리 radiance 데이터)가 들어오기 직전이라 기준이 바뀌었다 — 모든 참고 자료(Unreal·Filament·논문·강의 D항)와 식이 같아져 "π를 빼야 한다"는 함정이 사라지고, IBL에서 irradiance 맵의 π 포함 여부를 따질 필요가 없어진다. 두 규약은 **직접광 세기를 ×π 하면 화면이 같다**("BRDF가 π를 들고 있느냐, 빛 세기가 들고 있느냐"의 차이). 엔진들이 BRDF 쪽에 두는 이유는 빛 세기를 물리 단위(lux·candela)로 쓰기 위해서다.
- **규약 전환과 BRDF 교체를 한 스위치로 분리해 확인했다.** `USE_GGX = false`에서 "화면 그대로"가 π 전환의 증거, `true`에서 "꼬리가 생김"이 GGX의 증거다. 둘을 한 번에 바꾸면 밝기 차이의 원인을 가를 수 없다. 비교가 끝난 뒤 스위치와 `SpecularBlinnPhong`은 삭제했다(G-3) — 강의 코드의 `useXXXMap` 플래그처럼 남겨 두면 "어느 쪽이 진짜인지" 모르는 분기가 쌓인다.
- **Unreal 형식을 따르되 두 곳은 의도적으로 다르다.** ① `SpecularGGX`가 **F를 곱하지 않고 D·Vis만** 반환한다 — Unreal은 diffuse가 F를 쓰지 않아 안에서 곱하지만 우리는 `diffuse × (1 − F)`라 F가 밖에서도 필요하다(덕분에 A/B 동안 `SpecularBlinnPhong`과 반환 모양이 같아 삼항식 한 줄이었다) ② diffuse에 `(1 − F)`를 곱한다(UE4 DefaultLit은 곱하지 않음, 10-2 결정 유지). 구조를 따라 할 때도 "그 구조가 어떤 전제에서 나왔는가"를 먼저 본다.
- **G 대신 가시성 Vis = G / (4·N·L·N·V)를 쓴다(`Vis_SmithJointApprox`, Heitz 2014 근사).** 강의 Schlick-GGX(k = (r + 1)² / 8)와 `/ (4·NdotI·NdotO)` 나눗셈 형태를 먼저 작성했는데, N·V = 0에서 **0 / 0 = NaN**이 나왔다(Bloom이 퍼뜨림). Vis 형태는 분모가 N·V = 0이어도 `N·L × a > 0`으로 남아 **0 나누기 보호가 식 자체에서 사라진다** — 엔진 BRDF 코드에 `max(…, ε)`가 거의 없는 이유다. 보호는 입구 두 줄(roughness 최솟값, N·V `abs + ε`)뿐이다.
- **ε가 정상 값을 잡아먹은 사례**: 첫 `DistributionGGX`가 `a2 / max(d², 1e-7)`였는데, d²의 정상 최솟값은 (a2_min)² ≈ 3.9 × 10⁻¹¹이라 roughness < 0.13에서 하이라이트 중심이 10~2500배 깎였다(0.05에서 160000 → 62.5). 분모 d의 최솟값이 a2 > 0이라 보호 자체가 불필요했다. **ε를 고르기 전에 그 분모가 정상적으로 가질 수 있는 최솟값을 계산한다.**
- **N·V 조기 반환은 diffuse까지 끈다.** `if (NdotV ≤ ε) return 0`을 넣었다가 뺐다 — 노멀 맵으로 꺾인 법선은 낮은 시선에서 N·V < 0이 흔하고(젖은 골목을 멀리 보는 핵심 각도), diffuse는 그 조건에서 아무 문제가 없다. 조기 반환은 "이 조건에서 **모든** 출력이 0이 맞나?"를 먼저 묻는다.
- **N·V는 Unreal 그대로 `saturate(abs(N·V) + 1e-5)`.** Filament는 `max(N·V, 1e-4)`다. `abs`는 시선 반대로 꺾인 미세면도 앞면처럼 반사를 받는다(양면 재질에 유리). 이 장면에서 눈에 띄는 차이는 없었다.
- **웅덩이 roughness 0.15 → 0.1.** 0.15는 "Blinn-Phong 지수가 4천이라 하이라이트가 아직 보이는 가장 낮은 값"이었다. GGX는 중심이 좁아져도 꼬리가 남아 0.1(N·H = 1에서 D ≈ 3200)에서도 반사 줄이 보인다(§5 "웅덩이 값 재튜닝" 트리거 ② 처리). 근거가 바뀐 값은 주석도 바꾼다.

### Skybox와 환경맵 경로 (10-3, 2026-10-27)

- **HDRI는 밤 광장(Poly Haven Hansaplatz)으로 골랐다.** 밤 골목 위로 보이는 하늘이 거의 검정이어야 룩이 유지되고(위 면 평균 선형 휘도 0.01), 10-4에서 웅덩이에 비칠 **수평선 근처 도시 불빛**(최대 약 13,800)이 있어야 반사가 보인다. 낮·노을 HDRI는 하늘이 골목보다 밝아 룩을 깨뜨린다.
- **변환은 외부 도구 없이 직접 쓴 스크립트로 했다.** equirect → 큐브 6면(2 × 2 슈퍼샘플, 쌍선형) → `R16G16B16A16_FLOAT` DDS(DX10 헤더) + 상자 필터 밉. BC6H 압축과 GGX prefiltered 밉은 하지 않았다 — 압축은 16MB가 문제되지 않고, prefiltered 밉은 10-5 IBL을 할 때(cmft 등) 별도 맵으로 만든다. 10-4에서는 상자 필터 밉을 roughness로 골라 읽어 "거칠수록 흐림"만 근사한다.
- **큐브맵은 `TextureType`을 거치지 않는다.** float HDR이라 sRGB 선택지가 없고(`DDS_LOADER_DEFAULT`), DDS 헤더가 포맷을 정한다. 그래서 `LoadTexture`의 `(경로, TextureType)` 캐시와 섞지 않고 `LoadCubeMap`·`m_cubeMaps`를 따로 뒀다. 2D 텍스처의 DDS 쿠킹(§4 "텍스처 파이프라인 방향")을 할 때 `TextureType` → `FORCE_SRGB`/`IGNORE_SRGB` 규칙을 함께 설계한다.
- **환경맵은 `FrameRenderData`로 흐른다.** 하늘(10-3)과 씬 반사(10-4)가 같은 맵을 쓰는 **프레임 단위** 데이터이고, `Renderer`가 `AssetManager`를 알면 안 되기 때문이다(AGENTS §3). 무효 핸들 = "환경 없음"이라 Albedo와 달리 **앱을 끝내지 않는다** — 하늘·반사는 없어도 데모가 성립하는 선택 입력이다. 대가로 경로 오타가 조용히 배경색이 된다(S-3에서 실제로 `Cubemaps\\skybox\\` 경로로 발생). 눈에 띄는 경고는 §5 예약.
- **`.xyww` + LESS_EQUAL + 깊이 쓰기 없음, 불투명 물체 다음에 그린다.** 깊이를 1로 고정하면 하늘이 무한히 먼 곳에 있고, 불투명 물체가 이미 쓴 깊이에 걸러져 **가려진 픽셀은 PS를 돌지 않는다**(Early-Z). 먼저 그리면 화면 전체에 PS를 돌린 뒤 덮어쓴다. LESS면 Clear 값 1과 같아 전부 실패한다. 강의 Ch11 `CubeMapping`도 물체 다음에 그리지만 **반지름 50인 구**를 일반 깊이 테스트로 그린다 — 카메라가 50m를 벗어나거나 far 평면보다 크면 하늘이 잘리거나 물체를 덮는다. `.xyww`는 크기와 무관하게 항상 가장 먼 곳이라 이 문제가 없다. 강의 Ch13 `SkyboxVS`도 `w = 0`으로 이동을 지우는 데까지만 하고 `.xyww`는 쓰지 않는다 — 우리 VS의 이동 제거는 Ch13, 깊이 고정은 일반적인 스카이박스 기법이다.
- **`environmentIntensity`를 Light cbuffer에 넣었다.** 스카이박스 PS와 10-4 씬 PS가 같은 값을 읽어야 하고, Light cbuffer의 패딩 자리(`pointLightCount` 뒤 12바이트)가 비어 있어 크기가 늘지 않는다. CPU 쪽은 `DirectionalLight`에 들어가 있는데 **의미상 방향광이 아니라 하늘광**이다 — `ambientStrength`와 함께 `SkyLight`로 뺄지는 §5 예약.
- **그리기를 `SkyBox` 클래스로 분리했다(`e805963`, 강의 Ch11 `CubeMapping` 기준).** Ch11은 기능마다 자기 셰이더·State·메시를 소유하고 `Initialize`/`Render`를 갖는 클래스로 묶는다. 스카이박스 VS/PS·깊이 상태·큐브 버퍼는 `DrawSkybox` 말고 아무도 쓰지 않아 자르는 선이 깨끗했다. 강의와 다르게 한 곳:

  | 강의 `CubeMapping` | `SkyBox` | 이유 |
  |---|---|---|
  | `Initialize`에서 DDS를 직접 읽고 SRV를 `public`으로 노출 | 환경맵을 소유하지 않고 `Render` 때 SRV를 받는다 | 로드는 `AssetManager` 캐시를 거치고, 10-4 씬 PS와 공유하는 프레임 데이터다. 강의 `ExampleApp`이 `m_cubeMapping.m_specularResView`를 메시 그룹마다 복사해 넣는 것이 이 공유 문제의 흔적이다 |
  | 자기 상수 버퍼(`viewProj`) | 공용 카메라 CB(VS `b1`) | 같은 데이터를 두 벌 두지 않는다 |
  | `Mesh`의 ComPtr 버퍼 | `BufferHandle`(소유자는 `GraphicsResourceManager`) | AGENTS §4 소유 구조 |
  | 자기 샘플러 | `Renderer`의 WRAP 샘플러를 받는다 | 같은 설정의 샘플러를 하나 더 만들 이유가 없다 |
  | 자기 Input Layout | **같음** — 자기 IL(씬과 같은 `inputElements`) | `Renderer`의 IL에 기대지 않는다 |

  **"언제·무엇으로"는 `Renderer`, "어떻게"는 `SkyBox`**다. 깊이 상태 복구를 `Renderer::DrawSkybox`가 하는 것도 이 경계 때문이다(`SkyBox`가 씬 기본 상태를 알면 하위 기능이 호출자의 사정을 안다). 함께 검토했지만 하지 않은 묶음: Ch13 `GraphicsPSO`/`GraphicsCommon`(강의를 Ch11까지로 한정하기로 함 — §5 예약), Ch11 `PostProcess`(Bloom 묶음, `ImageFilter`와 같은 전환 조건), Material 텍스처 슬롯 enum(강의 `Mesh`도 이름 있는 필드 방식이라 보류).
- **주석만 남은 리뷰**: G-3 뒤 남은 문제가 전부 주석이었다(삭제된 함수를 가리키는 `MIN_PERCEPTUAL_ROUGHNESS` 설명, 값과 반대되는 웅덩이 근거). 주석은 아무도 검증하지 않아 코드가 바뀔 때 가장 먼저 낡는다 — "왜 이 값인가"는 **그 값을 고른 근거**로 적어야 근거가 바뀔 때 고칠 신호가 된다.

### 환경 반사 — split-sum과 EnvBRDFApprox (10-4, 2026-10-28)

- **구조는 Karis 2013 split-sum을 따른다.** 환경광 적분 `∫ f · L · (n·l)`을 ① "미리 흐린 하늘"(방향·roughness의 함수)과 ② "하늘이 흰색일 때의 반사율"(roughness·N·V의 함수)의 곱으로 근사한다. ①이 `SampleEnvironmentRadiance`, ②가 `ComputeEnvironmentSpecular`다. ②의 적분 안에 BRDF(π 포함)와 `n·l`이 이미 들어 있으므로 **환경광에는 `/π`와 `N·L`이 없다** — 직접광(`ComputeDirectLighting`)과 나란히 보고 "빠뜨렸다"며 추가하면 반사가 크게 어두워지거나 비스듬한 곳에서 사라진다.
- **빛 쪽 / 표면 쪽으로 나눈 이유는 앞으로 따로 바뀌기 때문이다.** E-3은 표면 쪽 한 줄(Fresnel 식)만, **10-7 평면 반사는 빛 쪽만**(바닥의 radiance를 큐브맵 대신 반사 텍스처에서 읽음 — 같은 Fresnel을 써야 웅덩이와 벽이 같은 재질로 보인다), 10-5는 빛 쪽(prefiltered 맵·LOD 곡선)과 diffuse 형제 함수를 바꾼다. 실제로 E-3은 `ComputeEnvironmentSpecular` 안의 한 줄로 끝났고 `main`은 그대로였다. `environmentIntensity`는 `GetDirectionalLight`가 세기를 곱하듯 빛 쪽에서 곱한다. 텍스처·샘플러를 인자로 받는 이유는 `Lighting.hlsli`를 스카이박스 PS(큐브맵 t0)도 include해서 `register(t4)`를 여기 선언하면 슬롯이 충돌하기 때문이다 — 슬롯은 PS 파일이 정한다. 인자 이름을 `cubeMap`으로 한 것은 씬 PS 전역 `environmentMap`과 헷갈리지 않게 하려는 것.
- **환경맵은 `t4`, 무효면 `nullptr`.** Material 슬롯(t0~t3) 뒤에 붙여 기존 번호를 지켰다. 빈 슬롯은 0으로 읽혀 "반사 없음"이 정의된 결과다. 반대로 1×1 검정 fallback은 **Texture2D**라 `TextureCube` 슬롯에 걸면 차원 불일치(정의되지 않은 결과)라 쓰지 않는다. 강의 Ch11 `BasicMeshGroup::Render`도 큐브맵 SRV를 재질 텍스처와 한 배열로 메시마다 건다.
- **roughness → 밉은 `SampleLevel`로 직접 고른다.** `Sample`은 화면 미분으로 밉을 고르는데, 반사 벡터는 노멀 맵 때문에 픽셀마다 크게 꺾여 미분이 과대해지고 흐린 밉이 들쭉날쭉 선택된다. 강의 Ch11 `BasicPS`가 `Sample` + `pow(…, shininess)`로도 괜찮은 건 예제가 매끈한 구라서다. 대응은 선형(`r × (mipCount − 1)`)이며, Unity `PerceptualRoughnessToMipmapLevel`(`r(1.7 − 0.7r)`)·Unreal `ComputeReflectionCaptureMipFromRoughness`(log)는 중간 roughness를 더 흐리게 둔다 — 그 엔진들은 GGX prefiltered 맵 전제라 우리 상자 필터 밉에 그대로 맞는지는 눈으로 판단한다(§5 예약). `float(mipCount) − 1`은 **형변환이 먼저**다 — `uint`에서 빼면 빈 슬롯(밉 0개)에서 40억이 된다(크래시 없이 조용히 마지막 밉으로 잘림).
- **Schlick → `EnvBRDFApprox` (E-3).** `F_Schlick(f0, N·V)`은 roughness를 몰라 마른 아스팔트·벽도 스치는 각도에서 웅덩이처럼 60%를 반사했다 — 사용자가 "바닥이 금속처럼 보인다"고 관찰했다. Schlick을 `F0(1 − Fc) + Fc`로 쓰면 F0가 적분 밖으로 나와 ②가 `F0 × A + B`(A·B는 roughness·N·V만의 함수, Unreal BRDF LUT의 R·G)가 된다. `EnvBRDFApprox`는 그 LUT를 다항식으로 맞춘 것(Karis 2014, Lazarov 2013 기반)이라 상수는 유도값이 아니라 피팅값이다. 읽는 법: `exp2(−9.28·N·V)`가 `(1 − N·V)⁵` 역할, `r.x = 1 − roughness`가 스침 증가를 깎고, 같은 `a004`가 A에서 빠지고 B에 더해진다(Schlick 구조 그대로). **극한값으로 검증한다**: 매끈·정면 → F0, 매끈·수평 → 1, 거침 → 약 0.45 × F0. 이 함수를 옮겨 적을 때 `c0`·`c1`을 `float`로 선언해 X3206과 함께 모든 성분이 `1 − roughness`가 되는 버그가 실제로 났다(정면 반사율 약 1.84).
- **F0 추출 시점.** `0.08 × specular`가 직접광·환경광 두 곳이 되자 `DielectricSpecularToF0`(Unreal `ShadingCommon.ush` 이름)로 뽑았다 — 두 곳이 어긋나면 하이라이트와 하늘 반사의 반사율이 조용히 달라진다. 0.08은 IOR 1.5 → F0 0.04가 Specular 0.5가 되도록 잡은 Unreal 관례이고, 바닥 0.25 → 0.02는 **물**(IOR 1.33)의 반사율이다. N·V 보호식도 두 곳이지만 한 줄이라 세 번째(10-5)까지 둔다(§5).
- **에이전트 캡처 평가와 계획 변경 (2026-10-28).** 사용자 요청으로 빌드 없이 실행·캡처해 레퍼런스 사이버펑크 씬과 비교했다. 원인 셋: ① 정적 큐브맵(밤 광장 수평선 불빛)이 골목 자체 조명(ambient 0.045, 수직 방향광, 네온 셋)보다 밝아 벽에서 반사가 가장 밝은 항이 된다 — 좁은 골목은 원래 하늘을 거의 못 본다 ② 비스듬한 벽에서 노멀 맵이 반사 방향을 **기하 표면 아래**로 꺾어 큐브맵이 그 방향의 하늘을 돌려준다 → 은박지 무늬 ③ 레퍼런스 바닥의 핵심인 네온 반사는 큐브맵에 없다(10-7). ②의 해법 Horizon Occlusion(Russell 2015)은 가이드·예제 코드까지 마쳤지만, 사용자가 **"최종 결과물이 목표이니 구도와 씬을 먼저"**로 순서를 바꿔 §5 예약으로 보류했다(벽 재질이 씬 단계에서 바뀔 수 있음). 반사·안개·그림자는 "무엇을 비추고 가리느냐"로 평가되고, 웅덩이 마스크·10-7·룩 값이 네온 배치에 묶여 있어서 배치를 먼저 확정하면 재작업이 준다 — 10-W를 GGX보다 앞당긴 것과 같은 이유다.

### UV 타일링 (10-W W-1, 2026-10-23)

- **재질 단위 타일링을 먼저 했다.** Unity의 Tiling/Offset(`_BaseMap_ST`), Unreal의 `TexCoord` UTiling/VTiling, glTF `KHR_texture_transform`의 `scale`, Godot `uv1_scale`과 같은 표준 방식이다. 이름은 glTF·Godot 계열 `uvScale`. offset은 쓸 곳이 없어 예약(§5)했다.
- **PS에서 곱한다.** UV에 상수를 곱하는 건 선형이라 VS에서 곱해 보간하든 PS에서 곱하든 같다. Material cbuffer가 PS(`b2`)에만 걸려 있어 바인딩을 바꿀 필요가 없다. VS에서 텍스처를 읽게 되면(높이 맵) 그때 VS에도 건다(예약).
- **`tiledUV`를 따로 두고 `input.uv`를 덮어쓰지 않는다.** W-2의 roughness 맵은 albedo와 같은 타일링을 쓰지만, W-3 웅덩이 마스크는 반복되면 안 된다(반복을 깨는 게 목적). 비용 0인 선택으로 나중의 수정을 없앴다. 이름은 "원본과 다른, 반복이 적용된 UV"가 드러나게 `tiledUV`(Unity의 `TRANSFORM_TEX` 결과에 해당).
- **메시 UV를 고쳐서 공유를 살렸다.** 타일링을 넣자 왼쪽 벽(+X 면)과 오른쪽 벽(−X 면)에 다른 값이 필요해졌다. 원인이 크기가 아니라 **+X 면 UV가 90° 돌아간 데이터**였으므로 Material을 나누지 않고 `MakeCube`를 고쳤다. 나눴다면 텍스처 결이 두 벽에서 직각으로 어긋난 채 남았을 것이다. 규칙("u = 보는 사람의 오른쪽 = `cross(+Y, 시선)`, v = 아래")은 **이미 맞던 앞면·왼쪽 면으로 먼저 검산**하고 적용했다. UV 네 모서리는 순환 순서라 한 칸 회전 = 90°, 순서 뒤집기 = 거울이다. 10-6 노멀 맵의 탄젠트가 이 u 방향을 따르므로 그 선행 작업이기도 하다.
- **크기가 다른 끝벽만 Material을 분리했다(`endWallMat`).** `CreateMaterial`이 이름으로 캐시하고 `Material` 복사가 막혀 있어(`= delete`) 같은 설정을 세터로 한 번 더 넣는다. 재질이 더 늘면 예약한 **월드 정렬 UV**가 이 중복과 "크기별 값 계산"을 함께 없앤다(트리거: 에셋 배치 시작).
- **텍셀 밀도를 맞췄다.** 세 재질의 값은 달라 보이지만 모두 0.5회/m다. 면마다 결 크기가 달라 보이는 위화감을 없애는 기본 규칙이다.
- **확인 방법**: `return float4(frac(tiledUV), 0, 1);` UV 디버그 뷰 — 빨강이 보는 사람의 오른쪽으로, 초록이 아래로 차오르면 정상. UV 방향·스케일·텍스처 문제를 한 번에 가른다.
- **ImGui `Drag` 계열은 3번째 인자가 속도, 그다음이 최소·최대다**(`Slider`와 순서가 다르다). 처음에 `DragFloat2(label, v, 0.1f, 50.0f)`로 써서 속도 0.1, 최소 50, 최대 0이 됐고, ImGui는 `min >= max`면 범위를 적용하지 않아 경고 없이 "범위 없음"으로 동작했다. `SetUvScale`이 0 이하를 거부해 값은 망가지지 않았다(UI 범위 + 세터의 두 겹 방어).

### roughness 맵과 기본 텍스처 (10-W W-2, 2026-10-23 ~ 10-26)

- **맵이 없는 재질은 플래그가 아니라 기본 흰색 텍스처로 처리한다.** 강의 Ch13은 `useRoughnessMap` 플래그로 분기하지만, ① 상수 버퍼가 꽉 차 플래그에 16바이트가 더 필요하고 ② 셰이더 분기가 생기며 ③ AO·웅덩이 마스크(W-3)도 같은 흰색을 재사용할 수 있다. Unreal 기본 텍스처 파라미터와 같은 방식이다. **기본값은 그 맵이 섞이는 연산이 정한다**: 곱하는 맵은 흰색(1), 더하는 맵은 검정(0), 노멀 맵은 평평한 파랑(0.5, 0.5, 1). D3D11은 바인딩되지 않은 SRV를 0으로 읽는데, 곱하는 맵에는 가장 나쁜 기본값이다.
- **기본 텍스처는 `Renderer`가 소유한다.** "맵이 없으면 무엇을 쓸까"는 렌더링 정책이고, `Material`은 맵이 있다/없다만 안다. 포맷(`UNORM`)도 `Renderer`가 직접 정한다 — `TextureType`·`ToDxgiFormat`은 Asset 계층이라 `Renderer`가 알면 계층이 뒤집힌다(AGENTS §3). 렌더 타깃 포맷을 `Renderer`가 정하는 것과 같다.
- **필수 입력과 선택 입력의 실패 정책이 다르다.** albedo가 없으면 오류(앱 종료), roughness 맵이 없으면 정상(기본값). 다만 **지정했는데 망가진 맵**은 오류로 본다.
- **최솟값은 BRDF 입구에서 막는다.** 맵 × 배율, 그리고 W-3의 웅덩이 마스크까지 roughness를 만드는 경로가 늘어서, 호출하는 쪽마다 막기보다 값을 쓰는 `SpecularBlinnPhong`이 스스로 막는다(`FresnelSchlick`의 `saturate`와 같은 "입구에서 막기"). `MIN_PERCEPTUAL_ROUGHNESS`를 **먼저** 넣고 PS 샘플·바인딩을 넣는 순서로 했다 — 바인딩 전 중간 상태에서 `t1` = 0 → roughness 0 → `pow(1, ∞)` NaN 대신 "반사가 날카로워지는" 정도로 끝난다.
- **맵을 들일 때 평균을 보존한다.** `Asphalt023S` roughness 맵 평균이 약 0.65라 배율을 0.3 → 0.45로 올려 평균 0.29를 유지했다. 그래야 화면 변화가 "픽셀마다 달라짐" 때문만이 되고, 맵의 효과를 판단할 수 있다. 맵의 대비가 작아(거의 균일한 회색) 반사의 얼룩은 은은하다 — 큰 대비는 W-3이 맡는다.
- **실제 크기로 UV Scale을 정한다.** ambientCG API가 재질의 실제 크기를 준다(`Asphalt023S` 125 × 125cm). 1인칭 데모에서는 자갈이 실제 크기로 보여야 공간 감각이 맞는다.
- **`TextureType`이 가장 틀리기 쉬운 곳이다.** roughness 맵을 `Albedo`로 읽으면 평균 0.65가 0.39로 디코드되어 바닥 전체가 매끈해지고, 경고도 없다.
- **리뷰에서 잡은 문제**: ① `float3 roughnessSample = ….r` → `float`에 대입하며 X3206(값은 우연히 맞음 — "무해한 X3206"도 없앤다) ② 기본 흰색 경로에 SRV `nullptr` 검사 누락 → "핸들 고르기와 SRV 얻기"를 나눠 공통 검사로 해결 ③ 리팩터링 중 남은 옛 줄이 **`nullptr` 검사 전에 `->`로 역참조**(맵이 없는 모든 재질에서 크래시) → 선언과 동시에 삼항으로 초기화 ④ 사용자가 본 "크래시"는 **마지막 저장보다 45초 전에 빌드된 exe**였다. 종료 코드 −1(의도된 `false` 경로)과 `0xC0000005`(접근 위반)를 구분하고, `OutputDebugString`을 직접 받아 최신 빌드가 정상임을 확인했다.

### 탄젠트와 노멀 맵 (10-6, 2026-10-26)

- **W-3 웅덩이보다 먼저 했다(사용자 결정).** NormalDX가 W-2에서 이미 로컬에 있었고, 고인 물은 표면 요철을 덮어 **평평한 거울**이 되므로 노멀 맵이 먼저 있어야 W-3에서 "웅덩이에서는 노멀을 평평하게"를 함께 넣을 수 있다.
- **텍스처 슬롯 헬퍼를 새 슬롯보다 먼저 뺐다(N-1).** 예약 항목의 트리거(세 번째 슬롯)가 걸렸고, 복사한 뒤 정리하면 W-2b처럼 한쪽만 검사가 빠질 수 있어서다. `fallback`을 인자로 받아 **메커니즘(없으면 대신 쓰기)은 함수, 정책(무엇으로)은 호출부**가 맡는다. 리뷰에서 헬퍼가 `fallback`을 무시하고 항상 흰색을 쓰던 버그를 잡았다 — 유일한 호출부(roughness)가 흰색이라 결과가 같아 보였지만, 노멀 슬롯에서 모든 재질의 법선이 (1, 1, 1)로 꺾였을 것이다. "다른 값을 넘기면 다르게 동작하는가"가 헬퍼의 검사 기준이다. `Renderer.h`에는 `class Texture;` 전방 선언이 필요했다(포인터만 쓰므로 include 불필요).
- **W-1c의 UV 규칙이 탄젠트를 정했다.** 탄젠트 = u가 커지는 방향 = W-1c의 "보는 사람의 오른쪽"이라 면별 값이 표에서 바로 나오고, `cross(N, T)`가 모든 옆면에서 +v(아래)가 된다. 그래서 **DirectX 규약 노멀 맵(G = 아래)과 그대로 맞아 G 채널을 뒤집지 않는다.**
- **기본 탄젠트 (1, 0, 0)**: 탄젠트를 채우지 않는 생성기(`ModelLoader`)의 정점이 0이면 셰이더 `normalize(0)`이 NaN이 된다. 아무 단위 벡터라도 있으면 최악이 "방향이 틀린 노멀 맵"으로 끝난다(`IncidentLight` 조기 반환의 안전값과 같은 원칙).
- **탄젠트는 `model`, 노멀은 `invTranspose`.** 탄젠트는 표면 위의 방향이라 위치처럼 늘어나고, 노멀은 수직 방향이라 비균등 스케일(바닥 4 × 0.2 × 20)에서 따로 처리한다.
- **기본 노멀은 평평한 파랑(128, 128, 255).** 기본 텍스처 원칙("그 맵이 섞이는 방식이 정한다")의 세 번째 사례 — 곱셈은 흰색, 보간은 검정, 방향은 평평한 노멀(Unreal `DefaultNormal`). 128/255는 0.004 기울어지지만(0.2°) 최종 `normalize`가 맞춘다.
- **노멀 맵도 `Sample`(밉맵)로 읽는다.** 처음에 강의 Ch13 `GetNormal`의 `SampleLevel(…, lodBias)`를 따라 레벨 0만 읽었는데, 강의의 그 값은 ImGui 실험용이다. UV (3.2, 16) 반복에서 레벨 0만 읽으면 먼 바닥 법선이 튀어 specular가 지글거린다. 강의 코드를 가져올 때 "이 줄이 **기능** 때문인가 **시연** 때문인가"를 묻는다.
- **그람-슈미트에는 정규화된 N을 쓴다.** `T − (T·N)N`은 |N| = 1일 때만 N 성분을 정확히 지운다. 보간된 `input.normal`은 길이가 1이 아니다.
- **Rim도 노멀 맵 법선을 쓴다.** 같은 픽셀에서 조명과 Rim이 다른 법선을 쓰면 일관성이 깨진다.
- **경고로 잡은 버그**: VS에서 탄젠트를 계산만 하고 출력에 넣지 않음(X3578 "출력이 완전히 초기화되지 않음" — 채우지 않은 VS 출력은 GPU 레지스터의 쓰레기 값이 보간된다), `float4` → `float3` 대입은 `.xyz`로 명시해 X3206을 피함.
- **"배선 먼저, 데이터 나중"**: N-1~N-3 모두 화면이 그대로였고 탄젠트 색(바닥·끝벽 분홍빛 빨강, 왼쪽 벽 연한 파랑, 오른쪽 벽 올리브)과 법선 색(바닥 연두) 디버그 뷰로 배선을 확인했다. 그래서 N-4에서 화면이 바뀐 원인은 노멀 맵 데이터와 `TextureType`뿐이었다.
- **결과**: 채움광과 네온의 바닥 반사가 매끈한 타원에서 **자갈 결을 따라 부서지는 질감**이 됐다. specular는 `N·H`에 민감해(지수가 수십~수백) 노멀 맵의 몇 도 기울기가 diffuse보다 반사에서 훨씬 뚜렷하게 드러난다.
- **N-5 벽 노멀 맵 (2026-10-26)**: 벽에 붙일지 사용자가 물었고, 두 이유로 W-3보다 먼저 했다. ① 네온은 벽에 붙어 벽면을 거의 평행하게 비추므로(raking light) 요철이 바닥보다 잘 드러난다. ② 그때까지 실제 노멀 데이터로 검증된 면은 윗면(바닥)뿐이었다 — 왼쪽 벽(+X 면, T = +Z)·오른쪽 벽(−X 면, T = −Z)·끝벽(−Z 면, T = +X)이 탄젠트 표의 서로 다른 세 줄이라 한 번에 검증된다.
  - **텍스처를 교체했다(A안).** 옛 `cyberpunk_wall_albedo_v1.png`에는 짝이 되는 노멀 맵이 없고, 무엇보다 **음영이 그려져 있다**(균열 가장자리·줄눈의 그림자). PBR albedo는 빛이 빠진 색이어야 하는데, 그려진 그림자 위에 노멀 맵 조명이 더해지면 그림자가 두 번 생기고 빛 방향이 서로 어긋난다. 밝기에서 노멀을 만드는 B안은 "밝기 ≠ 높이"라 버렸다. albedo·노멀·roughness가 서로 맞는 CC0 세트(ambientCG `Bricks096`)로 바꿨다.
  - **스위치는 미뤘다.** 패널 텍스처는 그려진 음영이 벽보다 강하고(나사·베벨·토글이 왼쪽 위 빛), 정사각 텍스처가 0.4 × 0.6 큐브 6면에 늘어나 붙어 있어 노멀 맵보다 비율 문제가 먼저다(§5 예약).
  - **검증(N-5b)은 코드 없이** Editor에서 빛 하나(`EnvironmentFillLight`, Directional 0·Ambient 최소)를 벽 0.4m 앞에서 좌우·상하로 옮기며 "빛 쪽 돌 가장자리가 밝아지는가"로 했다. 세 벽 모두 가로·세로 정상. 어떤 축이 틀리면 증상으로 원인이 갈린다 — 한 벽의 가로만 반대면 그 면의 탄젠트 부호, 모든 벽의 세로가 반대면 `cross(N, T)` 순서나 GL 노멀 맵.
  - **룩(N-5c, 사용자 요청으로 에이전트가 빌드·캡처해 맞춤)**: 원본(1.0, 0.85)은 밝은 노란 베이지 모르타르가 네온을 너무 받아 **간판보다 벽이 튀고**, 실제 roughness 0.5에서 돌마다 플라스틱 광택이 맺혔다. BaseColor (0.5, 0.52, 0.6)은 네온 화면에서 가장 좋았지만 **전원 OFF 시작 화면에서 돌 무늬가 사라져** (0.65, 0.67, 0.75)로 올렸다. B를 남긴 이유는 노란 벽에 Pink가 섞여 탁한 주황이 되는 것을 막기 위해서다. 같은 벽이 OFF(약한 하늘빛·보조광)와 ON(강한 색광) 두 조명에 놓이므로 **룩은 두 장면에서 함께 본다.** roughness 배율은 상한 1.0(`Material::SetRoughness`의 clamp)이라 실제 최대가 맵 평균 0.58이다 — 더 거칠게 하려면 예약한 roughness 리맵이 필요하다.
  - **실수와 교훈**: 경로에 ambientCG 접미사 `-JPG`를 빠뜨려 −1 종료, 이를 고치려고 디스크 파일명을 바꾸다 절반만 바뀜, `CyberpunkAlley` 폴더를 통째로 지워 아직 쓰는 두 장(네온·스위치)까지 사라짐(git으로 복구). 결론적으로 Bricks096은 `-JPG`를 지운 이름으로 통일했다(`CREDITS.md`에 기록). `LoadTexture` 실패가 원인 없이 `return false`라 같은 증상이 연달아 나왔다 → 실패 로그를 §5에 예약했다.

### 웅덩이 마스크 (W-3, 2026-10-26)

- **웅덩이는 새 조명 기능이 아니라 표면 데이터다.** 흑백 마스크 하나로 roughness·albedo·normal 세 값을 보간한다(Lagarde "Water drop" 연재, Unreal Wet/Puddle 머티리얼 함수와 같은 모델). 보간은 `SurfaceData`를 채우기 전에 끝나므로 `ComputeDirectLighting`·ambient·Rim은 바뀐 값을 받기만 한다 — 10-1의 빛/표면 분리가 실전에서 확인된 자리다.
- **세 줄을 같은 모양 `lerp(마른 값, 웅덩이 값, mask)`로 통일했다.** 리뷰에서 albedo가 `lerp(1.0, surfaceColor × 배율, mask)`(배율 형태와 값 형태가 섞임)로 쓰여 **마스크 0인 모든 표면이 흰색**이 되는 버그를 잡았다. `lerp(float, float3, float)`은 스칼라가 조용히 확장돼 `fxc` 경고가 없다 → **t = 0과 t = 1을 대입해 보는 것**이 유일한 검사다.
- **노멀은 탄젠트 공간에서 `(0, 0, 1)`로 보간한다.** 탄젠트 공간의 "요철 없음"은 상수 하나이고 TBN을 곱하면 정확히 기하 법선 N이 된다. 월드 공간에서 하면 "요철 없음"이 픽셀마다 다른 N이고 `normalize`가 한 번 더 필요하다. 이 줄은 Unreal `FlattenNormal`과 같은 연산이라 예약한 "노멀 강도 배율"은 t만 상수로 바꾸면 된다.
- **마스크는 반복 없는 `input.uv`로 읽는다.** 아스팔트 결은 반복돼도 티가 안 나지만 웅덩이가 16번 반복되면 바로 보인다. W-1에서 `input.uv`를 덮어쓰지 않고 `tiledUV`를 따로 둔 것이 여기서 쓰였다.
- **기본 텍스처는 검정이고, 생성 코드는 묶지 않았다**(§5 예약 "기본 텍스처 생성 묶기" 처리). 기본값은 섞이는 방식의 항등원이라 곱셈(흰색)·보간과 덧셈(검정)·방향(평평한 노멀) 셋으로 **닫혀 있다** — AO는 흰색, metallic은 검정을 재사용하므로 네 번째가 생기지 않는다.
- **"0을 기대하는 테스트"를 피했다.** D3D11은 바인딩되지 않은 슬롯을 샘플하면 0을 돌려준다. 그래서 마스크 디버그 출력이 전부 검정인 것만으로는 바인딩을 증명할 수 없어, fallback을 잠깐 흰색으로 바꿔 **전부 흰색**이 나오는지 함께 확인했다(W-3a).
- **단계마다 화면이 바뀌지 않게 했다(배선 먼저, 데이터 나중).** W-3a(경로)·W-3b(식, 임시 `step(0.5, input.uv.x)`로 바닥 절반 비교 후 제거)는 화면 변화가 없어야 성공이었고, 화면은 마스크를 붙인 W-3c에서만 바뀌었다.
- **마스크는 "그림"이 아니라 레벨 데이터다.** 평면 반사에서 빛이 비치는 점은 카메라에서 빛까지 수평 거리의 `h / (h + H)`(눈높이 1.6m, 네온 2.2~2.8m → 36~42%) 지점이라, 걸어가는 동안 Pink·Cyan·Orange 하이라이트가 지나갈 자리에 웅덩이를 놓았다. 이미지 위쪽 = 끝벽(z = 20)이다(`MakeCube` 윗면 UV). 배치·생성 방법·"네온을 옮기면 다시 만든다"는 `Assets/Textures/Masks/CREDITS.md`. 같은 기하가 10-7 평면 반사(반사 행렬 = y 뒤집기)이므로 그때 간판도 이 자리에 비친다.
- **튜닝 (사용자 요청으로 에이전트가 캡처 비교)**: 시작값 0.15 / 0.5는 웅덩이가 물이 아니라 **검은 얼룩**으로 보였다 — 실제 웅덩이는 하늘·주변을 비춰 밝아 보이는데 간접광 반사가 없어(10-5 전) 어둡게 누른 만큼 꺼진다. albedo 배율을 0.75로 올려 "물이다"라는 신호를 반사에 맡겼다. roughness 0.1은 Blinn-Phong 지수 약 2만이라 Orange 반사가 점 하나로 줄어 0.15(약 4천)를 유지했다. **물리값과 "빠진 기능이 있는 지금 옳아 보이는 값"이 다르다** — 10-2c의 임시 specular 0.5와 같은 구조라, 10-5 IBL·10-2' GGX에서 다시 맞춘다(§5).

### HDR 씬 타깃, 후처리 패스와 선형 색공간

로드맵 11번(HDR + Tone Mapping + Bloom)의 Step 1~4와 3-d에서 내린 결정이다.

- **렌더 타깃은 `GraphicsResourceManager`가 소유하고, 리사이즈는 같은 슬롯에서 재생성한다.** 창 크기가 바뀔 때마다 `push_back`하면 append-only 구조에서 텍스처가 계속 쌓인다. 슬롯은 계속 "같은 렌더 타깃"을 뜻하므로 generation이 필요 없다. 새 객체를 비어 있는 임시 `ComPtr`에 만든 뒤 성공하면 교체해서, 실패해도 기존 타깃이 남고 `GetAddressOf` 누수도 피한다. 대신 재생성 전에 받은 raw RTV/SRV 포인터는 무효가 되므로 바인딩 직전에 매번 핸들로 조회한다.
- **포맷·크기 결정과 리사이즈 전파는 `Renderer`가 한다.** AppBase는 `Renderer::Resize`로 창 크기 변경만 알린다. 어떤 타깃이 몇 개이고 각각 어떤 크기인지(Bloom의 절반 크기 등)는 Renderer만 안다.
- **공개 함수는 약속으로, 셰이더는 하는 일로 이름 짓는다.** `EndScene`은 "씬 그리기가 끝나면 백버퍼에 최종 이미지가 있고 UI를 그릴 준비가 된다"는 약속이라 톤 매핑·Bloom이 추가돼도 AppBase가 바뀌지 않는다. 반대로 `copyPixelShader`는 복사만 하므로 톤 매핑은 새 셰이더로 만들었다.
- **전체 화면 삼각형**: 화면(-1~1)보다 큰 삼각형 하나를 `SV_VertexID`와 셰이더 안의 상수 배열로 만든다. 정점 버퍼·Input Layout이 필요 없고 모든 후처리 패스가 같은 VS를 쓴다. 클립 공간 y는 위로, UV v는 아래로 커지므로 v 부호를 뒤집는다.
- **바인딩 순서: 출력 교체 → 입력 바인딩 → Draw → 입력 해제.** 같은 텍스처를 RTV와 SRV로 동시에 걸 수 없기 때문이다. D3D11은 상태 기계라 각 패스는 자기가 쓰는 샘플러·Input Layout·상수 버퍼 슬롯을 직접 설정한다. 후처리가 PS `b0`를 쓰므로 `BeginFrame`이 매 프레임 Light 버퍼를 `b0`에 다시 거는 줄이 필수다.
- **백버퍼는 Clear하지 않는다.** 전체 화면 삼각형이 깊이 테스트·블렌딩 없이 백버퍼 전체를 매 프레임 덮어쓰기 때문이다. Clear 책임은 빈 공간이 생기는 HDR 씬 타깃으로 옮겼다.
- **`PostProcessSettings` 구조체와 별도 `PostProcessConstantData`**: Bloom Threshold/Strength/디버그 뷰가 곧 추가될 예정이라 `float` 하나가 아니라 값 묶음으로 만들었다(`DirectionalLight`와 같은 경로). CPU 의미 데이터와 GPU 16바이트 정렬 데이터는 분리한다. 프레임 데이터의 입구는 `FrameRenderData`뿐이므로 `BeginFrame`이 업로드하고 `EndScene`은 바인딩만 한다. `ToneMapper` enum의 명시적 숫자(0/1)는 셰이더 분기와의 약속이다.
- **선형 색공간** — 조명·Exposure·톤 매핑은 "값 = 빛의 양"을 전제로 한다.
  - 텍스처 디코드는 셰이더 `pow`가 아니라 `_SRGB` 포맷으로 한다. 필터링 전에 디코드돼 정확하고 한 곳(`D3D11Utils::CreateTexture`)만 바뀐다. 처음에는 파일 텍스처가 전부 색상 텍스처라 일괄 적용했고, 2026-09-18에 `TextureType`으로 분리했다(§4 "텍스처 색공간 선택").
  - 출력 인코드는 sRGB 백버퍼 RTV가 아니라 톤 매핑 셰이더 마지막에서 한다. 같은 백버퍼에 그리는 ImGui가 이중 인코드되지 않게 하기 위해서다.
  - 색 상수는 사람이 고르는 sRGB 값으로 두고 Renderer 경계에서 선형으로 바꾼다(`Transform` 전치, `bool`→`uint32_t`와 같은 원칙). 색상 선택기와 화면이 일치하고 기존 튜닝 색이 유지된다. 세기·거리·Exposure 같은 배율은 변환하지 않는다. sRGB 기준 곱(`desc.color * 0.12f`)은 거듭제곱이 곱에 분배되므로 화면에서도 그대로 유지된다.
  - 구현은 출력 인코드만 먼저 넣어 "뿌연 화면"(이중으로 밝아짐)을 의도적으로 만들고, RenderDoc으로 원인(`t0`가 `UNORM`)을 확인한 뒤 텍스처 포맷으로 고치는 순서로 진행했다.
- **ACES는 선형 워크플로 이후, 룩 재튜닝은 Bloom 이후.** 감마 처리 전에는 톤 매핑 곡선 비교가 의미 없고, 튜닝 값은 곡선과 Bloom 번짐에 따라 달라지므로 한 번에 맞추기 위해서다. Reinhard는 결과가 1에 닿지 않아 하이라이트가 답답하고 중간톤이 어두워서, ACES 근사 곡선을 기본으로 하고 비교용 전환을 남겼다.
- **스위치 Rim 강조는 Intensity만 켜고 끈다.** RimPower를 0으로 두면 정면 픽셀에서 `pow(0, 0)`이 NaN이 되고 곱셈으로도 사라지지 않는다.

### Bloom 파이프라인

로드맵 11번 Step 5~7과 반복 기능에서 내린 결정이다. 강의(`Hong_Graphics2/.../08_ShaderToys_Step6_BloomEffect`)의 `ImageFilter` 구조를 참고하되, 우리 구조에 맞게 바꾼 부분이 많다.

- **절반 해상도에서 처리한다.** 결과가 어차피 흐릿해서 품질 손해가 없고 픽셀 수가 1/4이 된다. 덤으로 절반 해상도의 1텍셀이 화면 2픽셀이라 같은 커널로 번짐 반경이 두 배가 된다.
- **추출 타깃도 float 포맷을 쓴다.** UNORM으로 만들면 추출 단계에서 1을 넘는 값이 잘려 Emissive 3과 8의 번짐 세기가 같아진다(강의는 8비트 백버퍼를 읽는 LDR Bloom이라 임계값 범위가 0~1이다).
- **분리형 블러**: 2D 가우시안이 x축·y축 함수의 곱으로 분해되므로 1D 두 번이 2D 한 번과 같고, 픽셀당 샘플이 25회에서 10회로 준다. 가중치 합이 1이라 반복해도 밝기가 보존된다.
- **`dx`/`dy`는 공용 상수 버퍼 하나에 둔다.** Bloom 타깃이 모두 같은 절반 크기라 가능하다. 강의가 필터마다 상수 버퍼를 갖는 이유는 다운샘플 단계마다 타깃 크기가 다르기 때문이며, 다단계로 확장하면 우리도 분리해야 한다.
- **Bright 타깃을 덮어쓰지 않고 남긴다.** 핑퐁을 BlurX/BlurY 두 개로만 돌리면 타깃 하나를 아낄 수 있지만, 그러면 디버그 뷰에서 "추출 결과"와 "블러 결과"를 구분할 수 없어 실패 원인을 좁히지 못한다.
- **합성은 톤 매핑 셰이더 안에서 한다.** 톤 매핑 전에 더해야 1을 넘는 합이 곡선에 눌린다(뒤에 더하면 네온 주변이 흰 덩어리가 되고, 감마 뒤에 더하면 선형 값과 감마 값을 섞게 된다). 마지막 후처리 패스가 이미 있으므로 강의의 `Combine` 전용 패스가 필요 없다.
- **exposure는 원본에만 곱한다.** Bloom 텍스처에는 추출 패스에서 이미 반영돼 있다. "화면에서 밝게 보이는 것이 번진다"를 만족시키려고 추출에서 곱했기 때문에, 합성에서 또 곱하면 노출을 올릴 때 번짐만 폭주한다.
- **반복 블러의 입력 사슬**: 첫 회차만 Bright에서 읽는다. 매 회차 Bright에서 읽으면 결과가 같아 반복이 무의미하고, 반대로 첫 회차부터 BlurY에서 읽으면 이전 프레임 내용을 읽어 프레임 간 되먹임(잔상)이 생긴다. 중간 타깃을 Clear하지 않는 최적화가 이 함정을 만든다.
- **디버그 뷰는 CPU 전용 값이다.** 셰이더가 분기하는 `ToneMapper`와 달리 "어떤 SRV를 바인딩할까"의 선택이라 상수 버퍼에 넣지 않았다. `PostProcessSettings`와 `PostProcessConstantData`를 분리해둔 것이 실제로 이득이 된 첫 사례다.
- **`DrawFullScreenPass` 헬퍼는 패스가 셋이 된 시점에 뽑았다.** 반복 기능이 함수 없이는 구현 자체가 불가능해서 "실제 변경 압력"이 확정된 시점이었다. 뷰포트 설정(패스가 아니라 구간 단위), null 검사(상태를 바꾸기 전에 끝내야 함), 디버그 뷰 선택(정책)은 호출자에 남겨 헬퍼를 작게 유지했다.
- **강의의 `ImageFilter` 같은 클래스는 만들지 않았다.** 필터가 10~20개일 때 루프로 찍어내려는 구조이고, 우리는 패스가 4~5개라 함수 하나로 충분하다. **전환 조건**: 다운샘플을 다단계로 늘려 패스마다 타깃 크기와 `dx`/`dy`가 달라지면 그때 도입한다.

### 룩 재튜닝

목표로 잡은 그림은 이렇다. **"비 온 뒤 한밤의 뒷골목. 전원이 꺼져 있을 땐 희미한 음영과 젖은 바닥의 윤곽만 보인다. 걷다 보면 스위치가 눈에 들어오고, 상호작용 거리에 들어오면 윤곽선이 밝아진다. 켜면 네온이 차례로 들어와 각자의 색으로 골목을 물들인다. 네온 자체는 매우 밝아 형태가 번지고, 주변 벽은 물들되 어둡다."** 아래 결정은 전부 이 문장에서 나왔다.

- **값을 만지기 전에 세팅 논리부터 고쳤다.** 슬라이더로 해결되지 않던 어색함의 원인은 세 가지였다 — 흰색 Directional Light `intensity 1.0`(한밤에 한낮의 태양), 보라색 `EnvironmentFillLight` `intensity 1.58` 상시 On(전원 Off인데 골목 한가운데가 보라색), Ambient `0.4`(선형이라 화면상 0.66). 이걸 놔두면 어떤 값을 넣어도 목표 문장에 도달할 수 없었다.
- **하늘광을 거의 수직으로 눕혔다.** 옆벽은 법선이 수평이라 `dot(-direction, normal)`이 0이 돼 하늘광을 전혀 받지 않는다. 결과적으로 **바닥만 차갑게 빛나고 옆벽은 네온이 닿는 곳만 밝아진다.** 골목 위 좁은 하늘에서 빛이 떨어지는 상황과 일치하고, 젖은 아스팔트가 하늘을 반사하는 레퍼런스 룩이 여기서 나온다.
- **방향을 단위 벡터로 저장한다.** 셰이더가 `normalize`하지 않으므로 길이가 곧 세기 배율이 된다. 기존 `(0, -0.5, 1)`은 길이 1.118이라 의도보다 12% 세게 들어가고 있었다.
- **Emissive는 올리고 Point Light는 내렸다.** 두 값은 물리적으로는 하나여야 하지만(발광체는 자기 밝기만큼 주변을 비춘다) 우리에겐 GI가 없어 따로 논다. 이 분리가 오히려 이득이다 — "네온은 매우 밝고 벽은 은은하게"를 두 값을 반대로 움직여 만들 수 있다.
- **세기는 채널별 휘도 가중치로 환산해 맞췄다.** `brightPass`가 `dot(color, (0.2126, 0.7152, 0.0722))`로 판정하므로 같은 Intensity가 같은 밝기가 아니다. 선형 변환 후 단위 세기당 휘도는 Pink 0.242 / Cyan 0.585 / Orange 0.284로, **Cyan이 Pink의 2.4배**다. 튜닝 전 값(3 / 8 / 5)은 실제 휘도가 0.73 / 4.68 / 1.42로 Cyan만 6배 이상 튀었고 Pink는 Threshold를 넘지도 못했다.
- **스위치에 대기 표시등(Emissive)을 넣었다.** 골목을 어둡게 만들자 스위치가 아예 안 보여서 "걷다 보면 스위치가 눈에 들어온다"가 성립하지 않았다. BaseColor만으로는 어두운 곳에서 빛나지 않는다. 세기는 Threshold 아래로 잡아 **번지지 않게** 했다 — 번지면 "네온"으로 읽히고 "표시등"으로 읽히지 않는다.
- **Rim 강조를 청록에서 호박색으로 바꿨다.** 골목 전체가 청록 계열이라 청록 강조는 Cyan 네온에 묻힌다. 보색인 호박색은 확실히 분리되고, 상용 게임의 상호작용 색 언어와도 맞는다.
- **주황 네온의 심지가 노랗게 뜨는 것은 고치지 않았다.** 톤 매핑이 채널별이라 R이 먼저 1에 닿고 G가 따라 올라간다. 계산상 심지를 진짜 주황으로 만들려면 G를 sRGB 0.115까지 내려야 하는데, `NeonSignDesc.color`가 Emissive와 Point Light에 공유되므로 **그러면 벽에 비치는 빛이 순수 빨강이 된다.** 실제 네온 사진도 심지는 하얗게 뜨고 색은 주변 헤일로가 담당하므로 현재 상태를 정상으로 본다. 한계는 §5에 남긴다.

---

## 5. 현재 알려진 문제와 보류 항목

### 구조

- `AppBase`가 초기화, Greybox 구성, Play UI, Play 입력, RenderItem 조립, 기능 객체 조율까지 담당해 여전히 크다. Editor UI는 `EditorUI`로 분리했으며, 실제 변경 압력이 확인되는 책임부터 추가 분리한다.
- `AppBase`의 멤버가 전부 `public`이고 전역 `g_appBase`로 접근 가능하다.
- 씬 정의가 `InitGreyBoxScene()`에 하드코딩돼 있어 변경 시 재컴파일이 필요하다.
- `GameObject`의 컴포넌트가 선택적이지 않다. `PointLightComponent`/`BoxCollisionComponent`만 `Has` 플래그를 쓰고 나머지는 포인터 null 검사로 판단해 규칙이 일관되지 않다.
- `Renderer`가 렌더 타깃 네 개와 셰이더 여덟 개(씬 VS·PS, 전체 화면 VS, 후처리 PS 다섯), 샘플러 두 개를 직접 들고 있다(스카이박스 셰이더는 `SkyBox`로 분리, `e805963`). 후처리 패스가 더 늘면 타깃·셰이더 묶음을 다루는 구조(강의 Ch11 `PostProcess`/`ImageFilter` 상당)가 필요해진다 — 전환 조건은 §4 "Bloom 파이프라인" 참고.

### 렌더링

- `Renderer` Material 경로는 유효한 Albedo Texture를 전제로 하며 기본 Material/Texture 정책이 없다. 없으면 앱이 종료된다.
- 드로우콜마다 InputLayout, Shader, Sampler를 다시 바인딩한다.
- 컬링이 없다. `CullMode`가 `D3D11_CULL_NONE`이고 프러스텀 컬링도 없다.
- ~~UV Tiling 정책이 없다~~ → **W-1에서 해결**(`ff34261`, §3·§4 "UV 타일링"). 크기가 제각각인 소품이 늘면 재질마다 값을 계산해야 하는 문제는 남는다(예약: 월드 정렬 UV).
- **비등방 필터링이 없다.** 밉맵 뒤 먼 바닥이 약간 뭉개진다. 지금은 보류 — §6의 로드맵 10번 항목 참고.
- 톤 매핑이 채널별로 적용돼, ACES에서 한 채널이 먼저 1에 닿으면 색조가 이동한다. 주황 네온의 심지가 노랗게 뜨는 것이 이 현상이다. 실제 네온도 심지는 하얗게 뜨므로 지금은 정상으로 두고 있으며, 근본 해결은 휘도 기준 톤 매핑이나 색조 보존 방식이다(§4 "룩 재튜닝").
- **`NeonSignDesc.color`가 Emissive 색과 Point Light 색을 공유한다.** 원래는 "둘이 항상 같아야 한다"는 근거로 한 필드로 묶었는데, 톤 매핑을 거치면 심지에 보이는 색과 벽에 비치는 색이 달라져서 **둘을 동시에 만족시킬 수 없다**(주황이 대표 사례). 심지 색을 우선하면 벽 빛이 순수 빨강이 되고, 벽 빛을 우선하면 심지가 노랗다. **전환 조건**: 네온별 색을 정밀하게 잡아야 하면 `lightColor`를 별도 필드로 분리한다(기본값은 `color`).
- ImGui에서 바꾼 값은 저장되지 않는다. 룩을 다시 조정하면 최종값을 코드 초기값(§3 "룩 재튜닝"의 표에 위치가 정리돼 있다)에 옮겨 적어야 한다.
- 출력 인코드가 정확한 sRGB 곡선이 아니라 `1/2.2`제곱 근사다(가장 어두운 구간에서만 차이). `toneMappingPixelShader.hlsl`의 Reinhard 분기에서 `pow` 음수 경고(X3571)가 난다(실제 입력은 0 이상).
- `Renderer::BeginFrame`에 옛 백버퍼 RTV 줄이 주석으로 남아 있고(`Renderer.cpp` 307·324행), `D3D11Utils`의 실패 로그에는 파일 이름이 없다(줄바꿈은 `6256ed2`로 해결).
- **Bloom이 단일 해상도 반복 방식이다.** 반복으로 넓히면 σ가 √n로만 커져 수확이 체감한다(1→4회에서 2배, 4→8회에서 1.4배). 더 넓은 번짐이 필요하면 다단계 다운샘플/업샘플(mip 체인)로 바꿔야 한다 — 현대 엔진의 표준 방식이다.
- **임계값이 하드 컷이다.** 경계 근처 픽셀이 카메라가 조금만 움직여도 켜졌다 꺼졌다 할 수 있다(시간적 불안정). 소프트 니(soft knee)로 부드럽게 하거나, 다운샘플 시 Karis 평균으로 반딧불이(firefly)를 억제하는 대응이 없다.
- Bloom 타깃 세 개를 앱 생명주기 내내 들고 있다. 상용 엔진은 프레임 내에서만 사는 일시적 리소스로 풀링·에일리어싱한다. 지금 규모(절반 해상도 float16 3장)에서는 문제가 아니다.
- **`BlendState`가 아예 없다.** 현재 전부 불투명 렌더링이라 투명·가산 합성이 불가능하다. 비·연기·먼지·스파크 같은 입자 계열을 넣으려면 알파/가산 State 두 개와 투명 패스 분리(깊이 쓰기 off + 뒤→앞 정렬)가 먼저 필요하다. `D3D11Utils`에 `CreateGeometryShader`와 Hull/Domain 경로도 없다. 상세는 §7 "강의 연계 아이디어" 참고.
- Material에 Metallic 맵이 없다(Roughness·Normal·웅덩이 마스크 맵은 10-W·10-6에서 들어왔다). **`ModelLoader`는 diffuse 텍스처 하나만 읽고**(`aiTextureType_DIFFUSE`) 탄젠트도 계산하지 않아, 외부 모델은 albedo만 있는 표면으로 보인다 — 씬 단계로 돌아가 외부 모델이 필요하면 태그 `archive/sc3-model-pipeline`의 모델 파이프라인을 꺼낸다(§5 예약).
- ~~젖은 바닥을 평가할 디테일이 없다~~ → **10-W에서 해결**(UV 타일링, roughness·노멀 맵, 웅덩이 마스크).
- **환경 반사가 하늘 큐브맵뿐이다(10-4).** 정적 큐브맵은 골목 벽이 하늘을 가린다는 것을 모르고, 골목 안 네온·간판을 담지 않는다. 그래서 웅덩이에는 네온 대신 광장 수평선이 비치고, 하늘이 골목 조명보다 밝아 벽이 금속처럼 보인다(§4 "환경 반사"의 캡처 평가). 바닥은 10-7 평면 반사, 벽의 은박지 무늬는 Horizon Occlusion(예약), 전체 세기는 밤하늘 HDRI(NightSky, P-1에서 교체)로 다룬다.
- **MSAA가 없다.** 강의 Ch11은 float 씬 타깃을 4× MSAA(`Texture2DMS`)로 만들고 후처리 전에 `ResolveSubresource`한다. 벽·바닥 경계, 네온 가장자리, 노멀 맵 하이라이트에 계단이 보인다(예약 목록).
- `MyPF.vcxproj`에서 `skyboxVertexShader.hlsl`/`skyboxPixelShader.hlsl`만 `ExcludedFromBuild`가 없다(다른 셰이더는 런타임 컴파일이라 빌드에서 제외). 빌드 때 FxCompile이 한 번 더 컴파일할 뿐 실행에는 영향이 없다. 맞추려면 다른 셰이더 항목처럼 네 구성에 `ExcludedFromBuild`를 넣는다.
- Blinn-Phong은 GGX보다 하이라이트 꼬리가 짧아 젖은 표면의 "번지는 테두리"가 약하다. roughness 대응식을 GGX 기준으로 맞춰 뒀으므로 분포 함수만 교체할 수 있다.
- 바닥 반사가 생기면서 UV 타일링 문제(아래)가 더 눈에 띌 수 있다.
- ~~Vertex Color가 최종 Pixel Color에 사용되지 않는다~~ → 2026-10-30 `Vertex`·Input Layout·`VS_INPUT`/`PS_INPUT`에서 지웠다(정점 56 → 44바이트). `GeometryGenerator::MakeSphere`(강의 12-1 기반, 탄젠트를 우리 +u 규약으로 고침)도 함께 들어왔다.
- 셰이더가 런타임 컴파일인데 핫 리로드가 없다.
- `D3D11Utils::CreateDepthBuffer`는 아무도 호출하지 않는 죽은 코드다.

### 리소스와 데이터

- `GraphicsResourceManager`는 append-only이며 개별 삭제, 슬롯 재사용, generation이 없다. 렌더 타깃만 리사이즈 시 같은 슬롯에서 재생성된다.
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
- RenderDoc Launch로 실행하면 빌드가 되지 않으므로, C++ 수정 후 빌드를 잊으면 옛 exe가 캡처된다(2026-09-15에 실제로 겪음). 셰이더만 바꿨을 때는 앱 재실행으로 충분하다.
- `EditorUI`는 자기가 가리키는 Camera·CameraController·PostProcessSettings·DirectionalLight·배경색보다 `AppBase`에서 먼저 선언돼 **나중에 파괴**된다. 소멸자가 없어 지금은 무해하다(AGENTS.md §4). `EditorUI`에 정리 코드를 넣게 되면 선언 순서를 먼저 확인한다.
- Directional Light 방향은 CPU(`Renderer::BeginFrame`)와 GPU(`GetDirectionalLight`)에서 **두 번** 정규화된다. 결과는 같아 무해하고, 한쪽으로 줄일지는 필요해질 때 판단한다.

### 확장 예약 목록

**작업 원칙 (2026-10-23 사용자 결정)**: 지금은 현재 로드맵 단계에 **필요한 기능만** 구현한다. 확장은 **설계 모양으로만** 열어 둔다(예: 상수 `1`을 나중에 인자로 바꾸기 쉬운 식 형태, 일반형 인자 이름). 공용 함수 추출, 최적화, 매개변수 정리는 **여기에 예약**해 두고, **트리거**(실제 사용처나 변경 압력)가 생기는 단계에서 꺼내 처리한다. AGENTS.md §3 "사용처 없는 추상화 금지"를 실행하는 방법이다. 항목을 처리하면 지우고 §8에 기록한다.

| 예약 항목 | 지금 형태 | 꺼낼 트리거 | 할 일 |
|---|---|---|---|
| `F_Schlick`의 `f90` 인자 | `f0 + (1 − f0) × w` (최대 반사율 1 고정) | 10-5 IBL diffuse(Lagarde: `f90 = max(1 − roughness, f0)`) 또는 specular 0 재질의 스침 반사를 꺼야 할 때(Unreal: `f90 = saturate(50 × f0)`) | `f0 + (f90 − f0) × w`로 확장(Filament 형태). 호출부는 `ComputeDirectLighting` 한 곳 |
| `F_Schlick` 등 F0 계열의 `float3`화 | `float` (비금속만). F0는 `DielectricSpecularToF0` 한 곳(10-4) | 10-8 metallic | Unreal처럼 형제 함수 `ComputeF0(specular, albedo, metallic)` = `lerp(DielectricSpecularToF0(specular), albedo, metallic)`을 추가하고, `F_Schlick`·`EnvBRDFApprox`의 F0 인자·반환을 `float3`로. 본문은 그대로 동작 |
| `Pow4`·`Pow5` 공용 수학 함수 | `F_Schlick` 안의 5제곱, `SpecularGGX` 안의 `roughness⁴`를 직접 곱셈 | 같은 거듭제곱을 두 번째 곳(IBL, 평면 반사)에서 쓸 때 | `Common.hlsli`에 `Pow4(x)`·`Pow5(x)` (Unreal `Common.ush`와 같음). 순수 수학이라 모든 셰이더가 쓸 수 있는 종류 |
| 파생 BRDF 값 묶음(`BRDFData`) / 내적 묶음(`BxDFContext`) | `ComputeDirectLighting` 안에서 빛마다 a2·F0를 계산하고, 내적(`NdotL`·`NdotV`·`NdotH`·`VdotH`)을 스칼라 인자로 넘긴다 | metallic(10-8) 이후 파생 값이 늘거나, IBL·clear coat처럼 내적을 쓰는 함수가 셋 이상 늘 때, 또는 빛 수가 늘어 비용이 보일 때 | 파생 값은 Unity URP `InitializeBRDFData`처럼 픽셀당 한 번 계산해 구조체로, 내적은 Unreal `BxDFContext`처럼 구조체 하나로 넘긴다 |
| IBL용 Smith 재매핑 | 직접광 Vis는 α를 그대로 쓴다(`Vis_SmithJointApprox`) | 10-5 IBL에서 BRDF LUT를 직접 굽거나 IBL 가시성을 계산할 때 | IBL 적분은 k = α / 2(Karis 2013)를 쓴다. 직접광 함수를 그대로 재사용하지 않는다 |
| `ComputeIndirectLighting` | `main`의 ambient 한 줄 + 환경 반사 세 줄(`reflectDir` → `SampleEnvironmentRadiance` → `ComputeEnvironmentSpecular`, 10-4) | 10-5 IBL | 간접광 함수로 묶는다: diffuse(irradiance, `SampleEnvironmentIrradiance` 형제 함수) × `(1 − F)` + specular. 가림 계수(Horizon·AO)도 여기서 곱한다 |
| **Horizon Occlusion** (Russell, Marmoset 2015) | 없음. 가이드·예제 코드는 2026-10-28에 제시(H-1 회색 시각화 → H-2 `environmentSpecular`에 곱하기) | **씬 배치 뒤 비스듬한 벽에 은박지 같은 반사가 보이면** | `Lighting.hlsli`에 `static const HORIZON_FADE = 1.3`, `ComputeHorizonOcclusion(reflectDir, geometricNormal)` = `saturate(1 + k · dot(R, Ng))²`. **기하 법선(PS `:37`의 `normal`)**을 넘긴다 — 노멀 맵 법선이면 `dot(reflect(−V, N), N) = dot(V, N) ≥ 0`이라 항상 1. 빛·표면 쪽이 아닌 가림이라 `main`에서 곱한다(Unreal `SpecularOcclusion`과 같은 자리) |
| **환경 LOD 곡선** | `ComputeEnvironmentLOD` 선형 `r × (mipCount − 1)` | 씬·HDRI 교체 뒤에도 마른 바닥에 하늘 형태가 알아볼 만큼 비칠 때 | 함수 한 줄을 Unity `r × (1.7 − 0.7r)` 곡선으로(웅덩이 0.1은 거의 그대로, 0.4는 0.57로 더 흐림). 10-5 prefiltered 맵으로 가면 그 맵의 대응식에 맞춘다 |
| N·V 보호식 공용화 | `saturate(abs(dot(N, V)) + 1e-5)`가 `ComputeDirectLighting`·`ComputeEnvironmentSpecular` 두 곳 | 세 번째 사용처(10-5 diffuse IBL 등) | 공용 함수로 뽑거나 `BxDFContext` 항목과 함께 처리 |
| **MSAA 4×** (강의 Ch11) | 없음 | **룩을 맞추기 직전 또는 영상 녹화 전** — 계단 현상이 화면 품질에 바로 드러난다(가는 전선처럼 얇은 형상이 생기면 더 급하다) | HDR 씬 타깃과 깊이 버퍼를 같은 샘플 수의 `Texture2DMS`로, `EndScene` 시작(하늘 다음, 후처리 전)에 일반 텍스처로 `ResolveSubresource`. 리사이즈 경로·`CheckMultisampleQualityLevels`도 함께 |
| **Diffuse IBL (irradiance 큐브맵)** (강의 Ch11 `g_diffuseCube`) | 상수 `ambientStrength` | 밤하늘 HDRI로 바꾼 뒤(지금 광장 HDRI는 벽을 광장 색으로 물들인다) | 오프라인 도구로 irradiance DDS를 굽고 `SampleEnvironmentIrradiance(…, normal)`. 10-5의 절반 |
| `F_Schlick` 주석 | "간접광/IBL은 N·V"라고 적혀 있다 | 다음에 `Lighting.hlsli`를 고칠 때 | 환경광 Fresnel은 이제 `EnvBRDFApprox`가 맡는다. `F_Schlick`은 직접광(V·H) 전용으로 주석을 고친다. `EnvBRDFApprox` 주석에도 출처·`F0 × A + B`·극한값 셋을 남긴다 |
| **stb_image Debug 최적화** | `D3D11Utils.cpp`가 `STB_IMAGE_IMPLEMENTATION`을 담아 Debug(`/Od` + 런타임 검사)로 컴파일된다 — 2K JPG 한 장 약 140ms | 텍스처가 늘어 시작이 느리다고 느낄 때(태그 브랜치에서는 51장에 7초) | 구현부를 `StbImage.cpp` 한 파일로 옮기고 그 파일만 Debug에서도 `/O2`·`BasicRuntimeChecks` 끔·JustMyCode 끔(태그 `archive/sc3-model-pipeline`의 `00d0644`에 그대로 있다). 외부 라이브러리라 디버깅에 잃는 것이 없다 |
| 텍스처 로드 병렬화 / 2D DDS 쿠킹 | 텍스처를 메인 스레드에서 하나씩 디코딩 + `GenerateMips` | stb 최적화 뒤에도 시작이 거슬리거나, 배포 빌드를 만들 때 | 싼 순서: ① 여러 스레드에서 디코딩하고 GPU 업로드만 메인 스레드 ② BC 압축 + 밉을 미리 구운 DDS(§4 "텍스처 파이프라인 방향", DirectXTex `texconv`) — 디코딩·밉 생성이 사라지고 VRAM도 4~8배 준다 |
| 모델 파이프라인 (노드 변환·PBR 맵·부품 꺼내기) | `ModelLoader`가 메시만 읽고 노드·노멀·roughness를 모른다 | 씬 단계로 돌아가 외부 모델(모듈러 키트 등)을 쓸 때 | 태그 `archive/sc3-model-pipeline`의 `9fa1e1b`~`ac2f713`을 꺼낸다(§8 해당 항목) |
| 셰이더 디버그 뷰 | 임시 코드(주석 처리 후 복구) | 항이 더 늘 때(Fresnel·IBL) | ImGui 드롭다운 + cbuffer 플래그로 Final / Diffuse / Specular / F 선택 |
| **`ModelLoader` 탄젠트·바이탄젠트 부호** | 모델 정점은 기본 탄젠트 (1, 0, 0)을 받는다(NaN 방지만 됨). `B = cross(N, T)` 고정 | 임포트 모델에 노멀 맵을 쓸 때, 또는 거울 대칭 UV를 가진 모델 | Assimp `aiProcess_CalcTangentSpace`로 `mTangents`를 채우고, `Vertex.tangent`를 `float4`로 늘려 w에 바이탄젠트 부호(handedness)를 담아 `B = cross(N, T) × w` |
| **노멀 강도 배율** | 노멀 맵을 그대로 쓴다 | 요철이 너무 세거나 약하다고 판단될 때 | W-3의 `normalTS = lerp(normalTS, (0, 0, 1), t)`와 같은 연산(Unreal `FlattenNormal`) — t 자리에 강도 상수를 넣는다. 재질마다 다르면 cbuffer 확장이 필요하다 |
| **웅덩이 값 재튜닝** | `PUDDLE_ROUGHNESS` 0.1(10-2'에서 0.15 → 0.1, 트리거 ② 처리), `PUDDLE_ALBEDO_SCALE` 0.75 — 간접광 반사가 없는 지금 "물처럼 보이게" 맞춘 임시값 | 10-5 IBL(웅덩이가 하늘빛을 비추게 되면 albedo를 0.5 근처로 다시 낮춰도 검은 얼룩이 되지 않는다) | 같은 캡처 위치(시작·중간·끝 근처)에서 다시 비교한다 |
| **웅덩이 상수의 cbuffer화 / F0 보간** | PS `static const` 두 개, F0는 바닥 specular(0.25 = 물)를 그대로 씀 | 바닥 외 재질에 웅덩이·젖음이 필요하거나 재질마다 다른 값이 필요할 때, 또는 ImGui에서 실시간 튜닝이 필요할 때 | Material cbuffer를 16바이트 늘려 상수를 옮기고, `specular`도 물 값(0.25) 쪽으로 보간한다 |
| **웅덩이 마스크 생성 스크립트** | 마스크 PNG만 커밋. 생성 방법은 `Masks/CREDITS.md`에 글로만 있다(스크립트는 세션 임시 폴더) | 네온·카메라 동선이 바뀌어 마스크를 다시 만들어야 할 때 | `CREDITS.md`의 배치 표대로 다시 생성한다. 자주 바뀌면 스크립트를 `Masks/`에 원본 소스로 커밋한다 |
| **법선 시각화 도구(기하 셰이더)** | 법선·탄젠트는 PS 색 디버그 뷰로 확인 | 모델의 탄젠트나 노멀 맵 방향을 디버깅해야 할 때 | 강의 연계 B(`NormalGS.hlsl`) — 정점마다 법선·탄젠트를 선분으로 그린다 |
| **roughness 리맵** | 최종 = 배율 × 맵 | 맵의 대비가 작아 반사 얼룩이 부족하다고 판단될 때 | 맵 값을 최소·최대 범위로 늘려 쓴다(Unreal 머티리얼의 RoughnessMin/Max 관례) |
| 쓰지 않는 옛 텍스처 파일 | `Assets/Textures/CyberpunkAlley/`의 `wet_asphalt_floor_albedo_v1.png`(W-2 이후)·`cyberpunk_wall_albedo_v1.png`(N-5 이후)가 남아 있지만 아무 코드도 읽지 않는다. 같은 폴더의 네온·스위치 텍스처는 **아직 쓴다** | 에셋 정리를 할 때 | 두 장만 지우거나 다른 용도로 쓴다. 폴더째 지우면 네온·스위치 로드가 실패해 −1로 종료된다(2026-10-26 실제로 발생) |
| **스위치 노멀 맵과 비율** | 패널 albedo 한 장, 음영이 그려져 있고 정사각 텍스처가 0.4 × 0.6 큐브 6면에 늘어나 붙는다 | 스위치 에셋을 교체하거나 모델로 바꿀 때 | 음영 없는 albedo + 노멀 맵 세트로 바꾸고, 앞면만 패널이 보이도록 비율·면을 정리한다. 그전까지 상호작용 표시는 Rim으로 충분하다 |
| **텍스처 로드 실패 로그** | `AssetManager::LoadTexture` 실패 시 호출부가 `return false`만 한다(종료 코드 −1, 원인 없음) | 텍스처가 더 늘어나거나, 경로 문제로 같은 증상이 다시 날 때 | 실패한 경로를 `OutputDebugStringW`로 남긴다. N-5에서 경로 오타 → 폴더 삭제가 연달아 같은 −1로 보였다 |
| **PBR 세트 경로 헬퍼** | 세트마다 Color·Roughness·NormalDX 경로를 각각 적는다(바닥·벽 두 세트) | PBR 세트가 세 번째로 늘 때 | "세트 폴더·이름 → 세 텍스처" 묶음 로드. 파일명 규칙(`-JPG` 유무)도 한 곳에서 정한다 |
| Directional 이중 정규화 | CPU·GPU 양쪽 | 셰이더 비용을 줄일 때 | 한쪽으로 통일 |
| **월드 정렬 UV / 트라이플래너** | 재질마다 `uvScale`(W-1). 크기가 다른 면은 Material을 나눠야 한다(끝벽 등) | **에셋·소품 배치를 시작할 때** — 크기가 다른 그레이박스 조각이 늘어 재질마다 스케일을 맞추기 번거로워지면 | 메시 UV 대신 `posWorld`와 법선 축으로 UV를 만든다(Unreal `WorldAlignedTexture`, Godot Triplanar). `uvScale`의 의미가 "반복 횟수"에서 "미터당 반복"으로 바뀐다. 그레이박스 재질에만 플래그로 켜고, 임포트 모델은 메시 UV를 유지한다. 노멀 맵의 탄젠트도 월드 축에서 만들 수 있다 |
| **반복 깨기 (anti-tiling)** | 텍스처를 그대로 반복 | **에셋 배치를 시작할 때** — 20m 골목에서 같은 무늬가 격자처럼 보이면 | 싼 순서: ① 웅덩이 마스크(W-3)와 데칼로 고유한 흔적 덧붙이기 ② 매크로 변화(큰 스케일 얼룩 텍스처를 곱함, Unreal 지형 기법) ③ 스토캐스틱 타일링(Unity Procedural Stochastic Texturing). ①부터 해 보고 부족할 때만 다음으로 |
| `uvOffset` | `uvScale`만 있다(`float2`) | 이음매·벽 모서리 정렬, 텍스처 아틀라스, 데칼 위치를 맞춰야 할 때 | `uvScale` 옆에 `float2 uvOffset`. 상수 버퍼에 16바이트 자리가 더 필요하다(지금 64바이트가 꽉 참). glTF `KHR_texture_transform`, Unity `_ST`의 zw |
| **비등방 필터링** | `MIN_MAG_MIP_LINEAR` | **W-1 타일링 뒤 먼 바닥이 흐려 보이면** (반복이 늘수록 비스듬한 먼 곳의 밉 레벨이 높아진다) | 씬 샘플러를 `D3D11_FILTER_ANISOTROPIC` + `MaxAnisotropy` 8~16. 한 줄 변경. 상세는 §6 "비등방 필터링" |
| **`SkyLight` 분리** | `ambientStrength`·`environmentIntensity`가 `DirectionalLight` 안에 있다(의미상 하늘광) | 10-4/10-5에서 하늘광 값이 더 늘 때(반사 세기 분리, irradiance 맵 등) | `SkyLight { cubemap, intensity, ambient }`로 묶어 `FrameRenderData`에 따로 둔다(Unreal `DirectionalLight` / `SkyLight` 구분). `FrameRenderData.environmentMap`도 그 안으로 |
| **환경맵 로드 실패 경고** | 실패하면 로그만 남고 배경색으로 대체(S-3 경로 오타가 조용히 지나감) | 에셋 경로가 다시 바뀌거나 배포 빌드(exe만 실행)를 만들 때 | Editor 패널에 "Environment map: none" 표시, 또는 Debug 빌드에서만 단언 |
| **HDRI 변환 스크립트 보관** | `hdr_to_cubemap.js`는 세션 임시 폴더에만 있다. 방법은 `HDRI/CREDITS.md`에 글로 | HDRI를 바꾸거나 해상도·밉을 다시 만들 때 | 스크립트를 `tools/`에 커밋한다. prefiltered 밉이 필요해지면(10-5) cmft 등 외부 도구로 바꾼다 |
| **`GraphicsPSO` / 공용 State 묶음** (강의 Ch13 Step2) | 패스마다 셰이더·IL·깊이 상태를 낱개로 걸고, `DrawSkybox`가 깊이 상태를 복구한다 | 강의를 Ch13까지 진행한 뒤, **같은 메시를 상태만 바꿔 여러 번 그리는 패스**가 생길 때 — 9번 그림자 깊이 패스, 10-7을 스텐실 거울로 할 때 | VS·PS·IL·Rasterizer·DepthStencil·stencilRef·Topology 구조체 + `SetPipelineState`. 변형은 복사 후 필드 변경. 강의의 수제 `operator=` 대신 기본 복사, 전역 `Graphics::` 대신 `Renderer` 멤버 |
| Material 텍스처 슬롯 enum | 텍스처마다 이름 있는 포인터와 Resolve 블록(4개) | Material 텍스처가 6개 이상으로 늘 때(AO·metallic·emissive 맵) | `MaterialTextureSlot` enum(= HLSL register) + 슬롯별 기본값 표 + 루프. 필수 슬롯은 "기본값이 무효인 슬롯"으로 표현 |
| Material cbuffer의 VS 바인딩 | PS `b2`에만 건다. UV 스케일은 선형이라 PS에서 곱해도 같다 | VS에서 텍스처를 읽을 때(높이 맵 VS 변위, 10-9) | `Renderer::DrawRenderItem`에서 VS에도 `b2`를 건다. 슬롯 규칙은 그대로(AGENTS §7) |

---

## 6. 바로 다음 작업

**로드맵 11번 완료 ✅ — HDR 씬 타깃, 톤 매핑, 선형 색공간, Bloom, 룩 재튜닝까지 전부 끝났다.**

완료한 단계: Step 1 렌더 타깃 생성(`74aebd8`) → Step 2 HDR 타깃 2-pass(`0e75485`) → Step 3 톤 매핑 + Exposure(`d5d36d7`) → Step 4 선형 색공간(`e17b870`) → 3-d ACES(`86f17a4`) → Rim 강조 이동(`765b68c`) → Step 5 밝은 부분 추출·디버그 뷰(`5ddb2ab`) → Step 6 분리형 블러(`80b825f`) → Step 7 합성(`4df455b`) → 블러 반복(`89f8d64`) → 룩 재튜닝(`050a1d9`). 설계 이유는 §4의 "HDR 씬 타깃, 후처리 패스와 선형 색공간", "Bloom 파이프라인", "룩 재튜닝", 기록은 §8 참고.

진행 순서는 그래픽스 강의 연계로 정한 **11번(HDR+Bloom) → 10번(Normal/Roughness+Fresnel+Cube Mapping+IBL) → 9번(Shadow Mapping)**을 그대로 따른다.

**단기 계획 — C안: 렌더링(PBR) 집중 (2026-10-29 사용자 결정)**

10-28에 정한 "씬 먼저(SC-1~SC-5)"는 브랜치 `SC3_MODEL_PIPELINE`에서 진행했다가 **폐기했다**(태그 `archive/sc3-model-pipeline`로 보관, §2·§8). 사용자가 강의 `Hong_Graphics_Part3_Chapter10~13` 전체를 근거로 삼아 **렌더링 효과, 특히 PBR을 직접 구현하는 데 집중**하기로 정했다. 씬은 지금의 그레이박스 골목(10-4 시점)을 시험대로 쓴다. 강의 근거 범위도 Ch11까지에서 **Ch10~13 전체**로 넓어졌다.

| 단계 | 강의 | 내용 | 완료 조건 |
|---|---|---|---|
| **P-1 IBL 정석 ← 바로 다음** | 12-1 UnrealPBR | NightSky의 **Specular(prefiltered)**로 반사를, **Diffuse(irradiance)**로 상수 ambient를 대체. `ComputeIndirectLighting`으로 간접광을 묶고 diffuse에 `(1 − F)`. BRDF 항은 `EnvBRDFApprox` 유지(LUT 파일이 없다) | 벽·바닥 그늘에 하늘색이 은은하게 들고, 웅덩이 반사가 roughness에 따라 GGX 모양으로 흐려진다. 상수 ambient를 끄고도 화면이 검게 죽지 않는다 |
| **P-2 metallic·AO·emissive 맵** | 12-2 PBRModels | `F0 = lerp(0.04, albedo, metallic)`(`float3`), `kd = (1 − F)(1 − metallic)`, AO는 간접광에만 곱함, emissive 맵. §5 예약 "F0 계열 `float3`화"·"`f90`"·"Material 텍스처 슬롯 enum" 트리거 | 금속 소품이 albedo 색으로 반사하고, AO가 틈을 어둡게 하되 직접광은 건드리지 않는다 |
| **P-3 깊이 버퍼 + 안개** | 13-3 DepthBufferAndFog | 깊이를 SRV로 읽을 수 있게 만들고(Typeless + DSV/SRV), 후처리에서 뷰 공간 거리로 안개. **톤 매핑 전 HDR**에서, 밤 골목 색으로 | 멀수록 골목 끝이 안개에 잠기고, 네온·Bloom이 안개 위에서 살아 있다 |
| **P-4 평면 반사** | 13-1 Mirror | 반사 행렬로 씬을 한 번 더 그려 웅덩이에 비춘다(10-7). 빛 쪽만 갈아 끼우는 구조(§4 "환경 반사") | 웅덩이에 네온과 벽이 실제로 비친다 |
| **P-5 PSO → 섀도 맵 → PCF → PCSS** | 13-2, 13-4~6 | 패스 상태 묶음(`GraphicsPSO`) 뒤 방향광·스포트 그림자 | 바닥에 그림자, 경계가 거리에 따라 부드러워진다 |
| **P-6 Sphere Light + Halo** | 13-7~8 | 네온관을 크기 있는 광원으로(Karis 2013 대표점 — 강의도 TODO), 광원 주변 광륜 | 하이라이트가 점이 아니라 관 모양이고, 안개 속 네온에 빛 번짐이 생긴다 |
| 이후 | 10 Geometry, 11-3 Height | 비(GS 빌보드), 높이 맵, Bloom 밉 체인 등 | |

진행 방식: 렌더링이 학습의 중심이라 **원래 규칙대로 에이전트가 가이드·리뷰하고 사용자가 작성**한다(AGENTS §2). 사용자가 원하면 예제 코드 + 각 변수의 의미와 개념(논문·상용 엔진 출처)을 연결한 설명을 준다(10-4에서 자주 요청한 방식). 강의 코드를 옮길 때 고칠 곳은 §7 "강의 PBR 자료 지도"의 목록을 먼저 본다.

P-3 안개와 P-4 평면 반사의 순서는 사용자가 정한다(둘 다 독립적이다). 씬 단계(로드맵 8)는 렌더링 단계가 일단락된 뒤 태그의 코드를 다시 꺼내 진행한다.

| 완성 조건 | 상태 |
|---|---|
| 시작 → 탐색 → 스위치 → 순차 점등이 끊김 없이 이어지는 1~2분 플레이 | 흐름 ✅, 시작 벽 통과 버그 남음 |
| 장면이 "골목"으로 읽히는 배치 (큐브 소품 + ambientCG 텍스처, 필요하면 Poly Haven CC0 모델) | ❌ — 렌더링(C안) 뒤. 태그 `archive/sc3-model-pipeline`에 1차 시도(양쪽 외관·소품·전선)가 있다 |
| **exe만으로 실행되는 빌드** (지금은 셰이더·에셋이 상대 경로라 작업 디렉터리 `MyPF/`가 필요) | ❌ |
| 시연 영상·스크린샷 | ❌ |
| README: 기술 하이라이트, 구조도, 실행 방법 | ❌ |


**다음에 이어서 할 작업 (순서)**

**작업 재개 체크리스트 (2026-10-23 기준)**: ① `Debug | x64` 빌드 후 실행해 `e9ab696` 상태(전원 Off의 파란 바닥 반사, On의 네온 색 줄무늬 반사)가 그대로인지 본다 ② §4 "Specular"·"Fresnel" 절과 `Lighting.hlsli`의 `ComputeDirectLighting`을 다시 읽는다 ③ §5 "확장 예약 목록"에서 이번 단계가 트리거인 항목을 먼저 확인한다 ④ 필요하면 `study-site/pipeline.html`로 PS 흐름을 복습한다.

그래픽스 강의의 기하 파이프라인 챕터를 녹일 자리는 §7 "강의 연계 아이디어"에 후보 목록으로 정리해뒀다 — 특히 **법선 시각화 디버그 툴은 로드맵 10번을 진행하는 동안 바로 써먹는다.**

**텍스처 색공간 선택 (로드맵 10번 선행 조건) — Step 1~3 전부 완료 ✅ (`ddb29c7` 경로, `0d0eac2` 캐시 키, `f91b0e6` 밉맵)**

왜 필요했는지: 모든 파일 텍스처가 `_SRGB`이면 노멀 맵을 넣었을 때 GPU가 디코드해 벡터가 왜곡된다. 평평한 면의 `(128, 128, 255)`가 `0.502^2.2 = 0.2195`가 되고 셰이더의 `N * 2 - 1`을 거치면 **0이어야 할 X·Y가 -0.561**(약 39° 기울어진 법선)이 된다. 셰이더에서 되돌릴 수 없는 이유는 sRGB 디코드가 **텍스처 필터링보다 먼저** 일어나기 때문이다. 구조와 결정 이유는 §3·§4의 "텍스처 색공간 선택" 참고.

- ✅ **Step 2 — 데이터 텍스처를 실제로 읽어 확인** (2026-09-21, 코드 변경 없음). 오른쪽 벽에만 `Data`로 읽은 벽 텍스처를 붙인 임시 Material(`testWallMat`, 새 이름 — `CreateMaterial`이 이름으로 캐시하고 세 벽이 `wallMat`을 공유하므로)을 전원 Off 상태(좌우 조명 동일)에서 비교해 **`Data` 쪽이 뿌옇게 밝게** 나오는 것을 확인했다. 디코드를 건너뛰면 셰이더가 감마 인코딩된 값을 그대로 받고 출력 인코딩이 한 번 더 밝히기 때문이다. 확인 후 임시 코드를 제거했다.

- ✅ **Step 3 — 밉맵** (`f91b0e6`, 2026-09-28). 모든 파일 텍스처가 `GenerateMips`로 전체 밉 체인을 갖는다. 구조는 §3, 결정 이유는 §4 "텍스처 색공간 선택", 기록은 §8.

1. **로드맵 10번 — 젖은 바닥 반사. ← 10-4까지 완료, 씬 단계(SC) 뒤 재개 (2026-10-28).**
   - **10-1 진행 상황 — 전부 완료 ✅** (설계 이유는 §4 "셰이더 구조 — 빛 쪽과 표면 쪽 분리", "Specular — 정규화된 Blinn-Phong과 재질값"):
     - ✅ 10-1a Material Roughness/Specular 값 경로 (`8e4cadc`)
     - ✅ A(R-1) `Common.hlsli` 신설, include 정리 (`8e4cadc`)
     - ✅ B(R-2) 카메라 cbuffer 공유, 슬롯 재배치 (`8e4cadc`)
     - ✅ C(R-3) 빛 쪽 정리 — C-1 Light cbuffer 이동, C-2 `IncidentLight`, C-3 `GetDirectionalLight`, C-4 `GetPointLight`와 `ComputePointLight` 삭제 (`f0273c2`, 화면 확인 완료)
     - ✅ D(R-4) 표면 쪽 정리 — `SurfaceData`, `ComputeDirectLighting`, `main`을 입력 → 직접광 → 간접광 → 연출 → 합성 순서로 (`b59242a`, 어셈블리 비교와 화면 확인)
     - ✅ E(10-1b) Blinn-Phong Specular — E-1 `viewDir` 전달, E-2 `SurfaceData`에 roughness/specular, E-3 `SpecularBlinnPhong`과 조립, 디버그 뷰로 모양 확인 (`e9ab696`)
     - ✅ F(10-1c) 정규화 항 **`(n + 8) / 8`** — 교과서의 `/(8π)`가 아니다. 우리 diffuse에 1/π가 없기 때문이다(§4) (`e9ab696`)
     - ✅ G(10-1d) 재질값 — 바닥 0.75 / 0.3 / 0.5, 벽 0.85 / 0.5, 스위치 0.4 / 0.5, 네온 0.3 / 0.5 (§3 표) (`e9ab696`)
   - ✅ **10-2 Fresnel** — 10-2a `FresnelSchlick`, 10-2b `ComputeDirectLighting` 조립(V·H, `F`, `1 − F`) (`6fd8742`), 10-2c 바닥 specular 0.5 → 0.25 (`04fab3c`). 화면 확인 완료. 설계 이유는 §4 "Fresnel — Schlick 근사".
   - **10-W 젖은 바닥 디테일 ← 진행 중 (2026-10-23 앞당김).** 균일한 바닥으로는 젖은 느낌과 반사 기능을 평가할 수 없어서(§5), 탄젠트 없이 할 수 있는 디테일을 GGX보다 먼저 한다.
     - ✅ **W-1 UV 타일링** (`ff34261`) — W-1a 데이터 경로 → W-1b PS `tiledUV` → W-1c `MakeCube` 옆면 UV 통일 → W-1d 재질별 값(0.5회/m)과 `endWallMat`. 화면 확인 완료. 설계 이유는 §4 "UV 타일링"
     - ✅ **W-2 ambientCG 아스팔트 + roughness 맵** (`9f5a1fa` 코드, `b0d253f` 에셋) — W-2a 에셋(`Asphalt023S` 2K, `CREDITS.md`, `PBR/.gitignore`, `white_1x1.png`) → W-2b `t1` 배선(기본 흰색, `roughnessFactor`, `MIN_PERCEPTUAL_ROUGHNESS`, 화면 그대로) → W-2c 바닥 연결(배율 0.45, UV (3.2, 16)). 화면 확인 완료. 바닥 BaseColor는 사용자가 ImGui로 맞춰 0.75 → **0.55**로 확정했다(`71feced` — ambientCG 아스팔트가 이전 텍스처보다 밝아 바닥이 벽보다 떠 보였다). 설계 이유는 §4 "roughness 맵과 기본 텍스처"
     - ✅ **10-6 탄젠트 + 노멀 맵 (W-3보다 앞당김, 2026-10-26)** (`88a0eb9` 코드, `5014670` 에셋) — N-1 텍스처 슬롯 헬퍼(`ResolveTextureSRV`) → N-2 정점 탄젠트 → N-3 노멀 맵 슬롯(`t2`, 기본 평평한 노멀) → N-4 바닥 NormalDX. 화면 확인 완료. 설계 이유는 §4 "탄젠트와 노멀 맵"
     - ✅ **N-5 벽 노멀 맵 (2026-10-26)** (`bd0272e` 코드, `fc397d8` 에셋) — N-5a ambientCG `Bricks096` 세트와 실제 크기 UV((10, 4)·(2, 4)) → N-5b 빛을 옮겨 옆면 탄젠트 세 줄 검증 → N-5c 룩(BaseColor (0.65, 0.67, 0.75), roughness 배율 1.0). 화면 확인 완료. 이유는 §4 "탄젠트와 노멀 맵"의 N-5 항목
     - ✅ **W-3 웅덩이 마스크 (2026-10-26)** (`5524d01` 코드, `f44ea8c` 에셋) — W-3a `t3` 경로와 기본 검정(화면 그대로, fallback 흰색으로 바인딩 증명) → W-3b 세 값 `lerp`(임시 `step` 마스크로 확인) → W-3c 직접 생성한 마스크를 바닥에 지정, 상수 0.15 / 0.75. 화면 확인 완료. 설계 이유는 §4 "웅덩이 마스크". **10-W(젖은 바닥 디테일)가 닫혔다**
     - W-4 노멀 맵은 탄젠트가 필요해 10-6으로 합친다. 병행: 반사에 비칠 Greybox 소품(쓰레기통·상자·배관) 몇 개
     - **에셋·소품 배치를 시작할 때 §5 "확장 예약 목록"의 월드 정렬 UV, 반복 깨기, `uvOffset`을 먼저 검토한다**(2026-10-23 사용자 결정). 상용 엔진의 타일링 관례(재질 단위 Tiling → 크기가 제각각이면 월드 정렬 → 반복이 보이면 데칼·매크로 변화)를 따른다.
   - **10번 나머지 순서 (2026-10-23 보강, 강의 11~13장 분석 반영 — 근거는 §7 "강의 PBR 자료 지도")**: GGX는 10-W 뒤로 간다. GGX의 장점(하이라이트의 긴 꼬리)은 매끈한 웅덩이 위에서 가장 잘 보이기 때문이다.
     - ✅ **10-2' GGX + Smith, π 규약 전환 (2026-10-27)** (`01f7d38` G-1, `fb58d71` G-3) — G-1 π 규약 전환(diffuse·specular `/π`, 직접광 세기 × π) + Unreal 형식 `D_GGX`·`Vis_SmithJointApprox`·`F_Schlick`·`SpecularGGX`와 A/B 스위치 → G-2 비교(`false` 화면 그대로, `true` 꼬리) → G-3 Blinn-Phong·스위치 삭제, 웅덩이 roughness 0.1. 화면 확인 완료. 설계 이유는 §4 "GGX + Smith와 π 규약 전환"
     - ✅ **10-3 DDS 로더 + Skybox(HDRI) (2026-10-27)** (`26a062a` S-1·S-2, `81d104c` S-3, `e805963` `SkyBox` 분리) — S-1 DDS 큐브맵 로드 경로(`CreateCubeMapTexture` → `InitializeCubeMap` → `LoadCubeMap`) → S-2 스카이박스 VS/PS·깊이 상태·`DrawSkybox`(임시 큐브맵) → S-3 Poly Haven Hansaplatz를 float DDS로 변환, `environmentIntensity` → 강의 Ch11 `CubeMapping` 기준으로 `SkyBox` 클래스 분리. 화면 확인 완료. 설계 이유는 §4 "Skybox와 환경맵 경로"
     - ✅ **10-4 환경 반사 (2026-10-28)** (`b6aee16` E-1·E-2, E-3 미커밋) — E-1a `t4` 배선(무효면 `nullptr`, 화면 그대로) → E-1b 반사 벡터로 크롬 골목(임시 출력) → E-2 roughness → 밉 LOD + Schlick, 빛 쪽 / 표면 쪽 함수 분리, `DielectricSpecularToF0` → E-3 `EnvBRDFApprox`. Horizon Occlusion은 가이드까지 하고 씬 단계 뒤로 보류(§5). 설계 이유는 §4 "환경 반사 — split-sum과 EnvBRDFApprox"
     - **다음은 P-1 IBL 정석** — 위 "C안" 표. 10-5(IBL)·10-8(metallic)·10-7(평면 반사)이 각각 P-1·P-2·P-4가 됐다
     - **10-5** IBL(split-sum) — `main`의 ambient 한 줄을 `ComputeIndirectLighting`으로, `PI` 사용. IBL용 Fresnel은 N·V
     - ~~10-6 탄젠트 + 노멀 맵~~ → W-3 앞으로 당겨 완료. AO 맵은 IBL(10-5)에서 간접광에 곱할 때 들인다
     - **10-7 ★ 평면 반사** — 13 Step1 Mirror(반사 행렬). 강의는 스텐실로 선명한 거울을 그리지만, 우리는 **반사 씬을 렌더 타깃에 그리고 roughness만큼 블러**(기존 Bloom 블러 재사용)해서 젖은 바닥에 쓴다. 골목 안 네온과 벽이 **실제로** 비치는 유일한 방법이라 효과가 가장 크다
     - **10-8** (선택) metallic — 금속 소품이 생길 때. `F_Schlick`을 `float3`로 바꾼다
     - **10-9** (선택) 높이 맵 — 평평한 큐브라 VS 변위는 안 맞는다. POM(PS) 또는 테셀레이션(11 Step3 HS/DS)
   - **10번 뒤**: 9 섀도 맵 → PCF → PCSS(13 Step4~6) → PSO/BlendState 정리(13 Step2) → 12 안개 + Halo(HDR에서, 13 Step3·7) · 비(10 Step1~2). 개선 항목: Bloom 밉 체인(Ch13 BloomDown/Up), 네온 튜브 면광원(13 Step8 대표점 — 강의도 TODO).
   - **노멀 맵 메모**: 밉에서 법선을 평균하면 길이가 1보다 짧아진다(방향이 다른 단위 벡터의 평균). 노멀 맵을 샘플한 뒤 셰이더에서 **반드시 `normalize`**한다.
   - **비등방 필터링**: 밉맵 뒤 먼 바닥이 약간 뭉개진다. 비스듬히 보는 바닥은 픽셀 하나가 덮는 영역이 골목 방향으로 길쭉해서, 트라이리니어가 긴 축 기준으로 레벨을 골라 옆 방향까지 과하게 흐려지기 때문이다. 샘플러를 `D3D11_FILTER_ANISOTROPIC` + `MaxAnisotropy` 8~16으로 바꾸면 개선된다. 지금은 거슬리지 않아 보류했고, 젖은 바닥 반사로 먼 바닥 디테일이 중요해지면 적용한다.
   - Cube Map/IBL 단계에서 큐브맵을 **DDS + DirectXTK `DDSTextureLoader`**로 읽고, `TextureType`이 `DDS_LOADER_FORCE_SRGB`/`IGNORE_SRGB`를 고르는 규칙을 같이 설계한다(§4 "텍스처 파이프라인 방향").
   - 바닥에 네온이 번지는 느낌은 지금 Point Light Specular에서 나오고, 정적 Cube Map은 골목 안 네온을 반사하지 못한다. 그 한계를 10-7 평면 반사가 메운다. Bloom과 HDR이 이미 있으므로 젖은 표면의 하이라이트가 곧바로 번진다.

**참고 — 로드맵 10번용 테스트 에셋 (2026-09-17 논의, 도입 보류)**

Poly Haven `rusty_metal_04`(CC0)를 검토했고 **지금 넣지 않기로 했다.** 스페큘러 항이 없는 상태에서 albedo만 넣으면 "녹슨 무늬가 인쇄된 콘크리트"가 되고 금속이라는 성질이 전혀 안 드러난다. 벽 albedo가 바뀌면 방금 확정한 룩 값도 흔들린다. 반대로 로드맵 10번에서 넣으면 기능이 켜질 때마다 화면이 바뀌는 것을 눈으로 확인할 수 있어 학습·포트폴리오 양쪽에 낫다고 판단했다.

그때 참고할 사실:

- 현재 씬은 Normal Map 효과가 잘 드러나는 조건이다. 네온이 벽에서 **15cm** 거리(Pink glow `x = -1.95`, 벽 `x = -2.1`)라 빛이 벽을 스치듯 지나가고(grazing angle), HDR·Bloom이 이미 있어 금속 하이라이트가 1을 넘으면 곧바로 번진다.
- 원본 배포본의 Normal/Roughness/Metallic은 **DWAA 압축 EXR**이다. `stb_image`는 EXR을 못 읽고 DWAA는 손실 압축이라, **Poly Haven에서 PNG로 다시 받는 것**이 변환보다 낫다.
- 파일명 `nor_gl`이 뜻하듯 **OpenGL 규약(Y+)** 노멀 맵이다. DirectX는 Y- 규약이라 **G 채널을 뒤집어야 한다**. 안 뒤집으면 요철이 반대로 보이는데 조명 탓으로 오인하기 쉽다.
- Displacement(height) 맵은 Parallax나 Tessellation이 있어야 쓴다. 로드맵에 없으므로 받지 않아도 된다.

**룩을 다시 조정하고 싶을 때의 순서** (값이 서로 영향을 주므로 이 순서를 지킨다)

1. Strength 0으로 두고 Exposure·Ambient로 기본 밝기를 잡는다.
2. Bright 뷰를 보며 Threshold — 네온과 Rim만 남기고 벽이 보이지 않게 한다.
3. Blur 뷰를 보며 Blur Iterations로 번짐 폭을 정한다.
4. Final로 돌아와 Strength를 0부터 올린다.
5. 네온 세 개의 Emissive 세기 균형(채널별 휘도 가중치는 §4 참고).
6. ACES ↔ Reinhard 최종 선택.

확정값을 옮겨 적을 위치는 §3 "룩 재튜닝"의 표에 정리돼 있다.

10번 다음이 9번(Shadow Mapping)이다.

**참고 — Shadow Mapping (로드맵 9번)**: 설계 가이드가 이미 나와 있다. 오늘 만든 렌더 타깃 기반과 RenderDoc 확인 방식을 재사용한다.

1. `GraphicsResourceManager`에 GPU 전용 Depth+SRV 겸용 텍스처 생성 기능 추가(`DXGI_FORMAT_R32_TYPELESS`로 만들어 DSV는 `D32_FLOAT`, SRV는 `R32_FLOAT`) — RTV+SRV 렌더 타깃 생성은 있지만 DSV 겸용 텍스처는 아직 없다.
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
8. 실제 골목 에셋 배치와 Scene 편집 보강 (2026-10-28에 앞당겨 브랜치에서 진행했다가 2026-10-29 폐기 — 태그 `archive/sc3-model-pipeline`. 렌더링 C안 뒤에 재개)
9. Shadow Mapping (그래픽스 강의 연계로 10·11번 다음 순서로 미룸 — §6 참고)
10. Normal/Roughness Material과 젖은 바닥 반사 ← **진행 중 — §6 "C안"(P-1 IBL → P-2 metallic → P-4 평면 반사)으로 이어감** (Fresnel·GGX·Cube Mapping·IBL·재질 맵·**평면 반사**를 여기 묶어서 진행. 선행 조건인 텍스처 색공간 선택·밉맵은 완료. 10-1 Specular(2026-09-30)·10-2 Fresnel(2026-10-23) 완료, 10-W 젖은 바닥 디테일 완료(W-1 UV 타일링, W-2 roughness 맵, 10-6 탄젠트·노멀 맵(앞당김), N-5 벽 노멀 맵, W-3 웅덩이 마스크 — 2026-10-26), 10-2' GGX + Smith·π 규약 전환 완료(2026-10-27), 10-3 DDS + Skybox 완료(2026-10-27), 10-4 환경 반사 완료(2026-10-28). 나머지는 §6 "C안"의 P-1~P-6 — 강의 근거는 아래 "강의 PBR 자료 지도")
11. ✅ HDR Scene Target, Bloom과 Tone Mapping (그래픽스 강의 연계로 9·10번보다 먼저 진행. HDR 씬 타깃·Exposure·Reinhard/ACES 톤 매핑·선형 색공간 2026-09-15, Bloom Step 5~7과 블러 반복·룩 재튜닝 2026-09-17에 완료)
12. 안개, 비와 색조 보정 (기하 파이프라인 챕터의 기하 셰이더·빌보드가 여기 본체다 — 아래 "강의 연계 아이디어" 참고)
13. 충돌/이동 제한, 디버그 UI와 최적화 (기본 플레이어-벽 충돌은 9번보다 먼저 앞당겨 완료 ✅ — `BoxCollisionComponent`/`PlayerCollision`. 이동 제한 나머지와 디버그 UI·최적화는 그대로 보류. 디버그 UI는 아래 "강의 연계 아이디어" A그룹으로 상당 부분 해결된다)
14. 라이선스 정리와 1~2분 최종 연출

### 강의 연계 아이디어 (기하 파이프라인 챕터 — 미착수 후보)

그래픽스 강의의 **기하 셰이더 / 빌보드 / 파이어볼 / 기하 분할(Tessellation)** 챕터를 포트폴리오에 녹일 자리를 미리 뽑아둔 목록이다(2026-09-18 논의). 로드맵 항목이 아니라 **후보 풀**이며, 작업하다 필요할 때 꺼내 쓴다. 필요 시점 기준으로 묶었다.

**선행 조건 — BlendState가 아직 없다.** 현재 렌더러는 전부 불투명 렌더링이고 `BlendState`/`OMSetBlendState`가 코드에 전혀 없다(2026-09-18 확인). 아래에서 **입자·연기·스파크 계열은 전부** 다음이 먼저 필요하다.

- 알파 블렌드 + 가산 블렌드 State 두 개
- 투명 패스 분리 — 불투명을 전부 그린 뒤 깊이 쓰기를 끄고 뒤에서 앞으로 정렬해 그린다
- 가산 블렌딩은 덧셈이라 순서를 타지 않으므로 정렬이 필요 없다(알파 블렌드만 정렬이 필요하다)

`D3D11Utils`에 `CreateGeometryShader`도 없고 Hull/Domain 경로도 없다.

**A. 선행 조건 없이 지금 할 수 있는 것** — 블렌딩이 필요 없어서 현재 구조 그대로 된다.

| 아이디어 | 기법 | 왜 지금 쓸모 있나 |
|---|---|---|
| **네온 예열 깜빡임** | 절차적 노이즈 | `PointLightSequence`가 네온을 1.4초 간격으로 **딱** 켠다. 실제 네온은 몇 번 지직거리다 안정된다. `runtimeEmissiveMultiplier`를 잠깐 흔들기만 하면 핵심 연출이 물리적으로 느껴진다. **노력 대비 효과 최대.** 단, §7의 "`PointLightSequence` 전제"대로 다른 코드가 상태를 건드리면 개수가 어긋나니 시퀀스 안에서 처리한다 |
| **Point Light 범위 와이어프레임** | 기하 셰이더 | `lightRange`를 ImGui에서 만지는데 그 범위가 화면에 안 보인다. 룩 재튜닝도 이걸 감으로 했다 |
| **콜라이더 박스 와이어프레임** | 기하 셰이더 | `BoxCollisionComponent`가 붙은 5개가 전부 안 보인다. 특히 `startWall`은 Mesh가 없는 투명 벽이라 존재 자체가 안 보인다 |
| **법선 시각화** | 기하 셰이더 | **로드맵 10번에서 반드시 필요해진다.** 아래 B 참고 |
| **스위치 상호작용 Ray 표시** | 기하 셰이더 | `CanInteract`의 Ray-BoundingBox 판정이 왜 안 잡히는지 볼 수 있다 |
| 단일 패스 와이어프레임 | 기하 셰이더 | 에디터 표시 모드 |

위 시각화는 전부 **"점/삼각형 → 선분 여러 개"**라는 같은 패턴이라 하나 만들면 나머지는 금방이다. 로드맵 13번의 "디버그 UI"가 상당 부분 여기서 해결된다.

**B. 로드맵 10번(젖은 바닥 반사) 진행 중에 쓸 것**

| 아이디어 | 기법 | 비고 |
|---|---|---|
| **법선/탄젠트 시각화** | 기하 셰이더 | 노멀 맵 버그는 "화면이 이상한데 원인을 모르겠다" 유형이다. `nor_gl`의 G 채널 미반전이나 sRGB 오독으로 법선이 약 39° 틀어진 것을 **눈으로 즉시 잡는다.** 강의 듣자마자 만들어두면 10번 내내 쓴다 |
| 물웅덩이 파문(실제 기하) | Tessellation | **실루엣이 변하는 것은 테셀레이션뿐**이다. 노멀 맵으로는 못 한다 |
| `disp` 맵 활용 | Tessellation | §6의 "테스트 에셋" 항목에서 "쓸 데 없다"고 적어둔 displacement 맵을 쓸 수 있게 된다 |
| 거리 기반 LOD | Tessellation | 포트폴리오 항목으로 쓸 만하다 |

테셀레이션은 이 씬에서 **실익이 가장 적다.** 벽·바닥이 평면이라 노멀 맵이 훨씬 싸게 90%를 준다. 물웅덩이 파문만 고유한 가치가 있다.

**C. 로드맵 12번(안개·비)에서 쓸 것** — 대부분 BlendState 선행

| 아이디어 | 기법 | 비고 |
|---|---|---|
| **비** | 기하 셰이더 + 빌보드 | 12번 본체. 점 하나 → 카메라를 향한 길쭉한 쿼드. 늘어나는 방향은 카메라가 아니라 **속도**를 따라야 빗줄기로 보인다 |
| 빗방울 착지 물보라 | 빌보드 | 비와 바닥을 연결한다 |
| **환기구 수증기** | 빌보드 | 사이버펑크 골목 필수 요소 |
| **빛 속 먼지** | 빌보드 | HDR·Bloom이 이미 있어 밝은 입자가 **자동으로 번진다** |
| 떠다니는 재·불티 | 빌보드 | 블레이드 러너 질감 |
| 광원 halo | 빌보드 | 안개가 들어온 뒤. Bloom과 역할이 겹치니 주의 |

**D. 연출을 채울 때 꺼낼 것** (로드맵 14번 최종 연출 전후)

| 아이디어 | 기법 | 비고 |
|---|---|---|
| **스위치 스파크 + 순간 섬광** | 파이어볼(절차적+가산) | 데모에서 **플레이어가 행동하는 유일한 순간**이다. 지금은 E를 누르면 색만 바뀐다 |
| **머리 위 케이블** | 기하 셰이더(선분→튜브) | 골목 위가 뻥 뚫려 휑하다. 전선 몇 가닥이 실루엣을 만든다. 로드맵 9번(그림자) 뒤에는 **전선 그림자가 벽에 드리운다** |
| **네온 주위를 도는 나방** | 빌보드 | 실제 네온은 벌레를 끈다. 작지만 기억에 남는다 |
| 드럼통 불 | 파이어볼 + Point Light | 차가운 네온과 대비되는 따뜻한 광원. 흔들리는 빛이 정적인 씬에 생명을 준다 |
| 고장난 네온 지직거림 | 절차적 노이즈 | 네온 하나만 불안정하게 |
| 노출 배선 아크 | 파이어볼 | 스위치 주변 위험감 |
| 전원 켤 때 케이블 따라 흐르는 펄스 | 절차적 노이즈 | 스위치와 네온의 인과관계를 시각화 |
| 아스팔트 틈의 잡초 | 기하 셰이더 | 방치된 골목의 퇴락감 |
| 골목 위로 보이는 먼 도시 불빛 | 빌보드 | 지오메트리 없이 깊이감 |
| 홀로그램 광고판 글리치 | 파이어볼 | 사이버펑크 클리셰 |

**Point Light 슬롯 여유**: 상한 8개 중 현재 **4개만 쓴다**(네온 3 + `EnvironmentFillLight` 1). 드럼통 불, 스파크 섬광, 환기구 아래 조명 같은 것을 **4개까지 더** 상한 변경 없이 넣을 수 있다.

**기하 셰이더의 성능 평판**: 출력 개수가 가변이라 결과를 버퍼링해야 하고 병렬성이 떨어져 현대 GPU에서 느리다는 평가가 있다. 실무는 인스턴싱이나 컴퓨트 셰이더로 대체하는 추세다. 이 규모에서는 문제없고 학습용으로는 개념이 명확하지만, **왜 요즘 잘 안 쓰는지를 같이 알아두면 면접에서 오히려 가점이다.**

### 강의 PBR 자료 지도 (11~13장, 2026-10-23 분석)

렌더링 품질을 최대한 끌어올리기 위해 강의 폴더(`Hong_Graphics2/Graphics_Part2` 04~08장, `Hong_Graphics3/HongGraphicsPart3` 09장, `Hong_Graphics_Part3_Chapter10~13`)를 전부 훑었다. 셰이더 코드는 대부분 `13_LightAndShadow_Step8_UnrealSphereLight`가 가장 완성된 형태다. 아래 경로는 `Hong_Graphics_Part3_Chapter10~13` 기준이다.

| 강의 단계 | 기법 | MyPF 적용 | 참고 위치 |
|---|---|---|---|
| Part2 08 Step2~5 | 큐브맵, 환경 매핑, IBL, Fresnel(입문판) | 10-2~10-5 복습 | |
| 11 Step2 NormalMapping | 탄젠트, TBN, GL/DX 노멀 전환 | 10-6 | 13 Step8 `BasicPS.hlsl` `GetNormal`(43~65행, `invertNormalMapY`) |
| 11 Step3 HeightMapping | VS 변위, 테셀레이션 HS/DS | 10-9(선택) | `BasicVertexShader.hlsl`, `TessellatedQuadHS/DS.hlsl` |
| 11 Step4 HDRI | DDS 큐브맵 로드, float 텍스처 | 10-3 | `D3D11Utils.cpp` `CreateDDSTextureFromFileEx`(483행) |
| 11 Step5 HDRPipeline | float 렌더 타깃 | ✅ 완료(로드맵 11) | |
| 12 Step1 UnrealPBR | GGX, Smith, metallic | 10-2', 10-8 | 13 Step8 `BasicPS.hlsl` `NdfGGX`(101), `SchlickGGX`(117), 조립(301~330) |
| 12 Step2 PBRModels | glTF PBR 텍스처 로딩 | 10-6 / 모델 파이프라인 | metallicRoughness 묶음(G = roughness, B = metallic) |
| 13 Step1 Mirror | 평면 반사(반사 행렬 + 스텐실) | **10-7** | `ExampleApp.cpp` `Matrix::CreateReflection`(262), 바닥을 거울로(59) |
| 13 Step2 PSO | 상태 묶음 `GraphicsPSO` | 반사·그림자·투명 패스 전 | `GraphicsCommon.cpp` |
| 13 Step3 DepthBufferAndFog | 깊이 → 뷰 공간 복원, 거리 안개 | 12 | `PostEffectsPS.hlsl` `TexcoordToView` |
| 13 Step4~6 | 섀도 맵 → PCF → PCSS | 9 | `BasicPS.hlsl` `PCF_Filter`(146), `PCSS`(185) |
| 13 Step7 Halo | 광원 주변 볼륨 광륜(Ray-Sphere 적분) | 12 | `PostEffectsPS.hlsl` `HaloEmission`(63) |
| 13 Step8 UnrealSphereLight | 구형 면광원 대표점 | 네온 튜브 면광원(개선) | `BasicPS.hlsl:293` — **강의도 TODO** |
| Ch13 Bloom | 다단계 Down/Up 체인 | Bloom 개선(§5) | `BloomDownPS.hlsl`, `BloomUpPS.hlsl` |
| 10 Step1~5 | 기하 셰이더, 빌보드, 법선 선, 파이어볼, 테셀레이션 | 12 비·입자, 디버그 도구 | 위 "강의 연계 아이디어" A~D |

**가져올 때 그대로 옮기면 안 되는 곳** (코드를 직접 읽고 확인)

1. **π 규약이 섞여 있다.** `NdfGGX`는 `/π`(107행)인데 diffuse는 `kd * albedo`(311행)로 π가 없다. 그래서 강의 화면은 specular가 diffuse 대비 3.14배 약하다. 10-2'에서 우리 쪽을 정석(diffuse = albedo / π)으로 바꿨으므로 **강의 D항은 그대로 쓰고 diffuse만 `/π`로** 보면 된다.
2. **후처리가 LDR에서 돈다.** `PostEffectsPS.hlsl:99`가 `clamp(0, 1)` 뒤에 Halo와 안개를 더하고, 안개 색은 흰색 하드코딩이다. 우리는 **톤 매핑 전 HDR**에서, 밤 골목 색으로 한다.
3. **Halo가 `lights[1]` 하나만 본다.** Point Light 배열 전체로 일반화한다.
4. **구형 면광원 대표점이 미완성(TODO)이다.** 네온 튜브를 면광원으로 보이게 하려면 Karis 2013 방식을 직접 구현한다 — 포트폴리오 차별점이 될 수 있다.
5. **IBL 매직 넘버.** `SpecularIBL`의 밉 `2 + roughness * 5`와 LUT 좌표 `1 - roughness`는 강의가 쓴 IBLBaker 출력 기준이다. 다른 도구(CMFT, cmgen)로 만들면 밉 개수와 LUT 방향을 다시 확인한다.
6. **Fresnel 매개변수 이름.** Ch13 `SchlickFresnel(F0, NdotH)`는 이름과 달리 직접광에서 **V·H**(309행), IBL에서 **N·V**(71행)를 받는다.
7. **(2026-10-29 추가) 12 Step2 직접광 루프는 `lightVec`을 정규화하지 않는다.** `light[i].position - posWorld`를 그대로 N·L과 하프 벡터(`normalize(pixelToEye + lightVec)`)에 쓴다 — 거리가 1m를 넘으면 N·L이 1을 넘고 하프 벡터가 빛 쪽으로 쏠린다. Ch13 `LightRadiance`에서는 고쳐져 있다. MyPF 직접광(Unreal 형식 GGX + Smith, `ComputeDirectLighting`)이 이미 더 정확하므로 **12장에서는 IBL·metallic·AO 부분만** 가져온다.
8. **(2026-10-29 추가) IBL 입력 = 사용자가 IBL Baker로 구운 NightSky.** Specular(512², 밉 10, prefiltered)·Diffuse(128², irradiance)·Env(하늘)가 있고 BRDF LUT는 없다(`HDRI/CREDITS.md`). 그래서 P-1에서는 항 ②를 LUT 대신 `EnvBRDFApprox`로 둔다. 강의의 Diffuse IBL은 `kd * albedo * irradiance`로 π가 없는데, irradiance 맵이 "반구 적분 ÷ π"를 이미 담고 있는지(IBL Baker 규약) 화면 밝기로 확인한 뒤 우리 정석 π 규약과 맞춘다.
9. **(2026-10-29 추가) Diffuse IBL의 kd.** 강의는 `kd = lerp(1 − F, 0, metallic)`이고 F를 **N·V**의 Schlick으로 구한다 — roughness를 모르는 Schlick이라 10-4에서 본 "거친 면이 스치는 각도에서 번들거림"이 diffuse 쪽에서는 반대로(diffuse가 지나치게 꺼짐) 나타날 수 있다. P-1에서 `EnvBRDFApprox`의 F 값을 함께 쓸지 비교한다.

**사이버펑크 젖은 골목에서 효과가 큰 순서**: ① 평면 반사 + roughness 블러(바닥이 평면 하나라 조건이 완벽하고, 골목 안 네온이 실제로 비친다) ② Normal + Roughness 맵(젖은 정도가 픽셀마다 다르다) ③ GGX + Smith(하이라이트의 긴 꼬리) ④ 그림자 PCF → PCSS ⑤ 안개 + Halo(HDR) ⑥ IBL(골목이 좁아 효과는 상대적으로 작다) ⑦ Bloom 밉 체인. 기법은 "가장 고급"이 아니라 **"이 씬의 조건에 가장 맞는 것"**으로 고른다는 판단이 평면 반사를 1순위로 둔 근거다. (2026-10-29: 사용자가 단기 목표를 "PBR을 강의대로 직접 구현"으로 정해, 실제 순서는 §6 "C안"처럼 IBL → metallic을 앞세운다. 이 목록은 데모 화면 기준의 참고 순위로 남긴다.)

### 에셋 정책 (2026-10-23 라이선스 확인)

| 출처 | 라이선스 | 공개 저장소 커밋 | 사용 |
|---|---|---|---|
| **ambientCG** | **CC0** (공식 라이선스 페이지: 복사·수정·배포·상업 사용 허용, 출처 표기 불필요) | ✅ | **주력** |
| **Poly Haven** | CC0 | ✅ | 텍스처 + **HDRI**(10-3) |
| **FreePBR** | 무료는 비상업용, 상업 권한은 유료($19). 작성자가 **"다른 사이트에 재배포하지 말 것"**을 요청(About 페이지) | ❌ 원본 파일을 공개 GitHub에 올리면 이 요청에 어긋난다 | 로컬 실험용만(`.gitignore`) 또는 쓰지 않는다 |
| 직접 제작 | 본인 | ✅ | **네온 간판 글자 emissive 마스크**처럼 사이트에서 구하기 어려운 것 |

참고로 강의 에셋 `Assets/Textures/PBR/Bricks075A_4K`는 ambientCG, `patchy-meadow1`은 FreePBR이다.

**ambientCG 파일 → `TextureType`**

| 파일 | `TextureType` | 주의 |
|---|---|---|
| `_Color` | `Albedo` | sRGB |
| `_NormalDX` | `NormalMap` | **`_NormalGL`이 아니라 DX를 받는다.** G 채널 뒤집기가 필요 없다 |
| `_Roughness`, `_AmbientOcclusion`, `_Metalness`, `_Displacement` | `Data` | 선형. 처음엔 분리 텍스처로 쓰고, 쿠킹 단계(§4 "텍스처 파이프라인 방향")에서 ORM 한 장으로 묶는다 |

**저장소 관리**: 4K가 아니라 **2K**로 받는다(골목 크기·화면 해상도에 충분, 용량 1/4). `MyPF/Assets/Textures/PBR/<에셋ID>/`로 나누고, 출처와 라이선스를 적은 `CREDITS.md`를 함께 둔다. 에셋이 수십 MB를 넘기면 Git LFS를 검토한다.

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

### 2026-10-28 ~ 29 — 씬 작업 브랜치 (폐기, 태그 `archive/sc3-model-pipeline` 보관)

- 한 일: "씬 먼저" 계획으로 `SC3_MODEL_PIPELINE` 브랜치에서 커밋 10개를 진행했다. R-1·R-2는 사용자가 참고 코드를 보고 작성, SC-3 이후는 **사용자 요청으로 에이전트가 직접 구현·빌드**하고 사용자가 화면을 확인했다.
  - R-1·R-2 (`32b4bbb`, 사용자): `InitGreyBoxScene` 재작성 — 바닥 중심 = 원점, `AlleyLayout`(기준 치수 → 유도 함수), `GreyBoxDesc`/`GreyBoxFactory`·`SurfaceMaterialDesc`·`PbrTextureSet`, 양쪽 인도(연석 0.15m, 1.5m), 차도 Poly Haven `asphalt_02`(3m), 인도 `concrete_pavement`(1.8m)
  - SC-3 (`9fa1e1b`·`2d39e41`): `ModelLoader`가 aiNode 트리를 순회해 부품 이름·누적 변환(Assimp 열 벡터 → SimpleMath 전치는 `ToMatrix` 한 곳), `aiProcess_CalcTangentSpace`, 노멀·roughness(`DIFFUSE_ROUGHNESS`) 경로. `ModelPart`에 `name`·`localTransform`(월드 = local × 오브젝트), `AssetManager::CreateModelFromParts`(키트에서 부품 꺼내기, 첫 노드 기준으로 재배치). `tools/cook_gltf_dx_normals.ps1`(glTF OpenGL 노멀 → DirectX로 굽고 `_dx.gltf` 사본). 씬 PS가 roughness를 **G 채널**에서 읽음(glTF ARM과 흑백 맵 공용)
  - 외관 (`5e6ca80`·`ac2f713`): `FacadeDesc`(origin·yaw·줄별 모듈) + `FacadeFactory`, `ModelPlacement` + `CreateCompositeModel`로 건물 하나 = GameObject 하나(Unreal Packed Level Actor 방식)
  - 꾸미기 (`2d17124`·`aac81df`): Poly Haven Hidden Alley 컬렉션(공장 외관 키트, 셔터, 실외기, 벽등, 배관, 전봇대). `ModelLoadOptions::skipTranslucentParts`(BLEND 재질 Part 제외), 재질 이름으로 전구만 발광, `FacadeDesc::attachments`, `StreetPropFactory`, `GeometryGenerator::MakeCable`(포물선 처짐 원통) 전선 11가닥
  - 로딩 (`00d0644`): 임시 계측으로 Debug 씬 로드 9.0초 중 텍스처 51장이 7.0초임을 확인 → `stb_image` 구현부를 `StbImage.cpp`로 옮겨 **Debug에서도 `/O2`·런타임 검사 끔** → 텍스처 2.1초, 씬 4.2초(Release 2.4초)
- 폐기 이유: 사용자가 단기 목표를 "강의 10~13장을 근거로 렌더링, 특히 PBR 구현"으로 바꿨다(§6 "C안"). 브랜치는 로컬 태그로만 보관하고 로컬·원격 브랜치는 지웠다. NightSky HDRI 세 파일만 `main`으로 가져왔다(스테이징).
- **나중에 다시 쓸 것** (씬 단계로 돌아갈 때 태그에서 꺼낸다): 모델 파이프라인 전체(노드 변환·탄젠트·PBR 맵·부품 꺼내기·합치기), 노멀 굽기 도구, `GreyBoxFactory`·`AlleyLayout`, `FacadeFactory`, `MakeCable`, `StbImage.cpp` 분리. 렌더링 단계에서 먼저 필요해질 수 있는 것: **stb_image 최적화**(텍스처가 늘면 바로 효과), roughness G 채널 규약(P-2에서 ARM 패킹 맵을 쓸 때).
- 리뷰·실행 중 배운 것: ① SimpleMath를 헤더로만 써서 `Matrix::Identity`·`Zero` 같은 정적 상수는 `SimpleMath.cpp`에 있어 **링크되지 않는다**(LNK2001) — `Matrix()`가 단위 행렬 ② glTF 노멀 맵은 규격상 OpenGL 규약이다 ③ Poly Haven glTF는 노드에 이동만 있고 앞면이 원본 +Z(왼손 변환 후 −Z)다 ④ VS에서 실행 중인 exe는 LNK1168로 덮어쓸 수 없다 — 측정용 빌드는 `OutDir`·`IntDir`를 따로 준다.

### 2026-10-28 — 큐브맵 환경 반사 (10-4) ✅, 계획 변경: 씬 먼저

- 완료한 작업 (전부 사용자 작성, 에이전트는 설계 가이드·예제 코드·리뷰):
  - E-1a (`b6aee16`): `DrawRenderItem`의 `t4` 바인딩(유효할 때만 `GetSRV`, 아니면 `nullptr`), `pixelSRVs[5]`·개수 5, HLSL `TextureCube environmentMap : register(t4)`
  - E-1b: `reflect(−viewDir, surfaceData.normal)`로 큐브맵을 읽어 임시 출력 — 크롬 골목 확인 후 삭제
  - E-2 (`b6aee16`): 사용자가 "확장성을 고려해 메서드로" 요청 → `DielectricSpecularToF0`, `ComputeEnvironmentLOD`, `SampleEnvironmentRadiance`(빛 쪽), `ComputeEnvironmentSpecular`(표면 쪽)로 분리, `ComputeDirectLighting`의 F0도 공용 함수로. `finalColor`에 `environmentSpecular`
  - E-3 (미커밋, 사용자가 커밋 예정): 사용자가 "바닥이 금속처럼 보인다"고 관찰 → `EnvBRDFApprox`로 교체. 출처(Karis 2013 split-sum, Karis 2014, Lazarov 2013)와 각 줄의 의미를 연결한 설명을 사용자가 요청했다
- 확인한 결과: 각 단계 `fxc /T ps_5_0`으로 씬 PS·스카이박스 PS 컴파일(경고 0개), E-1b·E-2는 사용자 화면 확인. 2026-10-28 사용자 요청으로 에이전트가 **빌드 없이** 실행·캡처(exe가 마지막 셰이더 수정보다 새로웠음)해 레퍼런스 사이버펑크 씬과 비교 — 벽 은박지 무늬, 밝은 광장 하늘, 네온 반사 부재(§4)
  - 리뷰 중 잡은 문제: ① 웅덩이 마스크 블록을 복사한 `ResolveTextureSRV(puddleMaskTexture, …)` + `return false` ② `if (!IsValid())` 조건 반전 + 자기 자신을 검사하는 조건 연산자(둘 다 `nullptr`로 수렴) ③ `PSSetShaderResources` 개수 4 ④ `Texture2D`로 선언(큐브 SRV와 차원 불일치 — 쓰지 않는 선언이라 컴파일러가 지워 증상이 숨음) ⑤ 반사를 계산만 하고 출력하지 않음(죽은 코드 제거) ⑥ `Sample` 대신 `SampleLevel`, `float4` 대신 `float3` ⑦ `EnvBRDFApprox`의 `const float c0 = float4(…)` — X3206, 모든 성분이 `1 − roughness`가 되어 정면 반사율 약 1.84 ⑧ 요청 밖 `F_Schlick`의 `pow` 변경(되돌림)
- 계획 변경: 10-4 뒤 Horizon Occlusion 가이드·예제까지 했으나, 사용자가 **"최종 결과물이 목표이니 구도와 작은 사이버펑크 씬을 먼저, 그 뒤 필요한 작업을 하나씩"**으로 결정. Horizon Occlusion은 §5 예약, 로드맵 8번(씬)을 10-7보다 앞으로. 강의 Ch11 파이프라인과의 비교(골격 일치, MSAA·diffuse IBL·밉 체인 Bloom·`ImageFilter` 차이)도 이날 했다 — 앞의 둘은 §5 예약, 뒤의 둘은 기존 §5 항목
- 남아 있는 문제: `hansaplatz_2k.hdr` 추적 해제(§2). `Renderer.cpp` 환경맵 `nullptr` 블록 위 의도 주석이 비어 있음(빈 줄). `F_Schlick`·`EnvBRDFApprox` 주석(§5). EditorUI ambient 슬라이더 주석 복원, 스카이박스 셰이더 `ExcludedFromBuild`(10-3에서 이어짐)
- 다음에 이어서 할 작업: ~~SC-1 구도 확정~~ → 씬 작업은 브랜치에서 진행 후 폐기(바로 위 항목), 2026-10-29부터 §6 "C안" P-1 IBL.
- 중요한 설계 결정과 이유: §4 "환경 반사 — split-sum과 EnvBRDFApprox".

### 2026-10-27 — DDS 큐브맵 + Skybox(HDRI), `SkyBox` 분리 (10-3) ✅

- 완료한 작업 (전부 사용자 작성·커밋, 에이전트는 설계 가이드·참고 코드·리뷰):
  - S-1·S-2 (`26a062a`): `GraphicsResourceManager::CreateCubeMapTexture`(DirectXTK DDS 로더), `Texture::InitializeCubeMap`, `AssetManager::LoadCubeMap`(별도 캐시), `FrameRenderData.environmentMap`, 스카이박스 VS/PS, 깊이 상태(LESS_EQUAL + 쓰기 없음), `Renderer::DrawSkybox`(`EndScene` 시작)
  - S-3 (`81d104c`): Poly Haven Hansaplatz 2K → `hansaplatz_cube512.dds`(에이전트가 Node.js 스크립트로 변환), `HDRI/CREDITS.md`·`.gitignore`, `environmentIntensity`(Light cbuffer 패딩 자리, `DirectionalLight` 0.25, EditorUI 로그 슬라이더), `Cubemaps/` 임시 폴더 정리
  - `SkyBox` 분리 (`e805963`): 사용자가 데이터 단위 묶음 후보를 물어 에이전트가 분석 — 처음 제안한 Material 슬롯 enum·Ch13 `GraphicsPSO`는 사용자가 "강의 Ch11까지 기준"으로 정해 철회, Ch11 `CubeMapping` 대응으로 `SkyBox` 클래스(자기 VS/PS/IL/깊이 상태/큐브 버퍼, 환경맵·카메라 CB·샘플러는 받음)를 선정. `Renderer`는 "언제·무엇으로"만 남김
- 확인한 결과: 사용자 화면 확인 — 골목 위 어두운 하늘 띠, 물체가 하늘을 가림, `Environment Intensity` 0에서 검정, 분리 전후 동일.
  - 리뷰 중 잡은 문제: ① VS `.xyzw` 오타로 하늘이 씬 전체를 덮음(에이전트가 사용자 요청으로 실행·캡처해 확인) ② 큐브맵 경로 `Cubemaps\\skybox\\` 오타 — 선택 입력이라 조용히 배경색 ③ `.gitignore`에도 불구하고 원본 `.hdr`가 커밋됨(§2) ④ `SkyBox.cpp`/`.h`가 vcxproj에 없어 링크 실패 예정 ⑤ `Renderer.h`에 옮긴 멤버 3개 잔존
- 남아 있는 문제: `hansaplatz_2k.hdr` 추적 해제(§2). EditorUI ambient 슬라이더 위 주석이 S-3 중 지워짐(복원 권장). 스카이박스 셰이더 `ExcludedFromBuild` 누락(§5). §5 예약 — `SkyLight` 분리, 환경맵 로드 실패 경고, HDRI 변환 스크립트 보관, `GraphicsPSO`, Material 슬롯 enum. 시작 벽 통과(N-5 항목, 10-7 단계에서).
- 다음에 이어서 할 작업: **10-4 환경 반사**.
- 중요한 설계 결정과 이유: §4 "Skybox와 환경맵 경로".

### 2026-10-27 — GGX + Smith, π 규약 전환 (10-2') ✅

- 완료한 작업:
  - G-1 (`01f7d38`, 사용자 커밋): π 규약 전환 — diffuse `albedo × (1 − F) / π`, Blinn-Phong `(n + 8) / (8π)`, 직접광 세기 5곳 `× DirectX::XM_PI`(ambient·Emissive·Rim 제외). Unreal 형식 `D_GGX(a2, NdotH)`·`Vis_SmithJointApprox(a2, NdotV, NdotL)`·`F_Schlick`(이름 변경)·`SpecularGGX`(D·Vis, F 제외), 입구 roughness 최솟값, `NdotV = saturate(abs(N·V) + 1e-5)`, 임시 `USE_GGX` 스위치. 사용자가 "에이전트가 참고 코드를 쓰고 사용자가 보고 직접 작성"하는 방식을 요청해, 에이전트가 함수별 변경 설명과 함께 참고 코드를 제시했다(소스는 사용자가 작성)
  - G-2: `USE_GGX = false`(화면 그대로 → π 전환 확인), `true`(꼬리 → GGX 확인) 사용자 화면 확인
  - G-3 (`fb58d71`, 사용자 커밋): `SpecularBlinnPhong`·`USE_GGX` 삭제, 웅덩이 `PUDDLE_ROUGHNESS` 0.15 → 0.1(사용자). 주석은 사용자 요청으로 에이전트가 갱신(`MIN_PERCEPTUAL_ROUGHNESS`, `D_GGX`·`Vis`·`SpecularGGX` 출처와 의도, 웅덩이 근거, `DirectionalLight.h` 블록 복원과 π 규약 설명)
- 확인한 결과: `fxc` 경고 없음, 사용자 화면 확인.
  - 리뷰 중 잡은 문제: ① `DistributionGGX`의 `max(d², 1e-7)`가 roughness < 0.13에서 하이라이트 중심을 깎음 ② N·V 조기 반환이 diffuse까지 꺼서 노멀 맵 바닥에 검은 점(+ `false` 화면도 바뀜) ③ G 형태 `D·F·G / (4·N·V·N·L)`의 0 / 0 = NaN ④ 삭제된 함수를 가리키는 주석, 값과 반대되는 웅덩이 근거 주석
- 남아 있는 문제: 이 문서 갱신이 미커밋(사용자가 "문서만 갱신" 요청). §5 — `Pow4`·`Pow5`, `BRDFData`/`BxDFContext`, IBL용 Smith 재매핑(k = α / 2), 웅덩이 albedo 재튜닝(10-5). 시작 벽 통과(N-5 항목).
- 다음에 이어서 할 작업: **10-3 DDS 로더 + Skybox(HDRI)**.
- 중요한 설계 결정과 이유: §4 "GGX + Smith와 π 규약 전환".

### 2026-10-26 — 웅덩이 마스크 (W-3) ✅ — 10-W 완료

- 완료한 작업 (`5524d01` 코드, `f44ea8c` 에셋):
  - W-3a `Material::m_puddleMaskTexture`, `Renderer` 기본 검정(`black_1x1.png`, 에이전트 생성)과 `t3` 바인딩(`ResolveTextureSRV`), PS 선언·샘플(`input.uv`). 사용자가 Material 생성자 초기화를 중괄호로 통일
  - W-3b PS 세 값 `lerp` — `normalTS` → `(0, 0, 1)`, albedo × `PUDDLE_ALBEDO_SCALE`, roughness → `PUDDLE_ROUGHNESS`
  - W-3c 마스크 `Masks/alley_floor_puddle_mask.png`(에이전트가 스크립트로 생성, 시안 3번 — 벽 물줄기 끊기, 웅덩이 분리, 면적 18% → 22%)와 `Masks/CREDITS.md`, `AppBase`가 `Data`로 로드해 `floorMat`에 지정(사용자). 상수 튜닝(사용자 요청으로 에이전트가 캡처 비교, 근거 주석 포함)
- 확인한 결과: `fxc` 경고 없음. 사용자 화면 확인 — W-3a 화면 그대로·검정/흰색 디버그 정상, W-3b 임시 마스크로 바닥 절반만 어둡고 평평하고 하이라이트가 날카로움, W-3c 웅덩이 위치가 지도와 일치하고 벽은 변화 없음.
  - 리뷰 중 잡은 문제: ① albedo `lerp`의 양 끝이 섞여 마스크 0이 흰색 ② 노멀 보간 누락(두 번째 리뷰에서도 저장 전)
- 남아 있는 문제: §5 예약 — 웅덩이 값 재튜닝(10-5·10-2'), 웅덩이 상수 cbuffer화·F0 보간, 마스크 생성 스크립트 보관. 시작 벽 통과(N-5 항목).
- 다음에 이어서 할 작업: **10-2' GGX + Smith**. 그 전에 시작 벽 통과를 짧게 고칠지 사용자가 정한다.
- 중요한 설계 결정과 이유: §4 "웅덩이 마스크".

### 2026-10-26 — 벽 노멀 맵 (N-5) ✅

- 완료한 작업 (`bd0272e` 코드, `fc397d8` 에셋):
  - N-5a 벽 텍스처를 ambientCG `Bricks096`(Color·Roughness·NormalDX, 2048×1024, CC0)로 교체해 `wallMat`·`endWallMat`에 지정(사용자). UV Scale을 2m × 1m 추정 크기 기준으로 (10, 4)·(2, 4)(사용자 요청으로 에이전트가 수정·주석)
  - N-5b Editor에서 빛 하나를 옮겨 세 벽(+X·−X·−Z 면)의 탄젠트 부호 검증 — 코드 변경 없음
  - N-5c 룩 값 BaseColor (0.65, 0.67, 0.75)·roughness 배율 1.0(사용자 요청으로 에이전트가 빌드·캡처하며 후보 3개 비교, 근거 주석 포함)
  - `PBR/CREDITS.md`에 Bricks096 행·크기 추정 근거·파일명 변경 기록(에이전트). `.gitignore`는 기존 패턴이 하위 폴더에 적용돼 수정 없음
- 확인한 결과: 사용자 화면 확인 — 돌 비율 정상, 세 벽 요철 방향 정상, 네온 ON에서 간판이 벽보다 돋보이고 OFF에서 돌 무늬가 보임.
  - 리뷰 중 잡은 문제: ① 경로에 `-JPG` 누락(−1 종료) ② `CyberpunkAlley` 폴더 삭제로 쓰는 중인 네온·스위치 텍스처까지 사라짐(복구) ③ 디스크 파일명을 절반만 바꿈 ④ `endWallMat` 미연결 ⑤ UV Scale을 정사각 텍스처 값 그대로 둠
- **시작 벽 통과가 재현됐다.** 자동 캡처에서 Play 직후 창 포커스가 빠져 W가 먹지 않고 S(3초 × 4m/s)만 적용되자, 카메라가 `startWall`(z = −0.1, 두께 0.2) 뒤로 빠져나가 골목을 밖에서 보는 장면이 다시 찍혔다. 시작 위치 z = 0에서 **뒤로 걷기만 해도** 통과된다는 뜻이다. 원인은 아직 보지 않았다 — 시작 위치가 `startWall` 앞면과 정확히 맞닿아 `PushOutOfBox`의 최근접점 거리가 0이 되는 경우(밀어낼 방향이 0벡터)가 가장 유력하다. 별도 버그 수정 단계로 다룬다.
- 남아 있는 문제: §5 예약 — 스위치 노멀 맵과 비율, 텍스처 로드 실패 로그, PBR 세트 경로 헬퍼, 쓰지 않는 옛 텍스처 두 장. 위의 시작 벽 통과.
- 다음에 이어서 할 작업: **W-3 웅덩이 마스크**.
- 중요한 설계 결정과 이유: §4 "탄젠트와 노멀 맵"의 N-5 항목.

### 2026-10-26 — 탄젠트 + 노멀 맵 (10-6, W-3보다 앞당김) ✅

- 완료한 작업 (`88a0eb9` 코드, `5014670` 에셋):
  - N-1 `Renderer::ResolveTextureSRV(texture, fallback)` — 예약 항목 "텍스처 슬롯 해석 헬퍼" 처리
  - N-2 `Vertex.tangent`(기본 1, 0, 0), `MakeCube` 면별 탄젠트, Input Layout `TANGENT`(44), `VS_INPUT.tangent`·`PS_INPUT.tangentWorld`, VS `model` 변환
  - N-3 `Material.m_normalTexture`, 기본 평평한 노멀(`flat_normal_1x1.png`, 에이전트 생성), PS `t2`, TBN(그람-슈미트), Rim도 노멀 맵 법선
  - N-4 바닥 NormalDX(`TextureType::NormalMap`), `PBR/.gitignore`에서 NormalDX 제외 해제, `CREDITS.md` 갱신(에이전트)
- 확인한 결과: `fxc` 경고 없음. 사용자 화면 확인 — N-1~N-3 화면 그대로, 탄젠트·법선 디버그 색 일치, N-4 바닥 반사가 자갈 결을 따라 부서짐, 먼 바닥 지글거림 없음.
  - 리뷰 중 잡은 문제: ① `Renderer.h`에 `Texture` 미선언(C4430 연쇄 에러) ② 헬퍼가 `fallback`을 덮어써 항상 흰색 ③ VS 탄젠트를 출력에 안 넣음(X3578) ④ 노멀 맵 `SampleLevel(…, 0)`(밉맵 미사용) ⑤ 그람-슈미트에 정규화 안 된 N ⑥ 기본 노멀 생성 후 `IsValid` 검사 누락
- 남아 있는 문제: §5 예약 — `ModelLoader` 탄젠트·바이탄젠트 부호, 기본 텍스처 생성 묶기, 노멀 강도, 기하 셰이더 법선 시각화. 시작 벽 통과 여부 수동 확인은 여전히 남아 있다.
- 다음에 이어서 할 작업: **W-3 웅덩이 마스크**(`t3`, 기본 검정, 웅덩이에서 노멀 평평하게).
- 작업 규칙 (2026-10-26 사용자 정정): **에이전트는 사용자가 요청하지 않으면 빌드·실행·스크린샷 캡처를 하지 않는다.** N-1 리뷰와 N-4 검증에서 에이전트가 먼저 빌드·실행했던 것을 정정받았다. 리뷰 검증은 코드 읽기와 `fxc`까지만 하고, 빌드·화면 확인은 사용자에게 요청한다(AGENTS §2).
- 중요한 설계 결정과 이유: §4 "탄젠트와 노멀 맵".

### 2026-10-23 ~ 10-26 — W-2 ambientCG 아스팔트 + roughness 맵 ✅

- 완료한 작업:
  - W-2a: 사용자가 ambientCG `Asphalt023S`(2K-JPG, CC0, 125 × 125cm)를 받았다. 에이전트가 `PBR/.gitignore`(지금 안 쓰는 NormalDX·AO·Displacement는 로컬 유지·커밋 제외, NormalGL과 `.blend`·`.mtlx`·`.tres`·`.usdc` 제외 — 사용자 파일은 지우지 않음), `PBR/CREDITS.md`, `Default/white_1x1.png`(70바이트, Node로 생성)를 만들었다
  - W-2b: `Material::m_roughnessTexture`, cbuffer `roughness` → `roughnessFactor`, `Renderer::m_defaultWhiteTextureHandle` 생성과 `t0`·`t1` 바인딩, PS `roughnessTexture`(`t1`) 샘플, `MIN_PERCEPTUAL_ROUGHNESS`
  - W-2c: 바닥 텍스처를 ambientCG Color(`Albedo`)·Roughness(`Data`)로 교체, 배율 0.45, UV (3.2, 16)
- 확인한 결과: `fxc` 경고 없음. 최신 빌드를 실행해 `OutputDebugString`을 받아 실패 로그가 없음을 확인. 사용자 화면 확인(W-2b 그대로, W-2c 바닥 교체·은은한 반사 얼룩·먼 거울 반사 유지).
- 남아 있는 문제: 핸들 검사 실패(albedo·roughness)에는 아직 로그가 없다(선택). 2026-10-26 에이전트의 자동 입력 캡처에서 Play 중 뒷걸음질(S) 뒤 카메라가 시작 벽 뒤(골목 밖)에 있는 장면이 한 번 찍혔다 — 재현 조건이 불확실해 사용자가 직접 S로 시작 벽을 밀어 보고 확인할 것(시작 위치 z = 0이 `startWall` 앞면 z = 0과 정확히 맞닿아 있어 `PushOutOfBox`의 최근접점 거리가 0이 되는 경우일 가능성).
- 마무리: 사용자가 ImGui로 바닥 BaseColor를 0.55로 정해 코드에 옮기고 주석과 함께 커밋했다(`71feced`).
- 다음에 이어서 할 작업: **W-3 웅덩이 마스크**. 커밋 기록: 코드 `9f5a1fa`, 에셋 `b0d253f`(코드와 분리해 에셋 교체·되돌리기가 서로 섞이지 않게), BaseColor `71feced`, 바닥 주석은 사용자 요청으로 에이전트가 근거(출처·sRGB·평균 보존·물의 F0·실제 크기)를 보강했고 사용자가 다시 다듬었다.
- 중요한 설계 결정과 이유: §4 "roughness 맵과 기본 텍스처". 핵심은 **기본 흰색 텍스처로 분기 없이 선택 입력을 처리**, **평균 보존**, **실제 크기 기반 UV**, **안전장치를 먼저 깔고 배선**.

### 2026-10-23 — W-1 UV 타일링, Editor 패널 스크롤 ✅

- 완료한 작업 (`ff34261`):
  - W-1a `Material::uvScale` → `MaterialConstantData` 패딩 자리 → `Renderer` 업로드 → Inspector `UV Scale`
  - W-1b PS `tiledUV = input.uv * uvScale`, 원본 UV 보존
  - W-1c `MakeCube` 오른쪽 면(+X)·뒷면(+Z) UV를 옆면 규칙으로 통일. 뒷면은 사용자가 같은 방법으로 직접 풀었다(UV 네 줄 한 칸 회전)
  - W-1d 바닥 (2, 10), 옆벽 (10, 2), 끝벽 전용 `endWallMat` (2, 2) — 0.5회/m
  - Editor 패널 스크롤(사용자 요청으로 에이전트가 작성): 상시 세로 스크롤바 + Inspector `AutoResizeY`. `DragFloat2` 인자 순서 수정도 에이전트가 작성
  - 확장 예약 목록에 월드 정렬 UV, 반복 깨기, `uvOffset`, 비등방 필터링(트리거 갱신), Material cbuffer의 VS 바인딩을 추가하고, 에셋 배치를 시작할 때 함께 검토하기로 했다
- 확인한 결과: `fxc` 경고 없음, 사용자 빌드·화면 확인 — 줄무늬 제거, 왼쪽·오른쪽 벽이 `wallMat` 하나로 함께 정상, 끝벽 독립, 패널 스크롤 동작.
  - 리뷰 중 잡은 문제: ① `DragFloat2`에 Slider식 인자 순서(범위 미적용, 세터가 막음) ② `endWallMat`을 만들고 `endWall`에 연결하지 않음(컴파일은 통과, 화면만 그대로)
  - 예상된 실패를 먼저 확인: W-1b 뒤 공유 `wallMat`을 (10, 2)로 두면 왼쪽 벽만 가로·세로가 뒤바뀌고 끝벽이 찌그러지는 것을 사용자가 보고, 원인이 "크기"와 "UV 방향" 두 가지임을 구분했다.
- 남아 있는 문제: 상수 버퍼(64바이트)가 꽉 찼다 — 다음 상수 필드는 구조체를 16바이트 늘려야 한다. 바닥은 여전히 균일하다(W-2·W-3).
- 다음에 이어서 할 작업: **W-2 ambientCG 아스팔트 + roughness 맵**(에셋 정책 §7: CC0, 2K, `_NormalDX`는 10-6에서, `CREDITS.md`).
- 중요한 설계 결정과 이유: §4 "UV 타일링". 핵심은 **메시 데이터 문제는 메시에서 고친다**(재질 분리로 덮지 않는다)와 **텍셀 밀도 통일**이다.

### 2026-10-23 — 10-2 Fresnel 완료, 젖은 바닥 디테일(10-W) 앞당김 ✅

- 완료한 작업: 10-2a `FresnelSchlick(f0, cosTheta)`, 10-2b `ComputeDirectLighting`에 `VdotH`·`F`·`diffuseWeight`·`diffuseTerm` 조립(`6fd8742`), 10-2c 바닥 specular 0.5 → 0.25와 주석 정리(`04fab3c`). 작업 원칙 "지금 필요한 것만 구현하고 확장은 §5 예약 목록에 트리거와 함께"를 정하고 예약 목록을 만들었다(`6fd8742`). 변수명은 엔진 관례(Unity 계열 `NdotL`/`VdotH`, BRDF 식의 `F`, Unreal `Fc` ↔ `fresnelWeight`, `diffuseTerm`)로 맞췄다.
- 확인한 결과: `fxc` 컴파일 경고 없음. `FresnelSchlick` 단독 어셈블리로 곱셈 최적화 확인. 검산 0° → 0.04, 85° → 0.649. 사용자가 화면(색 정상, 먼 바닥 반사 강화)을 확인했다.
  - 리뷰 중 잡은 문제: ① 변수 `cosIncidentAngle`에 실제로는 `1 − cos`가 들어 있었고 `reflectPercent`는 0~1 비율이었다(이름 정정) ② 이름 변경 중 옛 이름이 남아 컴파일 불가 상태 ③ `float diffuseTerm = albedo * w`로 **R 채널만 남는 잘림**(X3206). 내 쪽 정정: 처음 권한 인자 이름 `VdotH`를 사용처(IBL·평면 반사의 N·V)를 보고 `cosTheta`로 바꿨다.
- 남아 있는 문제: 바닥이 균일해 젖은 느낌과 반사 기능을 평가할 수 없다(사용자 관찰, §5). 커밋 메시지 정정: 내 커밋이 이미 사용자 커밋(`6fd8742`)에 들어간 셰이더까지 설명해서, push 전에 내용(바닥 specular)에 맞게 메시지를 고쳤다.
- 다음에 이어서 할 작업: §6 **10-W** — W-1 UV 타일링 → W-2 ambientCG 아스팔트 + roughness 맵 → W-3 웅덩이 마스크. 그 뒤 10-2' GGX.
- 중요한 설계 결정과 이유: §4 "Fresnel — Schlick 근사". 요약:
  - **평가할 수 있는 화면을 먼저** — 균일한 반사면은 "광택 칠한 바닥"으로 보인다. 기능을 더 쌓기 전에 차이가 드러나는 테스트 장면(웅덩이 대비)을 만든다. 상용 엔진 팀이 기능마다 테스트 레벨을 두는 것과 같은 이유다.
  - **roughness 맵을 노멀 맵보다 먼저** — 효과는 크고 기반 비용(UV만 필요, 탄젠트 불필요)은 작다.
  - **GGX를 10-W 뒤로** — GGX의 긴 꼬리는 매끈한 웅덩이 위에서 가장 잘 보인다.

### 2026-10-23 — 강의 폴더 PBR 자료 분석, 로드맵 10 보강, 에셋 정책 ✅

- 완료한 작업: 사용자가 "강의를 활용해 PBR·metallic 등을 적용하고 렌더링 품질을 최대한 높이며, 에셋은 직접 만들거나 freepbr·ambientCG 같은 무료 사이트에서 가져오겠다"는 방향을 정했다. 강의 폴더 세 곳을 단계별로 훑고 핵심 셰이더(13 Step8 `BasicPS.hlsl`, `PostEffectsPS.hlsl`, `ExampleApp.cpp`)를 직접 읽어 §7 "강의 PBR 자료 지도"를 만들었다. §6 로드맵 10의 남은 순서를 보강했다(10-2' GGX, 10-7 평면 반사, 10-8 metallic, 10-9 높이 맵). 10-2 Fresnel 가이드(10-2a `FresnelSchlick` 선언·Schlick 표·V·H vs N·V·에너지 분배)를 제공했고 사용자 구현 대기 중이다.
- 확인한 결과: ambientCG 라이선스 페이지(CC0, 재배포·상업 사용 허용)와 FreePBR About 페이지(무료는 비상업, 재배포하지 말라는 요청)를 직접 확인했다. 강의 코드의 π 규약 혼재, LDR 후처리, `lights[1]` 하드코딩, 구형 면광원 TODO, IBLBaker 기준 매직 넘버를 찾았다.
- 남아 있는 문제: 에셋은 아직 하나도 받지 않았다(10-6에서 도입). 저장소 공개 여부에 따라 FreePBR 사용 범위가 정해진다.
- 다음에 이어서 할 작업: 10-2a `FresnelSchlick` 작성 → 10-2b 조립 → 10-2c 바닥 specular 0.25 비교.
- 중요한 설계 결정과 이유:
  - **GGX를 IBL보다 먼저** — split-sum의 BRDF LUT가 GGX 기준이라, Blinn-Phong 직접광과 섞으면 같은 roughness에서 하이라이트와 환경 반사의 퍼짐이 어긋난다.
  - **평면 반사를 로드맵에 추가하고 효과 1순위로** — 바닥이 평면 하나라는 씬 조건에 정확히 맞고, 정적 큐브맵이 못 하는 골목 안 네온 반사를 해결한다. 기존 렌더 타깃·블러 인프라를 재사용한다.
  - **CC0만 커밋** — 포트폴리오는 공개 저장소에 올라가므로 재배포가 허용된 에셋만 저장소에 둔다.

### 2026-10-23 — 작업 재개 점검: 문서·설정 동기화 ✅

- 완료한 작업:
  - 약 3주 공백 뒤 `git log`, 실제 소스, 세 문서(AGENTS/HANDOFF/CLAUDE)를 대조했다. 렌더러 코드는 `e9ab696` 이후 바뀌지 않았고, 문서에 빠져 있던 것은 `a431f1f`(학습 사이트)였다.
  - 학습 사이트의 "문서와 다른 점"을 코드로 하나씩 재확인해 AGENTS.md를 고쳤다: §4 파괴 순서의 `EditorUI` 예외, §6 3번의 ImGui 기록/드로우 분리, §6 7·8번 순서(커서 복귀 → `PointLightSequence::Update`), §7 b0 재바인딩("첫 프레임부터"), §7 Directional 정규화(CPU·GPU 양쪽, 부호 반전만 한 곳), §7 `invTranspose`(전치 두 번 + `w = 0`), §8 학습 사이트 위치.
  - 사용자 결정으로 설정·주석을 고쳤다: `.claude/skills/study-site/SKILL.md`의 마크다운 이스케이프(`\---`, `\#`, `\-`) 복구, `.claude/launch.json`의 사라진 서버 항목 삭제, `DirectionalLight.h`의 낡은 주석("셰이더가 normalize하지 않으므로") 수정 — 동작 변화 없음.
- 확인한 결과:
  - `main` = `origin/main` = `a431f1f`. 진행률 약 78%, 로드맵 11 완료, 10-1 완료, 다음은 10-2 Fresnel — 문서의 다음 작업 지정은 정확했다.
  - SKILL.md는 frontmatter가 `\---`로 시작해 스킬 설명이 `\---`로 인식되고 있었다. launch.json은 다른 세션의 scratchpad(`…/a3ed454c-…/scratchpad/serve-study-site.js`)를 가리켰고 그 파일은 이미 없었다. 사이트는 `fetch`를 쓰지 않아 파일로 바로 열린다.
  - `CLAUDE.md`는 `@AGENTS.md` import와 우선 규칙뿐이라 바꿀 것이 없었다.
- 남아 있는 문제: §5 — Fresnel 없음(바닥 specular 0.5 임시 보정), 환경 반사·재질 맵 없음, UV 타일링, `EditorUI` 파괴 순서 메모, 방향광 이중 정규화(무해). study-site HTML의 "문서와 다른 점" 목록은 다음 사이트 갱신 때 정리.
- 다음에 이어서 할 작업: §6 재개 체크리스트 → **10-2 Fresnel**.
- 중요한 설계 결정과 이유:
  - **학습 사이트가 찾은 차이를 그대로 옮기지 않고 코드로 재확인한 뒤 반영했다.** 사이트도 문서의 일종이라, 문서끼리 맞추는 게 아니라 코드를 기준으로 맞춘다(AGENTS §10).
  - **학습 사이트 위치는 AGENTS §8, 내용과 상태는 HANDOFF §3에** 둔다. AGENTS는 "어디에 무엇이 있고 무엇을 하면 안 되는가", HANDOFF는 "지금 어떤 상태인가"를 담는 기존 역할 분담을 따랐다.
  - **프리뷰 서버를 새로 만들지 않고 항목을 지웠다.** 정적 페이지라 서버가 필요 없고, 저장소 밖 임시 경로를 가리키는 설정은 다른 PC(노트북)에서도 동작하지 않는다.

### 2026-09-30 — 셰이더 표면 쪽 정리와 Specular, 재질값 (로드맵 10-1 완료) ✅

- 완료한 작업:
  - (`f0273c2`, `40b5567`) C 단계 화면 확인 후 커밋, 문서 갱신 커밋.
  - (`b59242a`) D(R-4) — D-1 `SurfaceData { normal, albedo }`, D-2 `ComputeDirectLighting(SurfaceData, IncidentLight)`로 방향광·점광원의 diffuse 식을 하나로 합치고 `main`을 입력 → 직접광 누적 → 간접광 → 연출 → 합성 순서로 정리.
  - (`e9ab696`, push 완료) E-1 `ComputeDirectLighting`에 `viewDir` 매개변수, E-2 `SurfaceData`에 `roughness`/`specular`, E-3 `SpecularBlinnPhong`과 조립(N·L 조기 반환, L+V NaN 검사, F0), F 정규화 `(n+8)/8`, G 재질값(에이전트가 사용자 요청으로 `AppBase.cpp`·`NeonSign.cpp`에 직접 넣고 빌드·실행·스크린샷으로 세 번 비교).
- 확인한 결과:
  - D-2 전후를 `fxc /Fc` 어셈블리로 비교해 명령어 수(134)가 같고 덧셈 순서만 다름을 확인, 화면도 기준과 같았다.
  - E-3 디버그 뷰(specular ×20): 네온 아래 바닥의 하이라이트가 시선을 따라 미끄러지고, roughness 0.2에서 **작아지면서 어두워지는** 에너지 손실을 관찰 → F 정규화 후 해소.
  - G: 전원 Off에서 채움광이 젖은 바닥에 파란 줄무늬로 반사, On에서 네온 색의 길쭉한 반사, 벽은 무광, 에일리어싱 없음. 사용자가 육안으로 확인했다.
  - 리뷰 중 잡은 문제: ① D-2 첫 시도에서 **`SurfaceData`를 채우기 전에** 함수를 부르고 반환값을 `albedo` 필드에 넣음 — 방향광이 화면에서 사라지는 상태였다. HLSL은 초기화 안 된 구조체 전달에 경고가 없다 ② `SpecularBlinnPhong`을 호출부보다 **아래에** 정의해 `undeclared identifier` ③ 분포 함수가 `pow(NdotH, n)`이 아니라 **지수 n을 그대로 반환**(매개변수 `NdotH`가 본문에서 안 쓰인 것이 신호) ④ 계산한 specular를 **반환식에 더하지 않음** — ③과 ④가 서로를 가려 화면으로는 드러나지 않았다 ⑤ 지역 변수 `specular`가 cbuffer 전역 `specular`를 가림(shadowing, 에러 없음) → `specularTerm`.
  - 에이전트의 설명 오류 두 가지를 바로잡았다: "N·L ≤ 0 검사로 `L = −V`가 원천 차단된다"(CullMode NONE에서 뒷면을 보면 아니다), roughness 0.8의 n을 3.9로 계산(정답 2.9).
- 남아 있는 문제: §5 — Fresnel 없음(바닥 specular 0.5 임시 보정), 환경 반사 없음, 재질 맵 없음, UV 타일링.
- 다음에 이어서 할 작업: §6 — 10-2 Fresnel → 바닥 specular 0.25 재비교 → 10-3 Cube Map/Skybox.
- 중요한 설계 결정과 이유: §4 "셰이더 구조"(보강)와 "Specular — 정규화된 Blinn-Phong과 재질값". 요약:
  - **정규화는 `(n+8)/8`** — 우리 diffuse에 1/π가 없으니 specular에서도 뺀다. 절대값이 아니라 두 항의 **비율**이 맞아야 한다.
  - **E와 F를 나눔** — 에너지 손실을 먼저 보고 정규화의 의미를 확인했다.
  - **젖은 표면 = 어둡게 + 매끈하게**, specular는 Fresnel 전까지 0.5.
  - **원본은 사람이 고르기 쉬운 단위, 변환은 사용 직전 한 곳** — 색(sRGB → 선형)과 roughness(지각적 → α)가 같은 원칙이다.

### 2026-09-29 — Material Roughness/Specular, 셰이더 구조 정리 A~C (로드맵 10-1 진행 중)

- 완료한 작업:
  - (`8e4cadc`, push 완료) 10-1a `Material` Roughness/Specular 값 경로(Material → `MaterialConstantData` → HLSL → Inspector). A(R-1) `Common.hlsli` 신설 — `PI`, `VS_INPUT`/`PS_INPUT`을 VS/PS에서 옮기고 vcxproj에 `None`으로 등록, PS는 쓰는 것을 직접 include(`Common` → `Lighting`), `Lighting.hlsli`의 include를 가드 안으로. B(R-2) 카메라 cbuffer를 `Common`으로 합치고 VS·PS 모두 `b1`, PS Material을 `b2`로, C++은 PS 배열 순서만 `{camera, material}`로 바꿨다.
  - (`f0273c2`) C(R-3) Light cbuffer를 `Lighting.hlsli`로 이동하고 전역 이름을 `dirLight*`/`lightPad`로 변경, `IncidentLight` 선언, `GetDirectionalLight()`(부호 반전 + `normalize`), `GetPointLight(index, posWorld)`, `main`이 두 함수를 쓰도록 교체, `ComputePointLight` 삭제.
- 확인한 결과:
  - 매 단계 코드 리뷰 + `fxc`(ps_5_0/vs_5_0)로 컴파일 확인. 최종 상태 VS·PS 모두 컴파일 성공.
  - B-1'은 "셰이더만 바꾸면 형태는 보이고 색이 깨진다"를 먼저 예측한 뒤 C++을 고치는 순서로 진행했다.
  - 리뷰 중 잡은 문제: ① 공유 cbuffer 패딩을 `pad`로 지어 PS의 Light cbuffer `pad`와 `redefinition`(VS는 Light cbuffer가 없어 통과 — **include 파일은 include하는 셰이더마다 충돌 여부가 다르다**) ② 멤버 이름을 `cameraPos`로 바꾸고 PS 사용처를 안 고침(첫 에러에서 멈춰 가려져 있었다) ③ Light cbuffer를 복사만 하고 PS 원본을 안 지워 `redefinition of 'direction'` ④ `normalize(v);`로 **반환값을 버림** — HLSL 내장 함수는 순수 함수라 인자를 고치지 않는다. 경고도 없고 입력이 이미 단위 벡터라 화면으로는 안 드러난다 ⑤ `GetPointLight`에서 **정규화한 뒤 `length`**를 재서 `d`가 항상 1 — 모든 점광원이 범위·감쇠 없이 씬 전체를 비추는 상태였다. 같은 수정에서 안전 기본값을 조기 반환 전에 덮어써 `d ≈ 0`일 때 NaN 방향이 반환될 수 있었다. 둘 다 "구조체 필드를 임시 변수로 쓴 것"이 원인이라 지역 변수 `toLight`로 분리해 해결.
  - C 단계 완료 후 화면이 기준과 같고(네온 아래 빛 웅덩이 포함), 스위치 순차 점등도 정상임을 사용자가 확인했다(2026-09-30).
- 남아 있는 문제: §5 — `main`의 diffuse 식 의도된 중복(D에서 해소), Roughness/Specular 미사용, Specular 항 없음.
- 다음에 이어서 할 작업: §6 10-1 진행 목록 — D-1 `SurfaceData` → D-2 `ComputeDirectLighting` → E Specular.
- 중요한 설계 결정과 이유: §4 "셰이더 구조 — 빛 쪽과 표면 쪽 분리". 요약:
  - **리팩터링을 먼저, Specular는 한 곳에** — 빛 M종 × 표면 항 N개를 M+N으로.
  - **`Common`의 기준은 "모두가 쓰는가"** — Material은 셰이더별 계약이라 제외(강의 Ch13, Unity URP와 같다).
  - **카메라는 b1** — 사용자 제안. 두 스테이지 모두 연속 바인딩, 강의 Ch13과 같은 슬롯.
  - **리팩터링은 "화면이 바뀌면 안 되는" 작은 단계로** — 한 번에 한 가지만 바꾸고 결과를 예측한 뒤 확인한다. ④⑤처럼 컴파일은 통과하는 버그를 화면 비교가 잡는다.

### 2026-09-28 — 텍스처 밉맵, 텍스처 파이프라인 방향 결정 (로드맵 10번 선행 조건 완료) ✅

- 완료한 작업:
  - (`f91b0e6`) 모든 파일 텍스처에 `GenerateMips`로 전체 밉 체인을 만든다. 3-a 현상 관찰 → 3-b `D3D11Utils::CreateTexture`에 Context 인자 → 3-c 업로드 방식 변경(`MipLevels = 1`인 채) → 3-d 밉 켜기 → 3-e RenderDoc 확인 순서로 진행했다. 구조는 §3, 결정 이유는 §4 "텍스처 색공간 선택".
  - (`6256ed2`) `D3D11Utils.cpp`의 모든 `OutputDebugStringW`(19곳)에 줄바꿈을 붙였다. 3-b 중 사용자가 같이 정리한 것이라 밉맵 커밋과 분리했다.
  - Step 2(데이터 텍스처 실제 확인)는 2026-09-21에 코드 변경 없이 끝냈다(§3).
  - 그래픽스 강의 폴더를 분석해 **텍스처 파이프라인 방향**을 정했다 — 지금은 PNG + `GenerateMips`, 큐브맵은 로드맵 10번에서 DDS, 2D 텍스처의 DDS 쿠킹은 에셋이 늘어날 때(§4 "텍스처 파이프라인 방향").
- 확인한 결과:
  - 같은 위치(골목 입구, 시선 약간 아래) 스크린샷 비교에서 중·원거리 바닥의 날카로운 흰 점이 사라지고 부드러운 회청색이 됐다. **먼 곳이 어두워지지 않았다** — 선형 공간 평균의 신호다. RenderDoc에서 밉 레벨 생성과 높은 레벨의 밝기 유지를 확인했다.
  - 3-a 스크린샷에서 **서로 다른 문제 둘**을 구분했다. 중·원거리 흰 점(에일리어싱, 이번 대상)과 가까운 바닥·벽의 z 방향 줄무늬(UV 타일링, 이번에 못 고침). 섞어 보면 "밉맵이 줄무늬를 못 고쳤다"로 오판한다.
  - 리뷰 중 잡은 문제: ① `UpdateSubresource`의 원본 데이터 자리에 `img` 대신 **`&format`**(스택의 4바이트 enum 주소)을 넘겼다 — 드라이버가 거기서 16MB를 읽어 크래시나 노이즈 텍스처가 된다. 매개변수가 `const void*`라 **어떤 포인터든 암묵 변환되어 컴파일러가 못 잡는다** ② `CreateTexture2D`·`CreateShaderResourceView` 실패 경로에서 `img`가 샜다 ③ 누수를 고치며 `UpdateSubresource`를 **`hr` 검사보다 앞**으로 옮겨, 생성 실패 시 `nullptr` 리소스에 복사하는 상태가 됐다 — 성공 경로에선 멀쩡해서 화면으로는 안 드러난다 ④ 주석 처리한 `initData` 블록이 남아 있었다.
- 남아 있는 문제: §5 참고. UV Tiling 정책 없음(가까운 곳 줄무늬), 비등방 필터링 보류(먼 바닥 약간 뭉개짐), 실패 로그에 파일 이름 없음.
- 다음에 이어서 할 작업: §6 — 로드맵 10번(젖은 바닥 반사). 노멀 맵 샘플 뒤 `normalize` 필수.
- 중요한 설계 결정과 이유: §4 "텍스처 색공간 선택"과 "텍스처 파이프라인 방향"에 정리했다. 핵심 요약:
  - **밉을 로드맵 10번보다 먼저** — 스페큘러·노멀 맵·Bloom이 들어온 뒤엔 원인 후보가 넷이 된다.
  - **GPU `GenerateMips`** — `_SRGB` 포맷에서 평균이 선형 공간에서 계산된다. CPU로 줄이면 밉이 어두워진다.
  - **Context는 `D3D11Utils`까지만** — `GraphicsResourceManager` 공개 시그니처를 지켜 `Texture`가 DX11을 모르게 둔다.
  - **"만든 것은 확인한 뒤에 쓰고, 빌린 것은 다 쓴 즉시 돌려준다"** — `HRESULT`는 다음 줄을 실행해도 되는지 알려주는 신호라 그 줄보다 먼저 본다. `img` 수명을 짧게 잡을수록 챙길 반환 경로가 준다(자동화하면 RAII).
  - **큐브맵·IBL엔 DDS가 필수** — specular IBL의 밉은 평균이 아니라 거칠기별 컨볼루션이라 `GenerateMips`로 못 만든다. DDS를 들이면 `TextureType`이 `FORCE_SRGB`/`IGNORE_SRGB`를 골라 "용도가 원본" 원칙을 지킨다.

### 2026-09-21 — 텍스처 캐시 키 (Step 1 완료) ✅

- 완료한 작업 (`0d0eac2`): `AssetManager`의 텍스처 캐시 키를 경로에서 `(경로, TextureType)`으로 바꿨다. 1-e 이후 텍스처가 용도를 갖게 됐는데 캐시만 경로로 조회해서, 같은 파일을 다른 용도로 요청하면 먼저 로드된(포맷이 다른) 리소스가 조용히 돌아오던 문제다. 이로써 텍스처 색공간 선택 Step 1이 끝났다.
  - `TextureKey = std::pair<std::string, TextureType>`, private 중첩 해시 함자 `TextureKeyHash`, `unordered_map`의 세 번째 템플릿 인자로 연결
  - `LoadTexture`가 키를 지역 변수로 한 번 만들어 `find`와 `emplace`에 같이 넘긴다
- 확인한 결과:
  - 임시 테스트를 `InitGreyBoxScene`에 넣고 F5로 실행해 출력 창에서 두 가지를 확인했다. ① 같은 경로 + `Albedo` 두 번 → **같은 포인터**(캐시 유지, 회귀 확인) ② 같은 경로 + `Data` → **다른 포인터**(용도 분리, 새 기능 확인). 확인 후 테스트 코드를 제거했고 `AppBase.cpp` diff가 비어 있음을 확인했다.
  - 리뷰 중 잡은 문제: ① 해시 함자 본문이 비어 반환값이 없었다(C4716) ② `.cpp`의 `find`/`emplace`가 문자열 키를 넘겼다 ③ `emplace`를 고치면서 **`std::move(texture)`까지 지워** 값이 사라졌다 — 컴파일 에러이지만 본질은 소유권으로, 옮기지 않으면 함수 끝에서 `Texture`가 삭제돼 반환 포인터가 댕글링이 된다 ④ 테스트가 판정 없이 태그만 찍었다 ⑤ 테스트에서 "용도 분리" 확인이 빠져 **1-f 이전 코드도 통과하는** 상태였다 ⑥ 분리 판정 조건이 `==`로 **뒤집혀** 정반대를 보고했다 ⑦ `nullptr` 검사가 없어 로드 실패도 "분리 OK"로 통과할 수 있었다(출력 창에 `LoadTexture() failed`가 없으면 결과가 유효하다는 것으로 확인)
- 남아 있는 문제: §5 참고. 밉맵 없음, UV Tiling 정책 없음.
- 다음에 이어서 할 작업: §6 — Step 2(데이터 텍스처를 실제로 읽어 색 텍스처와 비교) → 밉맵 검토 → 로드맵 10번.
- 중요한 설계 결정과 이유: §4 "텍스처 색공간 선택"에 추가했다. 핵심 요약:
  - **`pair` + 해시 함자.** `pair`의 `operator==`를 공짜로 쓰고 해시만 직접 준다. 표준은 두 원소를 어떻게 합칠지가 하나로 정해지지 않아 `std::hash<std::pair>`를 제공하지 않는다. 키 원소가 셋 이상이 되면 전용 구조체로 승격한다.
  - **`hash_combine`으로 섞는다.** MSVC의 `std::hash`는 정수·열거형도 FNV-1a로 섞지만 GCC·Clang은 값을 그대로 쓴다(MSVC `type_traits` 소스로 확인). **`std::hash` 구현에 기대지 않는 방식**이어야 컴파일러가 바뀌어도 안전하다. 언리얼의 `GetTypeHash()` + `HashCombine()`과 같은 구조다.
  - **키를 지역 변수 하나로 만들어 재사용한다.** 조회 키와 저장 키가 어긋나면 캐시가 영원히 히트하지 않는 조용한 버그가 되는데, 변수 하나를 같이 쓰면 원천적으로 불가능하다. 덤으로 `emplace`가 템플릿이라 중괄호 목록을 추론하지 못하는 문제도 피한다.
  - **테스트는 회귀와 새 기능을 둘 다 본다.** 한쪽만 보면 "변경 전 코드도 통과" 또는 "캐시를 고장 내도 통과"하는 빈틈이 생긴다.

### 2026-09-18 ~ 09-19 — 텍스처 색공간 선택 경로 (로드맵 10번 선행 조건) ✅

- 완료한 작업 (`ddb29c7`): 호출자가 **용도**를 말하면 색공간이 따라 정해지는 경로를 네 계층에 뚫었다. Step 1의 1-a~1-e에 해당한다. 값과 구조는 §3, 결정 이유는 §4의 "텍스처 색공간 선택" 참고.
  - 1-a `TextureType.h`(`Albedo`/`NormalMap`/`Data`) → 1-b `D3D11Utils::CreateTexture`가 `DXGI_FORMAT` 수용 → 1-c `GraphicsResourceManager` → 1-d `Texture::Initialize`가 `TextureType`을 받아 `ToDxgiFormat`으로 변환 → 1-e `AssetManager::LoadTexture`와 호출부 5곳.
  - 매 단계 **빌드가 통과하고 화면이 안 바뀌는** 순서로 쪼갰다. 비계는 기본 인자가 아니라 **눈에 보이는 리터럴**을 한 칸씩 위로 올리는 방식을 썼다 — 지울 곳이 코드에 드러나 있어 빠뜨리지 않는다.
- 이어서 한 작업: **1-f 캐시 키**는 2026-09-21 항목(`0d0eac2`)에서 끝냈다.
- 확인한 결과:
  - 단계마다 코드 리뷰 + 빌드로 확인했고, 마지막 전체 리빌드(`/t:Rebuild`)까지 통과했다. **화면 변화 없음** — 순수 리팩터링이므로 이것이 완료 조건이었다.
  - `DXGI_FORMAT` 리터럴이 `Texture.cpp`의 변환 함수와 `GraphicsDevice.cpp`의 스왑체인에만 남은 것을 grep으로 확인했다(비계 완전 철거).
  - 리뷰 중 잡은 문제: ① `DrawFullScreenPass`에서 `OMSetRenderTargets`를 맨 뒤로 옮긴 변경이 섞여 있었다 — 입력을 걸 때 직전 패스의 RTV가 살아 있어 **모든 Bloom 패스의 SRV가 null이 되는** 상태였다(§7의 바인딩 순서 규칙 위반). 되돌렸다. ② `ToDxgiFormat`을 헤더에 선언하고 `.cpp`에 자유 함수로 정의해 **서로 다른 두 함수**가 생겼고, 반환 타입 때문에 헤더가 DXGI를 요구해 변환 지점을 `Texture.cpp`로 정한 이유 자체가 무너졌다. ③ `switch`가 모든 경로에서 반환하지 않았다. ④ 각 단계에서 호출부 갱신을 빠뜨려 컴파일 에러가 났다.
  - `enum class`를 쓴 덕을 봤다. `DXGI_FORMAT`(C 스타일 enum)을 `TextureType` 자리에 넘긴 실수를 컴파일러가 막았다 — 그냥 `enum`이었다면 둘 다 `int`로 변환돼 조용히 통과했을 것이다.
- 남아 있는 문제(당시): 캐시 키 미완(→ 2026-09-21 해결), 밉맵 없음, UV Tiling 정책 없음.
- 다음에 이어서 할 작업(당시): 1-f 마무리 → Step 2 → 밉맵 검토 → 로드맵 10번.
- 중요한 설계 결정과 이유: §4 "텍스처 색공간 선택"에 정리했다. 핵심 요약:
  - **`bool isSrgb`가 아니라 용도 enum.** 언리얼이 `TC_Normalmap`을 고르면 sRGB를 자동으로 끄는 것과 같은 발상 — **용도가 원본이고 색공간은 파생값**이다. 모순된 조합을 타입으로 막는다.
  - **변환 지점은 `Texture.cpp`.** 근거는 헤더 의존성이다. 이렇게 하면 `AssetManager.h`와 `Texture.h` 둘 다 DX11을 모르는 채로 남는다.
  - **파일명 휴리스틱은 쓰지 않는다.** 언리얼에서는 사람이 검토하는 임포트 단계가 있어 안전하지만, 우리는 그 단계가 없어 추측이 곧 런타임 동작이 된다.
  - **`NormalMap`과 `Data`를 미리 나눴다.** 오늘은 같은 포맷이지만 BC5와 G채널 반전이 붙을 자리가 다르다.
  - **기본 인자를 두지 않았다.** 조용한 실패보다 컴파일 에러를 택했다.

### 2026-09-17 — 룩 재튜닝과 `main` 머지, 로드맵 11번 종료 ✅

- 완료한 작업 (`050a1d9`): 기능 검증용 임시값으로 남아 있던 조명·후처리 값을 밤 골목 기준으로 전부 다시 잡았다. 확정값은 §3 "룩 재튜닝", 결정 이유는 §4 "룩 재튜닝" 참고.
  - 하늘광을 흰색 `1.0`에서 차가운 청색 `(0.45, 0.60, 1.0)` `0.18`로 낮추고 방향을 거의 수직인 단위 벡터로 바꿨다.
  - `EnvironmentFillLight`를 보라색 `1.58`/range 6에서 청색 `0.55`/range 24로 바꿨다.
  - Ambient `0.4` → `0.045`, Exposure `1.0` → `2.0`, Threshold `1.0` → `1.2`, Strength `0.5` → `0.8`, Iterations `1` → `5`.
  - 네온 세 개의 Emissive를 올리고(3/8/5 → 18/7/15) Point Light를 내렸다(2.0/2.0/5.0 → 1.4/0.7/1.4).
  - 전원 스위치 색을 다시 잡고 `ApplyVisualState`에 대기 표시등(Emissive)을 추가했다. Rim 강조 색을 청록 → 호박색, 세기 2.5 → 3.0.
  - `EditorUI`의 Ambient 슬라이더를 로그 스케일(`0.001~1.0`)로 바꿨다.
- 완료한 작업 (`3c0edda`): 위 내용을 §3·§4·§5·§6·§7·§9에 반영하고 `AGENTS.md` §1·§7을 갱신했다.
- 완료한 작업 (`5794467`): `HDR_SCENE_TARGET`을 `main`에 `--no-ff`로 머지하고 `main`과 기능 브랜치를 원격에 push했다. 33개 파일, +1416/-155.
- 확인한 결과: 에이전트가 세 차례 빌드·실행하고 화면을 캡처해 확인했으며, 사용자가 최종 확인했다. 전원 Off에서 차가운 청색 골목과 끝벽 빨간 대기 표시등이 유도등처럼 보이고, On에서 네온 심지가 번지며 벽이 은은하게 물든다. 144 FPS 유지.
  - 캡처를 보고 고친 것: ① 1차에서 벽이 빨강/청록 젤을 씌운 것처럼 진했다 → Point Light 세기를 절반으로 내렸다. ② 2차에서 스위치 대기 표시등이 평평하게 날아가 빨간 스티커처럼 보였다 → Emissive 세기 `0.5` → `0.20`.
  - 머지: 충돌 없었다. 사전에 `git diff $(git merge-base HEAD main) main`이 비어 있음을 확인했다 — `main`의 `fcc6de1`은 순수 머지 노드였고 그것이 담은 `9d195bd`가 이미 우리 브랜치 조상이라 내용상 겹치는 변경이 없었다. 머지 후 **전체 리빌드(`/t:Rebuild`)로 트리가 정상임을 확인**했다(기존 경고 `AppBase.cpp(224)` C4244 2건 외 없음).
  - 머지 후 `main`과 `HDR_SCENE_TARGET`의 `git diff`가 비어 있는 것, 원격 기능 브랜치 둘 다 `main`에 포함된 것(`merge-base --is-ancestor`)을 확인했다.
- 남아 있는 문제: §5 참고. 주황 네온 심지의 색조 이동(정상으로 판단), `NeonSignDesc.color`가 Emissive와 Point Light를 공유해 둘을 따로 잡을 수 없는 한계, 데이터 텍스처용 sRGB 선택 인자 없음, `CreateTexture`의 밉맵 없음. 원격 `HDR_SCENE_TARGET`/`WORK_CLAUDE`가 정리되지 않고 남아 있다(내용은 `main`에 전부 포함).
- 다음에 이어서 할 작업: §6 — sRGB 선택 인자 → 로드맵 10번. 로드맵 10번용 테스트 에셋 검토 결과도 §6에 적어뒀다.
- 중요한 설계 결정과 이유: §4 "룩 재튜닝"에 정리했다. 핵심 요약:
  - **값보다 세팅 논리가 먼저였다.** 흰색 태양 상시 On, 보라색 상시 조명, 선형 Ambient 0.4 세 가지를 고치지 않으면 어떤 값으로도 목표 룩에 도달할 수 없었다.
  - 하늘광을 수직으로 눕히면 옆벽이 `dot = 0`이 돼 하늘광을 안 받는다. 바닥만 차갑게 빛나고 옆벽은 네온이 닿는 곳만 밝아지는 구도가 여기서 나온다.
  - Emissive와 Point Light가 따로 노는 것(GI 부재)을 이용해 "네온은 밝고 벽은 은은하게"를 만들었다.
  - 세기 균형은 감이 아니라 채널별 휘도 가중치로 환산해 맞췄다.
  - 스위치 대기 표시등은 Threshold 아래로 잡아 번지지 않게 했다 — 번지면 표시등이 아니라 네온으로 읽힌다.
  - **머지를 `--no-ff`로 했다.** 포트폴리오 저장소라 커밋 히스토리를 보여줄 일이 생기고, 그때 "HDR + Bloom이 어디서 어디까지였나"가 머지 커밋 하나로 드러나는 편이 읽기 좋다고 판단했다. 실제로는 `main`이 갈라진 뒤 따로 움직여서 fast-forward가 불가능한 상태이기도 했다.
  - **로드맵 10번용 테스트 에셋(Poly Haven `rusty_metal_04`) 도입을 보류했다.** 스페큘러가 없는 지금 넣으면 금속이라는 성질이 드러나지 않고 확정한 룩만 흔들린다. 상세와 그때 참고할 사실은 §6 참고.

### 텍스처 색공간 선택

로드맵 10번 선행 조건 작업의 결정들이다(2026-09-18 ~ 09-21). 상용 엔진이 같은 문제를 어떻게 푸는지 보고 규모에 맞게 줄였다.

- **`bool isSrgb`가 아니라 용도 enum을 받는다.** 언리얼에서 `CompressionSettings`를 `TC_Normalmap`으로 바꾸면 sRGB가 자동으로 꺼진다. 즉 **용도가 원본이고 색공간은 파생값**이다. 호출부가 실제로 아는 것은 "이건 노멀 맵이다"이지 색공간이 아니며, 용도를 받으면 "노멀 맵인데 sRGB로 읽는" 모순된 조합이 **표현 불가능**해진다.
- **가져오지 않은 것**: 임포트 파이프라인, 파일명 휴리스틱, Texture Group/LOD 정책, BC 압축, 스트리밍. 특히 **파일명 추론은 언리얼에서는 안전하지만 우리에겐 위험하다** — 언리얼은 임포트 시점에 한 번 추측하고 사람이 고칠 수 있는 반면, 우리는 임포트 단계가 없어 추측이 곧 런타임 동작이 된다.
- **변환 지점을 `Texture.cpp`로 정한 근거는 헤더 의존성이다.** `AssetManager`가 변환하면 `Texture::Initialize`가 `DXGI_FORMAT`을 받아야 해서 `Texture.h`가 DX11을 알게 된다. `Texture`가 변환하면 **두 공개 헤더 모두 DX11을 모르는 채로 남고** DXGI 의존이 `.cpp` 하나에 갇힌다. `Renderer::SrgbToLinear`와 같은 자리 — 의미를 다루는 마지막 계층이 GPU 배선에 넘기기 직전에 한 번 변환한다.
- **`TextureType.h`에 변환 함수를 두지 않았다.** 그러면 의존성이 하나도 없는 그 헤더가 DXGI를 끌어오고 `AppBase`까지 전파된다.
- **`NormalMap`과 `Data`를 나눴다** — 오늘은 같은 포맷을 반환하지만 BC5 압축과 **OpenGL(Y+)↔DirectX(Y-) G채널 반전**이 붙을 자리가 `NormalMap`이다. `BoxCollisionComponent`를 정직하게 이름 지었던 것과 같은 전략으로, 갈라지는 날 enum 값 하나에 동작을 붙이면 호출부는 바뀌지 않는다.
- **`Albedo`라 이름 지은 이유**: glTF와 언리얼은 `BaseColor`를 쓰지만 이 프로젝트는 `Material::SetAlbedoTexture`/HLSL `albedoTexture`로 이미 albedo를 쓰고, **`BaseColor`는 색 상수(`SetBaseColor`)로 이미 점유돼 있다.** 외부 관례보다 내부 일관성을 택했다.
- **`TextureUsage`라는 이름은 피했다** — 같은 함수 안의 `D3D11_USAGE`(CPU/GPU 접근 패턴)와 뜻이 겹쳐 읽는 사람이 멈칫한다.
- **기본 인자를 쓰지 않았다.** 아래 세 계층은 호출부가 하나씩이라 기본값이 아낄 것이 없고, 매개변수를 맨 끝에 밀어 넣어야 해서 "입력 → 출력(`ComPtr&`)" 그룹핑이 깨진다. `LoadTexture`는 호출부가 5곳이라 기본값이 일을 줄여주지만 **동시에 가장 위험한 자리**다 — 앞으로 노멀 맵을 부를 때마다 사람이 고르는 지점이라, 빠뜨리면 조용히 sRGB가 된다. 비계는 기본 인자 대신 **눈에 보이는 리터럴**을 한 단계씩 위로 올리는 방식으로 썼다.
- **캐시 키는 리소스의 정체성을 결정하는 모든 입력이다.** 용도가 생긴 순간 입력이 경로 하나에서 둘로 늘었으니 키도 `(경로, TextureType)`이어야 한다. 언리얼에서 같은 원본 파일을 다른 설정으로 두 번 임포트하면 에셋이 둘 생기는 것과 같다. 세 가지 선택지(① 문자열에 용도를 덧붙인 합성 키 ② `pair` + 해시 함자 ③ 같은 경로를 다른 용도로 부르면 실패시키기) 중 ②를 택했다 — 코드가 몇 줄 안 되고 정체성이 자료구조에 그대로 드러난다.
- **해시는 `hash_combine` 관용구로 섞는다.** MSVC의 `std::hash`는 정수·열거형도 FNV-1a로 섞지만 GCC·Clang은 값을 그대로 쓴다. 코드가 `std::hash`의 구현을 가정하면 안 되므로, 어느 쪽이든 잘 섞이는 방식을 쓴다.
- **해시 함자는 `AssetManager` 안에 private으로 중첩했다.** 이 캐시 말고는 쓸 곳이 없다는 의도가 드러나고 `My` 네임스페이스에 이름이 늘지 않는다. 키 별칭 `TextureKey`는 네임스페이스 수준에 뒀다.
- **Mesh/Material/Model 캐시는 바꾸지 않았다.** 그쪽은 여전히 이름 하나가 곧 정체성이다. 문제가 없는 곳까지 일반화하지 않는다.
- **밉맵을 로드맵 10번보다 먼저 넣었다.** 먼 곳에서 픽셀 하나가 텍셀 여러 개를 덮는데 한 점만 읽어 반짝이는 에일리어싱은 지금은 "먼 바닥이 좀 지글거린다" 수준이지만, 스페큘러 하이라이트(작고 밝음)·노멀 맵(픽셀마다 법선이 다름)·HDR Bloom(1을 넘는 값이 번짐)이 들어오면 **번지며 깜빡이는** 문제가 된다. 그때 원인을 가리려면 변수가 넷이 되니 변수가 적은 지금 막았다.
- **밉은 GPU `GenerateMips`로 만든다.** CPU에서 줄이면 `_SRGB` 바이트를 그대로 평균해 **밉이 어두워진다**(0.9와 0.1을 그대로 평균하면 0.5, 빛의 양으로는 $0.5^{2.2} \approx 0.22$인데 원래 평균은 약 0.40). `GenerateMips`는 SRV로 읽고 RTV로 쓰는 렌더링이라 `_SRGB` 포맷이면 **읽을 때 디코드, 쓸 때 인코드되어 평균이 선형 공간에서 계산된다.** 색공간을 `TextureType`으로 제대로 나눠둔 결정이 여기서 한 번 더 효과를 봤다. 오프라인 DDS는 §4 "텍스처 파이프라인 방향" 참고.
- **Context는 `D3D11Utils`까지만 내려보낸다.** `UpdateSubresource`·`GenerateMips`는 Device(리소스를 만든다)가 아니라 Context(GPU에게 일을 시킨다)의 함수라 `D3D11Utils::CreateTexture`에 `ID3D11DeviceContext*`를 `device` 바로 뒤에 추가했다(`UpdateBuffer`와 같은 "Device/Context → 입력 → 출력" 순서). `GraphicsResourceManager::CreateTexture`의 공개 시그니처는 바꾸지 않고 내부에서 `m_graphicsDevice->GetContext()`를 넘긴다 — 공개 시그니처에 넣으면 `Texture`가 DX11을 알게 되어 1-d의 경계가 무너진다.
- **초기 데이터 대신 `UpdateSubresource`로 올린다.** `CreateTexture2D`에 초기 데이터를 넘기면 모든 밉 레벨의 데이터가 필요한데 우리는 레벨 0뿐이다. 빈 텍스처(`USAGE_DEFAULT` — `IMMUTABLE`은 생성 후 못 바꾼다)를 만들고 레벨 0만 채운 뒤 나머지는 GPU에 맡긴다.
- **업로드 변경(3-c)과 밉 켜기(3-d)를 나눠서 진행했다.** 한 번에 했다가 화면이 검게 나오면 업로드와 밉 생성 중 어디가 틀렸는지 가릴 수 없다. 3-c에서 `MipLevels = 1`인 채로 화면이 그대로인지 먼저 봤다.
- **모든 파일 텍스처에 밉을 만든다.** 노멀·러프니스 맵은 오히려 더 필요하다. 밉이 필요 없는 텍스처(UI 등)가 생기면 그때 `TextureType`에 규칙을 붙인다(언리얼 `MipGenSettings`의 `NoMipmaps` 자리).

### 텍스처 파이프라인 방향 — PNG / DDS / 쿠킹 (2026-09-28 결정)

그래픽스 강의 폴더(`Hong_Graphics2/Graphics_Part2/08_ShaderToys_*`, `Hong_Graphics3/HongGraphicsPart3`)를 분석하고 정한 방향이다. **지금은 PNG로 가고, 필요가 생길 때마다 한 칸씩 옮긴다.**

**강의가 실제로 하는 것** — DDS는 **큐브맵에만** 쓴다. 일반 2D 텍스처는 `stbi_load` → `CreateTexture2D`에 `MipLevels = 1`, `IMMUTABLE`로 우리와 같다(밉 없음). 큐브맵은 DirectXTex의 `texassemble`로 여섯 면을 DDS로 묶고 DirectXTK `CreateDDSTextureFromFileEx` + `D3D11_RESOURCE_MISC_TEXTURECUBE`로 읽는다. 강의 에셋 헤더를 직접 읽어보면 `*_specularIBL.dds`는 1024² 큐브에 **밉 11장, BC6H**(HDR 압축), `*_diffuseIBL.dds`는 128² 큐브에 밉 8장이다.

**큐브맵·IBL에 DDS가 필수인 이유** — specular IBL의 밉은 텍셀 평균이 아니라 **거칠기별로 풍경을 컨볼루션한 결과**다. 셰이더가 러프니스로 레벨을 골라 읽는다. 반구 적분이라 실시간에 할 수 없어 오프라인 도구(CMFT, IBLBaker 등)로 계산해 밉 레벨에 저장하며, 이걸 담을 수 있는 포맷이 DDS다. `GenerateMips`(2×2 평균)로는 만들 수 없고, HDR이라 PNG에도 담을 수 없다.

**단계별 경로**

| 시점 | 할 일 | 이유 |
|---|---|---|
| 지금 (색공간 Step 3) | 2D 텍스처는 PNG + `GenerateMips` | 텍스처 4장. 파이프라인을 만들 이유가 아직 없다(§3 원칙). 밉을 직접 만들어봐야 쿠킹 도구가 무엇을 대신하는지 안다 |
| 로드맵 10번 | **큐브맵만 DDS** — DirectXTK `DDSTextureLoader` | 위 이유로 사실상 유일한 선택. DirectXTK는 이미 vcpkg에 설치돼 있어(`SimpleMath.h`와 같은 폴더) **새 의존성이 없다** |
| 에셋이 늘어날 때 | 2D 텍스처도 **오프라인 쿠킹 → DDS** | VRAM·로딩이 실제 문제가 되는 시점 |

**DDS 도입 시 색공간 규칙** — DDS는 포맷 안에 색공간이 박혀 있다(`BC7_UNORM_SRGB` vs `BC7_UNORM`). 그대로 읽으면 파일도 색공간을 결정해 **결정권자가 둘**이 되고, 노멀 맵을 실수로 `_SRGB`로 구우면 `TextureType::NormalMap`으로 불러도 sRGB로 읽힌다. 설치된 DirectXTK의 `DDS_LOADER_FORCE_SRGB`(0x1) / `DDS_LOADER_IGNORE_SRGB`(0x2)를 **`TextureType`이 고르게** 해서 PNG 경로의 `ToDxgiFormat`과 같은 원칙(용도가 원본)을 지킨다. 로드맵 10번에서 이 규칙을 같이 설계한다.

**최종 형태 (전환 조건이 왔을 때)**

```
Assets/Source/   사람이 다루는 원본 PNG/EXR  (용도 접미사: _D, _N, _R ...)
      │  cook_textures 스크립트가 texconv를 호출. 접미사로 포맷·옵션 선택
      ▼
Assets/Cooked/   엔진이 읽는 BC 압축 + 밉 포함 DDS
      ▼
LoadTexture(path, TextureType)
   .dds → DDSTextureLoader (TextureType이 FORCE/IGNORE_SRGB 선택)
   .png → stb + GenerateMips (빠른 실험용으로 유지)
```

- 쿠킹 단계에서는 파일명 추론이 **안전하다.** 런타임 추론을 거부한 이유(§4 "텍스처 색공간 선택")는 사람이 검토할 단계가 없어서였는데, 쿠킹은 오프라인이라 결과를 확인할 수 있다. 언리얼이 임포트 시점에 파일명으로 추측해도 되는 이유와 같다. 런타임 결정권은 여전히 `TextureType`에 있다.
- 용도별 포맷(2048², 밉 포함): Albedo **BC7_UNORM_SRGB** ≈5.3MB / NormalMap **BC5_UNORM** ≈5.3MB / Data **BC4_UNORM** ≈2.7MB / HDR 큐브맵 **BC6H**. 현재 PNG RGBA8은 ≈21.3MB라 4~8배 줄어든다.
- `NormalMap`을 `Data`와 나눠둔 결정이 여기서 쓰인다. BC5는 X·Y만 저장해 **셰이더가 $z = \sqrt{1 - x^2 - y^2}$로 복원**해야 하고, OpenGL 규약(`nor_gl`) 노멀 맵은 **쿠킹 시 `texconv -inverty`**로 G 채널을 뒤집으면 셰이더를 건드리지 않아도 된다. 둘 다 `NormalMap` 한 곳에만 붙는다.
- 도구: 여섯 면 묶기 `texassemble`, 포맷 변환·압축 `texconv`(둘 다 DirectXTex), IBL 필터링 CMFT. **현재 셋 다 설치돼 있지 않다.**

**2D 텍스처를 쿠킹으로 옮기는 전환 조건** — 텍스처가 10~20장을 넘을 때 / 로드맵 8번에서 실제 골목 에셋을 들일 때 / VRAM이나 로딩 시간이 실제 문제로 측정될 때. 원본 → 쿠킹 → 런타임 파이프라인은 에셋 파이프라인 이해를 보여주는 포트폴리오 이야깃거리이기도 하지만, 데모(로드맵 9·10·12번)가 먼저다.

### 2026-09-17 — Bloom 구현과 Rim 강조 책임 이동 (로드맵 11번 Step 5~7) ✅

- 완료한 작업:
  - (`765b68c`) 스위치 Rim 강조를 `PowerSwitch`로 옮겼다. `SetHighlighted(bool)`는 값이 바뀔 때만 `ApplyVisualState`를 부르고, `ExitPlayMode`가 강조를 끄며, `GetGameObject()`를 제거했다. `CanInteract` 호출이 프레임당 3회에서 1회로 줄었다.
  - Step 5 (`5ddb2ab`): 절반 크기 Bright 타깃, `brightPassPixelShader`(exposure 적용 후 가중 휘도 임계값), 추출 패스와 뷰포트 전환·복구, `PostProcessSettings`에 Threshold와 디버그 뷰, EditorUI 패널.
  - Step 6 (`80b825f`): `blurX`/`blurY` 셰이더(5탭 σ=1 가우시안), `dx`/`dy` 상수, CLAMP 샘플러, BlurX/BlurY 타깃, `DrawFullScreenPass` 헬퍼 추출, `PostProcess.hlsli`로 cbuffer 선언 분리, 디버그 뷰 Blur X/Blur.
  - Step 7 (`4df455b`): 톤 매핑 셰이더가 `t1`로 Blur를 받아 `hdr * exposure + bloom * strength`로 합성. 헬퍼에 두 번째 SRV(기본값 `nullptr`) 추가, EditorUI Strength 슬라이더.
  - (`89f8d64`) 블러 반복 횟수(1~10)와 핑퐁 누적.
  - 강의 프로젝트(`Hong_Graphics2/Graphics_Part2/08_ShaderToys_Step6_BloomEffect`)의 `ImageFilter`·`SamplingPixelShader`·`BlurX/Y`·`Combine`을 읽고 우리 구조와 대조했다. 가중치·분리형 블러·`dx`/`dy`·CLAMP는 그대로 가져왔고, 백버퍼 SRV 방식·사각형 메시·`copyFilter`·필터 클래스는 가져오지 않았다(이유는 §4 "Bloom 파이프라인").
- 확인한 결과:
  - 단계마다 코드 리뷰 후 사용자가 빌드·실행과 RenderDoc으로 확인했고, 새 셰이더 세 개는 에이전트가 `fxc.exe`로 `ps_5_0` 컴파일을 확인했다.
  - 확인한 것: Bright 뷰에 네온과 스위치 Rim만 남음, Blur X 뷰가 가로로만 늘어남, Blur 뷰가 사방으로 번짐, Final 뷰가 Strength 0에서 합성 전과 동일, 반복 1→4에서 번짐 폭 약 2배, 반복을 올려도 화면 밝기 유지(가중치 합 1), 잔상 없음, `Draw(3)` 개수가 `1 + 2n + 1`.
  - 리뷰·디버깅 중 잡은 버그: ① `Initialize`의 null 검사가 인자가 아닌 멤버(`m_gameObject`)를 봐서 항상 즉시 반환 → 고친 뒤에는 nullptr 역참조, ② `SetHighlighted`가 값이 아니라 true/false로 분기해 강조가 꺼지지 않음, ③ 절반 크기 0 방지 누락, ④ `!!`로 조건이 뒤집힘, ⑤ `||` 단락 평가로 두 번째 리사이즈가 실행되지 않고 실패 로그도 구분 불가, ⑥ 로그가 실제 만든 크기가 아닌 값을 계산해 찍음, ⑦ 추출 패스에 `Draw`·VS·IA 누락, ⑧ BlurX가 Bright가 아닌 HDR을 읽음, ⑨ `&m_clampSamplerState`(`ComPtr::operator&`)로 샘플러가 Release됨, ⑩ 디버그 뷰 입력 바인딩이 출력 교체보다 앞서 SRV가 null이 됨, ⑪ 기본 매개변수 뒤에 일반 매개변수를 두고 정의에도 기본값을 씀, ⑫ 합성을 톤 매핑·감마 뒤에서 함, ⑬ `t1` 해제 누락, ⑭ 반복 루프가 매 회차 Bright에서 읽어 반복이 무의미, ⑮ 고친 뒤 첫 회차가 이전 프레임 BlurY를 읽어 프레임 간 되먹임.
- 남아 있는 문제: §5 참고. 룩 재튜닝 미완(값이 전부 기본값), 단일 해상도 반복 방식의 √n 수확 체감, 하드 컷 임계값의 시간적 불안정과 반딧불이 대응 없음, 데이터 텍스처용 sRGB 선택 인자 없음, `BeginFrame`의 옛 RTV 주석.
- 다음에 이어서 할 작업: §6의 룩 재튜닝 → `main` 머지 → 로드맵 10번.
- 중요한 설계 결정과 이유: §4 "Bloom 파이프라인"에 정리했다. 핵심 요약:
  - 절반 해상도 float 타깃에서 추출·블러하고, 합성은 톤 매핑 셰이더 안에서 톤 매핑 **전에** 한다. exposure는 원본에만 곱한다(Bloom에는 추출 단계에서 이미 반영).
  - 분리형 블러와 합이 1인 가중치, 반복은 첫 회차만 Bright에서 읽어 누적시킨다.
  - 디버그 뷰는 CPU 전용 값이라 상수 버퍼에 넣지 않고, Bright 타깃을 보존해 단계별로 눈으로 확인할 수 있게 했다.
  - 패스가 셋이 된 시점에 `DrawFullScreenPass`를 뽑되, 뷰포트·null 검사·정책 선택은 호출자에 남겼다. 강의식 필터 클래스는 다단계 다운샘플이 필요해질 때로 미뤘다.

### 2026-09-15 — HDR 씬 타깃, 톤 매핑, 선형 색공간 (로드맵 11번 Step 1~4, 3-d) ✅

- 완료한 작업:
  - (`9d195bd`) 골목 텍스처를 `Assets/Textures/CyberpunkAlley/`의 PNG 4장으로 교체하고 `greyBoxMat`을 `wallMat`/`floorMat`으로 분리했다.
  - Step 1 (`74aebd8`): `D3D11Utils::CreateRenderTargetTexture`, `GraphicsResourceManager::CreateRenderTarget`/`GetRTV`/`ResizeRenderTarget`, Renderer의 `R16G16B16A16_FLOAT` HDR 씬 타깃과 `Renderer::Resize`.
  - Step 2 (`0e75485`): `D3D11Utils::CreateVertexShader`, `fullscreenVertexShader.hlsl`/`copyPixelShader.hlsl`, `Renderer::EndScene`, `BeginFrame`의 출력 대상을 HDR 타깃으로 교체.
  - Step 3 (`d5d36d7`): 씬 셰이더 `saturate` 제거, `toneMappingPixelShader.hlsl`(Reinhard), `PostProcessSettings`/`PostProcessConstantData`와 Exposure 경로, `.editorconfig`. EditorUI의 Exposure 슬라이더 부분은 사용자 요청으로 에이전트가 작성했다.
  - Step 4 (`e17b870`): 톤 매핑 셰이더 출력 감마 인코드, 파일 텍스처 `R8G8B8A8_UNORM_SRGB`, `Renderer::SrgbToLinear`로 Material/Light/배경 색 변환.
  - 3-d (`86f17a4`): ACES 근사 곡선(사용자 작성)과 `ToneMapper` enum·상수 버퍼·셰이더 분기·Tone Mapper 콤보박스(사용자 요청으로 에이전트 작성).
  - (`10fa116`) Play에서 상호작용 가능할 때만 스위치 Rim 강조(사용자 작성).
  - 그래픽스 디버거 RenderDoc 사용법을 작업과 함께 익혔다: Launch 설정(Working Directory), F12 캡처, Event Browser/API Inspector/Pipeline State/Texture Viewer/Mesh Viewer, Range Auto-fit, Pixel/Vertex Debug, Highlight Drawcall, `renderdoccmd convert`로 캡처를 XML로 바꿔 리소스 포맷 검색.
- 확인한 결과:
  - 단계마다 코드 리뷰 후 사용자가 빌드·실행과 RenderDoc 캡처로 확인했고, 셰이더는 에이전트가 `fxc.exe`로 컴파일을 확인했다.
  - 확인한 것: 로그로 HDR 타깃 크기와 리사이즈 후 index 유지, ①② 중간 단계의 의도된 검은 화면(`Draw(3)`의 `t0`가 아무도 그리지 않은 HDR 텍스처임을 추적), ③ 이후 원래와 같은 화면, 3-a에서 HDR 텍스처의 네온 픽셀이 1을 넘고 백버퍼에서는 1로 잘림, Reinhard 적용 후 네온 밝기 구분, PS `b0`의 exposure/toneMapper 값, 4-b에서 씬 `DrawIndexed`의 `t0`가 `_SRGB`, 4-c에서 `b1`/`b0` 색 값이 ImGui 값보다 작아짐.
  - 리뷰·디버깅 중 잡은 버그: ① `BindFlags`에 `||`를 써서 값이 1(`VERTEX_BUFFER`)이 됨, ② `ResizeRenderTarget` 범위 검사 `>`(→ `>=`), ③ HDR 타깃 높이 자리에 너비를 넘겼는데 고정 문자열 로그가 이를 가림, ④ `OMSetRenderTargets` 개수 0으로 출력이 없어 ImGui까지 사라짐, ⑤ 수정 후 빌드하지 않고 RenderDoc에서 옛 exe를 실행, ⑥ `BeginFrame`의 Light 버퍼 `b0` 바인딩이 주석 처리돼 두 번째 프레임부터 조명이 사라질 상황(2026-10-23 정정: 그 줄이 Light `b0`의 유일한 바인딩이라 실제로는 첫 프레임부터 사라진다), ⑦ 새 셰이더가 CP949로 저장됨(→ `.editorconfig`), ⑧ RenderDoc에서 ImGui 폰트/HDR 텍스처를 보고 `_SRGB`가 없다고 오인(`renderdoccmd convert`로 `_SRGB` 텍스처 4개 생성 확인), ⑨ `ACESFilm` 분모에서 `x` 누락(`2.43 * +0.59`), ⑩ 강조를 끌 때 `RimPower`를 0으로 설정(`pow(0, 0)` NaN 위험).
- 남아 있는 문제: §5 참고. Rim 강조 책임이 AppBase에 있음(`GetGameObject` 노출, ESC 후 강조 잔존), 룩 재튜닝 전(ACES 채널별 색조 이동 포함), 데이터 텍스처용 sRGB 선택 인자 없음, 출력 감마 `1/2.2` 근사와 Reinhard 분기 `pow` 경고, `copyPixelShader` 미사용, `BeginFrame`의 옛 RTV 주석. Bloom은 아직 없다.
- 다음에 이어서 할 작업: §6 순서대로 Rim 강조를 `PowerSwitch`로 이동 → Bloom Step 5~7 → 룩 재튜닝.
- 중요한 설계 결정과 이유: §4 "HDR 씬 타깃, 후처리 패스와 선형 색공간"과 "PowerSwitch 컴포지션"의 재검토 항목에 정리했다. 핵심 요약:
  - 렌더 타깃은 같은 슬롯에서 재생성하고(핸들 유지, generation 불필요, raw pointer는 매번 조회), 포맷·크기와 리사이즈 전파는 Renderer가 맡는다.
  - `EndScene`이라는 약속 이름으로 AppBase를 Bloom까지 고정하고, 패스마다 "출력 → 입력 → Draw → 해제"와 필요한 상태 전부를 직접 설정한다.
  - 선형 색공간은 `_SRGB` 텍스처 포맷 + 톤 매핑 셰이더 출력 인코드 + Renderer 경계의 색 상수 변환으로 구성했다(ImGui 이중 인코드 회피, 선택기와 화면 일치).
  - ACES는 선형 워크플로 이후, 룩 재튜닝은 Bloom 이후에 한 번에 한다.
  - PowerSwitch는 상속을 쓰지 않고 컴포지션을 유지하되, 두 번째 상호작용 오브젝트가 생기면 `InteractableComponent`로 전환한다.
  - 인코딩 문제의 재발을 막기 위해 `.editorconfig`로 확장자별 인코딩을 강제했다.

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

- `git pull` 후 `main`의 HEAD가 최소 `f91b0e6` 이후(이 문서의 커밋)인지 확인한다.
- 밉맵 확인: 골목 입구에서 먼 바닥을 보며 천천히 걸었을 때 흰 점이 깜빡이지 않고 부드럽게 흐려져 있어야 한다. 가까운 바닥의 z 방향 줄무늬는 UV 타일링 문제라 정상이다. 로드맵 11번과 텍스처 색공간 작업은 전부 `main`에 있으므로 기능 브랜치를 체크아웃할 필요가 없다.
- 룩 확인: 전원 Off에서 골목이 차가운 청색이고 끝벽의 빨간 대기 표시등이 보이는지, On에서 네온이 번지면서도 벽은 은은하게만 물드는지 확인한다. 모니터 밝기에 따라 다르게 보이므로 너무 어둡거나 밝으면 Exposure부터 조정한다(§6의 조정 순서 참고).
- Play에서 상호작용 거리 안에서 스위치를 바라볼 때만 가장자리가 호박색으로 밝아지고(Rim 강조), HUD의 `[E] Interact`가 같은 타이밍으로 뜨는지 확인한다. ESC나 Alt+Tab으로 나가면 Editor에서 강조가 꺼져 있어야 한다.
- Editor 패널 Post Process에서 Tone Mapper(Reinhard/ACES)와 Exposure를 바꾸면 화면이 즉시 바뀌고, 전원을 켰을 때 네온 세 개의 밝기가 서로 다르게 보이는지 확인한다.
- Bloom 확인: 전원을 켠 뒤 Editor로 나와 Debug View를 Bright → Blur X → Blur → Final로 돌려본다. Bright에 네온과 Rim만, Blur X는 가로로만, Blur는 사방으로 번져야 한다. Strength 0이면 Bloom이 없는 화면과 같고, Blur Iterations를 올리면 번짐이 넓어진다.
- 창 크기 변경·최대화·최소화 후 복원에도 화면이 정상인지 확인한다(HDR 씬 타깃 재생성).
- 저장소 루트에 `.editorconfig`가 있는지 확인한다. RenderDoc을 쓴다면 Launch 설정의 Executable Path/Working Directory를 그 PC의 경로로 맞추고, C++ 수정 후에는 먼저 빌드한다.
- `AGENTS.md`와 이 문서의 마지막 갱신일이 같은지 확인한다.
- `git status`에서 사용자 변경과 로컬 `MyPF/imgui.ini` 변경을 구분한다.
- `GeometryGenerator::MakeCube`가 1m 단위인지 확인한다.
- Greybox 바닥/벽과 PowerSwitch Transform이 §3의 수치와 같은지 확인한다.
- `InputSystem::WasKeyPressed`와 `EndFrame`이 연결됐는지 확인한다.
- Play에서 스위치를 바라보며 상호작용 거리(Ray-BoundingBox 교차) 안에 있을 때 E로 전원과 스위치 색상이 On/Off 전환되는지 확인한다.
- 등록된 네온 세 개(Pink/Cyan/Orange)와 연결 Point Light가 1.4초 간격으로 함께 켜지고 역순으로 꺼지는지 확인한다.
- 시퀀스에 등록되지 않은 `EnvironmentFillLight`는 전원과 무관하게 항상 켜져 있는 것이 정상이다. 다만 도시 불빛이 새어 들어온 것을 흉내내는 값이라 **색이 눈에 띄면 안 된다** — 예전처럼 보라색 덩어리로 보이면 세팅이 되돌아간 것이다.
- 대규모 Unreal식 명명 마이그레이션이나 범용 ECS를 시작하지 않는다.
- 사용자가 요청하지 않으면 빌드, 실행, commit, push하지 않는다.
