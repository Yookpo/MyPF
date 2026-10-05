\---

description: MyPF DX11 프로젝트를 분석하여 학습용 웹사이트를 생성하거나 갱신한다.

\---



\# MyPF Study Site



목적은 사용자가 자신이 작성한 DirectX 11 프로젝트를

다시 분석하고 공부할 수 있는 웹사이트를 만드는 것이다.



반드시 AGENTS.md의 규칙을 따른다.



\## 중요한 규칙



\- .cpp / .h / .hlsl / .hlsli는 수정하지 않는다.

\- 사이트 파일은 study-site/ 아래에서만 생성하거나 수정한다.

\- AGENTS.md만 복사해서 문서화하지 않는다.

\- 실제 소스 코드를 읽고 현재 구현을 확인한다.

\- 코드와 문서가 다르면 실제 코드를 기준으로 한다.

\- 코드에 존재하지 않는 구조를 추측하지 않는다.



\## 학습 관점



설명할 때 다음을 연결한다.



\- File

\- Class

\- Function

\- Caller

\- Callee

\- DX11 API

\- GPU Pipeline Stage

\- 관련 HLSL

\- Resource ownership



\## 우선 작성할 페이지



1\. Architecture

2\. Frame Analysis

3\. Rendering Pipeline



이 세 페이지가 검증되기 전에는

사이트 범위를 과도하게 확장하지 않는다.

