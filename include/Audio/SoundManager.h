#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

enum class SoundType {
    ATTACK_SWORD,
    ATTACK_MAGIC,
    ENEMY_HIT,
    PLAYER_HIT,
    PICKUP_ITEM,
    LEVEL_UP,
    GAME_OVER,
    POTION_USE
};

class SoundManager {
private:
    std::unordered_map<std::string, Sound> sounds;
    std::unordered_map<std::string, Music> music;

    float masterVolume;
    float musicVolume;
    float sfxVolume;

public:
    SoundManager();
    ~SoundManager();

    void initialize();

    // Sound effects
    void playSound(SoundType soundType);
    void playCustomSound(const std::string& filePath);

    // Music
    void playMusic(const std::string& filePath, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();

    // Volume control
    void setMasterVolume(float volume);
    void setMusicVolume(float volume);
    void setSFXVolume(float volume);

    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    float getSFXVolume() const { return sfxVolume; }
};
