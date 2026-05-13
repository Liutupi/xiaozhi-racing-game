#include "number_2048_game.h"

#include <esp_random.h>

#include <algorithm>

namespace {

constexpr uint32_t kBg = 0x17130f;
constexpr uint32_t kBoard = 0xb3a397;
constexpr uint32_t kEmpty = 0xc7b9ac;
constexpr uint32_t kTextDark = 0x6c635b;
constexpr uint32_t kTextLight = 0xf8f5f0;
constexpr uint32_t kAccent = 0xffd166;
constexpr int kSize = 4;

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

int Index(int row, int col) {
    return row * kSize + col;
}

}  // namespace

Number2048Game::~Number2048Game() {
    Stop();
}

void Number2048Game::Start(Display* display) {
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

void Number2048Game::Stop() {
    if (display_ != nullptr) {
        DisplayLockGuard lock(display_);
        DestroyUi();
    }
    running_ = false;
    display_ = nullptr;
}

bool Number2048Game::HandleClick() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    if (game_over_) {
        ResetGame();
        return true;
    }

    if (ApplyMove(direction_)) {
        SpawnTile();
        won_ = won_ || std::any_of(board_.begin(), board_.end(), [](uint16_t value) { return value >= 2048; });
        game_over_ = !HasMove();
    }
    DrawBoard();
    DrawStatus();
    return true;
}

bool Number2048Game::HandleDoubleClick() {
    return MoveLeft();
}

bool Number2048Game::MoveRight() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    direction_ = static_cast<Direction>((static_cast<uint8_t>(direction_) + 1) % 4);
    DrawStatus();
    return true;
}

bool Number2048Game::MoveLeft() {
    if (!running_ || display_ == nullptr) {
        return false;
    }

    DisplayLockGuard lock(display_);
    direction_ = static_cast<Direction>((static_cast<uint8_t>(direction_) + 3) % 4);
    DrawStatus();
    return true;
}

void Number2048Game::CreateUi() {
    layer_ = lv_obj_create(lv_screen_active());
    lv_obj_set_size(layer_, width_, height_);
    lv_obj_set_pos(layer_, 0, 0);
    StyleBox(layer_, kBg, kBg, 0, 0);

    title_label_ = AddLabel(layer_, 5, 3, 38, kAccent);
    lv_label_set_text(title_label_, "2048");
    score_label_ = AddLabel(layer_, 42, 3, 44, kTextLight, LV_TEXT_ALIGN_CENTER);
    dir_label_ = AddLabel(layer_, width_ - 42, 3, 38, kAccent, LV_TEXT_ALIGN_RIGHT);

    lv_obj_t* board = lv_obj_create(layer_);
    const int board_side = std::min(width_ - 10, height_ - 42);
    const int board_x = (width_ - board_side) / 2;
    const int board_y = 24;
    lv_obj_set_size(board, board_side, board_side);
    lv_obj_set_pos(board, board_x, board_y);
    StyleBox(board, kBoard, kBoard, 0, 4);

    const int gap = 3;
    const int tile = (board_side - gap * 5) / 4;
    for (int row = 0; row < kSize; ++row) {
        for (int col = 0; col < kSize; ++col) {
            const int idx = Index(row, col);
            tile_boxes_[idx] = lv_obj_create(board);
            lv_obj_set_size(tile_boxes_[idx], tile, tile);
            lv_obj_set_pos(tile_boxes_[idx], gap + col * (tile + gap), gap + row * (tile + gap));
            StyleBox(tile_boxes_[idx], kEmpty, kEmpty, 0, 3);

            tile_labels_[idx] = lv_label_create(tile_boxes_[idx]);
            lv_obj_set_width(tile_labels_[idx], tile);
            StyleLabel(tile_labels_[idx], kTextDark, LV_TEXT_ALIGN_CENTER);
            lv_obj_center(tile_labels_[idx]);
        }
    }

    message_label_ = AddLabel(layer_, 4, board_y + board_side + 3, width_ - 8, kTextLight, LV_TEXT_ALIGN_CENTER);
    footer_label_ = AddLabel(layer_, 4, height_ - 13, width_ - 8, 0x94a3b8, LV_TEXT_ALIGN_CENTER);
    lv_label_set_text(footer_label_, "GPIO39 dir | BOOT move");
}

void Number2048Game::DestroyUi() {
    if (layer_ != nullptr) {
        lv_obj_del(layer_);
    }
    layer_ = nullptr;
    title_label_ = nullptr;
    score_label_ = nullptr;
    dir_label_ = nullptr;
    message_label_ = nullptr;
    footer_label_ = nullptr;
    tile_boxes_.fill(nullptr);
    tile_labels_.fill(nullptr);
}

