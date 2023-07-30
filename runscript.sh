set -v

make lfrun.out

./lfrun.out inputs/case01-input.txt

python3 draw_circle_layout.py global_test.txt ./outputs/global_case01.png

python3 draw_tile_layout.py transform_test.txt ./outputs/transform_case01.png