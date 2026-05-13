#ifndef NUMBER_MEMORY_GAME_H_
#define NUMBER_MEMORY_GAME_H_

#include "display.h"

#include <lvgl.h>

#include <array>
#include <cstdint>

class NumberMemoryGame {
public:
    NumberMemoryGame() = default;
    ~NumberMemoryGame();

    void Start(Display* display);
    void Stop();
    bool HandleClick();
    bool HandleDoubleClick();
    bool MoveRight();
    bool MoveLeft();
    bool IsRunning() const { return running_; }

private:
    static constexpr int kRows = 4;
    static constexpr int kCols = 4;
    static constexpr int kCardCount = kRows * kCols;

    Display* display_ = nullptr;
    lv_obj_t* layer_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* score_label_ = nullptr;
    lv_obj_t* message_label_ = nullptr;
    lv_obj_t* footer_label_ = nullptr;
    std::array<lv_obj_t*, kCardCount> card_boxes_{};
    std::array<lv_obj_t*, kCardCount> card_labels_{};
    std::array<uint8_t, kCardCount> values_{};
    std::array<bool, kCardCount> visible_{};
    std::array<bool, kCardCount> matched_{};

    bool running_ = false;
    bool finished_ = false;
    bool pending_hide_ = false;
    int width_ = 0;
    int height_ = 0;
    int selected_ = 0;
    int first_ = -1;
    int second_ = -1;
    int matches_ = 0;
    int attempts_ = 0;

    void CreateUi();
    void DestroyUi();
    void ResetGame();
    void ShuffleCards();
    void DrawCards();
    void DrawCard(int index);
    void DrawStatus(const char* message = nullptr);
    void MoveCursor(int delta);
    void HidePending();
};

#endif  // NUMBER_MEMORY_GAME_H_
