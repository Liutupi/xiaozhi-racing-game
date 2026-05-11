#include "racing_game.h"
#include "racing_sfx.h"

#include <esp_random.h>
#include <algorithm>

static constexpr int kFrameMs = 50;
constexpr int kPlayerYMargin = 30;
constexpr int kCarW = 20;
constexpr int kCarH = 26;
constexpr int kLaneMarkW = 2;
constexpr int kLaneMarkH = 10;
constexpr int kLaneMarkGap = 18;

void StylePlain(lv_obj_t* obj, lv_color_t color) {
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 2, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

void StyleTransparent(lv_obj_t* obj) {
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

lv_obj_t* AddBlock(lv_obj_t* parent, int x, int y, int w, int h, lv_color_t color, int radius = 2) {
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_pos(obj, x, y);
    StylePlain(obj, color);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    return obj;
}

static const lv_color_t kCarBodyColors[] = {
    lv_color_hex(0xef4444),  // Player F1
    lv_color_hex(0x4ade80),  // Sports
    lv_color_hex(0xfacc15),  // SUV
    lv_color_hex(0x60a5fa),  // Pickup
    lv_color_hex(0xf472b6),  // Truck
};

static const lv_color_t kCarAccentColors[] = {
    lv_color_hex(0x38bdf8),  // Player
    lv_color_hex(0xbbf7d0),  // Sports
    lv_color_hex(0xfef08a),  // SUV
    lv_color_hex(0xbfdbfe),  // Pickup
    lv_color_hex(0xfbcfe8),  // Truck
};

static lv_color_t CarColor(CarType type) {
    auto idx = static_cast<uint8_t>(type);
    if (idx >= static_cast<uint8_t>(CarType::kCount)) idx = 0;
    return kCarBodyColors[idx];
}

static lv_color_t CarAccent(CarType type) {
    auto idx = static_cast<uint8_t>(type);
    if (idx >= static_cast<uint8_t>(CarType::kCount)) idx = 0;
    return kCarAccentColors[idx];
}

void DrawPlayerF1(lv_obj_t* car, lv_color_t body, lv_color_t accent) {
    const lv_color_t dark = lv_color_hex(0x0a0f1f);
    const lv_color_t wheel = lv_color_hex(0x020617);
    const lv_color_t flame_inner = lv_color_hex(0xfef08a);
    const lv_color_t flame_outer = lv_color_hex(0xf97316);

    AddBlock(car, 4, 0, 12, 2, accent, 1);
    AddBlock(car, 2, 2, 16, 3, body, 2);
    AddBlock(car, 3, 5, 14, 5, dark, 2);
    AddBlock(car, 6, 7, 8, 2, accent, 1);
    AddBlock(car, 1, 10, 18, 5, body, 3);
    AddBlock(car, 0, 15, 20, 3, lv_color_hex(0xdc2626), 1);
    AddBlock(car, 3, 18, 14, 2, dark, 1);
    AddBlock(car, 6, 20, 8, 1, lv_color_hex(0x475569), 1);
    AddBlock(car, 7, 21, 6, 3, flame_outer, 1);
    AddBlock(car, 9, 22, 2, 2, flame_inner, 1);
    AddBlock(car, 5, 0, 3, 2, lv_color_hex(0xfffbeb), 1);
    AddBlock(car, 12, 0, 3, 2, lv_color_hex(0xfffbeb), 1);
    AddBlock(car, 0, 12, 3, 5, wheel, 1);
    AddBlock(car, 17, 12, 3, 5, wheel, 1);
    AddBlock(car, 1, 18, 4, 4, wheel, 1);
    AddBlock(car, 15, 18, 4, 4, wheel, 1);
}

void DrawSportsCar(lv_obj_t* car, lv_color_t body, lv_color_t accent) {
    const lv_color_t glass = lv_color_hex(0x0f172a);
    const lv_color_t glass_hi = lv_color_hex(0x7dd3fc);
    const lv_color_t wheel = lv_color_hex(0x020617);

    AddBlock(car, 4, 0, 12, 4, body, 3);
    AddBlock(car, 2, 4, 16, 7, body, 3);
    AddBlock(car, 4, 4, 12, 5, glass, 2);
    AddBlock(car, 6, 5, 8, 2, glass_hi, 1);
    AddBlock(car, 1, 11, 18, 8, body, 4);
    AddBlock(car, 3, 18, 14, 4, accent, 2);
    AddBlock(car, 3, 24, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 14, 24, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 0, 9, 3, 7, wheel, 1);
    AddBlock(car, 17, 9, 3, 7, wheel, 1);
    AddBlock(car, 0, 16, 3, 7, wheel, 1);
    AddBlock(car, 17, 16, 3, 7, wheel, 1);
}

void DrawSuv(lv_obj_t* car, lv_color_t body, lv_color_t accent) {
    const lv_color_t glass = lv_color_hex(0x0f172a);
    const lv_color_t glass_hi = lv_color_hex(0xcffafe);
    const lv_color_t wheel = lv_color_hex(0x020617);

    AddBlock(car, 2, 0, 16, 2, body, 2);
    AddBlock(car, 2, 2, 16, 10, body, 2);
    AddBlock(car, 4, 3, 12, 5, glass, 1);
    AddBlock(car, 7, 4, 6, 2, glass_hi, 1);
    AddBlock(car, 1, 12, 18, 7, body, 3);
    AddBlock(car, 4, 18, 12, 3, accent, 1);
    AddBlock(car, 4, 22, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 13, 22, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 0, 9, 3, 7, wheel, 1);
    AddBlock(car, 17, 9, 3, 7, wheel, 1);
    AddBlock(car, 0, 17, 3, 7, wheel, 1);
    AddBlock(car, 17, 17, 3, 7, wheel, 1);
}

void DrawPickup(lv_obj_t* car, lv_color_t body, lv_color_t accent) {
    const lv_color_t glass = lv_color_hex(0x0f172a);
    const lv_color_t wheel = lv_color_hex(0x020617);

    AddBlock(car, 4, 0, 8, 2, body, 2);
    AddBlock(car, 4, 2, 8, 8, body, 2);
    AddBlock(car, 5, 3, 6, 3, glass, 1);
    AddBlock(car, 12, 4, 6, 6, accent, 1);
    AddBlock(car, 13, 5, 4, 1, body, 1);
    AddBlock(car, 1, 10, 18, 7, body, 3);
    AddBlock(car, 4, 16, 12, 4, accent, 1);
    AddBlock(car, 4, 22, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 13, 22, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 0, 8, 3, 7, wheel, 1);
    AddBlock(car, 17, 8, 3, 7, wheel, 1);
    AddBlock(car, 0, 17, 3, 7, wheel, 1);
    AddBlock(car, 17, 17, 3, 7, wheel, 1);
}

void DrawTruck(lv_obj_t* car, lv_color_t body, lv_color_t accent) {
    const lv_color_t glass = lv_color_hex(0x0f172a);
    const lv_color_t glass_hi = lv_color_hex(0xcffafe);
    const lv_color_t wheel = lv_color_hex(0x020617);

    AddBlock(car, 1, 0, 18, 2, body, 2);
    AddBlock(car, 1, 2, 18, 12, body, 2);
    AddBlock(car, 3, 3, 14, 5, glass, 1);
    AddBlock(car, 6, 4, 8, 2, glass_hi, 1);
    AddBlock(car, 2, 14, 16, 6, body, 3);
    AddBlock(car, 6, 17, 8, 2, accent, 1);
    AddBlock(car, 4, 22, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 13, 22, 3, 2, lv_color_hex(0xff5d6c), 1);
    AddBlock(car, 0, 8, 3, 7, wheel, 1);
    AddBlock(car, 17, 8, 3, 7, wheel, 1);
    AddBlock(car, 0, 16, 3, 7, wheel, 1);
    AddBlock(car, 17, 16, 3, 7, wheel, 1);
}

lv_obj_t* CreateCar(lv_obj_t* parent, CarType type) {
    lv_obj_t* car = lv_obj_create(parent);
    lv_obj_set_size(car, kCarW, kCarH);
    StyleTransparent(car);

    lv_color_t body = CarColor(type);
    lv_color_t accent = CarAccent(type);

    switch (type) {
    case CarType::kPlayer:  DrawPlayerF1(car, body, accent); break;
    case CarType::kSports:   DrawSportsCar(car, body, accent); break;
    case CarType::kSuv:      DrawSuv(car, body, accent); break;
    case CarType::kPickup:   DrawPickup(car, body, accent); break;
    case CarType::kTruck:    DrawTruck(car, body, accent); break;
    default: break;
    }

    return car;
}

void RedrawCar(lv_obj_t* car, CarType type) {
    lv_obj_clean(car);
    lv_color_t body = CarColor(type);
    lv_color_t accent = CarAccent(type);
    switch (type) {
    case CarType::kPlayer:  DrawPlayerF1(car, body, accent); break;
    case CarType::kSports:   DrawSportsCar(car, body, accent); break;
    case CarType::kSuv:      DrawSuv(car, body, accent); break;
    case CarType::kPickup:   DrawPickup(car, body, accent); break;
    case CarType::kTruck:    DrawTruck(car, body, accent); break;
    default: break;
    }
}

RacingGame::~RacingGame() {
    Stop();
}

void RacingGame::Start(Display* display) {
    if (running_ || display == nullptr) {
        return;
    }

    display_ = display;
    width_ = display_->width();
    height_ = display_->height();
    road_w_ = std::min(90, width_ - 20);
    lane_w_ = road_w_ / 3;
    road_w_ = lane_w_ * 3;
    road_h_ = height_ - road_y_ - 4;
    road_x_ = (width_ - road_w_) / 2;

    {
        DisplayLockGuard lock(display_);
        CreateUi();
        ResetGame();
    }

    esp_timer_create_args_t timer_args = {
        .callback = [](void* arg) {
            static_cast<RacingGame*>(arg)->Tick();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "racing_game",
        .skip_unhandled_events = true,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_, kFrameMs * 1000));
    running_ = true;
    RacingSfx::GetInstance().Play(RacingSfxEvent::kStart);
}

void RacingGame::Stop() {
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
    game_over_ = false;
    display_ = nullptr;
}

bool RacingGame::HandleClick() {
    if (!running_) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (game_over_) {
        ResetGame(true);
        return true;
    }

    player_lane_ = (player_lane_ + 2) % 3;
    SetCarPosition(player_car_, player_lane_, height_ - kPlayerYMargin);
    RacingSfx::GetInstance().Play(RacingSfxEvent::kMove);
    return true;
}

bool RacingGame::HandleDoubleClick() {
    return MoveLeft();
}

bool RacingGame::MoveRight() {
    if (!running_) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (game_over_) {
        ResetGame(true);
        return true;
    }

    player_lane_ = (player_lane_ + 1) % 3;
    SetCarPosition(player_car_, player_lane_, height_ - kPlayerYMargin);
    RacingSfx::GetInstance().Play(RacingSfxEvent::kMove);
    return true;
}

bool RacingGame::MoveLeft() {
    if (!running_) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (!game_over_) {
        player_lane_ = (player_lane_ + 2) % 3;
        SetCarPosition(player_car_, player_lane_, height_ - kPlayerYMargin);
        RacingSfx::GetInstance().Play(RacingSfxEvent::kMove);
    }
    return true;
}

void RacingGame::CreateUi() {
    layer_ = lv_obj_create(lv_screen_active());
    lv_obj_set_size(layer_, width_, height_);
    lv_obj_set_pos(layer_, 0, 0);
    StylePlain(layer_, lv_color_hex(0x07111f));
    lv_obj_set_scrollbar_mode(layer_, LV_SCROLLBAR_MODE_OFF);

    const lv_color_t sky_colors[] = {
        lv_color_hex(0x1a0533),
        lv_color_hex(0x0e0728),
        lv_color_hex(0x080b25),
        lv_color_hex(0x07111f),
    };
    for (int i = 0; i < 4; ++i) {
        sky_bands_[i] = AddBlock(layer_, 0, i * 9, width_, 9, sky_colors[i], 0);
    }

    sun_ = lv_obj_create(layer_);
    lv_obj_set_size(sun_, 30, 30);
    lv_obj_set_pos(sun_, (width_ - 30) / 2, road_y_ - 16);
    StylePlain(sun_, lv_color_hex(0xf59e0b));
    lv_obj_set_style_radius(sun_, 15, 0);

    for (int i = 0; i < 4; ++i) {
        sun_stripes_[i] = AddBlock(layer_, 0, road_y_ - 12 + i * 4, width_, 2, lv_color_hex(0x07111f), 0);
    }

    lv_obj_t* road_bg = lv_obj_create(layer_);
    lv_obj_set_size(road_bg, road_w_ + 4, road_h_);
    lv_obj_set_pos(road_bg, road_x_ - 2, road_y_);
    StylePlain(road_bg, lv_color_hex(0x0f172a));
    lv_obj_set_style_border_width(road_bg, 0, 0);

    road_ = lv_obj_create(layer_);
    lv_obj_set_size(road_, road_w_ - 4, road_h_);
    lv_obj_set_pos(road_, road_x_ + 2, road_y_);
    StylePlain(road_, lv_color_hex(0x1a2436));

    AddBlock(layer_, road_x_ - 6, road_y_, 6, road_h_, lv_color_hex(0x0f2a3b), 1);
    AddBlock(layer_, road_x_ + road_w_, road_y_, 6, road_h_, lv_color_hex(0x2a0f2b), 1);
    road_edges_[0] = AddBlock(layer_, road_x_ - 4, road_y_, 2, road_h_, lv_color_hex(0x22d3ee), 1);
    road_edges_[1] = AddBlock(layer_, road_x_ + road_w_ + 2, road_y_, 2, road_h_, lv_color_hex(0xe879f9), 1);

    for (size_t i = 0; i < lane_marks_.size(); ++i) {
        lv_obj_t* mark = lv_obj_create(layer_);
        lv_obj_set_size(mark, kLaneMarkW, kLaneMarkH);
        StylePlain(mark, lv_color_hex(0x6ee7b7));
        lane_marks_[i] = mark;
    }

    score_label_ = lv_label_create(layer_);
    lv_obj_set_pos(score_label_, 4, 1);
    lv_obj_set_style_text_color(score_label_, lv_color_hex(0xc4f1ff), 0);
    lv_label_set_text(score_label_, "SCORE 0");

    speed_label_ = lv_label_create(layer_);
    lv_obj_set_pos(speed_label_, 4, road_y_ - 11);
    lv_obj_set_style_text_color(speed_label_, lv_color_hex(0x4ade80), 0);
    lv_label_set_text(speed_label_, "SPD 3");

    speed_bar_bg_ = lv_obj_create(layer_);
    lv_obj_set_size(speed_bar_bg_, 56, 3);
    lv_obj_set_pos(speed_bar_bg_, 38, road_y_ - 6);
    StylePlain(speed_bar_bg_, lv_color_hex(0x1e293b));
    lv_obj_set_style_radius(speed_bar_bg_, 1, 0);

    speed_bar_fill_ = lv_obj_create(layer_);
    lv_obj_set_size(speed_bar_fill_, 0, 3);
    lv_obj_set_pos(speed_bar_fill_, 38, road_y_ - 6);
    StylePlain(speed_bar_fill_, lv_color_hex(0x4ade80));
    lv_obj_set_style_radius(speed_bar_fill_, 1, 0);

    message_label_ = lv_label_create(layer_);
    lv_obj_set_width(message_label_, width_ - 8);
    lv_obj_set_pos(message_label_, 4, height_ / 2 - 18);
    lv_obj_set_style_text_align(message_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(message_label_, lv_color_white(), 0);
    lv_obj_add_flag(message_label_, LV_OBJ_FLAG_HIDDEN);

    crash_overlay_ = lv_obj_create(layer_);
    lv_obj_set_size(crash_overlay_, width_, height_);
    lv_obj_set_pos(crash_overlay_, 0, 0);
    StylePlain(crash_overlay_, lv_color_hex(0xef4444));
    lv_obj_set_style_bg_opa(crash_overlay_, LV_OPA_30, 0);
    lv_obj_add_flag(crash_overlay_, LV_OBJ_FLAG_HIDDEN);

    streak_left_ = AddBlock(layer_, road_x_ - 8, 0, 2, 4, lv_color_hex(0x67e8f9), 1);
    lv_obj_add_flag(streak_left_, LV_OBJ_FLAG_HIDDEN);
    streak_right_ = AddBlock(layer_, road_x_ + road_w_ + 6, 0, 2, 4, lv_color_hex(0xf0abfc), 1);
    lv_obj_add_flag(streak_right_, LV_OBJ_FLAG_HIDDEN);

    for (auto& p : particles_) {
        p.obj = lv_obj_create(layer_);
        lv_obj_set_size(p.obj, 3, 3);
        StylePlain(p.obj, lv_color_hex(0xfef08a));
        lv_obj_add_flag(p.obj, LV_OBJ_FLAG_HIDDEN);
        p.life = 0;
    }

    player_car_ = CreateCar(layer_, CarType::kPlayer);

    const CarType kInitTypes[] = {
        CarType::kSports, CarType::kSuv, CarType::kPickup, CarType::kTruck
    };
    for (size_t i = 0; i < traffic_.size(); ++i) {
        traffic_[i].type = kInitTypes[i];
        traffic_[i].body = CreateCar(layer_, traffic_[i].type);
        lv_obj_add_flag(traffic_[i].body, LV_OBJ_FLAG_HIDDEN);
    }
}

void RacingGame::DestroyUi() {
    if (layer_ != nullptr) {
        lv_obj_del(layer_);
    }
    layer_ = nullptr;
    road_ = nullptr;
    score_label_ = nullptr;
    message_label_ = nullptr;
    player_car_ = nullptr;
    sun_ = nullptr;
    speed_label_ = nullptr;
    speed_bar_bg_ = nullptr;
    speed_bar_fill_ = nullptr;
    crash_overlay_ = nullptr;
    streak_left_ = nullptr;
    streak_right_ = nullptr;
    sky_bands_.fill(nullptr);
    sun_stripes_.fill(nullptr);
    lane_marks_.fill(nullptr);
    road_edges_.fill(nullptr);
    for (auto& car : traffic_) {
        car = {};
    }
}

void RacingGame::ResetGame(bool play_sfx) {
    game_over_ = false;
    player_lane_ = 1;
    score_ = 0;
    speed_ = 3;
    frame_ = 0;
    mark_offset_ = 0;
    spawn_countdown_ = 10;

    lv_obj_add_flag(message_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(crash_overlay_, LV_OBJ_FLAG_HIDDEN);
    shake_timer_ = 0;
    shake_amplitude_ = 0;
    lv_obj_set_pos(layer_, 0, 0);
    for (auto& p : particles_) {
        p.life = 0;
        lv_obj_add_flag(p.obj, LV_OBJ_FLAG_HIDDEN);
    }
    SetCarPosition(player_car_, player_lane_, height_ - kPlayerYMargin);
    for (auto& car : traffic_) {
        car.active = false;
        car.y = -40;
        lv_obj_add_flag(car.body, LV_OBJ_FLAG_HIDDEN);
    }
    UpdateUi();
    if (play_sfx) {
        RacingSfx::GetInstance().Play(RacingSfxEvent::kRestart);
    }
}

void RacingGame::Tick() {
    if (!running_ || display_ == nullptr) {
        return;
    }

    DisplayLockGuard lock(display_);

    if (shake_timer_ > 0) {
        shake_timer_--;
        int ox = (esp_random() % (shake_amplitude_ * 2 + 1)) - shake_amplitude_;
        int oy = (esp_random() % (shake_amplitude_ * 2 + 1)) - shake_amplitude_;
        lv_obj_set_pos(layer_, ox, oy);
        if (shake_timer_ == 0) {
            lv_obj_set_pos(layer_, 0, 0);
            lv_obj_add_flag(crash_overlay_, LV_OBJ_FLAG_HIDDEN);
        }
    }

    for (auto& p : particles_) {
        if (p.life <= 0) continue;
        p.life--;
        if (p.life <= 0) {
            lv_obj_add_flag(p.obj, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        int px = lv_obj_get_x(p.obj);
        int py = lv_obj_get_y(p.obj);
        lv_obj_set_pos(p.obj, px + p.vx, py + p.vy);
        p.vy += 1;
        int opa = p.life * 32;
        if (opa > 255) opa = 255;
        lv_obj_set_style_bg_opa(p.obj, opa, 0);
    }

    if (!game_over_ && speed_ >= 5) {
        lv_obj_clear_flag(streak_left_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(streak_right_, LV_OBJ_FLAG_HIDDEN);
        int sy = road_y_ + (frame_ * speed_ * 3) % road_h_;
        lv_obj_set_pos(streak_left_, road_x_ - 8, sy);
        lv_obj_set_pos(streak_right_, road_x_ + road_w_ + 6, sy);
    } else {
        lv_obj_add_flag(streak_left_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(streak_right_, LV_OBJ_FLAG_HIDDEN);
    }

    if (game_over_) {
        return;
    }

    frame_++;
    mark_offset_ = (mark_offset_ + speed_) % kLaneMarkGap;

    if (--spawn_countdown_ <= 0) {
        SpawnTraffic();
        spawn_countdown_ = std::max(12, 28 - speed_ * 2 - static_cast<int>(esp_random() % 8));
    }

    for (auto& car : traffic_) {
        if (!car.active) {
            continue;
        }
        car.y += speed_;
        if (CheckCollision(car)) {
            game_over_ = true;
            shake_timer_ = 6;
            shake_amplitude_ = 4;
            lv_obj_clear_flag(crash_overlay_, LV_OBJ_FLAG_HIDDEN);
            int px = LaneCenterX(player_lane_);
            int py = height_ - kPlayerYMargin + kCarH / 2;
            const lv_color_t kDebrisColors[] = {
                lv_color_hex(0xfef08a), lv_color_hex(0xf97316),
                lv_color_hex(0xef4444), lv_color_hex(0xffffff),
            };
            for (auto& p : particles_) {
                if (p.life > 0) continue;
                p.life = 8 + (esp_random() % 6);
                p.vx = (static_cast<int>(esp_random() % 7) - 3);
                p.vy = -(static_cast<int>(esp_random() % 5) + 2);
                lv_obj_set_pos(p.obj, px + (esp_random() % 12) - 6, py);
                StylePlain(p.obj, kDebrisColors[esp_random() % 4]);
                lv_obj_set_style_bg_opa(p.obj, 255, 0);
                lv_obj_clear_flag(p.obj, LV_OBJ_FLAG_HIDDEN);
            }
            RacingSfx::GetInstance().Play(RacingSfxEvent::kCrash);
            lv_label_set_text_fmt(message_label_, "CRASH!\nSCORE %d\nclick restart", score_);
            lv_obj_clear_flag(message_label_, LV_OBJ_FLAG_HIDDEN);
            return;
        }
        if (car.y > height_ + kCarH) {
            car.active = false;
            score_++;
            RacingSfx::GetInstance().Play(RacingSfxEvent::kScore);
            if (score_ % 8 == 0 && speed_ < 7) {
                speed_++;
            }
            lv_obj_add_flag(car.body, LV_OBJ_FLAG_HIDDEN);
        } else {
            SetCarPosition(car.body, car.lane, car.y);
        }
    }

    UpdateUi();
}

void RacingGame::SpawnTraffic() {
    auto it = std::find_if(traffic_.begin(), traffic_.end(), [](const Car& car) {
        return !car.active;
    });
    if (it == traffic_.end()) {
        return;
    }

    it->lane = esp_random() % 3;
    it->y = -kCarH;
    it->type = static_cast<CarType>(1 + (esp_random() % 4));
    it->active = true;
    RedrawCar(it->body, it->type);
    lv_obj_clear_flag(it->body, LV_OBJ_FLAG_HIDDEN);
    SetCarPosition(it->body, it->lane, it->y);
}

void RacingGame::UpdateUi() {
    lv_label_set_text_fmt(score_label_, "SCORE %d", score_);

    if (speed_label_ != nullptr && speed_bar_fill_ != nullptr) {
        lv_color_t spd_color;
        switch (speed_) {
        case 1: case 2: spd_color = lv_color_hex(0x4ade80); break;
        case 3: case 4: spd_color = lv_color_hex(0xfacc15); break;
        case 5: case 6: spd_color = lv_color_hex(0xf97316); break;
        default:        spd_color = lv_color_hex(0xef4444); break;
        }
        lv_obj_set_style_text_color(speed_label_, spd_color, 0);
        lv_label_set_text_fmt(speed_label_, "SPD %d", speed_);
        StylePlain(speed_bar_fill_, spd_color);
        lv_obj_set_style_radius(speed_bar_fill_, 1, 0);
        int bar_w = speed_ * 8;
        if (bar_w > 56) bar_w = 56;
        lv_obj_set_size(speed_bar_fill_, bar_w, 3);
    }

    for (size_t i = 0; i < lane_marks_.size(); ++i) {
        int divider = static_cast<int>(i % 2) + 1;
        int step = static_cast<int>(i / 2);
        int x = road_x_ + divider * lane_w_ - kLaneMarkW / 2;
        int y = road_y_ + mark_offset_ + step * kLaneMarkGap;
        while (y > road_y_ + road_h_) {
            y -= road_h_;
        }
        lv_obj_set_pos(lane_marks_[i], x, y);
    }
}

void RacingGame::SetCarPosition(lv_obj_t* car, int lane, int y) {
    if (car == nullptr) {
        return;
    }
    lv_obj_set_pos(car, LaneCenterX(lane) - kCarW / 2, y);
}

int RacingGame::LaneCenterX(int lane) const {
    return road_x_ + lane * lane_w_ + lane_w_ / 2;
}

bool RacingGame::CheckCollision(const Car& car) const {
    const int player_y = height_ - kPlayerYMargin;
    if (car.lane != player_lane_) {
        return false;
    }
    return car.y + kCarH > player_y + 4 && car.y < player_y + kCarH - 4;
}
