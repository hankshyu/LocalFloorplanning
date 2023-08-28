import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import math
import time


def draw_block(ax, x, y, width, height, type):

    if "BLOCK" in type:
        color = "#FCC"
    elif "OVERLAP" in type:
        color = "#888"
    else:
        color = "#BBB"

    ax.add_patch(
        patches.Rectangle(
            (x, y),
            width,
            height,
            fill=True,
            edgecolor="#000",
            facecolor=color,
            alpha=1.0  # 0.3 original
        )
    )


png_size = (16, 12)
txt_name = sys.argv[1]
png_name = sys.argv[2]
# fread = open(txt_name, 'r')
# f = fread.read().split("\n")


# window_width = int(f[0].split(" ")[0])
# window_height = int(f[0].split(" ")[1])

fig = plt.figure(figsize=png_size, dpi=300)

ax = fig.add_subplot(111)
# ax.set_xbound(0, window_width)
# ax.set_ybound(0, window_height)

# i = 2
# name2pos = {}

# for block in range(total_block_number):

line_count = 0
with open(txt_name, 'r') as fread:
    while True:
        line = fread.readline()


        if not line:
            break
        line_count +=1
        ss = line.split(" ")
        if line_count == 1:
            window_width = int(ss[0])
            window_height = int(ss[1])
            ax.set_xbound(0, window_width)
            ax.set_ybound(0, window_height)
        else:
            draw_block(ax, int(ss[0]), int(ss[1]), int(ss[2]), int(ss[3]), ss[4])
            fread.readline()
            fread.readline()
            fread.readline()
            fread.readline()
            fread.readline()
            fread.readline()






plt.savefig(str(sys.argv[1])[:-4]+".png")

plt.savefig(png_name)