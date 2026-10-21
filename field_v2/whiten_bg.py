# -*- coding: utf-8 -*-
"""CAD 截图（深色主题）转白底。
背景→白；亮色线/字→深色；灰色场地、黄/蓝填充原样保留；
蓝色启停区整块保护（白字仍为白字）。"""
import numpy as np
from PIL import Image
from scipy import ndimage

SRC = r'C:\Users\35037\.dsh\attachments\v1\objects\8a\8aadadb171d834792a97cd73616c163ac6a0253b999d5eb6446c268976676ee0'
DST = r'D:\Desktop\git\bydcar_g491vet6\field_v2\背景白色版.png'

a = np.asarray(Image.open(SRC).convert('RGB')).astype(np.float64)
bg = np.array([33, 40, 48], float)

bgm = np.abs(a - bg).max(axis=2) <= 10
nz = lambda col, tol: np.abs(a - np.array(col, float)).max(axis=2) <= tol
blue = nz((40, 47, 135), 14) | nz((41, 49, 137), 14)
keep = nz((199, 200, 202), 7) | nz((248, 215, 49), 24) | blue

# 蓝色启停区是矩形：整块保护，白字维持原样
protect = np.zeros_like(bgm)
lab, n = ndimage.label(blue)
for i in range(1, n + 1):
    m = lab == i
    if m.sum() < 300:
        continue
    ys, xs = np.where(m)
    protect[ys.min() - 1:ys.max() + 2, xs.min() - 1:xs.max() + 2] = True

L = 0.299 * a[..., 0] + 0.587 * a[..., 1] + 0.114 * a[..., 2]
t = np.clip((L - 45.0) / (255.0 - 45.0), 0, 1)
inv = 255.0 * (1.0 - t)

out = a.copy()
out[bgm & ~protect] = [255, 255, 255]
rest = ~keep & ~bgm & ~protect
g = np.repeat(inv[..., None], 3, axis=-1)
out[rest] = g[rest]

Image.fromarray(np.clip(out, 0, 255).astype(np.uint8)).save(DST)
print('saved', DST)
