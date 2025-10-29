#include "MainMenu.h"
#include "SaveSystem.h"
#include <iostream>
#include <ctime>

MainMenu::MainMenu()
    : currentState(MenuState::MAIN_MENU),
      selectedOption(0),
      masterVolume(0.7f),
      musicVolume(0.5f),
      graphicsQuality(2),
      enableParticles(true),
      playerName(""),
      inputtingName(false) {
    saveExists = SaveSystem::saveExists("saves/savegame.json");
}

MenuState MainMenu::update() {
    // MAIN MENU - Selection
    if (currentState == MenuState::MAIN_MENU) {
        return updateMainMenu();
    }

    // NEW GAME - Player name input
    if (currentState == MenuState::NEW_GAME) {
        return updateNewGame();
    }

    // LOAD GAME - Confirmation
    if (currentState == MenuState::LOAD_GAME) {
        return updateLoadGame();
    }

    // SETTINGS - Volume adjustment
    if (currentState == MenuState::SETTINGS) {
        return updateSettings();
    }

    return currentState;
}

MenuState MainMenu::updateMainMenu() {
    // Navigation
    if (IsKeyPressed(KEY_UP)) selectedOption--;
    if (IsKeyPressed(KEY_DOWN)) selectedOption++;

    // Calculate max options
    int maxOptions = saveExists ? 4 : 3;
    if (selectedOption < 0) selectedOption = maxOptions - 1;
    if (selectedOption >= maxOptions) selectedOption = 0;

    // Selection
    if (IsKeyPressed(KEY_ENTER)) {
        if (saveExists) {
            // With save: NEW, RESUME, SETTINGS, QUIT
            if (selectedOption == 0) {
                currentState = MenuState::NEW_GAME;
                inputtingName = true;
                playerName = "";
            }
            else if (selectedOption == 1) {
                currentState = MenuState::LOAD_GAME;
            }
            else if (selectedOption == 2) {
                currentState = MenuState::SETTINGS;
                selectedOption = 0;
            }
            else if (selectedOption == 3) {
                return MenuState::PLAYING;
            }
        } else {
            // Without save: NEW, SETTINGS, QUIT
            if (selectedOption == 0) {
                currentState = MenuState::NEW_GAME;
                inputtingName = true;
                playerName = "";
            }
            else if (selectedOption == 1) {
                currentState = MenuState::SETTINGS;
                selectedOption = 0;
            }
            else if (selectedOption == 2) {
                return MenuState::PLAYING;
            }
        }
    }

    return currentState;
}

MenuState MainMenu::updateNewGame() {
    if (!inputtingName) {
        // Name already entered, show confirmation
        if (IsKeyPressed(KEY_ENTER)) {
            return MenuState::PLAYING;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            currentState = MenuState::MAIN_MENU;
            selectedOption = 0;
            return currentState;
        }
    } else {
        // Still inputting name
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32 && key <= 126) && playerName.length() < 20) {
                playerName += (char)key;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && playerName.length() > 0) {
            playerName.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER) && playerName.length() > 0) {
            inputtingName = false;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            currentState = MenuState::MAIN_MENU;
            selectedOption = 0;
            playerName = "";
            inputtingName = false;
            return currentState;
        }
    }

    return currentState;
}

MenuState MainMenu::updateLoadGame() {
    if (IsKeyPressed(KEY_ENTER)) {
        return MenuState::PLAYING;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        currentState = MenuState::MAIN_MENU;
        selectedOption = 0;
        return currentState;
    }

    return currentState;
}

MenuState MainMenu::updateSettings() {
    if (IsKeyPressed(KEY_LEFT)) {
        masterVolume = std::max(0.0f, masterVolume - 0.1f);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        masterVolume = std::min(1.0f, masterVolume + 0.1f);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        currentState = MenuState::MAIN_MENU;
        selectedOption = 0;
        return currentState;
    }

    return currentState;
}

void MainMenu::draw() {
    BeginDrawing();
    ClearBackground(Color{20, 20, 30, 255});

    if (currentState == MenuState::MAIN_MENU) {
        drawMainMenu();
    }
    else if (currentState == MenuState::NEW_GAME) {
        drawNewGame();
    }
    else if (currentState == MenuState::LOAD_GAME) {
        drawLoadGame();
    }
    else if (currentState == MenuState::SETTINGS) {
        drawSettings();
    }

    EndDrawing();
}

void MainMenu::drawMainMenu() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // Background
    DrawRectangle(0, 0, screenWidth, screenHeight, Color{20, 20, 30, 255});

    // Title
    std::string title = "DUNGEON CRAWLER v2.0";
    int titleWidth = MeasureText(title.c_str(), 56);
    DrawText(title.c_str(), (screenWidth - titleWidth) / 2, 80, 56, GOLD);

    // Subtitle
    std::string subtitle = "An Epic RPG Adventure";
    int subtitleWidth = MeasureText(subtitle.c_str(), 20);
    DrawText(subtitle.c_str(), (screenWidth - subtitleWidth) / 2, 160, 20, GRAY);

    // Build menu options
    std::vector<std::string> options;
    options.push_back("NEW GAME");
    if (saveExists) options.push_back("RESUME GAME");
    options.push_back("SETTINGS");
    options.push_back("QUIT");

    // Draw options
    int startY = screenHeight / 2 + 50;
    for (size_t i = 0; i < options.size(); i++) {
        Color color = (int)i == selectedOption ? LIME : WHITE;
        std::string prefix = (int)i == selectedOption ? "> " : "  ";
        std::string text = prefix + options[i];

        int textWidth = MeasureText(text.c_str(), 32);
        DrawText(text.c_str(), (screenWidth - textWidth) / 2, startY + (i * 70), 32, color);
    }

    // Instructions
    DrawText("Use UP/DOWN arrow keys to navigate", (screenWidth - 350) / 2, screenHeight - 80, 14, GRAY);
    DrawText("Press ENTER to select", (screenWidth - 250) / 2, screenHeight - 50, 14, GRAY);
}

