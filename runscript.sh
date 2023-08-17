set -v
make lfrun.out
./lfrun.out inputs/$1-input.txt | tee log/$1.log
# python3 draw_circle_layout.py outputs/ppmoduleResult.txt outputs/ppmodule_$1.png
python3 draw_rect_layout.py outputs/global_test.txt outputs/global_$1.png
python3 draw_tile_layout.py outputs/phase1.txt outputs/P1_$1.png
python3 draw_tile_layout.py outputs/phase2_1.txt outputs/P2.1_$1.png
python3 draw_tile_layout.py outputs/phase2.txt outputs/P2_$1.png
python3 draw_tile_layout.py outputs/phase3.txt outputs/P3_$1.png
