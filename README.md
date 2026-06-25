# 🗡️ Dungeon Crawler v2.0

A fast-paced, top-down **C++ dungeon crawler** built with **Raylib**, featuring a tiered enemy system, a full spell arsenal, companion mechanics, procedural maps, and a rich visual effects pipeline.

---

## 🎮 Gameplay Overview

Battle through procedurally generated dungeon floors, leveling up your character, looting items, unlocking powerful spells, and even **taming a fallen god** to fight at your side. Every floor scales the enemy difficulty — pushing you to master your spells, inventory, and companions.

---

## ✨ Features

### ⚔️ Player Combat
- Melee attacks with **critical hits**, knockback, and weapon variety
- Full **attack slash animations** (regular & crit, direction-aware)
- **4 weapon types** — Wooden Sword, Iron Katana, Steel Dagger, Shuriken
- **Throwable items** — Shuriken, Throwing Knife, Magic Orb

### 🔮 Spell System
Spells are unlocked as you level up:

| Spell | Unlock | Description |
|---|---|---|
| **Fireball** | Level 8 | Launches a traveling fireball projectile |
| **Whirlwind** | Level 15 | Spinning blades AoE around the player |
| **Shadow Burst** | Level 20 | Expanding dark ring that damages all nearby enemies |
| **Blink Strike** | Tame Shadow Paladin | Teleport dash + explosive arrival slash |

> ⚡ Chain Lightning and Frost Nova are reserved for upper floors.

### 👹 Enemy Roster — Tiered Threat System
Enemies scale with player level and spawn progressively harder variants:

| Tier | Enemies |
|---|---|
| **D** (Common) | Goblin, Skeleton, Slime, Fire Hound, Bat, Fire/Dark/Light Spirit |
| **C** (Uncommon) | Chimera Ant, Werewolf, Cerberus, Cyclops, Minotaur, Stone Golem, Salamander Man, Honey Bee, Skeleton Hound |
| **B** (Rare) | Skeleton Knight, Elf Girl, Goblin Giant, Mage, Lava Golem, Imp, Ancient Mummy |
| **A** (Elite) | Red Orc, Witch, **Fallen Shadow Paladin**, Harpy Queen |
| **S** (Boss) | Necromancer, Dragon, Titan, Goblin Mama, Frost King, Abyssal Hydra |

Each enemy class has a **unique AI** (dashing, teleporting, stomping, multi-head attacks) and a **class-appropriate attack animation** (slash, magic orb, stomp, bite, shadow, or fire burst).

### 🧙 Companion System

#### Fallen Shadow Paladin
A god-tier companion unlocked by **taming the Fallen Shadow Paladin boss** after breaking him in combat. He is slightly nerfed after taming for balance, but remains a formidable ally:

- Follows the player in a **protective square zone**
- Actively hunts the **nearest enemy**
- Can be **summoned/unsummoned** at will (cooldown applies)
- **Takes damage** from enemies — he is mortal as a companion
- Triggers **purple slash VFX** on each attack
- Enemies will actively target him — drawing aggro away from you

#### Seeds of Evolution
A consumable item that spawns **3 Goblin-type specimens**:
- Agile, fast attackers — low health but high attack speed
- Attack the nearest enemy autonomously
- **Permanent death** — use another seed to summon more

### 🎒 Inventory & Items
- **Consumable Potions** — Health (+75 HP), Speed (+50% / 15s), Stealth (15s), Rage (×1.3 dmg / 12s), Mana, Holy Water of Life (+500 HP)
- **Equipment** — Shield Pendant, Ring of Fire, Amulet of Ice, Boots of Swiftness, Cloak of Invisibility, Paladin Necklace
- **Legendary Weapons** — Scorching Gauntlet (Lv 10+), Demon King Long Sword (post-Paladin kill), Venom Sword
- **Food** — Meat, Apple, Bread, Magical Fruit, Cheese
- **Currency** — Essence Stones (common), Orbs (rare)
- **Quest Items** — Ancient Key, Treasure Map, Mystical Rune
- **Seeds of Evolution** — Summon goblin specimens

