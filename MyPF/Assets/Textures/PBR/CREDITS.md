# PBR 텍스처 출처

이 폴더의 텍스처는 재배포가 허용된 CC0 에셋만 둔다(에셋 정책: `MyPF/docs/CODEX_HANDOFF.md` §7).

| 폴더 | 에셋 | 출처 | 라이선스 | 실제 크기 | 받은 날짜 | 커밋한 파일 |
|---|---|---|---|---|---|---|
| `Asphalt023S_2K/` | Asphalt 023 S (2K-JPG) | [ambientCG](https://ambientcg.com/view?id=Asphalt023S) | [CC0 1.0](https://docs.ambientcg.com/license/) | 1.25m × 1.25m | 2026-10-23 | `_Color.jpg`, `_Roughness.jpg`, `_NormalDX.jpg`, 미리보기 `Asphalt023S.png` |
| `Bricks096_2K/` | Bricks 096 (2K-JPG, 2048×1024) | [ambientCG](https://ambientcg.com/view?id=Bricks096) | [CC0 1.0](https://docs.ambientcg.com/license/) | 약 2m × 1m (추정\*) | 2026-10-26 | `Bricks096_2K_Color.jpg`, `_Roughness.jpg`, `_NormalDX.jpg`, 미리보기 `Bricks096.png` (받은 이름의 `-JPG`를 지웠다\*\*) |

ambientCG는 출처 표기를 요구하지 않지만 권장하므로 기록한다.

\* ambientCG가 Bricks096의 실제 크기를 공개하지 않는다(API `dimensionX/Y = 0`). 이미지가 2:1이고 큰 돌이 폭의 약 1/4(40~50cm)이라 2m × 1m로 추정했다. 벽 UV Scale이 이 값에서 나온다.

\*\* ambientCG가 주는 이름은 `Bricks096_2K-JPG_Color.jpg` 형식이다. Bricks096은 `-JPG`를 지운 이름으로 저장했고 `AppBase.cpp` 경로도 그 이름을 쓴다(Asphalt023S는 받은 이름 그대로다). 다시 받으면 이름을 똑같이 바꾼다.

## 용도

| 파일 | `TextureType` | 쓰는 곳 |
|---|---|---|
| `*_Color.jpg` | `Albedo` (sRGB) | 바닥(Asphalt023S)·벽(Bricks096) albedo |
| `*_Roughness.jpg` | `Data` (선형) | roughness 맵 (`roughnessFactor`와 곱한다) |
| `*_NormalDX.jpg` | `NormalMap` (선형) | 노멀 맵. DirectX 규약(G = 아래)이라 셰이더에서 G 채널을 뒤집지 않는다. `_NormalGL`은 쓰지 않는다 |

`_AmbientOcclusion`, `_Displacement`는 로컬에만 있고 `.gitignore`로 커밋에서 뺐다. 쓰기 시작하는 단계에서 `.gitignore`의 해당 줄을 지우고 이 표에 추가한다.
