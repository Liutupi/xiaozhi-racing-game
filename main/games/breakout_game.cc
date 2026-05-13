#include "breakout_game.h"

#include <algorithm>
#include <cstdlib>

namespace {

constexpr int kFrameMs = 35;
constexpr int kBallSize = 4;
constexpr int kPaddleH = 5;
constexpr int kPaddleStep = 9;
constexpr int kHeaderH = 19;
constexpr int kBrickTop = 28;
constexpr int kBrickGap = 2;
constexpr int kBrickH = 8;

constexpr uint32_t kBg = 0x07111f;
constexpr uint32_t kPanel = 0x0f172a;
constexpr uint32_t kText = 0xf8fafc;
constexpr uint32_t kMuted = 0x94a3b8;
constexpr uint32_t kAccent = 0xffd166;
constexpr uint32_t kPaddle = 0xe879f9;
constexpr uint32_t kBall = 0x67e8f9;

constexpr uint32_t kBrickColors[] = {
    0x22c55e,
    0xf59e0b,
    0xef4444,
    0x60a5fa,
};

void StyleBox(lv_obj_t* obj, uint32_t bg, uint32_t border, int border_width, int radius) {
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(obj, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(border), 0);
    lv_obj_set_style_border_width(obj, border_width, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

lv_obj_t* AddLabel(lv_obj_t* parent, int x, int y, int w, const char* text, uint32_t color,
                   lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, w);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, align, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_set_style_text_line_space(label, 0, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_label_set_text(label, text);
    return label;
}

}  // namespace

BreakoutGame::~BreakoutGame() {
    Stop();
}

void BreakoutGame::Start(Display* display) {
    if (running_ || display == nullptr) {
        return;
    }

    display_ = display;
    width_ = display_->width();
    height_ = display_->height();
    paddle_y_ = height_ - 18;

    {
        DisplayLockGuard lock(display_);
        CreateUi();
        ResetGame();
    }

    esp_timer_create_args_t timer_args = {
        .callback = [](void* arg) {
            static_cast<BreakoutGame*>(arg)->Tick();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "breakout_game",
        .skip_unhandled_events = true,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_, kFrameMs * 1000));
    running_ = true;
}

void BreakoutGame::Stop() {
    if (timer_ != nullptr) {
        esp_timer_stop(timer_);
        esp_timer_delete(timer_);
        timer_ = nullptr;
    }

    if (display_ != nullptr) {
        DisplayLockGuard lock(display_);
        DestroyUi();
    }

    running_ = false;
    display_ = nullptr;
}

bool BreakoutGame::HandleClick() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (finished_) {
        ResetGame();
    } else if (waiting_) {
        LaunchBall();
    } else {
        MovePaddle(-kPaddleStep);
    }
    return true;
}

bool BreakoutGame::HandleDoubleClick() {
    return MoveLeft();
}

bool BreakoutGame::MoveRight() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (!finished_) {
        MovePaddle(kPaddleStep);
    }
    return true;
}

bool BreakoutGame::MoveLeft() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (!finished_) {
        MovePaddle(-kPaddleStep);
    }
    return true;
}

