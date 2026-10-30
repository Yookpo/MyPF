# HDRI 출처

이 폴더에는 재배포가 허용된 CC0 HDRI와 그것을 변환한 큐브맵만 둔다(에셋 정책: `MyPF/docs/CODEX_HANDOFF.md` §7).

| 파일 | 원본 | 출처 | 라이선스 | 받은 날짜 |
|---|---|---|---|---|
| `hansaplatz_cube512.dds` | Hansaplatz (2K `.hdr`, 2048 × 1024 equirect) | [Poly Haven](https://polyhaven.com/a/hansaplatz) | [CC0 1.0](https://polyhaven.com/license) | 2026-10-27 |
| `NightSkyEnvHDR.dds`, `NightSkyDiffuseHDR.dds`, `NightSkySpecularHDR.dds` | Poly Haven의 밤하늘 HDRI — **정확한 에셋 이름 확인 필요** | [Poly Haven](https://polyhaven.com/hdris) | [CC0 1.0](https://polyhaven.com/license) | 2026-10-28 |

## `NightSky*HDR.dds` (IBL Baker로 구움, 2026-10-28)

- **만든 방법**: 사용자가 Poly Haven 밤하늘 HDRI를 IBL Baker(강의 12장에서 쓰는 도구)에 넣어 구웠다. 원본 HDRI 이름을 알게 되면 위 표를 채운다.
- **형식**: 레거시 DDS 헤더, FourCC 116 = `R32G32B32A32_FLOAT`(32비트 float — hansaplatz의 16비트보다 두 배 크다), 큐브맵 6면(`DDSCAPS2_CUBEMAP` 전 면)
  - `NightSkyEnvHDR.dds`: 512² × 6, 밉 10단계 — 하늘(Skybox)용 원본 환경
  - `NightSkySpecularHDR.dds`: 512² × 6, 밉 10단계 — **roughness별로 미리 거른(prefiltered) specular IBL**. 강의 코드는 밉을 `2 + roughness × 5`(13장) 또는 `3 + roughness × 5`(12장)로 고른다 — IBL Baker 출력 기준 매직 넘버라 이 파일로 다시 확인한다
  - `NightSkyDiffuseHDR.dds`: 128² × 6, 밉 1단계 — **irradiance(diffuse IBL)**. 법선 방향으로 읽는다
- IBL Baker는 BRDF LUT(`*Brdf.dds`)도 만들지만 이 폴더에는 없다. LUT 대신 `EnvBRDFApprox`(10-4)를 쓰거나, 필요하면 다시 굽는다.
- 2026-10-29 현재 코드(`AppBase::InitGreyBoxScene`)는 아직 `hansaplatz_cube512.dds`를 읽는다. P-1 IBL 단계에서 NightSky로 바꾼다.

원본 `hansaplatz_2k.hdr`(6.8MB)는 로컬에만 두고 `.gitignore`로 커밋에서 뺐다. 위 링크에서 다시 받을 수 있다.

## `hansaplatz_cube512.dds`

- **내용**: 함부르크 한자플라츠의 밤 광장. 하늘은 거의 검정(위 면 평균 선형 휘도 0.01)이고 수평선 근처에 가로등·창문 불빛(최대 약 13,800)이 있다. 골목 위로는 어두운 하늘 띠가, 10-4 환경 반사에서는 웅덩이에 도시 불빛이 비친다.
- **형식**: DDS(DX10 헤더) `DXGI_FORMAT_R16G16B16A16_FLOAT`, `D3D11_RESOURCE_MISC_TEXTURECUBE`, 면 512 × 512 × 6, 전체 밉 10단계, 16MB. 선형 HDR이라 sRGB 변환이 없다.
- **변환**: equirect → 큐브맵을 직접 작성한 Node.js 스크립트로 했다(외부 도구 없음). 면마다 2 × 2 슈퍼샘플로 equirect를 쌍선형 샘플하고, 밉은 2 × 2 상자 필터로 만들었다. 면 순서·방향은 D3D 규약(+X, −X, +Y, −Y, +Z, −Z, 왼손 좌표계, +Y 위)이고, equirect 가운데가 +Z(골목 안쪽 방향), 오른쪽이 +X다(좌우 반전 없음, 십자 펼침으로 확인).
- **밉은 단순 축소다.** roughness에 맞춘 prefiltered(GGX) 밉이 아니다. 10-5 IBL을 하게 되면 irradiance·prefiltered 맵을 따로 만든다(cmft 등).
