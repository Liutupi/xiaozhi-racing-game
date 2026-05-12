#ifndef RACING_GAME_H_
#define RACING_GAME_H_

#include "display.h"

#include <esp_timer.h>
#include <lvgl.h>

#include <array>
#include <cstdint>
#include <nvs.h>

enum class CarType : uint8_t {
    kPlayer = 0,
    kSports,
    kSuv,
    kPickup,
    kTruck,
    kCount
};

class RacingGame {
public:
    RacingGame() = default;
    ~RacingGame();

    void Start(Display* display);
    void Stop();
    bool HandleClick();
    bool HandleDoubleClick();
    bool MoveRight();
    bool MoveLeft();
    bool IsRunning() const { return running_; }

private:
    struct Car {
        lv_obj_t* body = nullptr;
        CarType type = CarType::kPlayer;
        int lane = 0;
        int y = -40;
        bool active = false;
    };

    struct Particle {
        lv_obj_t* obj = nullptr;
        int vx = 0;
        int vy = 0;
        int life = 0;
    };

    struct Coin {
        lv_obj_t* obj = nullptr;
        int lane = 0;
        int y = -20;
        bool active = false;
    };

    Display* display_ = nullptr;
    esp_timer_handle_t timer_ = nullptr;
    lv_obj_t* layer_ = nullptr;
    lv_obj_t* road_ = nullptr;
    lv_obj_t* score_label_ = nullptr;
    lv_obj_t* message_label_ = nullptr;
    lv_obj_t* player_car_ = nullptr;
    lv_obj_t* sun_ = nullptr;
    lv_obj_t* speed_label_ = nullptr;
    lv_obj_t* speed_bar_bg_ = nullptr;
    lv_obj_t* speed_bar_fill_ = nullptr;
    lv_obj_t* high_score_label_ = nullptr;
    lv_obj_t* close_label_ = nullptr;
    std::array<lv_obj_t*, 4> sky_bands_{};
    std::array<lv_obj_t*, 4> sun_stripes_{};
    std::array<lv_obj_t*, 10> lane_marks_{};
    std::array<lv_obj_t*, 2> road_edges_{};
    std::array<Car, 4> traffic_{};
    std::array<Particle, 8> particles_{};
    std::array<Coin, 3> coins_{};
    lv_obj_t* crash_overlay_ = nullptr;
    lv_obj_t* streak_left_ = nullptr;
    lv_obj_t* streak_right_ = nullptr;
    int shake_timer_ = 0;
    int shake_amplitude_ = 0;
    int coin_spawn_timer_ = 30;
    int close_timer_ = 0;
    int prev_player_lane_ = 1;
    int road_narrow_timer_ = 0;
    bool road_narrowed_ = false;
    int closed_lane_ = -1;

    bool running_ = false;
    bool game_over_ = false;
    int width_ = 0;
    int height_ = 0;
    int road_x_ = 0;
    int road_y_ = 36;
    int road_w_ = 84;
    int road_h_ = 0;
    int lane_w_ = 28;
    int player_lane_ = 1;
    int score_ = 0;
    int32_t high_score_ = 0;
    int speed_ = 3;
    int frame_ = 0;
    int spawn_countdown_ = 0;
    int mark_offset_ = 0;

    void CreateUi();
    void DestroyUi();
    void ResetGame(bool play_sfx = false);
    void Tick();
    void SpawnTraffic();
    void UpdateUi();
    void SetCarPosition(lv_obj_t* car, int lane, int y);
    int LaneCenterX(int lane) const;
    bool CheckCollision(const Car& car) const;
    void LoadHighScore();
    void SaveHighScore();
};

#endif  // RACING_GAME_H_
