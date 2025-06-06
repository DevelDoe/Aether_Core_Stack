# Aether Core Stack

> Self-healing, resource-aware microservice guardian system — written in C.

### 💠 Components

| Agent     | Role                                                     |
|-----------|----------------------------------------------------------|
| **seer**      | Reports system + service health (CPU, MEM, NET, etc.)  |
| **sentinel**  | Monitors local services, restarts if needed            |
| **cssh**      | Central hub client: relays reports, escalates issues   |
| **reaper**    | Failsafe hard-reboot agent                             |

### 📂 Structure
The `agents` directory contains a subfolder for each daemon. `agents/_skelaton` serves as a minimal starting point with a `Makefile` and example source files.

- **seer** reports system and service health statistics.
- **sentinel** restarts local services when they misbehave.
- **reaper** triggers a hard reboot as a last resort.
- **cssh** relays information to the central hub.

### 🛠 Build
Run `make` inside `agents/_skelaton` to build the example agent.

