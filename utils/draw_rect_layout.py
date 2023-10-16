import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import math
import time


def draw_block(ax, x, y, width, height, color):
    # color = "#BBB"
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


def draw_circle(ax, x, y, radius, color):
    # color = "#FCC"
    ax.add_patch(
        patches.Circle(
            (x, y),
            radius,
            fill=True,
            edgecolor="#000",
            facecolor=color,
            alpha=1.0  # 0.3 original
        )
    )


txt_name = sys.argv[1]
png_name = sys.argv[2]
fread = open(txt_name, 'r')
f = fread.read().split("\n")


total_block_number = int(f[0].split(" ")[1])
total_connection_number = int(f[0].split(" ")[3])
window_width = int(f[1].split(" ")[0])
window_height = int(f[1].split(" ")[1])
aspect_ratio = window_height / window_width

png_size = (16, 15*aspect_ratio)
fig = plt.figure(figsize=png_size)

ax = fig.add_subplot(111)
ax.set_xbound(0, window_width)
ax.set_ybound(0, window_height)

i = 2

ax.add_patch(
    patches.Rectangle(
        (0, 0),
        window_width,
        window_height,
        fill=False,
        edgecolor="#000",
        facecolor="#FFF",
        alpha=1.0  # 0.3 original
    )
)

name2pos = {}

for block in range(total_block_number):
    ss = f[i].split(" ")
    if ss[1] == "SOFT":
        x, y, w, h = float(ss[2]), float(ss[3]), float(ss[4]), float(ss[5])
        draw_block(ax, x, y, w, h, color="#FCC")
        plt.text(x + 5, y + 5, ss[0])
        name2pos[ss[0]] = (x + w / 2, y + h / 2)
    else:
        x, y, w, h = float(ss[2]), float(ss[3]), float(ss[4]), float(ss[5])
        draw_block(ax, x, y, w, h, color="#BBB")
        x_offset, y_offset = 0, 0
        if x == 0:
            x_offset -= 20
        elif x == window_width:
            x_offset += 3
        if y == 0:
            y_offset -= 13
        elif y == window_height:
            y_offset += 3
        plt.text(x + x_offset, y + y_offset, ss[0])
        name2pos[ss[0]] = (x + w / 2, y + h / 2)
    i += 1

j = i
max_value = 1
min_value = 1e10
for connection in range(total_connection_number):
    ss = f[j].split(" ")
    value = int(ss[2])
    if value > max_value:
        max_value = value
    if value < min_value:
        min_value = value
    j += 1

for connection in range(total_connection_number):
    ss = f[i].split(" ")
    x_values = [name2pos[ss[0]][0], name2pos[ss[1]][0]]
    y_values = [name2pos[ss[0]][1], name2pos[ss[1]][1]]
    value = float(ss[2])
    width = (value - min_value) / (max_value - min_value) * 14 + 1
    plt.plot(x_values, y_values, color="blue",
             linestyle="-", linewidth=width, alpha=0.5)
    i += 1

# plt.savefig(str(sys.argv[1])[:-4]+".png")

plt.savefig(png_name)
