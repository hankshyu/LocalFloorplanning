import sys
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import math
import time
import random


def draw_block(ax, x, y, width, height, color = "#FFF", aplha = 1.0):
    ax.add_patch(
        patches.Rectangle(
            (x, y),
            width,
            height,
            fill=True,
            edgecolor="#DDD",
            facecolor=color,
            alpha=aplha  # 0.3 original
        )
    )

used_color = ["#FFE", "#FFF", "#F00"]


def set_rand_color():
    (R, G, B) = (random.randint(0, 15), random.randint(0, 15), random.randint(0, 15))
    while R+G+B < 24:
        (R, G, B) = (random.randint(0, 15), random.randint(0, 15), random.randint(0, 15))
    (Rc, Gc, Bc) = (hex(R)[2], hex(G)[2], hex(B)[2])
    color = "#" + Rc + Gc + Bc
    
    while color in used_color:
        (R, G, B) = (random.randint(0, 15), random.randint(0, 15), random.randint(0, 15))
        while R+G+B < 20:
            (R, G, B) = (random.randint(0, 15), random.randint(0, 15), random.randint(0, 15))
        (Rc, Gc, Bc) = (hex(R)[2], hex(G)[2], hex(B)[2])
        color = "#" + Rc + Gc + Bc

    return color

def blend_color(colors):
    r = []
    g = []
    b = []
    colorCount = len(colors)
    for c in colors:
        r.append(int(c[1], 16) * 16)
        g.append(int(c[2], 16) * 16)
        b.append(int(c[3], 16) * 16)

    (br, bg, bb) = (0, 0, 0)
    for i in range(colorCount):
        br += r[i]
        bg += g[i]
        bb += b[i]

    (br, bg, bb) = (br//colorCount, bg//colorCount, bb//colorCount)
    (br, bg, bb) = (hex(br)[2:], hex(bg)[2:], hex(bb)[2:])
    if len(br) < 2:
        br += "0"
    if len(bg) < 2:
        bg += "0"
    if len(bb) < 2:
        bb += "0"

    blend = "#" + br + bg + bb

    return blend


########################################################
#                     main part
########################################################
png_size = (16, 12)
txt_name = sys.argv[1]
png_name = sys.argv[2]
fread = open(txt_name, 'r')
f = fread.read().split("\n")


total_block_number = int(f[0].split(" ")[1])
window_width = int(f[1].split(" ")[0])
window_height = int(f[1].split(" ")[1])

fig = plt.figure(figsize=png_size, dpi=300)

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
name2color = {}
overlapDict = {}

for block in range(total_block_number):
    ss = f[i].split(" ")
    bx, by, bw, bh = float(ss[2]), float(ss[3]), float(ss[4]), float(ss[5])
    plt.text(bx+bw/2, by+bh/2, ss[0])
    name2pos[ss[0]] = (bx+bw/2, by+bh/2)
    if ss[6] == "HARD_BLOCK":
        color = "#777"
    else:
        color = set_rand_color()
    name2color[ss[0]] = color
    i += 1
    ss = f[i].split(" ")
    subblockCount = int(ss[0])
    overlapCount = int(ss[1])
    i += 1
    for subblock in range(subblockCount):
        ss = f[i].split(" ")
        i += 1
        (sbx, sby, sbw, sbh) = (float(ss[0]), float(ss[1]), float(ss[2]), float(ss[3]))
        draw_block(ax, sbx, sby, sbw, sbh, color=color)

    for overlap in range(overlapCount):
        ss = f[i].split(" ")
        i += 1
        (sbx, sby, sbw, sbh) = (float(ss[0]), float(ss[1]), float(ss[2]), float(ss[3]))
        #draw_block(ax, sbx, sby, sbw, sbh, color=color, aplha=0.3)
        if (sbx, sby, sbw, sbh) in overlapDict:
            overlapDict[(sbx, sby, sbw, sbh)].append(color)
        else:
            overlapDict[(sbx, sby, sbw, sbh)] = [color]

for overlap in overlapDict:
    colors = overlapDict[overlap]
    (sbx, sby, sbw, sbh) = overlap
    blend = blend_color(colors)
    draw_block(ax, sbx, sby, sbw, sbh, color=blend)


while f[i].split(" ")[0] != "CONNECTION":
    ss = f[i].split(" ")
    i += 1
    (sbx, sby, sbw, sbh) = (float(ss[0]), float(ss[1]), float(ss[2]), float(ss[3]))
    draw_block(ax, sbx, sby, sbw, sbh, color="#FFE")


total_connection_number = int(f[i].split(" ")[1])
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
    if max_value == min_value:
        width = 15
    else:
        width = (value - min_value) / (max_value - min_value) * 14 + 1
    plt.plot(x_values, y_values, color="blue",
             linestyle="-", linewidth=width, alpha=0.5)
    i += 1


while i < len(f):
    if f[i] == "":
        break
    ss = f[i].split(" ")
    (bx, by, bw, bh) = (float(ss[0]), float(ss[1]), float(ss[2]), float(ss[3]))
    draw_block(ax, bx, by, bw, bh, color="#F00")
    i += 1


plt.savefig(png_name)
