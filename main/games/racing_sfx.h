#ifndef RACING_SFX_H_
#define RACING_SFX_H_

enum class RacingSfxEvent {
    kStart,
    kMove,
    kScore,
    kCrash,
    kRestart,
    kCoin,
    kClose,
};

class RacingSfx {
public:
    static RacingSfx& GetInstance();

    void Play(RacingSfxEvent event);

private:
    RacingSfx() = default;
    RacingSfx(const RacingSfx&) = delete;
    RacingSfx& operator=(const RacingSfx&) = delete;

    void EnsureTask();
};

#endif  // RACING_SFX_H_
