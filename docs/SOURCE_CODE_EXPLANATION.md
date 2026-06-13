# RunnerEngine 최종 소스코드 상세 설명

이 문서는 `RunnerEngine` 프로젝트의 최종 소스코드를 비전공자도 따라갈 수 있도록 설명한다. 코드는 C++와 DirectX 11을 사용하며, 복잡한 상용 엔진 구조가 아니라 학습용으로 만든 최소 구조의 러닝 액션 게임 프로토타입이다.

## 1. 전체 구조를 먼저 보는 이유

게임 프로그램은 한 파일만으로 움직이지 않는다. 창을 만드는 코드, 시간을 계산하는 코드, 키보드 입력을 읽는 코드, DirectX로 화면을 그리는 코드, 게임 규칙을 처리하는 코드가 서로 역할을 나누어 실행된다.

이 프로젝트의 중심 흐름은 다음과 같다.

1. `main.cpp`가 프로그램을 시작한다.
2. `Application`이 창, 그래픽, 시간, 입력, 씬을 관리한다.
3. `RunnerGameScene`이 실제 탈옥 런게임 규칙을 처리한다.
4. `GraphicsContext`, `Mesh`, `Material`, `Shader`, `MeshRenderer`가 DirectX 11 렌더링을 담당한다.
5. `ResourceManager`가 렌더링에 필요한 리소스를 만들어 보관한다.

## 2. `main.cpp`

`main.cpp`는 프로그램의 시작점이다.

Windows 데스크톱 프로그램은 일반적인 콘솔 프로그램의 `main()` 대신 `WinMain()`에서 시작한다. 그래서 이 파일은 `Application` 객체를 만들고, 게임 씬인 `RunnerGameScene`을 생성해서 넘긴다.

중요한 흐름은 다음과 같다.

```cpp
Engine::Application app;
auto scene = std::make_unique<Engine::RunnerGameScene>();
app.Initialize(instance, std::move(scene));
return app.Run();
```

`Application`은 게임 전체 실행을 담당하는 객체다. `RunnerGameScene`은 실제 게임 화면과 규칙을 담당하는 씬이다. `std::make_unique`는 객체를 안전하게 동적 생성하기 위해 사용한다. `std::move(scene)`은 씬의 소유권을 `Application`에게 넘긴다는 뜻이다.

## 3. `Application`

`Application`은 프로그램의 가장 큰 실행 관리자다.

주요 역할은 다음과 같다.

- Win32 창 생성
- DirectX 11 초기화
- 게임 씬 초기화
- 메인 루프 실행
- 매 프레임 입력 갱신
- 매 프레임 시간 계산
- 매 프레임 씬 업데이트와 렌더링 호출

게임은 한 번 그려지고 끝나는 프로그램이 아니다. 화면을 계속 갱신해야 한다. 그래서 `Application::Run()`은 창이 닫힐 때까지 반복 실행된다.

반복 흐름은 대략 다음 순서다.

1. 메시지 처리
2. `DeltaTime` 계산
3. 입력 상태 갱신
4. 씬 업데이트
5. DirectX 화면 지우기
6. 씬 렌더링
7. 텍스트와 프레임 출력

`DeltaTime`은 이전 프레임에서 현재 프레임까지 걸린 시간이다. 이 값이 있어야 컴퓨터 성능 차이가 있어도 움직임 속도를 일정하게 유지할 수 있다.

## 4. `WindowContext`

`WindowContext`는 Win32 창을 담당한다.

DirectX는 그릴 대상이 필요하다. 이 프로젝트에서는 Win32 창이 그 대상이다. `WindowContext`는 창 핸들인 `HWND`를 만들고, 키 입력이나 창 닫기 같은 Windows 메시지를 처리할 수 있게 도와준다.

`Esc` 키로 종료되는 흐름도 입력 처리와 창 메시지 처리 흐름에 연결된다.

## 5. `Time`

`Time`은 프레임 사이의 시간을 계산한다.

예를 들어 장애물이 왼쪽으로 움직일 때 단순히 `x -= 0.01f`처럼 쓰면 컴퓨터가 빠를수록 더 빨리 움직인다. 그래서 이 프로젝트는 `x -= speed * dt` 형태로 계산한다.

여기서 `dt`가 `DeltaTime`이다.

## 6. `Input`

