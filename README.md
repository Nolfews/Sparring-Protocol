# Sparring Protocol

> A arena duel against an AI that gets stronger every round. How many rounds can you survive?

Built with **Unreal Engine 5.8** by a team of 3 developers in **1 month**.

---

## Concept

The player fights an AI in an enclosed arena, one on one. The goal is simple: **win as many rounds in a row as possible**.

After each round won:

- the AI gets stronger (run speed, jump height, fire rate, damage, accuracy, reaction time)
- the player can reshape the arena by placing new elements (walls, and later blocks, ramps, cover)
- the AI also places elements on the map

Difficulty ramps up gradually: the first rounds are accessible, then the AI eventually becomes too strong to beat. The run ends at the first defeat and the score is saved.

## Game Flow

```
Tutorial round (empty arena, passive AI)
   -> Building tutorial
      -> Round 1 -> Build phase -> Round 2 -> Build phase -> ...
         -> Defeat: run ends, score saved, back to menu
```

1. **Tutorial round**: the AI does not fight back. The player learns the mechanics (movement, jump, dash, shooting).
2. **Build phase**: the player places elements on a grid. A guided tutorial walks them through it the first time.
3. **Combat rounds**: duel against the AI, whose stats increase each round.
4. **Defeat**: the number of consecutive rounds won is saved to the leaderboard (local top 10). After the first run, tutorials are skipped.

## Building System

The arena is based on a **block grid**. The player places elements while respecting the grid, with validation rules:

- the element must be inside the arena bounds
- the target cells must be free
- no placing too close to the spawn points
- a path must always exist between the two spawns (the AI must never be locked in)
- enough slots must remain

### Element Cap

The system uses a **cap on elements existing at the same time**:

```
MaxAllowed   = min(StartCount + RoundsWon * GrowthPerRound, HardCap)
SlotsToPlace = MaxAllowed - ElementsOnMap
```

Starting values: 3 elements at the beginning, +1 per round won, capped at 10. These values live in a Data Asset and can be tuned without recompiling.

### Planned Extensions

- elements destroyed by gunfire (player and AI)
- limited lifetime for elements (they break after N rounds)
- removing and editing elements during the build phase
- verticality: stacking blocks, ramps
- walls with slits to shoot from cover

## AI

The AI does not use machine learning. It relies on a **Behavior Tree** (with EQS and NavMesh) whose **parameters increase every round** through balancing curves:

- some stats plateau (accuracy, reaction time) so the AI stays "human-like"
- others keep rising slowly (damage, fire rate)
- between rounds, the AI places elements on valid slots, with some randomness

The goal is for early rounds to stay winnable, with a difficulty ramp that is clearly visible but never brutal.

## Technical Architecture

- **C++** for all gameplay logic: components (health, fire, dash, stats), round manager, grid and placement systems
- **Blueprints** for derived assets (meshes, effects, sounds), UI (UMG) and tuning
- **Data Assets / Data Tables** for balancing (per-round stats, arena rules, element definitions)
- Systems communicate through **delegates and interfaces** to stay decoupled

### Shared Character Base Class

```
ACombatCharacter
 ├─ APlayerCombatCharacter  (player input)
 └─ AEnemyCombatCharacter   (Behavior Tree)
```

The player and the AI share the same components, so each ability is only written once.

### Arena Module Layout

```
Source/SparringProtocol/
├── Arena/       (grid, pathfinding, ArenaManager, rules, spawns)
├── Placement/   (definitions, elements, validation, ghost, player component)
└── Tests/       (automation tests)
```

## Team Responsibilities

| Area | Scope |
|---|---|
| Player and combat | Character base class, shooting, health, dash, stats |
| AI | Behavior Tree, EQS, navigation, per-round stats |
| Terrain and building | Grid, placement, validation, elements, dynamic NavMesh |
| Rounds, UI, score | Round manager, HUD, menus, score saving (shared depending on progress) |

## Planned Schedule

| Week | Goals |
|---|---|
| 1 | Player controls, shooting, health, base arena, tutorial round, interface contracts |
| 2 | Basic AI, round system, single wall placement, dynamic NavMesh |
| 3 | Full validation, AI placement, element destruction |
| 4 | Balancing, UI, sound, polish, bug fixing |

Bonus features (editing, verticality, slit walls) are the first to be cut if the schedule slips.

## Conventions

### Language

Everything in the project is written in **English**: code, comments, asset names, commit messages and documentation.

### Asset Prefixes

| Prefix | Type |
|---|---|
| `BP_` | Blueprint |
| `DA_` | Data Asset |
| `WBP_` | Widget Blueprint |
| `M_` / `MI_` | Material / Material Instance |
| `SM_` | Static Mesh |
| `NS_` | Niagara System |
| `IA_` / `IMC_` | Input Action / Input Mapping Context |
| `L_` | Level |

### Working Rules

- Only one developer edits a given Blueprint, Data Asset or map at a time
- One content folder per domain to limit conflicts
- No logic in child Blueprints: references only (mesh, material, Data Asset)
- Shared headers (contracts between devs) are not changed without telling the team
- Do not rename a C++ variable used by Blueprints without a redirect
- Compile C++ with the editor closed

## Setup

1. Install **Unreal Engine 5.8**
2. Clone the repository:
   ```
   git clone <repository-url>
   ```
3. Right-click the `.uproject` file and choose **Generate Visual Studio project files**
4. Open the solution and build (`Development Editor` configuration)
5. Launch the project from the editor

## Team

- Dev 1: Nolan Papa
- Dev 2: Samuel Bleau
- Dev 3: Gabriel Villemonte
