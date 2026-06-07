# Survivor AI — Zombie Survival Behavior Tree (Unreal Engine 5)

A fully autonomous survivor AI built in **Unreal Engine 5** using a **Behavior Tree** architecture. The agent scavenges houses, manages its inventory, explores the map using a heatmap system, and flees from zombies using predictive movement — all while trying to complete its current objective.

---

## Features

### Decision Making — Behavior Tree
The agent uses a priority-based Behavior Tree with 4 branches evaluated left to right:

| Priority | Branch | Condition |
|---|---|---|
| 1 | **Attack** | Zombie visible + has weapon with ammo |
| 2 | **Looking for Items** | Needs an item + inventory not full |
| 3 | **Scavenging Houses** | Village in perceptor memory |
| 4 | **Exploring The Map** | Fallback — always runs |

Two services run continuously at the root:
- `BTS_MonitorStatus` — manages inventory, item usage, and need evaluation
- `BTS_Movement` — context steering with zombie repulsion and flanking

---

### Inventory Management
Handled by `BTS_MonitorStatus` (ticks every 0.5s):

- **Auto item use** — medkit when health ≤ 6, food when stamina < 3
- **Weapon cleanup** — empty pistols/shotguns removed automatically
- **Priority evaluation** — dynamically sets `NeededItemType` in the blackboard:
  1. Pistol (no pistol at all)
  2. Medkit (low health, no medkit)
  3. Food (low stamina, no food)
  4. Shotgun (normal priority)
  5. Shotgun override — if zombie visible and no shotgun ammo, elevates to highest need
- **`BTT_LookupItem`** — searches perceptor memory for closest remembered item matching the current need
- **`BTT_PickupItem`** — navigates to item, picks up on arrival, forgets it from perceptor memory

---

### Enemy Handling

**Fleeing (`BTT_Flee`):**
- Predicts zombie position using velocity extrapolation (`PredictZombiePosition`)
- Tactically flees *toward* the current objective (item / scavenge point / explore point) if not blocked
- Checks if zombie is obstructing the path using dot product — if blocked, calculates a flank direction
- Falls back to pure away direction if no tactical target exists
- Clears zombie from blackboard and stops sprinting once safe distance is reached

**Shooting (`BTT_Attack`):**
- Proximity check before firing
- Selects first weapon with remaining ammo (pistol preferred over shotgun)
- Rotates agent toward zombie before firing

**Context Steering (`BTS_Movement`):**
- Calculates repulsion force from all nearby zombies within `DangerRadius`
- If a tactical target exists, blends repulsion with a flank direction using `CrossProduct` to steer around zombies rather than purely away
- If no repulsion and no target, adds a spiral offset (`sin/cos`) to avoid walking in straight lines
- Velocity smoothed with `VInterpTo` and clamped to `MaxWalkSpeed`

---

### Exploration — Heatmap System (`AC_HeatMap`)
- Divides the world into a grid of cells (`CellSize = 200` world units)
- Tracks how many times each cell has been visited using a `TMap<FVector2D, int>`
- Only registers a new cell when the agent crosses a cell boundary (`LastRegisteredGrid` check)
- `FindNearestUnexploredLocation` — scans all cells within `MaxSearchRadius`, collects unvisited candidates, picks one **randomly** to avoid always exploring in the same direction
- `BTT_FindExplorePoint` — keeps existing target if agent hasn't arrived yet, only recalculates on arrival

---

### Scavenging Houses
- `BTT_PickNextHouse` — pops the next house from perceptor village memory
- `BTT_FindScavenge` — projects the house center onto the NavMesh to find a valid scavenge point
- `BTT_ScavengeScan` — performs a full 360° rotation to trigger perception and register nearby items
- A root-level service (`BTS_CheckVillage`) continuously checks if new houses are in perceptor memory and sets `HasVillageTarget` in the blackboard, allowing the scavenge branch to interrupt exploration immediately

---

## Architecture Overview

```
ROOT
├── [Service] Monitor Status & Inventory   (tick 0.5s)
├── [Service] Steering Movement            (tick 0.5s)
└── Selector
    ├── Attack Branch
    │   ├── [Dec] Has Enemy?
    │   ├── [Dec] Has Weapon?
    │   └── Attack Task
    ├── Looking for Items Branch
    │   ├── [Dec] Needs Item?
    │   ├── [Dec] Inventory Not Full?
    │   └── Memory Selector
    │       ├── No location → Find Item In Memory
    │       └── Has location → Pickup Item
    │       └── [Move] Saw Zombie? → Flee | No Zombie → Move To
    ├── Scavenging Houses Branch
    │   ├── [Dec] Sees Village?
    │   └── Scavenge Selector
    │       ├── No location → Pick House → Find Scavenge Point
    │       └── Has location → 360° Scan
    │       └── [Move] Saw Zombie? → Flee | No Zombie → Move To
    └── Exploring The Map Branch
        └── Explore Sequence
            ├── Find Heatmap Exploration Point
            └── [Move] Saw Zombie? → Flee | No Zombie → Move To
```

---

## File Overview

| File | Description |
|---|---|
| `AC_HeatMap_OlivierStan` | Heatmap component — tracks visited cells, finds unexplored targets |
| `BTS_MonitorStatus_OlivierStan` | Service — inventory management, item usage, need evaluation |
| `BTS_Movement_OlivierStan` | Service — context steering with zombie repulsion and flanking |
| `BTT_Attack_OlivierStan` | Task — weapon selection and firing at zombie |
| `BTT_Flee_OlivierStan` | Task — predictive flee with tactical target routing |
| `BTT_FindExplorePoint_OlivierStan` | Task — heatmap-based exploration point selection |
| `BTT_FindScavenge_OlivierStan` | Task — NavMesh projection inside house bounds |
| `BTT_LookupItem_OlivierStan` | Task — perceptor memory search for needed item |
| `BTT_PickNextHouse_OlivierStan` | Task — pops next house from village memory |
| `BTT_PickupItem_OlivierStan` | Task — navigate to and pick up item |
| `BTT_ScavengeScan_OlivierStan` | Task — 360° rotation scan at scavenge location |
| `BTT_Wander_OlivierStan` | Task — circle-based wander when no other target |

---

## Built With
- Unreal Engine 5
- C++
- Behavior Tree / Blackboard system
- UE Navigation System (NavMesh)
- AI Perception (via StudentPerceptor framework)
