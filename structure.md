# Codebase Structure

## Core Directories and Files

### `src/` - Main Source Code

#### `main.cpp`
- **What it does**: Entry point of the application
- **Key functions**:
  - `main()`: Initializes SFML window, ImGui interface, and main simulation loop
  - `drawGridlines()`: Renders the background grid (1m and 5m lines)
- **Handles**:
  - Window creation and event processing
  - Rendering loop (graphics, UI, path traces)
  - Physics updates through the World object
  - Tool and property panel rendering

#### `Config.hpp`
- **What it does**: Central configuration file with all constants
- **Contains**:
  - Window and world dimensions (4000x2250 pixels world, 800x450 default window)
  - Physics limits (gravity, density, force)
  - Graphics settings (colors, grid spacing)
  - Path trace settings (point radius, colors, fade time limits: 0.1-10.0s default 2.0s)
  - UI spacing and tool properties
  - Default solver (RK4) and calculation frequency (240 Hz)

---

### `src/core/` - Core Simulation Logic

#### `World.hpp / World.cpp`
- **What it does**: Container and manager for the entire simulation
- **Responsibilities**:
  - Maintains lists of all objects and connectors in the scene
  - Tracks simulation time and physics parameters (gravity, air density)
  - Calls physics updates on all objects each frame
  - Manages object/connector creation and deletion
  - Handles scene initialization and reset
  - Contains test scene setup for benchmarking solvers
- **Key methods**:
  - `update(float dt)`: Updates physics for all objects
  - `addObject()` / `removeObject()`: Manage objects
  - `addConnector()` / `removeConnector()`: Manage connectors
  - `loadTestScene()`: Creates a test scenario with spring

#### `Tools.hpp / Tools.cpp`
- **What it does**: User interaction tools for manipulating objects
- **Tool types**:
  - Push/Pool: Apply forces to objects
  - Selection: Select and grab objects
  - Creation: Create new objects
- **Responsibilities**:
  - Process mouse input
  - Apply forces or create objects based on user actions

#### `UI.hpp`
- **What it does**: ImGui interface rendering
- **Panels**:
  - Settings panel: Gravity, air density, solver selection, time step
  - Properties panel: Object-specific properties (mass, velocity, forces, **path trace controls**)
  - Tools panel: Selection and activation of tools
- **Features**:
  - Graph selection buttons for tracked quantities
  - **Path trace controls**: Enable/disable toggle, fade time slider (0.1-10.0s), clear trace button
  - Pause/play controls
  - Save/load options

---

### `src/engine/` - Physics Calculations

#### `ODE.hpp / ODE.cpp`
- **What it does**: Implements 8 different numerical integration methods
- **Solver types**:
  1. **Euler**: Simple first-order method (least accurate)
  2. **EulerS**: Semi-implicit Euler (better for energy)
  3. **RK2**: Runge-Kutta 2nd order
  4. **RK4**: Runge-Kutta 4th order (default, good balance)
  5. **DOPRI5**: Dormand-Prince 5th order (adaptive)
  6. **Verlet**: Velocity Verlet (specifically for mechanics)
  7. **AB**: Adams-Bashforth 4th order (multistep)
  8. **ABM**: Adams-Bashforth-Moulton 4th order (multistep)
- **How it works**:
  - Each solver takes current object state (position, velocity)
  - Calculates next state based on forces and time step
  - Returns updated position and velocity
- **Base class**: `ODESolver` - inherited by all solver implementations
- **Usage**: World calls the solver's `step()` method each frame

#### `Collision.hpp`
- **What it does**: Collision detection and response
- **Features**:
  - Impulse-based collision resolution
  - Friction modeling during collisions
  - Restitution (bounce) calculation
- **Checks**: Object-to-object and object-to-wall collisions

---

### `src/objects/` - Physical Objects and Forces

#### `Object.hpp / Object.cpp`
- **What it does**: Represents a physical object in the world (circle or rectangle)
- **Components**:
  - `Body`: Physical state (position, velocity, mass, acceleration)
  - `Shape`: SFML drawable shape for rendering
  - `PathTrace`: Historical position data with fade timing
- **Properties**:
  - Position, velocity, acceleration (Vec2)
  - Mass, density, dimensions
  - Can be static (immovable) or dynamic
  - Can apply forces (gravity, drag, friction)
  - **Path Tracing**:
    - `pathTraceEnabled`: Toggle to enable/disable path visualization
    - `pathTraceFadeTime`: Duration (seconds) before trace points fade and disappear (0.1-10.0s, default 2.0s)
    - `pathTrace`: Deque of timestamped positions recorded at regular intervals
- **Methods**:
  - `applyForce()`: Add a force source to the object
  - `update()`: Step physics forward and update path trace
  - `updatePathTrace()`: Records new trace points and ages existing ones
  - `drawPathTrace()`: Renders the path trace with fading color gradient
  - `clearPathTrace()`: Clears all recorded path points
  - Getters for body properties (position, velocity, kinetic energy, etc.)