void Number2048Game::ResetGame() {
    board_.fill(0);
    score_ = 0;
    direction_ = Direction::kUp;
    game_over_ = false;
    won_ = false;
    SpawnTile();
    SpawnTile();
    DrawBoard();
    DrawStatus();
}

void Number2048Game::DrawBoard() {
    for (int i = 0; i < static_cast<int>(board_.size()); ++i) {
        const uint16_t value = board_[i];
        StyleBox(tile_boxes_[i], TileColor(value), TileColor(value), 0, 3);
        lv_obj_set_style_text_color(tile_labels_[i], lv_color_hex(value >= 8 ? kTextLight : kTextDark), 0);
        if (value == 0) {
            lv_label_set_text(tile_labels_[i], "");
        } else {
            lv_label_set_text_fmt(tile_labels_[i], "%u", value);
        }
        lv_obj_center(tile_labels_[i]);
    }
}

void Number2048Game::DrawStatus() {
    lv_label_set_text_fmt(score_label_, "S %d", score_);
    lv_label_set_text(dir_label_, DirectionName());
    if (game_over_) {
        lv_label_set_text(message_label_, "GAME OVER | BOOT restart");
    } else if (won_) {
        lv_label_set_text(message_label_, "2048! keep going");
    } else {
        lv_label_set_text(message_label_, "Choose direction");
    }
}

void Number2048Game::SpawnTile() {
    std::array<int, 16> empty{};
    int count = 0;
    for (int i = 0; i < static_cast<int>(board_.size()); ++i) {
        if (board_[i] == 0) {
            empty[count++] = i;
        }
    }
    if (count == 0) {
        return;
    }
    const int idx = empty[esp_random() % count];
    board_[idx] = (esp_random() % 10 == 0) ? 4 : 2;
}

bool Number2048Game::ApplyMove(Direction direction) {
    bool moved = false;
    for (int i = 0; i < kSize; ++i) {
        std::array<uint16_t, 4> line{};
        for (int j = 0; j < kSize; ++j) {
            switch (direction) {
            case Direction::kLeft:  line[j] = board_[Index(i, j)]; break;
            case Direction::kRight: line[j] = board_[Index(i, kSize - 1 - j)]; break;
            case Direction::kUp:    line[j] = board_[Index(j, i)]; break;
            case Direction::kDown:  line[j] = board_[Index(kSize - 1 - j, i)]; break;
            }
        }
        const bool line_moved = MoveLine(line);
        moved = moved || line_moved;
        for (int j = 0; j < kSize; ++j) {
            switch (direction) {
            case Direction::kLeft:  board_[Index(i, j)] = line[j]; break;
            case Direction::kRight: board_[Index(i, kSize - 1 - j)] = line[j]; break;
            case Direction::kUp:    board_[Index(j, i)] = line[j]; break;
            case Direction::kDown:  board_[Index(kSize - 1 - j, i)] = line[j]; break;
            }
        }
    }
    return moved;
}

bool Number2048Game::MoveLine(std::array<uint16_t, 4>& line) {
    const auto before = line;
    std::array<uint16_t, 4> packed{};
    int out = 0;
    for (uint16_t value : line) {
        if (value != 0) {
            packed[out++] = value;
        }
    }

    for (int i = 0; i < kSize - 1; ++i) {
        if (packed[i] != 0 && packed[i] == packed[i + 1]) {
            packed[i] *= 2;
            score_ += packed[i];
            for (int j = i + 1; j < kSize - 1; ++j) {
                packed[j] = packed[j + 1];
            }
            packed[kSize - 1] = 0;
        }
    }
    line = packed;
    return line != before;
}

bool Number2048Game::HasMove() const {
    for (uint16_t value : board_) {
        if (value == 0) {
            return true;
        }
    }
    for (int row = 0; row < kSize; ++row) {
        for (int col = 0; col < kSize; ++col) {
            const uint16_t value = board_[Index(row, col)];
            if (col + 1 < kSize && value == board_[Index(row, col + 1)]) return true;
            if (row + 1 < kSize && value == board_[Index(row + 1, col)]) return true;
        }
    }
    return false;
}

uint32_t Number2048Game::TileColor(uint16_t value) const {
    switch (value) {
    case 0: return kEmpty;
    case 2: return 0xeee4da;
    case 4: return 0xede0c8;
    case 8: return 0xf2b179;
    case 16: return 0xf59563;
    case 32: return 0xf67c5f;
    case 64: return 0xf75f3b;
    case 128: return 0xedcf72;
    case 256: return 0xedcc61;
    case 512: return 0xedc850;
    case 1024: return 0xedc53f;
    default: return 0xedc22e;
    }
}

const char* Number2048Game::DirectionName() const {
    switch (direction_) {
    case Direction::kUp: return "UP";
    case Direction::kRight: return "RIGHT";
    case Direction::kDown: return "DOWN";
    case Direction::kLeft: return "LEFT";
    }
    return "UP";
}