void BreakoutGame::CreateUi() {
    layer_ = lv_obj_create(lv_screen_active());
    lv_obj_set_size(layer_, width_, height_);
    lv_obj_set_pos(layer_, 0, 0);
    StyleBox(layer_, kBg, kBg, 0, 0);

    lv_obj_t* header = lv_obj_create(layer_);
    lv_obj_set_size(header, width_, kHeaderH);
    lv_obj_set_pos(header, 0, 0);
    StyleBox(header, kPanel, kPanel, 0, 0);

    title_label_ = AddLabel(layer_, 5, 2, 44, "BRICK", kAccent);
    score_label_ = AddLabel(layer_, 48, 2, 44, "S 0", kText, LV_TEXT_ALIGN_CENTER);
    lives_label_ = AddLabel(layer_, width_ - 34, 2, 30, "L 3", kText, LV_TEXT_ALIGN_RIGHT);

    const int side = 8;
    const int brick_w = (width_ - side * 2 - (kCols - 1) * kBrickGap) / kCols;
    max_score_ = 0;
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const int index = row * kCols + col;
            Brick& brick = bricks_[index];
            brick.x = side + col * (brick_w + kBrickGap);
            brick.y = kBrickTop + row * (kBrickH + kBrickGap);
            brick.w = brick_w;
            brick.h = kBrickH;
            brick.obj = lv_obj_create(layer_);
            lv_obj_set_size(brick.obj, brick.w, brick.h);
            lv_obj_set_pos(brick.obj, brick.x, brick.y);
            StyleBox(brick.obj, kBrickColors[row], 0x0f172a, 1, 2);
        }
    }

    paddle_ = lv_obj_create(layer_);
    lv_obj_set_size(paddle_, paddle_w_, kPaddleH);
    StyleBox(paddle_, kPaddle, 0xf5d0fe, 1, 2);

    ball_ = lv_obj_create(layer_);
    lv_obj_set_size(ball_, kBallSize, kBallSize);
    StyleBox(ball_, kBall, kBall, 0, 2);

    message_label_ = AddLabel(layer_, 5, height_ / 2 - 12, width_ - 10, "", kText, LV_TEXT_ALIGN_CENTER);
    footer_label_ = AddLabel(layer_, 4, height_ - 12, width_ - 8, "BOOT launch/left | GPIO39 right", kMuted,
                             LV_TEXT_ALIGN_CENTER);
}

void BreakoutGame::DestroyUi() {
    if (layer_ != nullptr) {
        lv_obj_del(layer_);
    }
    layer_ = nullptr;
    title_label_ = nullptr;
    score_label_ = nullptr;
    lives_label_ = nullptr;
    message_label_ = nullptr;
    footer_label_ = nullptr;
    paddle_ = nullptr;
    ball_ = nullptr;
    for (auto& brick : bricks_) {
        brick.obj = nullptr;
    }
}

void BreakoutGame::ResetGame() {
    waiting_ = true;
    finished_ = false;
    won_ = false;
    score_ = 0;
    lives_ = 3;
    frame_ = 0;
    max_score_ = 0;
    paddle_w_ = std::min(36, width_ - 24);
    paddle_x_ = (width_ - paddle_w_) / 2;
    lv_obj_set_size(paddle_, paddle_w_, kPaddleH);

    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            Brick& brick = bricks_[row * kCols + col];
            brick.strength = static_cast<uint8_t>(kRows - row);
            max_score_ += brick.strength;
            UpdateBrick(brick);
        }
    }

    ResetBall(true);
    UpdateUi();
}

void BreakoutGame::ResetBall(bool show_message) {
    waiting_ = true;
    ball_dx_ = (frame_ % 2 == 0) ? 2 : -2;
    ball_dy_ = -3;
    ball_x_ = paddle_x_ + paddle_w_ / 2 - kBallSize / 2;
    ball_y_ = paddle_y_ - kBallSize - 1;
    lv_obj_set_pos(paddle_, paddle_x_, paddle_y_);
    lv_obj_set_pos(ball_, ball_x_, ball_y_);
    if (show_message) {
        lv_label_set_text(message_label_, "BOOT launch");
        lv_obj_clear_flag(message_label_, LV_OBJ_FLAG_HIDDEN);
    }
}

void BreakoutGame::LaunchBall() {
    waiting_ = false;
    lv_obj_add_flag(message_label_, LV_OBJ_FLAG_HIDDEN);
}