`Input`은 키보드 상태를 관리한다.

이 프로젝트에서는 세 가지 방식으로 키를 확인한다.

- `IsKeyDown`: 현재 누르고 있는가
- `WasKeyPressed`: 이번 프레임에 막 눌렸는가
- `WasKeyReleased`: 이번 프레임에 막 떼었는가

이 차이가 중요한 이유는 다음과 같다.

`P` 치트키는 누르는 순간에만 점수 1000점을 올려야 한다. 계속 누르고 있다고 매 프레임 1000점씩 오르면 안 된다. 그래서 `WasKeyPressed('P')`를 쓴다.

가변 점프는 스페이스바에서 손을 떼는 순간을 알아야 한다. 그래서 `WasKeyReleased(VK_SPACE)`를 쓴다.

슬라이드는 키를 누르고 있는 동안 유지되어야 한다. 그래서 `IsKeyDown(VK_DOWN)`을 쓴다.

## 7. `Scene`

`Scene`은 게임 오브젝트들을 담는 공간이다.

`RunnerGameScene`은 `Scene`을 상속한다. 그래서 `RunnerGameScene`은 기본 씬 기능을 물려받고, 자기만의 게임 규칙을 추가한다.

씬은 다음 기능을 가진다.

- `CreateObject`: 게임 오브젝트 생성
- `Update`: 오브젝트 업데이트
- `Render`: 오브젝트 렌더링

## 8. `GameObject`, `Component`, `Transform`

`GameObject`는 화면에 존재하는 하나의 물체다. 플레이어, 장애물, 바닥, 배경, 창살이 모두 게임 오브젝트로 만들어진다.

`Transform`은 오브젝트의 위치, 크기, 회전을 가진다.

```cpp
position
scale
rotation
```

`Component`는 오브젝트에 붙는 기능이다. 이 프로젝트에서는 렌더링 기능인 `MeshRenderer`가 대표적인 컴포넌트다.

이 구조를 사용하는 이유는 물체 자체와 기능을 분리하기 위해서다. 예를 들어 같은 사각형 오브젝트라도 어떤 머티리얼을 붙이느냐에 따라 플레이어, 벽, 개, 배경처럼 다르게 보일 수 있다.

## 9. 렌더링 구조

DirectX 렌더링은 여러 객체가 함께 움직인다.

### `GraphicsContext`

`GraphicsContext`는 DirectX 11 장치와 백버퍼를 관리한다.

주요 역할은 다음과 같다.

- DirectX 11 장치 생성
- SwapChain 생성
- RenderTarget 생성
- 프레임 시작 시 화면 지우기
- 프레임 끝에서 백버퍼 표시
- 최소 UI 텍스트 출력

텍스트는 `QueueText()`로 예약된다. 현재 최종 UI는 `SCORE`, `GAME OVER`, `R RESTART`만 사용한다.

### `Shader`

`Shader`는 HLSL 파일을 컴파일하고 GPU에 연결한다.

이 프로젝트의 셰이더 파일은 다음이다.

```text
Shaders/BasicColor.hlsl
```

셰이더는 정점 위치, 색상, UV 좌표를 받아서 화면에 그릴 픽셀 색을 계산한다.

### `Mesh`

`Mesh`는 화면에 그릴 도형의 정점 정보를 가진다.

이 프로젝트는 기본적으로 사각형 하나를 만들어 사용한다. 플레이어, 장애물, 바닥, 배경은 모두 같은 사각형 mesh를 사용하지만 위치, 크기, 머티리얼이 달라서 다르게 보인다.

### `Material`

`Material`은 어떤 색이나 텍스처로 그릴지 정한다.

플레이어, 경찰, 개, 벽, 낮은 장애물, 배경은 작은 8x8 절차적 텍스처를 사용한다. 별도 이미지 파일을 불러오지 않고 코드에서 픽셀 배열을 만들어 텍스처로 등록한다.

### `MeshRenderer`

`MeshRenderer`는 `Mesh`와 `Material`을 묶어서 실제로 그린다.

`Transform`의 위치와 크기를 행렬로 바꾸고, 그 값을 셰이더에 전달한 뒤, mesh를 draw한다.

## 10. `ResourceManager`

`ResourceManager`는 mesh, shader, material을 만들고 보관한다.

