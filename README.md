# Soft Block Floorplanner (ICCAD Contest 2023 Problem D)

Soft block floorplanner is a VLSI floorplanner that can deal with soft blocks and preplaced blocks. This program is a submission for the [2023 ICCAD Contest Problem D](http://iccad-contest.org/tw/03_problems.html). This program is written by Kevin Chen, Ryan Lin, Hank Hsu with guidance from Prof. Iris H.R. Jiang of NTU GIEE.

## Prerequisites
1. Make sure your `gcc/g++` supports C++17 (C++14 should work too). 
2. Make sure to download the [Boost library (1.82.0)](https://www.boost.org/users/history/version_1_82_0.html) to somewhere on your machine.
3. Use `pip3` to install `numpy` and `matplotlib`.
```bash
pip3 install numpy
pip3 install matplotlib
```

## Installation

1. Clone the git repo.
```bash
git clone https://github.com/hankshyu/LocalFloorplanning.git
```

2. Modify the Makefile. In line 7, change the boost path to your own Boost path.
```Makefile
BOOSTPATH = ../boost_1_82_0    # example
```

3. Compile the floorplanner.
```bash
make
```

4. Create 3 directories:
```bash
mkdir answers
mkdir outputs
mkdir log
```

5. (Optional) Compile the verifier.
```bash
make verify
```

## Usage

### Running the solver:
```bash
./runscript.sh <case01~case06>
```

### Visualization
After running the script, you should see in `outputs/` a few pngs.
* `global_case#.png` : Floorplan after global floorplanning phase (Rectangle Gradient).
* `P2_case#.png` : Floorplan after rectilinear shapes are partitioned into rectangles (Corner stitching).
* `legal_case#.png` : Floorplan after legalization phase (DFS-based overlap migration).
* `floorplan_case#.png` : Final floorplan output with connectivity information.