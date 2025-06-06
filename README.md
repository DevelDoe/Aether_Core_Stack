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