리소스를 한 곳에서 만들면 같은 mesh나 shader를 여러 오브젝트가 같이 사용할 수 있다. 이 프로젝트에서는 `Quad` mesh 하나를 여러 오브젝트가 공유한다.

## 11. `RunnerGameScene`

`RunnerGameScene`은 실제 게임 규칙의 중심이다.

관리하는 주요 데이터는 다음과 같다.

- `Player player`
- `std::vector<Obstacle> obstacles`
- `std::vector<GameObject*> obstacleVisuals`
- `std::vector<GameObject*> jailBars`
- `score`
- `scoreAccumulator`
- `scrollSpeed`
- `gameState`
- `SpeedData`
- `SpawnData`
- `HitboxData`

### 왜 게임 로직을 여기 모았는가

이 프로젝트는 학습용 최소 구조다. 복잡한 매니저 클래스를 많이 만들면 초보자가 흐름을 따라가기 어려워진다. 그래서 플레이어, 장애물, 점수, 난이도, 게임오버 처리를 `RunnerGameScene` 안에 모았다.

## 12. 플레이어 구조

`Player`는 다음 값을 가진다.

- `position`: 현재 위치
- `velocity`: 현재 속도
- `state`: Run, Jump, Slide
- `collider`: 충돌 박스
- `isGrounded`: 바닥에 있는지 여부
- `normalHeight`: 서 있을 때 높이
- `slideHeight`: 슬라이드할 때 높이
- `jumpPower`: 점프 시작 속도
- `gravity`: 중력
- `groundY`: 플레이어 중심의 바닥 기준 Y

점프는 `velocity.y`와 `gravity`로 처리한다.

```cpp
player.velocity.y += player.gravity * dt;
player.position.y += player.velocity.y * dt;
```

이렇게 하면 위로 올라가다가 점점 속도가 줄고, 다시 아래로 떨어지는 움직임이 나온다.

슬라이드는 충돌 높이를 줄인다.

```cpp
player.collider.height = player.slideHeight;
```

렌더링 크기도 충돌 높이에 맞춰 줄어든다.

## 13. 가변 점프

스페이스바를 짧게 누르면 낮게 점프하고, 길게 누르면 높게 점프한다.

원리는 단순하다. 플레이어가 아직 올라가는 중일 때 스페이스바를 떼면 위로 가는 속도를 절반으로 줄인다.

```cpp
if (jumpReleased && player.velocity.y > 0.0f)
{
    player.velocity.y = player.velocity.y * 0.5f;
}
```

## 14. 장애물 구조

`Obstacle`은 구조체로 관리된다.

장애물 종류는 다음과 같다.

- `POLICE`
- `DOG`
- `WALL`
- `LOW_BAR`

모든 장애물은 `std::vector<Obstacle>` 안에 저장된다. 매 프레임 왼쪽으로 이동한다.

```cpp
obstacle.position.x -= scrollSpeed * dt;
```

화면 밖으로 나간 장애물은 벡터에서 제거되고, 연결된 시각 오브젝트는 화면 밖 위치로 이동한다.

## 15. 바닥 정렬

`POLICE`, `DOG`, `WALL`은 모두 같은 바닥 정렬 함수를 사용한다.

```cpp
float RunnerGameScene::GetGroundObstacleY(float height) const
{
    return GetGroundSurfaceY() + height * 0.5f;
}
```

현재 좌표계에서 오브젝트의 `position`은 중심 좌표다. 그래서 바닥 위에 세우려면 바닥선에 장애물 높이의 절반을 더해야 한다.

`LOW_BAR`는 바닥 기준이 아니라 플레이어 머리 높이 기준으로 배치한다. 서 있으면 충돌하고 슬라이드하면 피할 수 있게 하기 위해서다.

## 16. 충돌 판정

충돌은 AABB 방식이다. AABB는 회전하지 않는 사각형 충돌 박스다.

플레이어 충돌 박스는 실제 렌더링 크기보다 15퍼센트 작게 처리한다.

```cpp
box.width *= hitboxData.forgivingScale;
box.height *= hitboxData.forgivingScale;
```

이렇게 한 이유는 이미지 가장자리에 살짝 닿았는데 바로 죽는 느낌을 줄이기 위해서다.

최종 화면에서는 빨간 충돌 디버그 라인을 그리지 않는다. 충돌 계산만 내부적으로 유지한다.

