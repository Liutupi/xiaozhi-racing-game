#ifndef NUMBER_2048_GAME_H_
#define NUMBER_2048_GAME_H_

#include "display.h"

#include <lvgl.h>

#include <array>
#include <cstdint>

class Number2048Game {
public:
    Number2048Game() = default;
    ~Number2048Game();

    void Start(Display* display);
    void Stop();
    bool HandleClick();
    bool HandleDoubleClick();
    bool MoveRight();
    bool MoveLeft();
    bool IsRunning() const { return running_; }

private:
    enum class Direction : uint8_t {
        kUp,
        kRight,
        kDown,
        kLeft,
    };

    Display* display_ = nullptr;
    lv_obj_t* layer_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* score_label_ = nullptr;
    lv_obj_t* dir_label_ = nullptr;
    lv_obj_t* message_label_ = nullptr;
    lv_obj_t* footer_label_ = nullptr;
    std::array<lv_obj_t*, 16> tile_boxes_{};
    std::array<lv_obj_t*, 16> tile_labels_{};
    std::array<uint16_t, 16> board_{};

    bool running_ = false;
    bool game_over_ = false;
    bool won_ = false;
    int width_ = 0;
    int height_ = 0;
    int score_ = 0;
    Direction direction_ = Direction::kUp;

    void CreateUi();
    void DestroyUi();
    void ResetGame();
    void DrawBoard();
    void DrawStatus();
    void SpawnTile();
    bool ApplyMove(Direction direction);
    bool MoveLine(std::array<uint16_t, 4>& line);
    bool HasMove() const;
    uint32_t TileColor(uint16_t value) const;
    const char* DirectionName() const;
};

#endif  // NUMBER_2048_GAME_H_
