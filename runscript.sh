set -v
make clean
make lfrun.out
./lfrun.out ./inputs/$1-input.txt

python3 draw_circle_layout.py global_test.txt ./outputs/global_$1.png

python3 draw_tile_layout.py transform_test.txt ./outputs/transform_$1.png