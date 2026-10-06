# TRIFACT 코어 API

[English](CORE_API.md) | [한국어](CORE_API_KR.md)

## 소유권과 생성

`trifact_edge_init`은 `first < second < third < vertex_count`인 입력만 허용한다. 입력을 정렬하지 않으며 실패하면 출력 edge를 그대로 둔다.

`trifact_hypergraph_create`는 canonical·중복 없음·사전순 정렬을 만족하는 edge list를 복사한다. 일반 컨테이너는 정점 수가 3 이상이면 생성할 수 있고 빈 그래프도 허용한다. 생성 성공만으로 R3HFR 관계가 성립하지는 않는다.

`trifact_label_vector_create`는 factor 수나 관계 검증 없이 label을 복사한다. 길이가 0인 vector도 허용한다. graph와 vector가 복사한 배열을 소유하며 accessor는 해제할 때까지 유효한 읽기 전용 참조를 반환한다. 참조한 저장소를 수정하지 않는다.

객체 생성은 실패하면 출력 slot을 `NULL`로 만든다. 빈 출력 slot을 사용하고 기존 객체를 바꾸기 전에 먼저 해제한다. 해제 함수는 `NULL`을 허용한다. 해제 후 소유 포인터를 `NULL`로 바꾼 뒤 다시 해제해야 한다.

## Factorization 관계

`trifact_relation_validate(hypergraph, factor_count, labels, error)`는 공개 그래프와 witness가 R3HFR 명세를 만족할 때만 `TRIFACT_STATUS_OK`를 반환한다. 두 객체는 수정하지 않는다.

관계는 `n >= 6`, `n % 3 == 0`, `2 <= d <= (n-1)(n-2)/2`, `m = dn/3`, witness 길이 `m`, label 범위 `[0,d)`, 정점 degree `d`, 정점별 label 중복 금지를 검사한다. canonical edge와 simple 조건은 graph 생성기가 보장한다.

`error`는 생략할 수 있다. 제공하면 내부 status가 반환 status와 같아진다. 정점·edge index를 지정할 수 없는 경우와 성공 시에는 `SIZE_MAX`를 넣는다. 범위 밖 label은 첫 edge index, degree 오류는 첫 정점 index를 기록한다. 국소 중복은 같은 label이 다시 등장한 정점과 뒤쪽 incident edge를 기록한다.

| Status | 의미 |
|---|---|
| `NULL_ARGUMENT` | 필수 graph 또는 label 객체가 없다. |
| `INVALID_VERTEX_COUNT` | 관계의 정점 수 조건을 위반했다. |
| `INVALID_FACTOR_COUNT` | 관계의 degree 조건을 위반했다. |
| `SIZE_OVERFLOW` | 개수나 할당 곱셈이 `size_t`에 들어가지 않는다. |
| `EDGE_COUNT_MISMATCH` | 공개 edge 수가 `dn/3`과 다르다. |
| `WITNESS_LENGTH_MISMATCH` | witness 길이가 edge 수와 다르다. |
| `LABEL_OUT_OF_RANGE` | 후보 label이 `[0,d)` 밖이다. |
| `VERTEX_DEGREE_MISMATCH` | 공개 정점의 degree가 `d`가 아니다. |
| `INCIDENT_LABEL_COLLISION` | 같은 label로 한 정점이 두 번 등장한다. |
| `ALLOCATION_FAILURE` | 임시 검증 저장소를 확보하지 못했다. |

인자·파라미터 조건·예상 edge 수·witness 길이·label 범위를 먼저 확인하고 degree와 국소 중복을 검사한다. 임시 메모리는 `O(n + nd)`, 계산량은 `O(n + nd + m)`이다. 할당이나 산술 실패는 오류이며 factorization이 없다는 판정으로 해석하지 않는다.

## 테스트

`trifact.relation` CTest target은 정상 factorization과 각 실패 분류를 diagnostic index·diagnostic 생략 경로와 함께 검사한다. 코어는 코드 주석 없는 C17이다. 키 생성과 proof 구현은 이후 작업이다.

`trifact.relation-oracle`은 각 factor가 모든 정점을 중복 없이 덮는 edge cover인지 독립적으로 판정한다. 검증기의 degree counter나 incident-label table을 사용하지 않는다. 정점 6개·edge 0~4개인 모든 simple graph와 `d = 2`에 대한 `{0,1,2}`의 모든 label을 전수 검사한다. 크기 오류·불규칙 공개 그래프·범위 밖 label을 포함한 후보 424,996개 중 정확히 90개가 관계를 만족한다. 정점 6개·degree 3인 그래프, 정점 9개인 그래프, 정점 수 조건을 위반한 그래프의 label도 전수 검사한다. 이 제한된 검사는 구현 간 일치를 확인하며 암호학적 난도를 입증하지 않는다.
