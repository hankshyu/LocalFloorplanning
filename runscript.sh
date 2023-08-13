set -v
make lfrun.out
./lfrun.out inputs/$1-input.txt | tee log/$1.log
# python3 draw_circle_layout.py outputs/global_test.txt outputs/global_$1.png
python3 draw_rect_layout.py outputs/global_test.txt outputs/global_$1.png
python3 draw_tile_layout.py outputs/transform_test.txt outputs/transform_$1.png
python3 draw_tile_layout.py outputs/cornerStiching.txt outputs/CS_$1.png