- **Path Trace Features**:
  - Records position every 0.05 seconds when enabled
  - Points fade from bright blue to dim blue over the configured fade time
  - Automatically removes points that exceed fade time
  - Useful for visualizing object trajectories and motion patterns

#### `Body.hpp`
- **What it does**: Stores the pure physics state of an object
- **State variables**:
  - Position (x, y in world coordinates)
  - Velocity (vx, vy)
  - Acceleration (ax, ay)
  - Mass, rotation, angular velocity
- **Used by**: ODE solvers to compute next state

#### `Force.hpp`
- **What it does**: Different types of forces that can affect objects
- **Force types**:
  - Gravity: Downward acceleration
  - Drag: Proportional to velocity (air resistance)
  - Fiction: During collisions
  - Applied force: From user tools
- **Structure**: Each force has name, magnitude, and application method

#### `Connector.hpp / Connector.cpp`
- **What it does**: Links between two objects that apply forces
- **Connector types**:
  - **Spring**: Hooke's law force (F = -k*x - c*v)
    - Properties: stiffness, damping, resting length
    - Pulls and pushes objects to maintain distance
  - **Rope**: Provides tension-only constraint (can only pull, not push)
    - Prevents objects from moving farther apart than `totalLength`
    - Uses constraint-based penalty method with velocity damping
    - Reduces wobbling and instability through careful damping
    - Properties: total length, damping factor
  - **Strut**: Rigid rod constraint (can both push and compress)
    - Enforces a fixed distance between attachment points
    - Can represent rigid structural members
    - Uses moderate stiffness with critical damping to prevent oscillations
    - Properties: fixed length, stiffness, damping factor
- **Force Calculation**:
  - Base class calculates distance and direction between anchors
  - Each type applies constraint forces based on violation and relative velocity
  - Damping term proportional to velocity prevents rapid oscillations
- **Improvements in Rope/Strut**:
  - Replaced high-stiffness spring (1000 N/m) with moderate constraint stiffness (50-40 N/m)
  - Added velocity projection along connector direction for stability
  - Critical damping prevents wobbling and out-of-bounds launches
  - Penalty method with adaptive force magnitude ensures smooth constraint enforcement

#### `Tool.hpp`
- **What it does**: User tool interface
- **Tool properties**:
  - Active/inactive state
  - Name and icon
  - Applied force/effect parameters

---

### `src/math/` - Mathematical Utilities

#### `Vec2.hpp`
- **What it does**: 2D vector math
- **Operations**: Addition, subtraction, scalar multiplication, division, dot product, cross product
- **Used everywhere**: Positions, velocities, forces all use Vec2

#### `Util.hpp`
- **What it does**: General math helper functions
- **Common functions**:
  - `metersToPixels()`: Convert simulation units to screen pixels
  - `pixelsToMeters()`: Convert screen pixels to simulation units
  - Vector normalizing and magnitude calculations

#### `Line.hpp`
- **What it does**: Line segment math for collision detection
- **Uses**: Checking if lines intersect for rectangular object collisions

---

### `src/logging/` - Data Recording

#### `Logger.hpp / Logger.cpp`
- **What it does**: Records simulation state for analysis
- **Recording types**:
  1. **JSON logs**: Complete world state at each timestep
     - Stored in `world_log_[timestamp].json`
     - Contains all object positions, velocities, forces
  2. **Text logs**: Diagnostic messages
     - Stored in `log_[timestamp].txt`
  3. **Screenshots**: Screen captures
     - Saved to `screenshots/` folder
- **Data structure**:
  - `JSONLog`: Map of time → world state
  - `WorldState`: Map of object ID → body properties
- **Features**:
  - Automatic logging each update
  - Manual save/load through UI
  - Screenshot capture on demand
  - Reset clears logged data

---

### `src/graphing/` - Visualization of Data

#### `Grapher.hpp / Grapher.cpp`
- **What it does**: Creates graphs of simulation quantities over time
- **How it works**:
  1. User selects which object properties to graph (via buttons in UI)
  2. Grapher extracts historical data from Logger
  3. Uses gnuplot to render graph window
  4. Graphs pause simulation while open
- **Graphable properties**:
  - Position (x, y)
  - Velocity (vx, vy)
  - Acceleration (ax, ay)
  - Kinetic energy
  - Potential energy
  - Any applied force
- **Output**: Graph window with image save capability

---

## How the Simulation Works

### Initialization Phase (main.cpp → World)
1. Create SFML window (800x450)
2. Initialize World with default gravity (0, 10 m/s²) and air density (1.225)
3. Create ground (immovable bottom wall)
4. Initialize Logger and UI components

