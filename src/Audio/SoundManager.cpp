#include "SoundManager.h"
#include <iostream>

SoundManager::SoundManager()
    : masterVolume(0.7f), musicVolume(0.5f), sfxVolume(0.8f) {
    InitAudioDevice();
}

SoundManager::~SoundManager() {
    CloseAudioDevice();
}

void SoundManager::initialize() {
    // Load sound effects (placeholder - actual files needed)
    try {
        sounds["attack_sword"] = LoadSound("assets/sounds/sfx/attack.wav");
        sounds["hit"] = LoadSound("assets/sounds/sfx/hit.wav");
        sounds["pickup"] = LoadSound("assets/sounds/sfx/pickup.wav");
        sounds["levelup"] = LoadSound("assets/sounds/sfx/levelup.wav");
        std::cout << "Sounds loaded successfully" << std::endl;
    } catch (...) {
        std::cout << "Warning: Some sound files not found" << std::endl;
    }
}

void SoundManager::playSound(SoundType soundType) {
    std::string soundKey;

    switch (soundType) {
        case SoundType::ATTACK_SWORD: soundKey = "attack_sword"; break;
        case SoundType::ENEMY_HIT: soundKey = "hit"; break;
        case SoundType::PLAYER_HIT: soundKey = "hit"; break;
        case SoundType::PICKUP_ITEM: soundKey = "pickup"; break;
        case SoundType::LEVEL_UP: soundKey = "levelup"; break;
        default: return;
    }

    if (sounds.find(soundKey) != sounds.end()) {
        SetSoundVolume(sounds[soundKey], sfxVolume);
        PlaySound(sounds[soundKey]);
    }
}

void SoundManager::playCustomSound(const std::string& filePath) {
    Sound sound = LoadSound(filePath.c_str());
    SetSoundVolume(sound, sfxVolume);
    PlaySound(sound);
}

void SoundManager::playMusic(const std::string& filePath, bool loop) {
    Music mus = LoadMusicStream(filePath.c_str());
    mus.looping = loop;
    SetMusicVolume(mus, musicVolume);
    PlayMusicStream(mus);
    music[filePath] = mus;
}

void SoundManager::stopMusic() {
    for (auto& m : music) {
        StopMusicStream(m.second);
    }
}

void SoundManager::pauseMusic() {
    for (auto& m : music) {
        PauseMusicStream(m.second);
    }
}

void SoundManager::resumeMusic() {
    for (auto& m : music) {
        ResumeMusicStream(m.second);
    }
}

void SoundManager::setMasterVolume(float volume) {
    masterVolume = std::max(0.0f, std::min(1.0f, volume));
}

void SoundManager::setMusicVolume(float volume) {
    musicVolume = std::max(0.0f, std::min(1.0f, volume));
}

void SoundManager::setSFXVolume(float volume) {
    sfxVolume = std::max(0.0f, std::min(1.0f, volume));
}
