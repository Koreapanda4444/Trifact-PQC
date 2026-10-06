# TRIFACT

[English](README.md) | [한국어](README_KR.md)

> **실험적이며 검증되지 않은 연구 프로젝트입니다. 실제 환경에서 사용하지 마십시오.**

TRIFACT는 R3HFR(Random 3-Uniform Hypergraph Factorization Recovery) 문제를 기반으로 제안하는 양자내성 전자서명 설계를 연구하는 프로젝트입니다.

현재 프로젝트는 설계 및 실현 가능성 검증 단계입니다. 양자내성 안전성, EUF-CMA 안전성, 표준화된 보안 등급 또는 실제 데이터 보호에 사용할 수 있는 수준을 주장하지 않습니다.

## 연구 방향

- R3HFR 인스턴스 분포와 witness relation을 엄밀하게 정의합니다.
- 작고 재현 가능한 참조 모델을 구축합니다.
- SAT, exact-cover, peeling, 대칭성과 구조적 복구 공격을 평가합니다.
- 정의된 타당성 검증 단계를 기반 문제가 통과한 경우에만 proof 및 signature 계층으로 진행합니다.

## 저장소 구조

- `docs/` — 설계 명세, 결정 사항 및 평가 보고서
- `analysis/` — R3HFR solver, 실험 및 benchmark
- `include/` — public C header
- `src/` — C17 reference implementation
- `tests/` — 정확성, 실패 사례 및 회귀 테스트

## 개발

prototype은 C17과 CMake를 사용하며 GCC, Clang, MSVC build를 자동 검사합니다. [개발 환경](docs/DEVELOPMENT_KR.md)과 [코어 API](docs/CORE_API_KR.md)를 참고하십시오.

## 문서 언어

영문 문서를 기준으로 합니다. 한국어 번역본은 동일한 이름에 `_KR` 접미사를 붙여 제공합니다.

## 보안

TRIFACT는 새롭고 검증되지 않은 계산 난제를 기반으로 하는 연구용 소프트웨어입니다. 실제 환경에서 사용하거나 보안을 의존해서는 안 됩니다.
