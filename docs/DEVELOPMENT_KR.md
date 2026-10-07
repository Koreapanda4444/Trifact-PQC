# TRIFACT 개발 환경

[English](DEVELOPMENT.md) | [한국어](DEVELOPMENT_KR.md)

> **상태: 활성 개발 환경 설정**
>
> 현재 codebase는 연구용 prototype이다. Signature scheme을 구현하지 않으며 보안 용도로 사용해서는 안 된다.

## Toolchain

reference implementation은 다음을 사용한다.

- compiler-specific language extension을 사용하지 않는 C17;
- configuration 및 build용 CMake 3.24 이상;
- test 실행용 CTest;
- source formatting용 `clang-format` 18;
- warning을 error로 처리하는 compiler 설정;
- Linux의 GCC 및 Clang;
- Windows의 MSVC;
- 자동 check용 GitHub Actions.

초기 toolchain에는 third-party C library dependency가 없다. Solver library는 해당 analysis commit을 시작할 때 별도로 추가한다.

data representation, allocation, integer bound, canonical byte processing을 명시적으로 다루기 위해 reference implementation 언어로 C를 선택했다. C를 사용하더라도 이 프로젝트는 연구용 prototype이며 production에 적합해지는 것은 아니다.

## 저장소 구조

| 경로 | 목적 |
|---|---|
| `.github/workflows/ci.yml` | Formatting·엄격한 build·test·sanitizer·정적 분석 |
| `include/trifact/` | 공개 core·relation·incidence·witness·hash·entropy·연구·sampling API |
| `src/` | C17 구현과 비공개 운영 heap wrapper |
| `tests/` | 결정적 test·전수 oracle·불변식·테스트 allocator |
| `docs/` | 영문 명세와 대응하는 한국어 문서 |
| `analysis/` | 이후 solver·실험·benchmark output |
| `CMakeLists.txt` | Build 정의와 선택적 진단 검사 |
| `.clang-format` | Source formatting 규칙 |

현재 codebase는 소유 canonical hypergraph·label vector, native R3HFR relation validator, 검증된 정점 incidence 인덱스, witness label 정규화·동등성 비교를 구현한다. [코어 API](CORE_API_KR.md)에 해당 소유권·오류·core CTest target 8개를 정리한다. [원시 함수와 난수 공급자](CRYPTO_PROVIDERS_KR.md)는 구현한 SHAKE256·domain hash·OS entropy·연구 stream·sampling을 설명한다. [암호 테스트 벡터](CRYPTO_VECTORS_KR.md)는 독립 정답을 기록한다. Linux의 CTest target은 20개, Windows는 19개다. 추가 Linux test는 자신의 executable에서만 `getrandom`을 대체한다. Windows는 native 시스템 `bcrypt` library를 연결한다. KeyGen, codec, recovery solver, proof 및 signature는 이후 작업이다.

## Windows 설정

필요한 도구는 다음과 같다.

- Desktop development with C++ workload를 포함한 Visual Studio Build Tools;
- Command Prompt 또는 PowerShell에서 사용할 수 있는 CMake.

workload 이름에는 C++이 포함되지만 TRIFACT는 `LANGUAGES C`로 project를 설정하고 C17 source file을 compile한다.

repository root에서 다음을 실행한다.

