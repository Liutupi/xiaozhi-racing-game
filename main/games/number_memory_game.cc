#include "number_memory_game.h"

#include <esp_random.h>

#include <algorithm>

namespace {

constexpr uint32_t kBg = 0x07111f;
constexpr uint32_t kPanel = 0x0f172a;
constexpr uint32_t kCard = 0x16345f;
constexpr uint32_t kCardOpen = 0xfef3c7;
constexpr uint32_t kCardMatched = 0x064e3b;
constexpr uint32_t kAccent = 0xffd166;
constexpr uint32_t kText = 0xf8fafc;
constexpr uint32_t kDark = 0x111827;
constexpr uint32_t kMuted = 0x94a3b8;

void StyleBox(lv_obj_t* obj, uint32_t bg, uint32_t border, int border_width, int radius) {
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(obj, lv_color_hex(bg), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(border), 0);
    lv_obj_set_style_border_width(obj, border_width, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

void StyleLabel(lv_obj_t* label, uint32_t color, lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, align, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    lv_obj_set_style_text_line_space(label, 0, 0);
}

lv_obj_t* AddLabel(lv_obj_t* parent, int x, int y, int w, uint32_t color,
                   lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, w);
    StyleLabel(label, color, align);
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    return label;
}

}  // namespace

NumberMemoryGame::~NumberMemoryGame() {
    Stop();
}

void NumberMemoryGame::Start(Display* display) {
    if (running_ || display == nullptr) {
        return;
    }

    display_ = display;
    width_ = display_->width();
    height_ = display_->height();
    {
        DisplayLockGuard lock(display_);
        CreateUi();
        ResetGame();
    }
    running_ = true;
}

void NumberMemoryGame::Stop() {
    if (display_ != nullptr) {
        DisplayLockGuard lock(display_);
        DestroyUi();
    }
    running_ = false;
    display_ = nullptr;
}

bool NumberMemoryGame::HandleClick() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (finished_) {
        ResetGame();
        return true;
    }
    if (pending_hide_) {
        HidePending();
        DrawCards();
        DrawStatus("Try again");
        return true;
    }
    if (matched_[selected_]) {
        DrawStatus("Already matched");
        return true;
    }

    visible_[selected_] = true;
    if (first_ < 0) {
        first_ = selected_;
        DrawCards();
        DrawStatus("Pick match");
        return true;
    }
    if (selected_ == first_) {
        DrawCards();
        DrawStatus("Pick another");
        return true;
    }

    second_ = selected_;
    ++attempts_;
    if (values_[first_] == values_[second_]) {
        matched_[first_] = true;
        matched_[second_] = true;
        first_ = -1;
        second_ = -1;
        ++matches_;
        finished_ = matches_ == kCardCount / 2;
        DrawCards();
        DrawStatus(finished_ ? "All matched! BOOT restart" : "Match!");
    } else {
        pending_hide_ = true;
        DrawCards();
        DrawStatus("No match | BOOT hide");
    }
    return true;
}

bool NumberMemoryGame::HandleDoubleClick() {
    return MoveLeft();
}

bool NumberMemoryGame::MoveRight() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (!finished_) {
        MoveCursor(1);
    }
    return true;
}

bool NumberMemoryGame::MoveLeft() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (!finished_) {
        MoveCursor(-1);
    }
    return true;
}

