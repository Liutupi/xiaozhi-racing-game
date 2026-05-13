#ifndef BREAKOUT_GAME_H_
#define BREAKOUT_GAME_H_

#include "display.h"

#include <esp_timer.h>
#include <lvgl.h>

#include <array>
#include <cstdint>

class BreakoutGame {
public:
    BreakoutGame() = default;
    ~BreakoutGame();

    void Start(Display* display);
    void Stop();
    bool HandleClick();
    bool HandleDoubleClick();
    bool MoveRight();
    bool MoveLeft();
    bool IsRunning() const { return running_; }

private:
    struct Brick {
        lv_obj_t* obj = nullptr;
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        uint8_t strength = 0;
    };

    static constexpr int kRows = 4;
    static constexpr int kCols = 6;
    static constexpr int kBrickCount = kRows * kCols;

    Display* display_ = nullptr;
    esp_timer_handle_t timer_ = nullptr;
    lv_obj_t* layer_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* score_label_ = nullptr;
    lv_obj_t* lives_label_ = nullptr;
    lv_obj_t* message_label_ = nullptr;
    lv_obj_t* footer_label_ = nullptr;
    lv_obj_t* paddle_ = nullptr;
    lv_obj_t* ball_ = nullptr;
    std::array<Brick, kBrickCount> bricks_{};

    bool running_ = false;
    bool waiting_ = true;
    bool finished_ = false;
    bool won_ = false;
    int width_ = 0;
    int height_ = 0;
    int paddle_x_ = 0;
    int paddle_y_ = 0;
    int paddle_w_ = 34;
    int ball_x_ = 0;
    int ball_y_ = 0;
    int ball_dx_ = 2;
    int ball_dy_ = -3;
    int score_ = 0;
    int max_score_ = 0;
    int lives_ = 3;
    int frame_ = 0;

    void CreateUi();
    void DestroyUi();
    void ResetGame();
    void ResetBall(bool show_message);
    void LaunchBall();
    void Tick();
    void MovePaddle(int delta);
    void UpdateUi();
    void UpdateBrick(Brick& brick);
    void Finish(bool won);
    bool HitPaddle() const;
    bool HitBrick(Brick& brick);
};

#endif  // BREAKOUT_GAME_H_