```powershell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

기본 Visual Studio generator는 MSVC를 선택한다. 생성 파일은 `build/` 아래에 남는다.

## Linux 설정

GCC 사용:

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Clang 사용:

```bash
cmake -S . -B build-clang -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build-clang --parallel
ctest --test-dir build-clang --output-on-failure
```

## Sanitizer와 정적 분석

ASan과 UBSan은 선택 사항이며 일반 build에서는 비활성화한다. 지원되는 GCC 또는 Clang toolchain에서는 다음을 실행한다.

```bash
cmake -S . -B build-sanitizers -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON -DTRIFACT_ENABLE_SANITIZERS=ON
cmake --build build-sanitizers --parallel
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ctest --test-dir build-sanitizers --output-on-failure
```

Sanitizer 설정은 compiler와 linker 지원 여부를 확인한다. Instrumentation과 frame pointer 설정은 운영 core·테스트 allocator build·test executable로 전파된다. 지원하지 않는 요청은 configuration 단계에서 명시적으로 실패한다. CI는 Linux GCC·Clang에서 leak detection을 켜고 첫 sanitizer diagnostic에 중단한다. Windows MSVC는 일반 strict build와 test를 유지한다.

GCC 분석에는 `-fanalyzer`를 지원하는 compiler가 필요하며 기존 warning 규칙에 따라 발견 사항을 error로 처리한다.

```bash
cmake -S . -B build-analysis -DCMAKE_C_COMPILER=gcc -DBUILD_TESTING=OFF -DTRIFACT_ENABLE_GCC_ANALYZER=ON
cmake --build build-analysis --parallel
```

다른 compiler 또는 지원하지 않는 GCC에 이 option을 요청하면 configuration이 실패한다. Clang 설정은 text diagnostic을 출력하고 분석 발견 사항을 error로 처리하는 별도 core 분석 target을 제공한다.

```bash
cmake -S . -B build-clang-analysis -DCMAKE_C_COMPILER=clang -DBUILD_TESTING=OFF
cmake --build build-clang-analysis --target trifact-clang-analysis
```

Compiler 분석과 sanitizer coverage는 구현 검사이며 암호학적 안전성의 근거가 아니다. `BUILD_TESTING=OFF`인 운영 전용 build는 실패 주입 allocator를 compile하거나 link하지 않는다.

## Formatting

현재 C file을 다음 명령으로 검사한다.

```bash
git ls-files -z -- '*.c' '*.h' | xargs -0 clang-format-18 --dry-run --Werror
```

formatting 적용:

```bash
git ls-files -z -- '*.c' '*.h' | xargs -0 clang-format-18 -i
```

CI는 Ubuntu 24.04에서 지정한 formatter를 설치하고 비공개·테스트 header를 포함해 추적 중인 모든 C source와 header를 검사한다. 새로 stage 또는 commit한 C file은 이 manifest에 자동으로 들어간다. 추적하지 않는 새 file은 stage 전에 직접 format한다.

## Compiler check

GCC 및 Clang build는 다음을 활성화한다.

```text
-Wall
-Wextra
-Wpedantic
-Werror
-Wconversion
-Wsign-conversion
-Wshadow
-Wstrict-prototypes
```

MSVC build는 다음을 활성화한다.

```text
/W4
/WX
```

warning을 global하게 suppress하지 않는다. local suppression이 필요하면 양쪽 개발 문서에 근거를 남긴다. Source file에는 코드 주석을 넣지 않는다.

## CI 규칙

CI는 독립적인 여덟 가지 check를 수행한다.

1. Linux source formatting;
2. Linux GCC build 및 test;
3. Linux Clang build 및 test;
4. Windows MSVC build 및 test;
5. Linux GCC의 ASan·UBSan·leak 검사;
6. Linux Clang의 ASan·UBSan·leak 검사;
7. GCC core 정적 분석;
8. Clang core 정적 분석.

갱신을 완료로 판단하기 전에 모든 job이 통과해야 한다. CI의 repository-content permission은 read-only이며 package publish, key 생성, release 생성, repository 수정을 수행하지 않는다.

## Test 규칙

모든 implementation commit은 동작에 맞는 test를 추가하거나 갱신해야 한다. test는 다음을 만족해야 한다.

- deterministic해야 한다.
- network access에 의존하지 않는다.
- 예상 failure를 명시적으로 검사한다.
- 실제 secret material을 포함하지 않는다.
- 세 compiler에서 모두 유효해야 한다.
- undefined behavior가 없어야 한다.
- parameter 및 allocation limit 안에서 안전해야 한다.

test는 실패할 때 0이 아닌 process status를 반환해 CTest가 실패한 executable을 보고할 수 있게 한다.

## Integer 및 memory 규칙

implementation은 encoded protocol value에 `<stdint.h>`의 fixed-width integer type을 사용한다. in-memory size와 index에는 checked conversion 이후 `size_t`를 사용한다.

count, byte length, product, allocation size를 포함하는 모든 계산은 allocation 전에 overflow를 검사해야 한다. Parser input을 신뢰하지 않는다. 실패 시 partial output object를 정리해야 한다.

dynamic allocation은 object lifetime과 cleanup ownership을 영문·한국어 코어 API 또는 internal module contract에 문서화한 경우에만 추가한다.

## Dependency 규칙

새 dependency에는 구체적인 연구 목적이 필요하다.

추가 전 다음을 수행한다.

1. dependency가 필요한 module을 확인한다.
2. C standard library만으로 충분하지 않은지 확인한다.
3. CMake가 dependency를 찾는 방법을 정의한다.
4. 허용하는 dependency release range를 기록한다.
5. dependency가 없거나 있는 경우에 대한 test를 추가한다.
6. 재현에 필요한 native-library 및 solver setting을 기록한다.

SAT 또는 exact-cover dependency는 analysis target에 속하며 mathematical core의 public interface로 새어 들어가면 안 된다.

## Generated file

build directory, CMake state, compiler object, library, executable, debug database, editor state, log file은 commit하지 않는다.

raw benchmark data는 폐기 가능한 build artifact가 아니다. 이후 analysis 명세에서 재현을 위해 commit할 input과 result를 정한다.

## 현재 완료 조건

다음 조건을 만족하면 이 환경 설정이 완료된다.

- static library가 C17로 compile된다.
- toolchain smoke test가 CTest를 통해 통과한다.
- source가 formatting rule을 만족한다.
- GCC, Clang, MSVC build가 warning을 error로 처리한다.
- CI가 Linux 및 Windows에서 실행된다.
- core·암호 oracle, 보관 vector, entropy 실패, 할당 실패 test를 포함한 등록 CTest target이 모두 통과한다.
- GCC·Clang sanitizer build가 leak detection을 켜고 통과한다.
- GCC·Clang core 정적 분석에서 발견 사항이 없다.
- KeyGen, proof, signing 구현을 먼저 넣지 않는다.
