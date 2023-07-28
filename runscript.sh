set -v
make clean
make lfrun.out
./lfrun.out ./inputs/case1.txt

# python draw_circle_layout.py test.txt ./outputs/global_case1.png

python draw_tile_layout.py test.txt ./outputs/transform_case1.png
