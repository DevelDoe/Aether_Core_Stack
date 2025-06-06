# Aether Core Stack

> Self-healing, resource-aware microservice guardian system — written in C.

### 💠 Components

| Agent     | Role                                                     |
|-----------|----------------------------------------------------------|
| **cssh**      | Central hub server: relays, monitors, escalates, commands agents |
| **seer**      | Reports system + service health (CPU, MEM, NET, etc.)  |
| **sentinel**  | Monitors local services, restarts if needed            |
| **reaper**    | Failsafe hard-reboot agent                             |

### 📂 Structure
The `agents` directory contains a subfolder for each daemon. `agents/_skelaton` serves as a minimal starting point with a `Makefile` and example source files.

- **cssh** central services socket hub. Relays logs and health reports to admin clients and escalates issues if services fail to respond. Sends reboot command to reaper if needed, restart command to sentinel and relays commands from admin clients.
- **seer** reports system and service health statistics.
- **sentinel** restarts local services when they misbehave.
- **reaper** triggers a hard reboot as a last resort.


### 🛠 Build
Run `make` inside `agents/_skelaton` to build the example agent.