void MainMenu::drawNewGame() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));

    if (inputtingName) {
        // Input screen
        std::string title = "CREATE NEW GAME";
        int titleWidth = MeasureText(title.c_str(), 48);
        DrawText(title.c_str(), (screenWidth - titleWidth) / 2, 100, 48, LIME);

        std::string prompt = "Enter Your Adventurer Name:";
        int promptWidth = MeasureText(prompt.c_str(), 24);
        DrawText(prompt.c_str(), (screenWidth - promptWidth) / 2, screenHeight / 2 - 100, 24, WHITE);

        // Input box
        int boxX = screenWidth / 2 - 250;
        int boxY = screenHeight / 2 - 20;
        int boxWidth = 500;
        int boxHeight = 60;

        DrawRectangle(boxX, boxY, boxWidth, boxHeight, Color{50, 50, 50, 255});
        DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, LIME);

        // Name text
        DrawText(playerName.c_str(), boxX + 20, boxY + 15, 32, WHITE);

        // Blinking cursor
        int cursorX = boxX + 20 + MeasureText(playerName.c_str(), 32) + 5;
        if ((int)(GetTime() * 2) % 2 == 0) {
            DrawLine(cursorX, boxY + 10, cursorX, boxY + 50, LIME);
        }

        // Character limit indicator
        std::string charCount = std::to_string(playerName.length()) + "/20";
        int charCountWidth = MeasureText(charCount.c_str(), 14);
        DrawText(charCount.c_str(), (screenWidth - charCountWidth) / 2, screenHeight / 2 + 80, 14, GRAY);

        // Instructions
        DrawText("Type your name and press ENTER | BACKSPACE to delete | ESC to go back",
                50, screenHeight - 60, 14, YELLOW);
    } else {
        // Confirmation screen
        std::string text = "STARTING NEW GAME...";
        int textWidth = MeasureText(text.c_str(), 40);
        DrawText(text.c_str(), (screenWidth - textWidth) / 2, screenHeight / 2 - 50, 40, LIME);

        std::string playerText = "Player: " + playerName;
        int playerTextWidth = MeasureText(playerText.c_str(), 24);
        DrawText(playerText.c_str(), (screenWidth - playerTextWidth) / 2, screenHeight / 2 + 40, 24, WHITE);

        DrawText("Press ENTER to continue", (screenWidth - 270) / 2, screenHeight - 60, 16, YELLOW);
    }
}

void MainMenu::drawLoadGame() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));

    std::string title = "RESUME SAVED GAME";
    int titleWidth = MeasureText(title.c_str(), 48);
    DrawText(title.c_str(), (screenWidth - titleWidth) / 2, 100, 48, SKYBLUE);

    std::string loading = "Loading your adventure...";
    int loadingWidth = MeasureText(loading.c_str(), 24);
    DrawText(loading.c_str(), (screenWidth - loadingWidth) / 2, screenHeight / 2 - 50, 24, WHITE);

    // Loading animation
    int dots = ((int)(GetTime() * 2)) % 4;
    std::string dotStr = "";
    for (int i = 0; i < dots; i++) dotStr += ".";
    int dotsWidth = MeasureText(dotStr.c_str(), 24);
    DrawText(dotStr.c_str(), (screenWidth - dotsWidth) / 2, screenHeight / 2 + 50, 24, GRAY);

    DrawText("Press ENTER to continue | ESC to go back", 50, screenHeight - 60, 14, YELLOW);
}

void MainMenu::drawSettings() {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));

    int centerX = screenWidth / 2;
    int y = 100;
    int lineHeight = 80;

    // Title
    std::string title = "SETTINGS";
    int titleWidth = MeasureText(title.c_str(), 48);
    DrawText(title.c_str(), centerX - titleWidth / 2, y, 48, YELLOW);
    y += lineHeight;

    // Master Volume
    DrawText("Master Volume", 150, y, 24, WHITE);

    // Volume bar
    int barX = 500;
    int barY = y + 5;
    int barWidth = 300;
    int barHeight = 30;

    DrawRectangle(barX, barY, barWidth, barHeight, BLACK);
    DrawRectangleLines(barX, barY, barWidth, barHeight, GRAY);
    DrawRectangle(barX, barY, (int)(barWidth * masterVolume), barHeight, LIME);

    // Volume percentage
    std::string volText = std::to_string((int)(masterVolume * 100)) + "%";
    int volTextWidth = MeasureText(volText.c_str(), 20);
    DrawText(volText.c_str(), barX + barWidth + 30, barY + 5, 20, WHITE);

    y += lineHeight;

    // Graphics
    DrawText("Graphics Quality", 150, y, 24, WHITE);
    DrawText("HIGH", 500, y, 24, SKYBLUE);
    y += lineHeight;

    // Particles
    DrawText("Particle Effects", 150, y, 24, WHITE);
    DrawText("ON", 500, y, 24, GREEN);

    // Instructions
    DrawText("LEFT/RIGHT: Adjust Volume | ESC: Back to Menu", 50, screenHeight - 60, 14, YELLOW);
}