### 🗺️ Procedural Dungeon
- Tile-based map: **80×40 tiles** @ 32px each
- Solid collision detection — no wall-clipping
- Floor progression: every **10 levels** advances to the next floor
- Dynamic **camera following** the player with shake effects on impact

### 💫 Visual Effects Pipeline
Powered by `EffectSystem` — a centralized VFX manager with 20+ distinct effects:
- Player attack slashes (melee arc, gold crit version)
- Spell visuals — Fireball projectile, Frost Nova ring, Lightning bolt, Whirlwind blades, Shadow Burst ring
- Blink Strike — ghost trail at origin + explosive slash at destination
- Enemy windups (red pulse telegraph) + hit impact flashes
- Per-class enemy attack animations (slash, magic, stomp, bite, shadow, fire)
- Level-up gold burst, enemy death explosions
- **Companion slash** — direction-aware purple arc when the Paladin attacks

### 💾 Save System
- JSON-based save/load with automatic backups (`saves/savegame.json`)
- Persists player stats, inventory, level, and floor progress

---

## 🕹️ Controls

| Action | Key |
|---|---|
| Move | `W A S D` |
| Attack | `Left Mouse Button` |
| Fireball | `Q` |
| Whirlwind | `E` |
| Shadow Burst | `R` |
| Blink Strike | `F` |
| Summon / Unsummon Paladin | `P` |
| Use Item (Hotbar) | `1–4` |
| Open Inventory | `I` or `Tab` |
| Pause | `Esc` |

---

## 🛠️ Building & Running

### Prerequisites
- **C++17** or later
- **CMake 3.16+**
- **Raylib** (installed system-wide or bundled)
- Linux / Windows / macOS

### Clone & Build
```bash
git clone https://github.com/swaraj235/Dungeon-Crawler-2.git
cd Dungeon-Crawler-2

mkdir build && cd build
cmake ..
make -j4

# Run
./bin/DungeonCrawler2
```

---

## 📁 Project Structure

```
Dungeon-Crawler-2/
├── include/
│   ├── Core/              # Game loop, Config constants
│   ├── Entities/          # Character, Player, Enemy (all tiers)
│   ├── Systems/           # All system headers
│   ├── Audio/             # Audio manager
│   └── UI/                # HUD, menus
├── src/
│   ├── Core/              # Game.cpp — main update/render loop
│   ├── Entities/          # Player.cpp, Enemy.cpp (all AI implementations)
│   ├── Systems/           # EffectSystem, CompanionSystem, ItemSystem, etc.
│   ├── Audio/             # AudioManager.cpp
│   └── UI/                # HUD.cpp
├── assets/
│   ├── sprite/            # 30+ character & enemy sprites
│   └── sounds/            # SFX and background music
├── saves/                 # JSON save files
├── CMakeLists.txt
└── main.cpp
```

---

## 🧱 Architecture

The game uses a **component-system architecture** built with OOP:

- **`Character`** — Base class for all living entities (health, level, position, damage)
- **`Player`** — Extends `Character` with input handling, spells, inventory, and buffs
- **`Enemy`** — Extends `Character` with a 4-state AI (`IDLE → WANDERING → CHASING → ATTACKING`) and dynamic target selection (player or companion)
- **`CompanionSystem`** — Manages all active companions, their AI, follow behavior, and combat
- **`EffectSystem`** — Centralized VFX trigger and renderer for all in-game effects
- **`MapGenerator`** — Procedural dungeon tile generation with collision maps
- **`ItemSystem`** / **`PotionSystem`** / **`WeaponSystem`** — Modular item and combat logic
- **`CombatSystem`** — Shared hit detection and damage resolution
- **`ParticleSystem`** — Lightweight particle emitter for ambient and combat FX

---

## 📌 Roadmap

- [ ] Chain Lightning & Frost Nova (upper floors)
- [ ] Spell combo system (mixing two spells for hybrid effects)
- [ ] More boss encounters per floor
- [ ] Companion leveling system
- [ ] Additional Seeds of Evolution creature types
- [ ] Web dashboard integration

---

## 🧑‍💻 Author

**Swaraj** — Built as a C++ learning project and progressively expanded into a full game.