### Main Loop (60-120 FPS)
```
repeat each frame:
  1. Process user input (tools, pan, zoom)
  2. Update UI (check sliders, buttons)
  3. Call World.update(dt) with time step
  4. Log current state
  5. Render world and UI
  6. Display frame
```

### Physics Update (World.update → ODE Solver)
```
for each object:
  1. Gather all forces acting on it:
     - Gravity (if enabled)
     - Connector forces (springs, cables)
     - Drag force (if air density > 0)
     - User-applied forces (from tools)
     - Friction (during collisions)

  2. Calculate acceleration: a = F / m

  3. Use selected ODE solver to step forward:
     - Solver takes current state (position, velocity)
     - Solver calculates next position/velocity using selected method
     - Return new state

  4. Handle collisions:
     - Check object-to-object collisions
     - Check object-to-wall collisions
     - Apply impulse and friction corrections

  5. Update object position and velocity
  6. Log the new state to JSON

for each connector:
  - Update forces between connected objects
  - Track energy (for springs)
```

### Time Step Control
- Default calculation: 240 Hz (dt = 1/240 ≈ 0.004 seconds)
- Configurable: 30-1000 Hz
- Maximum single step: 0.05 seconds
- Can pause simulation entirely

### Coordinate System
- **Screen space** (graphics library):
  - Origin (0,0) at top-left
  - Right = +X, Down = +Y
  - Used for rendering
- **World space** (physics):
  - Origin (0,0) at bottom-left
  - Right = +X, Up = +Y
  - 1 meter ≈ 50 pixels (configurable in Config.hpp)
  - Conversion functions: `metersToPixels()`, `pixelsToMeters()`

### Grid System
- Minor gridlines: 1 meter apart (light)
- Major gridlines: 5 meters apart (darker)
- Center axes: Brightest color
- Moves with camera pan/zoom

---

## Data Flow Diagram

```
User Input (Mouse, Keyboard)
    ↓
Tools.cpp (interprets input)
    ↓
modifies Object positions/forces
    ↓
World.update(dt)
    ├→ For each object:
    │   ├→ Gather forces
    │   ├→ Call ODESolver.step()
    │   ├→ Collision detection
    │   └→ Update position/velocity
    ├→ Logger records state to JSON
    └→ Returns updated world state
    ↓
Rendering (main.cpp)
    ├→ drawGridlines()
    ├→ World.draw() (all objects)
    ├→ UI.hpp (ImGui panels)
    └→ Display frame
    ↓
User sees updated simulation
```

---

## How to Read the Code

### For Understanding Physics
1. Start: `src/engine/ODE.hpp` - See the 8 solver implementations
2. Then: `src/objects/Body.hpp` - Understand what state is being integrated
3. Then: `src/engine/Collision.hpp` - How collisions work

### For Understanding Object Behavior
1. Start: `src/objects/Object.hpp` - Object properties and methods
2. Then: `src/objects/Force.hpp` - Types of forces
3. Then: `src/objects/Connector.hpp` - How objects interact

### For Understanding the Simulation Loop
1. Start: `src/main.cpp` - Main loop structure
2. Then: `src/core/World.hpp` - How world updates
3. Then: `src/core/Tools.hpp` - How user input affects simulation

### For Adding Features
- New object type: Add to `src/objects/`
- New force type: Extend `Force.hpp`
- New ODE solver: Inherit from `ODESolver` in `src/engine/ODE.hpp`
- New tool: Extend tool system in `src/core/Tools.hpp`
- New UI panel: Add to `src/core/UI.hpp`

---

## Build System

- **Build tool**: CMake (version 3.14+)
- **Language**: C++17
- **Main dependencies**:
  - SFML 3.0.2 (graphics and windowing)
  - ImGui + ImGui-SFML (user interface)
  - nlohmann/json (logging)
  - fmt (formatted output)
  - matplot++ (graphing with gnuplot)

**Build command**: `cmake build && make` or `cmake --build build --config Release`

---

## Key Files Quick Reference

| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point and main loop |
| `src/Config.hpp` | All constants and settings |
| `src/core/World.hpp` | Simulation container and manager |
| `src/objects/Object.hpp` | Physical object definition |
| `src/engine/ODE.hpp` | Physics solvers (8 methods) |
| `src/objects/Connector.hpp` | Springs, ropes, and struts |
| `src/logging/Logger.hpp` | Data recording system |
| `src/graphing/Grapher.hpp` | Visualization |
| `src/math/Vec2.hpp` | 2D vector math |

---

## Notes

- Coordinate system conversion is important: screen uses down=+Y, physics uses up=+Y
- Simulation can run at different speeds; physics runs at fixed timestep, rendering at whatever FPS is possible
- Test scene is useful for benchmarking new solvers
- All quantities are logged automatically; users select what to graph in the UI