## 17. 점수

점수는 `scoreAccumulator`를 통해 천천히 오른다.

```cpp
scoreAccumulator += dt;
while (scoreAccumulator >= 0.05f)
{
    ++score;
    scoreAccumulator -= 0.05f;
}
```

게임오버 상태에서는 이 코드가 실행되지 않으므로 점수가 멈춘다.

## 18. 난이도

점수는 천천히 오르지만, 게임 템포는 별도의 난이도 값으로 조절한다.

처음 500점까지는 적응 구간이다.

```cpp
if (score <= speedData.adaptationScore)
{
    return 1.0f;
}
```

500점 이후부터 난이도가 올라간다.

```cpp
const float difficulty = 1.0f + static_cast<float>(scoreAfterAdaptation) / speedData.scorePerDifficulty;
```

스크롤 속도는 다음 방식으로 계산된다.

```cpp
scrollSpeed = speedData.baseSpeed * GetDifficulty();
```

난이도 UI는 만들지 않는다. 플레이어는 장애물이 빨라지고 더 자주 나오는 방식으로 난이도 상승을 느낀다.

## 19. 장애물 스폰 간격

장애물 간격은 점수가 오를수록 줄어든다.

하지만 점프로 절대 피할 수 없는 간격이 되면 안 된다. 그래서 점프 체공 시간 기반의 안전 간격을 계산한다.

```cpp
const float jumpAirTime = (-2.0f * player.jumpPower) / player.gravity;
const float safeJumpGap = scrollSpeed * jumpAirTime + spawnData.safetyPadding;
```

최종 간격은 템포 간격과 안전 간격을 비교해서 정한다.

```cpp
const float minClearance = (std::max)(tempoGap, safeJumpGap * 0.75f);
```

## 20. 2연속 장애물 패턴

1500점 이후에는 낮은 확률로 장애물 2개가 연속으로 나온다.

허용된 조합은 다음 세 가지다.

- `DOG -> LOW_BAR`
- `POLICE -> WALL`
- `WALL -> DOG`

불합리한 조합은 만들지 않는다. 예를 들어 `LOW_BAR -> LOW_BAR` 또는 `WALL -> LOW_BAR`처럼 같은 행동을 너무 연속으로 강요하거나, 점프 직후 바로 슬라이드를 강요하는 패턴은 제외했다.

## 21. 배경 전환

배경은 2개만 사용한다.

- 0번: 교도소 실내
- 1번: 교도소 외부 탈출 구간

전환 공식은 다음과 같다.

```cpp
const int backgroundStage = (score / 1000) % 2;
```

1000점마다 두 배경이 번갈아 나온다.

## 22. 게임오버

어떤 장애물이든 플레이어와 겹치면 즉시 게임오버다.

게임오버가 되면 다음이 멈춘다.

- 점수 증가
- 난이도 증가
- 장애물 스크롤

대신 감옥 창살이 플레이어 주변으로 내려오는 연출이 실행된다.

게임오버 UI는 다음 두 줄만 표시한다.

- `GAME OVER`
- `R RESTART`

## 23. 재시작

`R` 키를 누르면 `ResetGame()`이 실행된다.

`ResetGame()`은 씬 전체를 새로 만들지 않는다. 대신 필요한 값만 초기 상태로 돌린다.

초기화되는 주요 값은 다음과 같다.

- 플레이어 위치
- 플레이어 속도
- 플레이어 상태
- 점수
- 점수 누적 시간
- 배경 단계
- 게임 상태
- 스크롤 속도
- 창살 위치
- 장애물 벡터

이 방식은 단순하고 빠르며, 학습용 프로젝트에서 흐름을 이해하기 쉽다.

## 24. HLSL 셰이더

`BasicColor.hlsl`은 사각형을 화면에 그릴 때 사용된다.

정점 셰이더는 정점 위치에 world 행렬을 곱해서 화면 위치를 만든다.

픽셀 셰이더는 텍스처를 사용할지 색상을 사용할지 정하고 최종 색을 반환한다.

이 프로젝트는 하나의 기본 셰이더를 여러 오브젝트가 공유한다. 플레이어, 장애물, 배경은 서로 다른 머티리얼과 텍스처를 사용하지만 같은 셰이더로 렌더링된다.

