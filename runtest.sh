set -v
make clean
make lfrun.out
./lfrun.out
python3 draw_tile_layout.py outputs/artpc.txt outputs/artpc.png