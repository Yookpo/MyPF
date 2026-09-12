# CLAUDE.md

@AGENTS.md

---

## MyPF 우선 규칙

위 AGENTS.md가 이 프로젝트의 단일 진실 원본이다. 아래 일반 코딩 지침과 충돌하면 **AGENTS.md가 항상 우선한다.**

- **한국어로 응답한다.**
- **C++ / HLSL 코드는 사용자가 직접 작성한다.** 에이전트는 설계 가이드만 제공한다. 완성 코드는 사용자가 막혔다고 명시하고 요청할 때만 제시한다. 상세 형식은 AGENTS.md §2.
- **이 프로젝트에는 테스트 프레임워크가 없다.** 아래 §4의 "테스트를 작성해 통과시켜라"는 그대로 적용되지 않는다. 대신 **완료 조건을 문장으로 정의**하고, 사용자가 `Debug | x64`로 빌드·실행해 화면으로 확인한다. 빌드와 실행은 사용자가 요청할 때만 수행한다.
- **새로 쓰는 파일은 UTF-8 with BOM.** 전체 소스는 `5a5f463` 커밋에서 UTF-8 BOM으로 통일됐다(과거 CP949로 깨졌던 주석은 해결됨).
- `BufferHandle` / `TextureHandle` / `RenderItem` / `FrameRenderData`는 얇아 보여도 의도된 계층 경계다. §2의 "단일 사용 추상화 금지" 대상이 아니다.
- 진행 기록, 완료 기능 목록, 설계 결정 상세는 `MyPF/docs/CODEX_HANDOFF.md`에 있다. 자동 로드하지 않으므로 필요할 때 읽는다.

---

## 일반 코딩 지침

Behavioral guidelines to reduce common LLM coding mistakes.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

### 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

### 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

### 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

### 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals. For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

이 프로젝트에서 `verify`는 자동 테스트가 아니라 **사용자가 실행해 화면으로 확인하는 완료 조건**이다. 위 MyPF 우선 규칙을 참고한다.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.