void BreakoutGame::Tick() {
    if (!running_ || display_ == nullptr) {
        return;
    }

    DisplayLockGuard lock(display_);
    if (finished_) {
        return;
    }

    ++frame_;
    if (waiting_) {
        ball_x_ = paddle_x_ + paddle_w_ / 2 - kBallSize / 2;
        ball_y_ = paddle_y_ - kBallSize - 1;
        lv_obj_set_pos(ball_, ball_x_, ball_y_);
        return;
    }

    ball_x_ += ball_dx_;
    ball_y_ += ball_dy_;

    if (ball_x_ <= 0) {
        ball_x_ = 0;
        ball_dx_ = std::abs(ball_dx_);
    } else if (ball_x_ + kBallSize >= width_) {
        ball_x_ = width_ - kBallSize;
        ball_dx_ = -std::abs(ball_dx_);
    }

    if (ball_y_ <= kHeaderH) {
        ball_y_ = kHeaderH;
        ball_dy_ = std::abs(ball_dy_);
    }

    if (HitPaddle()) {
        const int center = paddle_x_ + paddle_w_ / 2;
        const int ball_center = ball_x_ + kBallSize / 2;
        ball_y_ = paddle_y_ - kBallSize;
        ball_dy_ = -std::abs(ball_dy_);
        ball_dx_ = std::clamp((ball_center - center) / 5, -3, 3);
        if (ball_dx_ == 0) {
            ball_dx_ = (ball_center < center) ? -1 : 1;
        }
    }

    for (auto& brick : bricks_) {
        if (HitBrick(brick)) {
            break;
        }
    }

    if (ball_y_ > height_) {
        --lives_;
        if (lives_ <= 0) {
            Finish(false);
            return;
        }
        ResetBall(true);
        UpdateUi();
        return;
    }

    if (score_ >= max_score_) {
        Finish(true);
        return;
    }

    lv_obj_set_pos(ball_, ball_x_, ball_y_);
    UpdateUi();
}

void BreakoutGame::MovePaddle(int delta) {
    paddle_x_ = std::clamp(paddle_x_ + delta, 0, width_ - paddle_w_);
    lv_obj_set_pos(paddle_, paddle_x_, paddle_y_);
    if (waiting_) {
        ball_x_ = paddle_x_ + paddle_w_ / 2 - kBallSize / 2;
        ball_y_ = paddle_y_ - kBallSize - 1;
        lv_obj_set_pos(ball_, ball_x_, ball_y_);
    }
}

void BreakoutGame::UpdateUi() {
    lv_label_set_text_fmt(score_label_, "S %d", score_);
    lv_label_set_text_fmt(lives_label_, "L %d", lives_);
}

void BreakoutGame::UpdateBrick(Brick& brick) {
    if (brick.obj == nullptr) {
        return;
    }
    if (brick.strength == 0) {
        lv_obj_add_flag(brick.obj, LV_OBJ_FLAG_HIDDEN);
        return;
    }
    const int color_count = sizeof(kBrickColors) / sizeof(kBrickColors[0]);
    const int color_index = std::clamp<int>(brick.strength - 1, 0, color_count - 1);
    StyleBox(brick.obj, kBrickColors[color_index], 0x0f172a, 1, 2);
    lv_obj_clear_flag(brick.obj, LV_OBJ_FLAG_HIDDEN);
}

void BreakoutGame::Finish(bool won) {
    finished_ = true;
    waiting_ = false;
    won_ = won;
    lv_label_set_text_fmt(message_label_, "%s\nSCORE %d\nBOOT restart", won_ ? "YOU WIN!" : "GAME OVER", score_);
    lv_obj_clear_flag(message_label_, LV_OBJ_FLAG_HIDDEN);
    UpdateUi();
}

bool BreakoutGame::HitPaddle() const {
    return ball_dy_ > 0 &&
           ball_y_ + kBallSize >= paddle_y_ &&
           ball_y_ <= paddle_y_ + kPaddleH &&
           ball_x_ + kBallSize >= paddle_x_ &&
           ball_x_ <= paddle_x_ + paddle_w_;
}

bool BreakoutGame::HitBrick(Brick& brick) {
    if (brick.strength == 0) {
        return false;
    }
    const bool hit = ball_x_ + kBallSize >= brick.x &&
                     ball_x_ <= brick.x + brick.w &&
                     ball_y_ + kBallSize >= brick.y &&
                     ball_y_ <= brick.y + brick.h;
    if (!hit) {
        return false;
    }

    --brick.strength;
    ++score_;
    ball_dy_ = -ball_dy_;
    UpdateBrick(brick);
    return true;
}
