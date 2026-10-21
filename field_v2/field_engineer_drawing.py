# -*- coding: utf-8 -*-
"""
竞赛场地 总平面布置图 (A2 横, 1:10)
=====================================
只调整中央黄色作业区，其余布局与原图一致。
运行:  python field_engineer_drawing.py
输出:  工程图_总平面.pdf（矢量） / 工程图_总平面.png（200 dpi）
"""
import os
import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle, Polygon, Arc
from matplotlib import font_manager

for _f in (r"C:\Windows\Fonts\simhei.ttf", r"C:\Windows\Fonts\simfang.ttf",
           r"C:\Windows\Fonts\msyh.ttc"):
    if os.path.exists(_f):
        font_manager.fontManager.addfont(_f)
plt.rcParams["font.family"] = ["SimHei", "FangSong", "Microsoft YaHei"]
plt.rcParams["axes.unicode_minus"] = False

FW, FH = 594.0, 420.0
PT = 72.0 / 25.4
INK = "#000000"
BLUE = "#1F3FBF"
YELLOW = "#FFD400"
RED = "#C0261F"
LIGHT = "#F5F5F5"

fig = plt.figure(figsize=(FW / 25.4, FH / 25.4))
ax = fig.add_axes([0, 0, 1, 1])
ax.set_xlim(0, FW)
ax.set_ylim(0, FH)
ax.set_aspect("equal")
ax.axis("off")


def L(x1, y1, x2, y2, lw=0.18, ls="-", c=INK, z=5):
    ax.plot([x1, x2], [y1, y2], color=c, lw=lw, ls=ls, zorder=z,
            solid_capstyle="butt", dash_capstyle="butt")


def T(x, y, s, fs=3.0, ha="center", va="center", rot=0, c=INK, bold=False, z=9, **kw):
    kw.update(dict(ha=ha, va=va, fontsize=fs * PT, color=c, rotation=rot, zorder=z,
                   linespacing=1.5))
    if bold:
        kw["fontweight"] = "bold"
    ax.text(x, y, s, **kw)


def box(x, y, w, h, lw=0.35, fc="none", z=6):
    ax.add_patch(Rectangle((x, y), w, h, facecolor=fc, edgecolor=INK, lw=lw,
                           fill=(fc != "none"), zorder=z))


# ---------------------------------------------------------------- 图框
for a, b, c_, d in ((25, 5, 590, 5), (590, 5, 590, 415),
                    (590, 415, 25, 415), (25, 415, 25, 5)):
    L(a, b, c_, d, lw=0.7)

# ---------------------------------------------------------------- 标题栏
TBX, TBY, TBW = 410, 5, 180
box(TBX, TBY, TBW, 72, lw=0.5)
L(TBX, TBY + 56, TBX + TBW, TBY + 56, lw=0.5)
for yy in (14, 28, 42):
    L(TBX, TBY + yy, TBX + TBW, TBY + yy, lw=0.25)
for xx in (22, 92, 116, 158):
    L(TBX + xx, TBY, TBX + xx, TBY + 56, lw=0.25)
T(TBX + TBW / 2, TBY + 64, "竞 赛 场 地   总 平 面 布 置 图", fs=6.2, bold=True)
for i, (a, b, c_, d) in enumerate([("设计", "—", "日期", "2026-10-20"),
                                   ("制图", "—", "比例", "1 : 10"),
                                   ("审核", "—", "图号", "CT-01"),
                                   ("批准", "—", "张数", "1 / 1")]):
    yy = TBY + 49 - i * 14
    T(TBX + 11, yy, a, fs=2.6)
    T(TBX + 57, yy, b, fs=2.6)
    T(TBX + 104, yy, c_, fs=2.6)
    T(TBX + 137, yy, d, fs=2.4)

# ---------------------------------------------------------------- 世界 -> 图纸
OX, OY, SC = 70.0, 130.0, 10.0
X = lambda x: OX + x / SC
Y = lambda y: OY + y / SC
PW = lambda x, y: (X(x), Y(y))

# ---------------------------------------------------------------- 场地
ax.add_patch(Rectangle((OX, OY), 240, 240, facecolor=LIGHT, edgecolor=INK,
                       lw=0.5, zorder=2))
L(OX - 7, Y(1200), OX + 247, Y(1200), lw=0.15, ls=(0, (14, 3, 2, 3)), z=4)
L(X(1200), OY - 7, X(1200), OY + 247, lw=0.15, ls=(0, (14, 3, 2, 3)), z=4)

