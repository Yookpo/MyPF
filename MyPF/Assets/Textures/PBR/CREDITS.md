# PBR 텍스처 출처

이 폴더의 텍스처는 재배포가 허용된 CC0 에셋만 둔다(에셋 정책: `MyPF/docs/CODEX_HANDOFF.md` §7).

| 폴더 | 에셋 | 출처 | 라이선스 | 실제 크기 | 받은 날짜 | 커밋한 파일 |
|---|---|---|---|---|---|---|
| `Asphalt023S_2K/` | Asphalt 023 S (2K-JPG) | [ambientCG](https://ambientcg.com/view?id=Asphalt023S) | [CC0 1.0](https://docs.ambientcg.com/license/) | 1.25m × 1.25m | 2026-10-23 | `_Color.jpg`, `_Roughness.jpg`, 미리보기 `Asphalt023S.png` |

ambientCG는 출처 표기를 요구하지 않지만 권장하므로 기록한다.

## 용도

| 파일 | `TextureType` | 쓰는 곳 |
|---|---|---|
| `*_Color.jpg` | `Albedo` (sRGB) | 바닥 albedo |
| `*_Roughness.jpg` | `Data` (선형) | 바닥 roughness 맵 (`roughnessFactor`와 곱한다) |

`_NormalDX`, `_AmbientOcclusion`, `_Displacement`는 로컬에만 있고 `.gitignore`로 커밋에서 뺐다. 쓰기 시작하는 단계에서 `.gitignore`의 해당 줄을 지우고 이 표에 추가한다.
