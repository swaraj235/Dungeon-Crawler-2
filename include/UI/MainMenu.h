#pragma once
#include "raylib.h"
#include <string>

enum class MenuState {
    MAIN_MENU,
    NEW_GAME,
    LOAD_GAME,
    SETTINGS,
    PLAYING,
    PAUSED
};

class MainMenu {
private:
    MenuState currentState;
    MenuState lastSelectedState;
    int selectedOption;
    bool saveExists;

    // Settings
    float masterVolume;
    float musicVolume;
    int graphicsQuality;
    bool enableParticles;

    // Name
    std::string playerName;
    bool inputtingName;

public:
    MainMenu();

    MenuState update();
    void draw();

    MenuState getState() const { return currentState; }
    MenuState getLastSelectedState() const { return lastSelectedState; }
    void setState(MenuState state) { currentState = state; }

    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    bool getEnableParticles() const { return enableParticles; }
    bool doesSaveExist() const { return saveExists; }
    std::string getPlayerName() const { return playerName; }

private:
    void drawMainMenu();
    void drawNewGame();
    void drawLoadGame();
    void drawSettings();

    MenuState updateMainMenu();
    MenuState updateNewGame();
    MenuState updateLoadGame();
    MenuState updateSettings();
};
