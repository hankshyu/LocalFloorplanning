set -v
make
./fprun inputs/$1-input.txt answers/$1-output.txt | tee log/$1.log
# python3 draw_circle_layout.py outputs/ppmoduleResult.txt outputs/ppmodule_$1.png
python3 utils/draw_rect_layout.py outputs/global_test.txt outputs/global_$1.png
# python3 utils/draw_tile_layout.py outputs/phase1.txt outputs/P1_$1.png
# python3 utils/draw_tile_layout.py outputs/phase2_1.txt outputs/P2.1_$1.png
# python3 utils/draw_tile_layout.py outputs/phase2.txt outputs/P2_$1.png
# python3 utils/draw_tile_layout.py outputs/legal.txt outputs/legal_$1.png

./utils/verify ./inputs/$1-input.txt ./answers/$1-output.txt
python3 utils/draw_floorplan.py ./inputs/$1-input.txt ./answers/$1-output.txt ./outputs/floorplan_$1.png