void NumberMemoryGame::CreateUi() {
    layer_ = lv_obj_create(lv_screen_active());
    lv_obj_set_size(layer_, width_, height_);
    lv_obj_set_pos(layer_, 0, 0);
    StyleBox(layer_, kBg, kBg, 0, 0);

    lv_obj_t* header = lv_obj_create(layer_);
    lv_obj_set_size(header, width_, 20);
    lv_obj_set_pos(header, 0, 0);
    StyleBox(header, kPanel, kPanel, 0, 0);

    title_label_ = AddLabel(layer_, 5, 3, 52, kAccent);
    lv_label_set_text(title_label_, "MEMORY");
    score_label_ = AddLabel(layer_, width_ - 58, 3, 54, kText, LV_TEXT_ALIGN_RIGHT);

    const int gap = 3;
    const int side = std::min((width_ - 10 - gap * 3) / 4, (height_ - 48 - gap * 3) / 4);
    const int grid_w = side * 4 + gap * 3;
    const int grid_x = (width_ - grid_w) / 2;
    const int grid_y = 27;
    for (int row = 0; row < kRows; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const int idx = row * kCols + col;
            card_boxes_[idx] = lv_obj_create(layer_);
            lv_obj_set_size(card_boxes_[idx], side, side);
            lv_obj_set_pos(card_boxes_[idx], grid_x + col * (side + gap), grid_y + row * (side + gap));
            StyleBox(card_boxes_[idx], kCard, 0x235a7c, 1, 4);

            card_labels_[idx] = lv_label_create(card_boxes_[idx]);
            lv_obj_set_width(card_labels_[idx], side);
            StyleLabel(card_labels_[idx], kText, LV_TEXT_ALIGN_CENTER);
            lv_obj_center(card_labels_[idx]);
        }
    }

    message_label_ = AddLabel(layer_, 4, height_ - 28, width_ - 8, kText, LV_TEXT_ALIGN_CENTER);
    footer_label_ = AddLabel(layer_, 4, height_ - 13, width_ - 8, kMuted, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(footer_label_, "GPIO39 cursor | BOOT flip");
}

void NumberMemoryGame::DestroyUi() {
    if (layer_ != nullptr) {
        lv_obj_del(layer_);
    }
    layer_ = nullptr;
    title_label_ = nullptr;
    score_label_ = nullptr;
    message_label_ = nullptr;
    footer_label_ = nullptr;
    card_boxes_.fill(nullptr);
    card_labels_.fill(nullptr);
}

void NumberMemoryGame::ResetGame() {
    visible_.fill(false);
    matched_.fill(false);
    selected_ = 0;
    first_ = -1;
    second_ = -1;
    matches_ = 0;
    attempts_ = 0;
    pending_hide_ = false;
    finished_ = false;
    ShuffleCards();
    DrawCards();
    DrawStatus("Find pairs");
}

void NumberMemoryGame::ShuffleCards() {
    for (int i = 0; i < kCardCount; ++i) {
        values_[i] = static_cast<uint8_t>(i / 2 + 1);
    }
    for (int i = kCardCount - 1; i > 0; --i) {
        const int j = esp_random() % (i + 1);
        std::swap(values_[i], values_[j]);
    }
}

void NumberMemoryGame::DrawCards() {
    for (int i = 0; i < kCardCount; ++i) {
        DrawCard(i);
    }
}

void NumberMemoryGame::DrawCard(int index) {
    const bool active = index == selected_;
    const bool open = visible_[index] || matched_[index];
    const uint32_t bg = matched_[index] ? kCardMatched : (open ? kCardOpen : kCard);
    const uint32_t border = active ? kAccent : (matched_[index] ? 0x22c55e : 0x235a7c);
    StyleBox(card_boxes_[index], bg, border, active ? 2 : 1, 4);
    lv_obj_set_style_text_color(card_labels_[index], lv_color_hex(open ? kDark : kText), 0);
    if (open) {
        lv_label_set_text_fmt(card_labels_[index], "%u", values_[index]);
    } else {
        lv_label_set_text(card_labels_[index], "?");
    }
    lv_obj_center(card_labels_[index]);
}

void NumberMemoryGame::DrawStatus(const char* message) {
    lv_label_set_text_fmt(score_label_, "%d/%d %d", matches_, kCardCount / 2, attempts_);
    if (message != nullptr) {
        lv_label_set_text(message_label_, message);
    }
}

void NumberMemoryGame::MoveCursor(int delta) {
    selected_ = (selected_ + delta + kCardCount) % kCardCount;
    DrawCards();
}

void NumberMemoryGame::HidePending() {
    if (first_ >= 0) {
        visible_[first_] = false;
    }
    if (second_ >= 0) {
        visible_[second_] = false;
    }
    first_ = -1;
    second_ = -1;
    pending_hide_ = false;
}
