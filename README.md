# Newton's Notepad

A physics simulator/playground with the following features:

- Simple 2D Graphics
- Easy to Use UI
- Many Physical Concepts:
  - Circular Objects
  - Rectangular Objects (In Development)
  - Gravity
  - Impulse-based Collisions
  - Impulse-based Friction
  - Air Drag
  - Springs
  - Cables / Struts
  - Push & Pool Tools
- Real Quantities & Units
  - The minor and major gridlines represent 1m and 5m respectively
  - In the code, right and down are taken to be positive for consistency with the graphics library
  - The position is standardized when exposed to the user, right and up being positive, and the ground being y=0
- Adjustable Time Step
- 7 ODE Solvers Using The Following Methods:
  - Euler's ("Euler")
  - Semi-Implicit Euler's ("EulerS")
  - Runge-Kutta 2nd Order ("RK2")
  - Runge-Kutta 4th Order ("RK4")
  - Dormand-Prince ("DOPRI5")
  - Velocity Verlet ("Verlet")
  - Adams-Bashforth 4th Order ("AB")
  - Adams-Bashforth-Moulton 4th Order ("ABM")
- Quantity Logging and Graphing for Analysis

This simulation was initially created as a yearly homework project for AP Calculus BC, aiming to research and compare different methods of numerically solving ordinary differential equations, hence why there are so many solver options.

## Installation

Download the latest release for your operating system and launch.

If graphing doesn't work, you may need to install [gnuplot](http://www.gnuplot.info/).

## Controls

- LMB to use tools or interact with objects
- Hold RMB and move your mouse to pan
- Scroll to zoom in/out
- Esc to toggle settings
- P to pause
- S to capture screenshot
- G to toggle graphing

## Logging

The simulation logs the state at each update and saves it on exit. You can also save manually through the simulation settings. Keep in mind that resetting the simulation also resets the logged values.

## Graphing

Graphing isn't live for performance concerns; the simulation pauses when you open the graph window.

Due to how the graphing library works, closing the window doesn't automatically restart the simulation, you have to press G.

You can select which quantities to graph, their entire history will be graphed, by accessing values stored in the logger. If the object doesn't exist for the entire graphed time period (due to being added to the scene later than other graphed objects), all it's properties in the missing timestamps are taken as NaN.

To select quantities, press the graph button next to the property in the object properties panel. Properties are object-specific, meaning you only select that specific object's property, not every object's. Pressing the button again deselects the quantity.

You can save the graph as an image through the graphing window.

## Test Setup

As this simulation was made to compare ODE solvers, a simple test scene with an object and a spring has been coded into it. You can load it through the simulation settings, and graph important quantities in the object properties panel. After loading a test scene, the simulation automatically pauses after 3 seconds.

## Notes

- The latest version of imgui-sfml requires imgui 1.91.x, so to compile the project you need to checkout v1.91.9b for that submodule.

## Copyright

Distributed under the GNU GPLv3 License. See `LICENSE` for more information.
