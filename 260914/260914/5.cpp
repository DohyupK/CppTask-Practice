#include "raylib.h"
#include <vector>
#include <cmath>

const int screenWidth = 800;
const int screenHeight = 600;

struct Paddle {
    Rectangle rect;
    float speed;
};

struct Ball {
    Vector2 position;
    Vector2 speed;
    float radius;
    Color color;
    bool active;
};

struct Brick {
    Rectangle rect;
    Color color;
    bool active;
};

struct Particle {
    Vector2 position;
    Vector2 speed;
    Color color;
    float alpha;
    float size;
};

struct TrailPoint {
    Vector2 position;
    Color color;
    float alpha;
};

int BrickGame() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Breakout Game - Auto Play");
    SetTargetFPS(60);

    Paddle paddle = { { (screenWidth - 120) / 2.0f, (float)(screenHeight - 40), 120.0f, 16.0f }, 9.0f };
    Ball ball = { { screenWidth / 2.0f, screenHeight / 2.0f }, { 8.0f, -8.0f }, 9.0f, WHITE, true };

    const int brickRows = 5;
    const int brickCols = 10;
    const float brickWidth = 70.0f;
    const float brickHeight = 22.0f;
    const float brickPadding = 10.0f;
    const float offsetLeft = 35.0f;
    const float offsetTop = 70.0f;

    Color rowColors[5] = { RED, ORANGE, YELLOW, GREEN, SKYBLUE };

    std::vector<Brick> bricks;
    for (int i = 0; i < brickRows; i++) {
        for (int j = 0; j < brickCols; j++) {
            Brick brick;
            brick.rect = {
                offsetLeft + j * (brickWidth + brickPadding),
                offsetTop + i * (brickHeight + brickPadding),
                brickWidth,
                brickHeight
            };
            brick.color = rowColors[i];
            brick.active = true;
            bricks.push_back(brick);
        }
    }

    std::vector<Particle> particles;
    std::vector<TrailPoint> trailHistory;

    // 오토플레이 기본값을 true로 설정하여 시작하자마자 공을 자동으로 따라가게 함
    bool autoPlay = true;

    bool gameOver = false;
    bool gameWon = false;

    while (!WindowShouldClose()) {
        // V 키를 누르면 오토플레이 켜기/끄기 토글 가능
        if (IsKeyPressed(KEY_V)) {
            autoPlay = !autoPlay;
        }

        if (!gameOver && !gameWon) {
            if (autoPlay) {
                // 공의 X 위치를 완벽하게 추적하도록 보간(Lerp) 적용
                // 공이 아무리 빨라져도 패들이 민첩하게 따라붙도록 계산식 개선
                float targetX = ball.position.x - (paddle.rect.width / 2.0f);
                paddle.rect.x += (targetX - paddle.rect.x) * 0.35f;
            }
            else {
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) paddle.rect.x -= paddle.speed;
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) paddle.rect.x += paddle.speed;
            }

            // 패들 화면 이탈 방지
            if (paddle.rect.x < 0) paddle.rect.x = 0;
            if (paddle.rect.x + paddle.rect.width > screenWidth) paddle.rect.x = screenWidth - paddle.rect.width;

            // 공 점진적 가속 유지
            float currentSpeed = sqrtf(ball.speed.x * ball.speed.x + ball.speed.y * ball.speed.y);
            float maxSpeed = 18.0f;
            if (currentSpeed < maxSpeed) {
                ball.speed.x *= 1.0005f;
                ball.speed.y *= 1.0005f;
            }

            // 공 이동
            ball.position.x += ball.speed.x;
            ball.position.y += ball.speed.y;

            // 공 잔상(Trail) 유지
            TrailPoint tp;
            tp.position = ball.position;
            tp.color = (ball.color.a == 0) ? WHITE : ball.color;
            tp.alpha = 0.6f;
            trailHistory.push_back(tp);
            if (trailHistory.size() > 15) {
                trailHistory.erase(trailHistory.begin());
            }

            // 벽 충돌 (좌/우/상단)
            if (ball.position.x - ball.radius < 0) { ball.position.x = ball.radius; ball.speed.x *= -1; }
            if (ball.position.x + ball.radius > screenWidth) { ball.position.x = screenWidth - ball.radius; ball.speed.x *= -1; }
            if (ball.position.y - ball.radius < 0) { ball.position.y = ball.radius; ball.speed.y *= -1; }

            // 바닥 충돌 (게임 오버)
            if (ball.position.y + ball.radius > screenHeight) {
                gameOver = true;
            }

            // 패들과 공 충돌
            if (CheckCollisionCircleRec(ball.position, ball.radius, paddle.rect)) {
                ball.speed.y *= -1;
                float hitFactor = (ball.position.x - (paddle.rect.x + paddle.rect.width / 2)) / (paddle.rect.width / 2);
                float speedMag = sqrtf(ball.speed.x * ball.speed.x + ball.speed.y * ball.speed.y);
                ball.speed.x = hitFactor * (speedMag * 0.7f);
            }

            // 벽돌과 공 충돌 처리 및 색상 변경
            bool allBricksCleared = true;
            for (auto& brick : bricks) {
                if (brick.active) {
                    allBricksCleared = false;
                    if (CheckCollisionCircleRec(ball.position, ball.radius, brick.rect)) {
                        brick.active = false;
                        ball.speed.y *= -1;
                        ball.color = brick.color; // 깬 벽돌 색상으로 공 색상 변경 유지

                        // 파티클 생성 유지
                        for (int p = 0; p < 15; p++) {
                            Particle pt;
                            pt.position = { brick.rect.x + brick.rect.width / 2, brick.rect.y + brick.rect.height / 2 };
                            float angle = (float)GetRandomValue(0, 360) * (PI / 180.0f);
                            float speed = (float)GetRandomValue(2, 7);
                            pt.speed = { cosf(angle) * speed, sinf(angle) * speed };
                            pt.color = brick.color;
                            pt.alpha = 1.0f;
                            pt.size = (float)GetRandomValue(3, 6);
                            particles.push_back(pt);
                        }
                        break;
                    }
                }
            }

            // 파티클 업데이트
            for (auto it = particles.begin(); it != particles.end();) {
                it->position.x += it->speed.x;
                it->position.y += it->speed.y;
                it->alpha -= 0.03f;
                if (it->alpha <= 0.0f) it = particles.erase(it);
                else ++it;
            }

            // 잔상 알파값 감소
            for (auto& tp : trailHistory) {
                tp.alpha -= 0.04f;
                if (tp.alpha < 0.0f) tp.alpha = 0.0f;
            }

            if (allBricksCleared) gameWon = true;
        }
        else {
            if (IsKeyPressed(KEY_R)) {
                ball.position = { screenWidth / 2.0f, screenHeight / 2.0f };
                ball.speed = { 8.0f, -8.0f };
                ball.color = WHITE;
                paddle.rect.x = (screenWidth - 120) / 2.0f;
                for (auto& brick : bricks) brick.active = true;
                particles.clear();
                trailHistory.clear();
                gameOver = false;
                gameWon = false;
            }
        }

        // --- 렌더링 로직 ---
        BeginDrawing();
        ClearBackground({ 15, 15, 25, 255 });

        if (!gameOver && !gameWon) {
            for (const auto& brick : bricks) {
                if (brick.active) DrawRectangleRounded(brick.rect, 0.3f, 4, brick.color);
            }

            Color paddleColor = autoPlay ? SKYBLUE : LIGHTGRAY;
            DrawRectangleRounded(paddle.rect, 0.5f, 4, paddleColor);

            for (const auto& tp : trailHistory) {
                DrawCircleV(tp.position, ball.radius * 0.8f, Fade(tp.color, tp.alpha));
            }

            Color currentBallColor = (ball.color.a == 0) ? WHITE : ball.color;
            DrawCircleV(ball.position, ball.radius, currentBallColor);
            DrawCircleV({ ball.position.x - 2, ball.position.y - 2 }, 3, WHITE);

            for (const auto& pt : particles) {
                DrawCircleV(pt.position, pt.size, Fade(pt.color, pt.alpha));
            }

            if (autoPlay) {
                DrawText("AUTO PLAY: ON (Press V to toggle)", 20, 20, 16, GREEN);
            }
            else {
                DrawText("AUTO PLAY: OFF (Press V to toggle)", 20, 20, 16, DARKGRAY);
            }
        }
        else if (gameOver) {
            DrawText("GAME OVER", screenWidth / 2 - MeasureText("GAME OVER", 50) / 2, screenHeight / 2 - 50, 50, RED);
            DrawText("Press 'R' to Restart", screenWidth / 2 - MeasureText("Press 'R' to Restart", 20) / 2, screenHeight / 2 + 20, 20, LIGHTGRAY);
        }
        else if (gameWon) {
            DrawText("VICTORY!", screenWidth / 2 - MeasureText("VICTORY!", 50) / 2, screenHeight / 2 - 50, 50, GREEN);
            DrawText("Press 'R' to Restart", screenWidth / 2 - MeasureText("Press 'R' to Restart", 20) / 2, screenHeight / 2 + 20, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}