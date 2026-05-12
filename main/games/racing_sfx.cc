#include "racing_sfx.h"

#include "audio_codec.h"
#include "board.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace {
constexpr int kQueueDepth = 8;
constexpr int kAmplitude = 8200;
constexpr float kPi = 3.14159265358979323846f;

QueueHandle_t g_queue = nullptr;
TaskHandle_t g_task = nullptr;

void AppendTone(std::vector<int16_t>& pcm, int sample_rate, int freq_hz, int duration_ms, int amplitude = kAmplitude) {
    const int samples = std::max(1, sample_rate * duration_ms / 1000);
    const size_t start = pcm.size();
    pcm.resize(start + samples);

    const float step = 2.0f * kPi * static_cast<float>(freq_hz) / static_cast<float>(sample_rate);
    float phase = 0.0f;
    for (int i = 0; i < samples; ++i) {
        const float envelope = 1.0f - static_cast<float>(i) / static_cast<float>(samples);
        pcm[start + i] = static_cast<int16_t>(std::sin(phase) * amplitude * envelope);
        phase += step;
    }
}

void AppendSilence(std::vector<int16_t>& pcm, int sample_rate, int duration_ms) {
    const int samples = std::max(1, sample_rate * duration_ms / 1000);
    pcm.insert(pcm.end(), samples, 0);
}

std::vector<int16_t> BuildSound(RacingSfxEvent event, int sample_rate) {
    std::vector<int16_t> pcm;
    pcm.reserve(sample_rate / 4);

    switch (event) {
    case RacingSfxEvent::kStart:
        AppendTone(pcm, sample_rate, 440, 55);
        AppendTone(pcm, sample_rate, 660, 75);
        AppendTone(pcm, sample_rate, 880, 90);
        break;
    case RacingSfxEvent::kMove:
        AppendTone(pcm, sample_rate, 720, 45, 6800);
        AppendSilence(pcm, sample_rate, 8);
        AppendTone(pcm, sample_rate, 540, 35, 5200);
        break;
    case RacingSfxEvent::kScore:
        AppendTone(pcm, sample_rate, 980, 45, 6400);
        AppendTone(pcm, sample_rate, 1320, 55, 6200);
        break;
    case RacingSfxEvent::kCrash:
        AppendTone(pcm, sample_rate, 180, 120, 9000);
        AppendSilence(pcm, sample_rate, 20);
        AppendTone(pcm, sample_rate, 95, 160, 8500);
        break;
    case RacingSfxEvent::kRestart:
        AppendTone(pcm, sample_rate, 520, 55);
        AppendTone(pcm, sample_rate, 780, 85);
        break;
    case RacingSfxEvent::kCoin:
        AppendTone(pcm, sample_rate, 1200, 28, 5600);
        AppendSilence(pcm, sample_rate, 6);
        AppendTone(pcm, sample_rate, 1600, 36, 5400);
        break;
    case RacingSfxEvent::kClose:
        AppendTone(pcm, sample_rate, 900, 30, 6200);
        AppendSilence(pcm, sample_rate, 8);
        AppendTone(pcm, sample_rate, 1400, 40, 6000);
        break;
    }

    AppendSilence(pcm, sample_rate, 20);
    return pcm;
}

void SfxTask(void*) {
    RacingSfxEvent event;
    while (true) {
        if (xQueueReceive(g_queue, &event, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        auto codec = Board::GetInstance().GetAudioCodec();
        if (codec == nullptr || codec->output_sample_rate() <= 0) {
            continue;
        }

        auto pcm = BuildSound(event, codec->output_sample_rate());
        codec->EnableOutput(true);
        codec->OutputData(pcm);
    }
}
}  // namespace

RacingSfx& RacingSfx::GetInstance() {
    static RacingSfx instance;
    return instance;
}

void RacingSfx::Play(RacingSfxEvent event) {
    EnsureTask();
    if (g_queue != nullptr) {
        xQueueSend(g_queue, &event, 0);
    }
}

void RacingSfx::EnsureTask() {
    if (g_task != nullptr) {
        return;
    }

    if (g_queue == nullptr) {
        g_queue = xQueueCreate(kQueueDepth, sizeof(RacingSfxEvent));
    }
    if (g_task == nullptr && g_queue != nullptr) {
        xTaskCreate(SfxTask, "racing_sfx", 4096, nullptr, 1, &g_task);
    }
}
