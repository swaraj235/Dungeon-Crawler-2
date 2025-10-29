# DungeonCrawler2

A C++ Dungeon Crawler game built using **Raylib** and **Object-Oriented Programming (OOP)** principles.  
The project demonstrates structured game development using **entities**, **systems**, and **components**, along with a modular design for extensibility.

---

## Features

- **Player & Enemies**
  - Player class with health, experience, and inventory
  - Enemies like Goblins, Skeletons, Hounds, and Orcs

- **Game Systems**
  - `EffectSystem` – Handles effects such as healing, buffs, or damage over time  
  - `ItemSystem` – Manages pickup, usage, and interaction of items  
  - `PotionSystem` – Implements healing and magical potion logic  
  - (Upcoming) `MagicalItemSystem` – Adds logic for magical items (damage boost, shield, speed, etc.)

- **Dungeon Map**
  - Procedurally or manually designed tile-based dungeon  
  - Collision detection and movement

- **Raylib Rendering**
  - 2D rendering for characters, enemies, and environments  
  - Texture-based sprites and animations

---

## Concepts Used

- **Object-Oriented Programming (OOP)**  
  - Classes: `Player`, `Enemy`, `Item`, `Character`, etc.  
  - Inheritance, Polymorphism, and Encapsulation

- **Game Loop Design**
  - Update → Render → Input Cycle
  - Real-time state management of entities

- **System Architecture**
  - Systems like `EffectSystem` and `PotionSystem` handle reusable logic

---

## Dependancies

- **C++17 or later**
- **Raylib** installed
- **CMake 3.16+**
- Compatible with **Windows**, **Linux**, or **macOS**

--

## How to Build & Run

### Clone the Repository
```bash
git clone https://github.com/swaraj235/Dungeon-Crawler.git
cd Dungeon-Crawler
```
--

## Folder Structure
<br>
DungeonCrawler2/<br>
├── include/           # Header files<br>
│   ├── Entities/      # Player, Enemy, Character<br>
│   ├── Systems/       # EffectSystem, PotionSystem, etc.<br>
│   └── ...<br>
├── src/               # Source code (.cpp files)<br>
├── assets/            # Sprites, textures, sounds<br>
├── CMakeLists.txt     # Build configuration<br>
└── main.cpp           # Game entry point<br>








