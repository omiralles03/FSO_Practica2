# Tron ASCII Racer: Multiprocess “Tron” in C

> A lightweight terminal-based “Tron” clone implemented in C to explore multi-process programming, shared memory, semaphores, and inter-process synchronization—all within a Unix `curses` interface.

---

## 🚀 Project Overview

This project walks through four progressive phases of implementing a multi-player “Tron” game in C:

1. **Phase 0**: Single-process, sequential game engine (`tron0.c`)  
2. **Phase 1**: Forked processes for user and opponent(s) with independent execution (`tron1.c`)  
3. **Phase 2**: Shared memory & semaphores for safe concurrent access to game state and the log file (`tron2.c`)  
4. **Phase 3**: Decoupled executables—separate binaries for controller (`tron3`) and opponent agents (`oponent3`) communicating via `execlp` & a shared curses buffer  

By the end, you have a robust, terminal-driven game engine that seamlessly coordinates multiple processes drawing on a shared screen buffer, synchronizing via POSIX semaphores, and logging gameplay events in real-time.

---

## 📁 Repository Structure

---

## 🛠️ Building & Running

1. **Clone**  
   ```bash
   git clone https://github.com/omiralles03/FSO_Practica2.git
   cd FSO_Practica2
    ```

2. **Building**
   ```bash
   make         # Compiles everything.
   ```

   ```bash
   make run     # Compiles everything and executes with test parameters.
   ```

   ```bash
   make clean   # Removes all compiled files.
   ```

---

## 📝 Usage Examples

---
