# Blocky: Scratch-like Visual Programming Environment

A desktop application implementing a visual programming environment similar to Scratch, built entirely in **C++** using the **SDL2** library. This project serves as a final project for the Fundamentals of Programming course, demonstrating object-oriented design, event-driven programming, and graphics rendering.

## 🧑‍💻 Team & Credits
*   **Group Number:** 1
*   **Team Members:**
    *   Omid Ziveh
    *   Radin Golmohammadi

## 🚀 Project Overview

The application allows users to drag and drop visual blocks to create programs. It features a runtime engine that interprets these blocks to animate sprites on a stage, supporting motion, looks, sound, control flow, and sensing operations.

### Key Features
*   **Visual Block Programming:** Drag-and-drop interface with snapping logic for connecting blocks.
*   **Sprite Management:** Support for multiple sprites, costumes, and properties (position, size, visibility).
*   **Runtime Engine:** A robust execution engine supporting loops, conditionals, and concurrent scripts.
*   **Debugger Tools:** Includes a "Black Box" system logger, step-by-step execution, and a "Watchdog" for infinite loop detection.
*   **Asset Management:** Built-in costume editor and sound manager.

## 🛠️ Technical Stack & Dependencies

This project is written in **C++** and requires the following libraries to build:

*   **SDL2:** Core windowing and rendering.
*   **SDL2_image:** Image loading (PNG/JPG) for sprites and backgrounds.
*   **SDL2_ttf:** TrueType font rendering for UI text and code blocks.
*   **SDL2_mixer:** Audio playback and management.
*   **SDL2_gfx:** Graphics primitives (circles, rounded boxes) for UI and Pen extension.

## 📂 Project Architecture

The codebase is modularized into several key directories:

*   **`src/frontend/`**: Handles all rendering, UI layout, and user input.
    *   `draw.cpp` / `input.cpp`: Core rendering and event handling loops.
    *   `block_utils.cpp`: Block definitions, labels, and colors.
    *   `palette.cpp`: The block category sidebar.
    *   `menus/`: UI panels for backgrounds, sprites, sounds, and costumes.
*   **`src/backend/`**: The logical core of the application.
    *   `runtime.cpp`: The execution engine that traverses the Abstract Syntax Tree (AST) of blocks.
    *   `block_executor_*.cpp`: Handlers for specific block categories (Motion, Looks, Sound, Sensing).
    *   `file_io.cpp`: Save/Load project functionality.
    *   `logic.cpp`: Block connection and validation logic.
*   **`src/common/`**: Shared definitions, structs (`Block`, `Sprite`, `Runtime`), and constants.
*   **`src/utils/`**: Helper utilities like the Logger and System Black Box.

## 🎮 User Guide

### Controls
*   **Mouse:** Drag blocks from the palette to the coding area. Snap blocks together to build scripts.
*   **Green Flag (Play Button):** Starts execution of all scripts beginning with a "Start" block.
*   **Stop Sign:** Stops all active scripts.
*   **Pause:** Click the Play button while running to pause execution.
*   **F12:** Toggle **Step-by-Step Mode**. Press **Space** to execute the next step.
*   **L Key:** Toggle the **System Logger (Black Box)** visibility.
*   **E Key:** Open the **Costume Editor** for the current sprite.

### Menus
1.  **File Menu:**
    *   **New:** Resets the project (with confirmation).
    *   **Save:** Saves the current project to `project.scratch`.
    *   **Load:** Loads a project from file.
2.  **Help Menu:**
    *   **System Logger:** View the execution history of blocks.
    *   **Debug Info:** Displays current coordinates and runtime stats.
3.  **Coding Area:**
    *   Click on argument boxes (white rectangles) to type values.
    *   Drag reporter blocks (circles) into argument slots.

## 📝 Implemented Functionality

### 1. Code Blocks (Mandatory & Optional)
| Category | Implemented Features |
| :--- | :--- |
| **Motion** | Move steps, Turn, Go to (x/y, random, mouse), Change x/y, Bounce on edge. |
| **Looks** | Say/Think, Switch Costume, Next Costume, Set/Change Size, Show/Hide. |
| **Sound** | Play sound, Stop all sounds, Set/Change Volume. |
| **Events** | When Green Flag clicked, When Key pressed, When Sprite clicked. |
| **Control** | Repeat, Forever, If, Wait, Stop All. |
| **Sensing** | Touching (Mouse/Edge), Distance to Mouse, Mouse X/Y/Down, Timer, Reset Timer. |
| **Operators** | Math (Add, Sub, Mul, Div, Mod, Abs, Sqrt, Sin, Cos...), Logic (And, Or, Not), String (Length, Letter, Join). |
| **Variables** | Set Variable, Change Variable (support for creation and display). |
| **Custom Blocks** | Define Block, Call Block (with parameters and scope management). |
| **Pen Extension** | Pen Up/Down, Clear, Stamp, Set Color, Set Size. |

### 2. System & Debugging
*   **Watchdog Timer:** Automatically detects and stops infinite loops that freeze the application.
*   **System Black Box:** Logs the last executed commands, helpful for debugging complex scripts.
*   **Save/Load:** Serializes block positions, arguments, and sprite properties into a readable text format.

### 3. UI Features
*   **Costume Editor:** A built-in paint tool (Pencil, Eraser, Lines, Rectangles, Circle, Fill) to edit sprite costumes.
*   **Sound Manager:** UI to manage project sounds (add from library, play, stop).
*   **Sprite Panel:** Manage multiple sprites (add, delete, toggle visibility).

## 🐛 Known Limitations
While the project covers the majority of requirements, the following are identified limitations based on the code analysis:
*   **Looks Blocks:** "Switch Backdrop" and "Layer" (Go to Front/Back) blocks are not fully implemented.
*   **Sensing:** The "Ask and Wait" block is not implemented.
*   **Pen Extension:** Brightness and Saturation sliders for the pen color are missing (only RGB selection is available).
*   **Custom Blocks:** Full UI interface for defining parameters visually is limited; currently parsed via block arguments.

---
*This project was developed as part of the curriculum for the Fundamentals of Programming course.*
