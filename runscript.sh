set -v
make lfrun.out
./lfrun.out inputs/$1-input.txt | tee log/$1.log
<<<<<<< HEAD
# python3 draw_circle_layout.py outputs/global_test.txt outputs/global_$1.png
python3 draw_rect_layout.py outputs/global_test.txt outputs/global_$1.png
python3 draw_tile_layout.py outputs/transform_test.txt outputs/transform_$1.png
python3 draw_tile_layout.py outputs/cornerStiching.txt outputs/CS_$1.png
=======
python3 draw_circle_layout.py outputs/ppmoduleResult.txt outputs/ppmodule_$1.png
python3 draw_tile_layout.py outputs/phase1.txt outputs/P1_$1.png
python3 draw_tile_layout.py outputs/phase2_1.txt outputs/P2.1_$1.png
python3 draw_tile_layout.py outputs/phase2.txt outputs/P2_$1.png
>>>>>>> orange