# 启停区
for (x, y, name) in ((2100, 2100, "启停区 1"), (2100, 0, "启停区 2"),
                     (0, 0, "启停区 3")):
    ax.add_patch(Rectangle((X(x), Y(y)), 30, 30, facecolor=BLUE, edgecolor=INK,
                           lw=0.35, zorder=3))
    T(X(x + 150), Y(y + 195), name, fs=3.0, c="white", bold=True)
    T(X(x + 150), Y(y + 105), "300 × 300", fs=2.3, c="white")

# ---------------------------------------------------------------- 原料区
RX, RY = 1200, 2400
ax.add_patch(Circle(PW(RX, RY), 18, fill=False, ec=INK, lw=0.15, ls=(0, (3, 2)), zorder=3))
ax.add_patch(Circle(PW(RX, RY), 15, facecolor="white", edgecolor=INK, lw=0.35, zorder=3))
ax.add_patch(Circle(PW(RX, RY), 10, facecolor="#E8A23C", edgecolor=INK, lw=0.25, zorder=3.1))
for k in range(3):
    th = np.deg2rad(90 + 120 * k)
    ax.add_patch(Circle(PW(RX + 85 * np.cos(th), RY + 85 * np.sin(th)), 2.5,
                        facecolor=YELLOW, edgecolor=INK, lw=0.25, zorder=3.2))

# ---------------------------------------------------------------- 边缘工位
ax.add_patch(Rectangle((X(910), Y(-150)), 58, 30, facecolor="white",
                       edgecolor=INK, lw=0.35, zorder=3))
for k in (-300, 0, 300):
    ax.add_patch(Circle(PW(1200 + k, 90), 2.0, facecolor=INK, edgecolor=INK, zorder=3.3))
T(X(1200), Y(-55), "暂 存 区", fs=3.0, bold=True)
T(X(1200), Y(-118), "3 × Ø40 @300", fs=2.2)

ax.add_patch(Rectangle((X(-150), Y(910)), 30, 58, facecolor="white",
                       edgecolor=INK, lw=0.35, zorder=3))
for k in (-300, 0, 300):
    ax.add_patch(Circle(PW(90, 1200 + k), 2.0, facecolor=INK, edgecolor=INK, zorder=3.3))
T(X(-62), Y(1200), "粗加工区", fs=3.0, bold=True, rot=90)
T(X(-118), Y(1200), "3 × Ø40 @300", fs=2.2, rot=90)

# 二维码板
ax.add_patch(Rectangle((X(2400), Y(1000)), 2.2, 40, facecolor="#D0D0D0",
                       edgecolor=INK, lw=0.35, zorder=3))

# ---------------------------------------------------------------- 中央作业区（本次调整）
CX = CY = 1200.0
RD = 800.0 / np.sqrt(2.0)
ax.add_patch(Circle(PW(CX, CY), 80, fill=False, ec=INK, lw=0.15,
                    ls=(0, (12, 3, 2, 3)), zorder=3.4))
ax.add_patch(Circle(PW(CX, CY), 40, fill=False, ec=RED, lw=0.5, zorder=3.45))
TH0 = {(-1, -1): 0, (1, -1): 90, (1, 1): 180, (-1, 1): 270}
for (sx, sy) in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
    cx, cy = CX + sx * RD, CY + sy * RD
    t0 = TH0[(sx, sy)]
    th = np.linspace(np.deg2rad(t0), np.deg2rad(t0 + 90), 90)
    pts = [PW(cx + 400 * np.cos(t), cy + 400 * np.sin(t)) for t in th]
    pts.append(PW(cx, cy))
    ax.add_patch(Polygon(pts, closed=True, facecolor=YELLOW, edgecolor=INK,
                         lw=0.4, zorder=3.5))
ax.add_patch(Circle(PW(CX, CY), 10, facecolor=YELLOW, edgecolor=INK, lw=0.4, zorder=3.6))

# ---------------------------------------------------------------- 尺寸标注
def arrow(p1, p2, lw=0.15, c=INK):
    ax.annotate("", xy=p1, xytext=p2,
                arrowprops=dict(arrowstyle="<|-|>", color=c, lw=lw,
                                shrinkA=0, shrinkB=0, mutation_scale=2.0 * PT), zorder=7)


