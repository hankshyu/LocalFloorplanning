set -v
make clean
make lfrun.out
./lfrun.out ./inputs/$1.txt

# python draw_circle_layout.py test.txt ./outputs/global_case1.png

python draw_tile_layout.py test.txt ./outputs/transform_$1.png
