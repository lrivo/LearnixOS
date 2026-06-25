# Init (PID 1)

Init is the **first userspace process** that the kernel spawns, that's why it always gets PID 1.
It has 3 foundamental jobs:
- **service initialization** -> spawning essential services like `NetworkManager` and the DNS resolver
- **zombie reaping**
- **never dying**

## Learnix's implementation
For now it is extermely simple:
- it spawns the shell
- it forever calls `wait()` to reap zombie processes (which `exit()` reparents to him if their original parent dies before them)