def dim_x(wx1, wx2, sy, label, wy_from, fs=3.0):
    sx1, sx2, sy0 = X(wx1), X(wx2), Y(wy_from)
    s = 1 if sy > sy0 else -1
    L(sx1, sy0 + s * 0.8, sx1, sy + s * 2.8, lw=0.12, z=6)
    L(sx2, sy0 + s * 0.8, sx2, sy + s * 2.8, lw=0.12, z=6)
    arrow((sx1, sy), (sx2, sy))
    T((sx1 + sx2) / 2, sy + 1.6 * s, label, fs=fs, z=9,
      va="bottom" if s > 0 else "top",
      bbox=dict(boxstyle="square,pad=0.05", fc="white", ec="none"))


def dim_y(wy1, wy2, sx, label, wx_from, fs=3.0):
    sy1, sy2, sx0 = Y(wy1), Y(wy2), X(wx_from)
    s = 1 if sx > sx0 else -1
    L(sx0 + s * 0.8, sy1, sx + s * 2.8, sy1, lw=0.12, z=6)
    L(sx0 + s * 0.8, sy2, sx + s * 2.8, sy2, lw=0.12, z=6)
    arrow((sx, sy1), (sx, sy2))
    T(sx - 1.6 * s, (sy1 + sy2) / 2, label, rot=90, fs=fs, z=9,
      ha="right" if s > 0 else "left",
      bbox=dict(boxstyle="square,pad=0.05", fc="white", ec="none"))


def lead(txt, wxy, sxy, fs=2.8, ha="center", c=INK):
    ax.annotate(txt, xy=PW(*wxy), xytext=sxy, fontsize=fs * PT, zorder=9,
                ha=ha, va="center", color=c,
                arrowprops=dict(arrowstyle="-|>", lw=0.15, color=c,
                                shrinkA=2, shrinkB=2, mutation_scale=2.0 * PT),
                bbox=dict(boxstyle="square,pad=0.1", fc="white", ec="none"))


dim_x(0, 2400, Y(0) - 32, "2400", 0, fs=3.6)
dim_y(0, 2400, X(0) - 30, "2400", 0, fs=3.6)

dim_x(2100, 2400, Y(2400) + 9, "300", 2400, fs=2.8)
dim_y(2100, 2400, X(2400) + 9, "300", 2400, fs=2.8)
T(X(2265), Y(2400) + 15, "其余启停区同", fs=2.2)

dim_x(0, 1200, Y(2400) + 25, "1200", 2400, fs=2.8)
lead("Ø300", (1200, 2512), (X(1040) - 20, Y(2660)), fs=2.8)
lead("Ø200", (1200, 2415), (X(1200) + 34, Y(2565)), fs=2.8)
lead("3 × Ø50", (1113, 2443), (X(1200) + 38, Y(2385)), fs=2.6)

dim_x(910, 1490, Y(0) - 18, "580", 0, fs=2.8)
dim_y(-150, 150, X(1490) + 9, "300", 1490, fs=2.6)
dim_y(910, 1490, X(150) + 9, "580", 150, fs=2.8)
dim_x(-150, 150, Y(910) + 9, "300", 910, fs=2.6)

dim_y(1000, 1400, X(2400) + 9, "400", 2400, fs=2.6)
T(X(2400) + 21, Y(1200), "二维码板", fs=2.5, rot=90)

lead("Ø1600", (1200, 2000), (X(1200), Y(2090)), fs=3.2)
lead("R400", (1483, 1483), (X(1100), Y(1790)), fs=3.2)
lead("Ø800", (1600, 1200), (X(1850), Y(1300)), fs=3.2)
lead("Ø200", (1100, 1200), (X(1200), Y(700)), fs=3.2)
ax.add_patch(Arc(PW(CX, CY), 52, 52, theta1=24, theta2=68, ec=INK, lw=0.15, zorder=7))
T(X(CX + 310 * np.cos(np.deg2rad(46))), Y(CY + 310 * np.sin(np.deg2rad(46))),
  "45°", fs=2.6)

