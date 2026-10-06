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
- source formatting용 `clang-format`;
- warning을 error로 처리하는 compiler 설정;
- Linux의 GCC 및 Clang;
- Windows의 MSVC;
- 자동 check용 GitHub Actions.

초기 toolchain에는 third-party C library dependency가 없다. Solver library는 해당 analysis commit을 시작할 때 별도로 추가한다.

data representation, allocation, integer bound, canonical byte processing을 명시적으로 다루기 위해 reference implementation 언어로 C를 선택했다. C를 사용하더라도 이 프로젝트는 연구용 prototype이며 production에 적합해지는 것은 아니다.

## 저장소 구조

```text
.github/workflows/ci.yml  formatting, build, test job
include/trifact/core.h     canonical core data type과 constructor
include/trifact/trifact.h  public umbrella header
src/core.c                 canonical core data model 구현
src/trifact.c              project-level compile-time check
tests/                     core-type 및 relation CTest executable
docs/                     영문 명세와 한국어 counterpart
analysis/                 이후 solver, experiment, benchmark output
CMakeLists.txt            build definition
.clang-format             source formatting rule
```

현재 codebase에는 storage를 소유하는 canonical hypergraph representation과 factor-label vector가 포함되어 있다. Native R3HFR relation validator를 구현했다. API와 소유권 규칙은 [코어 API](CORE_API_KR.md)에 정리한다. KeyGen, codec, recovery solver, proof 및 signature는 이후 작업이다.

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

## Formatting

현재 C file을 다음 명령으로 검사한다.

```bash
clang-format --dry-run --Werror \
  include/trifact/core.h include/trifact/relation.h include/trifact/trifact.h \
  src/core.c src/relation.c src/trifact.c \
  tests/test_core_types.c tests/test_relation.c tests/test_toolchain.c
```

formatting 적용:

```bash
clang-format -i \
  include/trifact/core.h include/trifact/relation.h include/trifact/trifact.h \
  src/core.c src/relation.c src/trifact.c \
  tests/test_core_types.c tests/test_relation.c tests/test_toolchain.c
```

새 C header와 source file을 만드는 commit은 같은 commit에서 CI formatting 명령에도 해당 파일을 추가해야 한다. 이후 tooling commit에서 explicit list를 checked manifest로 대체할 수 있다.

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

CI는 독립적인 네 가지 check를 수행한다.

1. Linux source formatting;
2. Linux GCC build 및 test;
3. Linux Clang build 및 test;
4. Windows MSVC build 및 test.

다음 작업으로 넘어가기 전에 모든 job이 통과해야 한다. CI의 repository-content permission은 read-only이며 package publish, key 생성, release 생성, repository 수정을 수행하지 않는다.

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
- native relation의 정상·실패 사례가 통과한다.
- KeyGen, proof, signing 구현을 먼저 넣지 않는다.
