# 🏓 Pong Game with Dear ImGui

A classic 2-player Pong game built in C++ utilizing **SDL3** for window management and input handling, **OpenGL3** for hardware-accelerated rendering, and **Dear ImGui** for the graphical user interface and game state overlays.

---

## ✨ Features

* **Classic Physics & Gameplay**: Snappy collision detection for ball bouncing and scoring mechanics.
* **Dynamic UI States**: Seamlessly transitions between menus and gameplay phases:
    * *Start Screen*: Clear overlay instructing the player to press Space to play.
    * *Match Point Detection*: Custom win/loss result screen triggered instantly when a player hits 10 points.
* **Clean Implementation**: ImGui core files and backends integrated cleanly into a single flat source folder architecture.

---

## 🚀 Getting Started

### Prerequisites

To compile and run this project, you will need:
* **Visual Studio** (Community or Professional) with the *Desktop development with C++* workload installed.
* **SDL3 Development Libraries**: Set up on your system and linked correctly in your project properties.
* **OpenGL**: Pre-installed natively with modern graphics drivers.

### Setup & Installation

1. **Clone the Repository**:
   `git clone https://github.com/YOUR_USERNAME/YOUR_REPOSITORY_NAME.git`

2. **Open the Project**:
   * Navigate to the root directory and double-click `ponggame.sln` to launch it in Visual Studio.

3. **Verify Project Tree**:
   Ensure the following essential ImGui assets are included under your `Source Files` filter in the Solution Explorer:
   * `imgui.cpp`, `imgui_demo.cpp`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`
   * `imgui_impl_sdl3.cpp`, `imgui_impl_opengl3.cpp`

4. **Build & Run**:
   * Set your build configuration drop-down to **x64**.
   * Go to **Build** -> **Rebuild Solution**.
   * Press **F5** to launch the game!

---

## 🎮 How to Play

* **Spacebar**: Starts the game from the main menu or resets after a match ends.
* **Player 1 (Left)**: Controls paddle up and down.
* **Player 2 (Right)**: Controls paddle up and down.
* **Win Condition**: First player to score **10 points** wins the match!

---

## 🛠️ Built With

* [Dear ImGui](https://github.com/ocornut/imgui) - Bloat-free immediate mode graphical user interface for C++.
* [SDL3](https://github.com/libsdl-org/SDL) - Simple DirectMedia Layer for cross-platform platform abstraction.
* [OpenGL](https://www.opengl.org/) - Industry standard graphics pipeline.
* [GLAD](https://glad.dav1d.de/) - Multi-Language GL Loader-Generator for modern OpenGL.