# ---------------------------------------------------------------- 图例
LGX, LGY = 350, 374
T(LGX, LGY + 16, "图    例", fs=4.2, bold=True, ha="left")
for i, (c, txt) in enumerate([(BLUE, "启停区（3 处）"), (YELLOW, "中央作业区 / 原料孔"),
                              ("white", "边缘工位台面"), (RED, "Ø800 叶片尖端圆"),
                              ("#D0D0D0", "二维码板")]):
    yy = LGY + 8 - i * 7.2
    ax.add_patch(Rectangle((LGX, yy - 1.5), 6, 3.2, facecolor=c, edgecolor=INK,
                           lw=0.2, zorder=6))
    T(LGX + 9, yy, txt, fs=2.5, ha="left")

# ---------------------------------------------------------------- 说明
NX, NY = 350, 322
T(NX, NY + 10, "说    明", fs=4.2, bold=True, ha="left")
notes = [
    "1. 尺寸单位 mm；比例 1:10；未注公差 ±5。",
    "2. 场地净尺寸 2400 × 2400，蓝色 300×300 为启停区，共 3 处。",
    "3. 边缘工位台面 580×300，跨边界 150/150；台面内侧 3×Ø40",
    "     定位点，间距 300。",
    "4. ★本次仅调整中央黄色作业区，其余布置与原图一致：",
    "     原方案  四片 R400 四分之一圆贴于 1400×1400 方框四角，",
    "              尖端距中心 590，外轮廓为方框；",
    "     调整后  四片角点内移至 Ø1600 圆上（45°/135°/225°/315°",
    "              方向，距中心 800），尖端与 Ø800 圆相切；",
    "              外轮廓改为 Ø1600 圆；中心保留 Ø200 圆盘。",
    "5. 黄色为地面哑光贴膜 PANTONE 109C；蓝色 PANTONE 293C。",
    "6. 外围虚线为设备工作台投影，不计入场地。",
    "7. 未注定位点均为 Ø40 黑色哑光，随台面一并制作。",
]
for i, s in enumerate(notes):
    T(NX, NY - 1 - i * 6.4, s, fs=2.35, ha="left")

# ---------------------------------------------------------------- 明细表
MX, MY, MW = 410, 76, 180
T(MX, MY + 56, "明  细  表", fs=3.6, bold=True, ha="left")
cols = [("序号", 9), ("名  称", 38), ("规  格", 63), ("数量", 14), ("备注", 56)]
xpos = [MX + sum(c[1] for c in cols[:i]) for i in range(len(cols) + 1)]
rows = [("1", "启停区", "300×300", "3", "蓝色"),
        ("2", "原料区", "Ø300 / Ø200 / 3×Ø50", "1", "圆形料仓"),
        ("3", "暂存区", "580×300", "1", "白色台面"),
        ("4", "粗加工区", "580×300", "1", "白色台面"),
        ("5", "二维码板", "400×400", "1", "立板"),
        ("6", "中央作业区", "Ø1600 / Ø800 / R400", "1", "黄色 ★已调整"),
        ("7", "定位点", "Ø40", "6", "黑色")]
top, hh, rh = MY + 46, 7, 5.6
for i, (name, w) in enumerate(cols):
    T((xpos[i] + xpos[i + 1]) / 2, top - hh / 2 + 1.2, name, fs=2.3)
L(MX, top - hh, MX + MW, top - hh, lw=0.25)
L(MX, top, MX + MW, top, lw=0.35)
for xp in xpos:
    L(xp, top - hh - rh * len(rows), xp, top, lw=0.22)
for j, r in enumerate(rows):
    yy = top - hh - rh * (j + 1)
    for i, v in enumerate(r):
        T((xpos[i] + xpos[i + 1]) / 2, yy + rh / 2, v, fs=2.2)
    if j < len(rows) - 1:
        L(MX, yy, MX + MW, yy, lw=0.15)
L(MX, top - hh - rh * len(rows), MX + MW, top - hh - rh * len(rows), lw=0.35)

# ---------------------------------------------------------------- 指北针
ax.add_patch(Circle((48, 392), 7, fill=False, ec=INK, lw=0.3))
ax.add_patch(Polygon([(48, 400), (45, 389), (48, 391), (51, 389)], closed=True,
                     facecolor=INK, edgecolor=INK, lw=0.2))
T(48, 380, "N", fs=3.0, bold=True)

here = os.path.dirname(os.path.abspath(__file__))
fig.savefig(os.path.join(here, "工程图_总平面.pdf"), facecolor="white")
fig.savefig(os.path.join(here, "工程图_总平面.png"), dpi=200, facecolor="white")
plt.close(fig)
print("done")


