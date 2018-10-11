# particle-test
A test of a simple particle simulation using various spatial data structures. Currently, only a naive quadtree is used which gets recreated each frame.

## Requirements
* SFML 2.5
* C++11

## Usage
To build on Linux or OS X:
```make```

On Windows, there is a Visual Studio 2017 Community solution file.

The particles will accelerate towards the cursor when clicking in the window. `Q` will show the quadtree structure, and `Space` will pause or resume the simulation.
