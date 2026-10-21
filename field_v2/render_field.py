# -*- coding: utf-8 -*-
"""
《风车·四象》竞赛场地 2400×2400 —— 图纸生成器
=================================================
运行:  python render_field.py
输出:  new_field_layout.png   布局图（带尺寸标注）
       new_field_flow.png     任务流线图（带路线与停靠点编号）
坐标系: 场地左下角 = (0,0)，X 向右，Y 向上，单位 mm。
"""
import os
import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Circle, Polygon
from matplotlib import font_manager

# ---------------------------------------------------------------- 字体
for _f in (r"C:\Windows\Fonts\msyh.ttc", r"C:\Windows\Fonts\msyhbd.ttc",
           r"C:\Windows\Fonts\simhei.ttf"):
    if os.path.exists(_f):
        font_manager.fontManager.addfont(_f)
plt.rcParams["font.family"] = "Microsoft YaHei"
plt.rcParams["axes.unicode_minus"] = False

# ---------------------------------------------------------------- 配色
C_FLOOR  = "#EFEFEF"
C_WALL   = "#C6C6C6"
C_START  = "#2E5BFF"
C_START_E= "#14319C"
C_MOD    = "#FCFCFC"
C_TURN   = "#FFC41F"
C_TURN_E = "#A97300"
C_PILLAR = "#5B5B5B"
C_CORR   = "#00A39B"
C_HAZ    = "#FFD34D"
C_HAZ_E  = "#8A6D00"
C_DIM    = "#595959"
C_ROUTE  = "#FF3B30"

# ---------------------------------------------------------------- 几何常量
S        = 2400.0
CX = CY  = 1200.0
START    = 300.0
INSET    = 150.0
MOD_W    = 580.0
MOD_D    = 300.0
TURN_R   = 350.0
PILL_R   = 100.0
CORR_W   = 300.0

START_ZONES = [("A", 150, 150, (1, 1)), ("B", 1950, 150, (-1, 1)),
               ("C", 1950, 1950, (-1, -1)), ("D", 150, 1950, (1, -1))]

# key, cx, cy, 朝向(V=模块长边平行X), 名称, 功能
MODULES = [
    ("N", 1200, 2400, "V", "工位①", "原料仓"),
    ("E", 2400, 1200, "H", "工位②", "粗加工台"),
    ("S", 1200, 0,    "V", "工位③", "成品暂存"),
    ("W", 0,    1200, "H", "工位④", "任务牌"),
]

PILLARS = [(1600, 700), (1700, 1600), (800, 1700), (700, 800)]
HAZARDS = [(1200.0, 1820.0, "H"), (1820.0, 1200.0, "V"),
           (1200.0, 580.0,  "H"), (580.0,  1200.0, "V")]


def rot_ccw(pt, k):
    x, y = pt[0] - CX, pt[1] - CY
    for _ in range(k % 4):
        x, y = -y, x
    return np.array([x + CX, y + CY])


# ================================================================ 图元
def draw_floor(ax):
    ax.add_patch(Rectangle((-50, -50), S + 100, S + 100, facecolor=C_WALL,
                           edgecolor="#9C9C9C", lw=1.2, zorder=1.6))
    floor = Rectangle((0, 0), S, S, facecolor=C_FLOOR, edgecolor="#2B2B2B",
                      lw=1.6, zorder=2.0)
    ax.add_patch(floor)
    for v in range(200, 2400, 200):
        ax.plot([v, v], [0, S], color="#DFDFDF", lw=0.6, zorder=2.02)
        ax.plot([0, S], [v, v], color="#DFDFDF", lw=0.6, zorder=2.02)
    ax.plot([CX, CX], [0, S], color="#B9B9B9", lw=0.9, ls=(0, (10, 6)), zorder=2.04)
    ax.plot([0, S], [CY, CY], color="#B9B9B9", lw=0.9, ls=(0, (10, 6)), zorder=2.04)
    return floor


def draw_corridors(ax, floor):
    half = CORR_W / 2.0
    s_end = CX - 380.0 / np.sqrt(2.0)
    p = np.array([1.0, -1.0]) / np.sqrt(2.0) * half
    base = [np.array([0, 0]) + p, np.array([s_end, s_end]) + p,
            np.array([s_end, s_end]) - p, np.array([0, 0]) - p]
    for k in range(4):
        poly = [rot_ccw(q, k) for q in base]
        patch = Polygon(poly, closed=True, facecolor="#00A39B", alpha=0.07,
                        edgecolor=C_CORR, lw=1.6, ls=(0, (8, 5)), zorder=2.3)
        patch.set_clip_path(floor)
        ax.add_patch(patch)
        for s0 in (500.0, 690.0, 860.0):
            c0 = rot_ccw(np.array([s0, s0]), k)
            u = np.array([CX, CY]) - c0
            u = u / np.linalg.norm(u)
            if np.linalg.norm(np.array([CX, CY]) - c0) < 420:
                continue
            pv = np.array([-u[1], u[0]])
            tip = c0 + u * 52
            ax.add_patch(Polygon([tip, c0 - u * 34 + pv * 52, c0 - u * 34 - pv * 52],
                                 closed=True, facecolor=C_CORR, alpha=0.55,
                                 edgecolor="none", zorder=2.36))


def draw_hazards(ax):
    for (x, y, o) in HAZARDS:
        w, h = (300, 80) if o == "H" else (80, 300)
        ax.add_patch(Rectangle((x - w / 2, y - h / 2), w, h, facecolor=C_HAZ,
                               edgecolor=C_HAZ_E, lw=1.0, hatch="////", zorder=2.6))


def draw_starts(ax, label=True):
    for (name, x, y, d) in START_ZONES:
        ax.add_patch(Rectangle((x, y), START, START, facecolor=C_START,
                               edgecolor=C_START_E, lw=1.6, zorder=3.4))
        cx, cy = x + START / 2, y + START / 2
        if label:
            ax.text(cx, cy + 26, f"启停区 {name}", ha="center", va="center",
                    color="white", fontsize=11.5, fontweight="bold", zorder=3.5)
            ax.text(cx, cy - 26, "START", ha="center", va="center",
                    color="#C9D6FF", fontsize=7.2, zorder=3.5)
        u = np.array([d[0], d[1]], float) / np.sqrt(2)
        pv = np.array([-u[1], u[0]])
        corner = np.array([x + (START if d[0] > 0 else 0),
                           y + (START if d[1] > 0 else 0)], float)
        base = corner - u * 84
        ax.add_patch(Polygon([corner - u * 18, base + pv * 26, base - pv * 26],
                             closed=True, facecolor="white", alpha=0.85,
                             edgecolor="none", zorder=3.5))


def draw_modules(ax, label=True, fs=9.2):
    for (k, cx, cy, o, t1, t2) in MODULES:
        if o == "V":
            rect = Rectangle((cx - MOD_W / 2, cy - MOD_D / 2), MOD_W, MOD_D, zorder=3.8)
            out = np.array([0.0, 1.0 if k == "N" else -1.0])
        else:
            rect = Rectangle((cx - MOD_D / 2, cy - MOD_W / 2), MOD_D, MOD_W, zorder=3.8)
            out = np.array([1.0 if k == "E" else -1.0, 0.0])
        rect.set_facecolor(C_MOD)
        rect.set_edgecolor("#2F2F2F")
        rect.set_linewidth(2.0)
        ax.add_patch(rect)
        # 内侧 3 个定位孔
        for off in (-150, 0, 150):
            c = np.array([cx, cy], float) - out * (MOD_D / 2) + \
                np.array([-out[1], out[0]]) * off
            ax.add_patch(Circle(tuple(c), 20, facecolor="#111111",
                                edgecolor="none", zorder=4.2))
        # 外侧换模块把手
        hc = np.array([cx, cy], float) + out * (MOD_D / 2 - 22)
        tng = np.array([-out[1], out[0]])
        ax.plot([hc[0] - tng[0] * 210, hc[0] + tng[0] * 210],
                [hc[1] - tng[1] * 210, hc[1] + tng[1] * 210],
                color="#BEBEBE", lw=7, solid_capstyle="round", zorder=3.9)
        if label:
            tc = np.array([cx, cy], float) + out * 52
            ax.text(tc[0], tc[1], f"{t1}\n{t2}", ha="center", va="center",
                    fontsize=fs, fontweight="bold", color="#202020",
                    linespacing=1.5, zorder=4.3)


def draw_turn(ax, dim=True):
    ax.add_patch(Circle((CX, CY), TURN_R, facecolor=C_TURN, edgecolor=C_TURN_E,
                        lw=2.6, zorder=3.3))
    ax.add_patch(Circle((CX, CY), TURN_R - 55, facecolor="none", edgecolor=C_TURN_E,
                        lw=1.0, ls=(0, (6, 4)), zorder=3.34))
    for k in range(4):
        th0 = np.deg2rad(45 + 90 * k)
        t = np.linspace(0, 1, 80)
        rr = (TURN_R - 22) - (TURN_R - 105) * t
        th = th0 + np.deg2rad(85) * t
        ax.plot(CX + rr * np.cos(th), CY + rr * np.sin(th), color=C_TURN_E,
                lw=1.7, alpha=0.8, zorder=3.35)
    for k in range(4):
        th = np.deg2rad(90 * k)
        u = np.array([np.cos(th), np.sin(th)])
        pv = np.array([-u[1], u[0]])
        c = np.array([CX, CY]) + u * (TURN_R - 58)
        quad = [c + u * 52 + pv * 26, c + u * 52 - pv * 26,
                c - u * 52 - pv * 26, c - u * 52 + pv * 26]
        ax.add_patch(Polygon(quad, closed=True, facecolor="#2B2B2B",
                             edgecolor="none", zorder=3.4))
    ax.add_patch(Circle((CX, CY), 50, facecolor="#FFFFFF", edgecolor=C_TURN_E,
                        lw=1.6, zorder=3.45))
    ax.add_patch(Circle((CX, CY), 14, facecolor=C_TURN_E, edgecolor="none",
                        zorder=3.46))


def draw_pillars(ax):
    for (x, y) in PILLARS:
        ax.add_patch(Circle((x, y), PILL_R + 16, facecolor="#8E8E8E", alpha=0.30,
                            edgecolor="none", zorder=3.5))
        ax.add_patch(Circle((x, y), PILL_R, facecolor=C_PILLAR, edgecolor="#1E1E1E",
                            lw=1.4, hatch="\\\\\\", zorder=3.55))


# ================================================================ 标注
def dim(ax, p1, p2, txy, text, rot=0, fs=9.5, ext=None):
    ax.annotate("", xy=tuple(p1), xytext=tuple(p2),
                arrowprops=dict(arrowstyle="<|-|>", color=C_DIM, lw=0.85,
                                shrinkA=0, shrinkB=0, mutation_scale=7), zorder=5.5)
    for q in (ext or []):
        ax.plot([q[0], q[2]], [q[1], q[3]], color=C_DIM, lw=0.55,
                ls=(0, (2, 3)), zorder=5.4)
    ax.text(txy[0], txy[1], text, ha="center", va="center", fontsize=fs,
            color=C_DIM, rotation=rot, zorder=5.6,
            bbox=dict(boxstyle="round,pad=0.18", fc="white", ec="none", alpha=0.9))


def leader(ax, xy, txy, text, fs=8.8, ha="center", va="center"):
    ax.annotate(text, xy=xy, xytext=txy, ha=ha, va=va, fontsize=fs,
                color="#2B2B2B", zorder=5.7, linespacing=1.45,
                arrowprops=dict(arrowstyle="-", color="#6A6A6A", lw=0.8,
                                shrinkA=3, shrinkB=3),
                bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="#CFCFCF", lw=0.6))


def draw_dims(ax):
    dim(ax, (0, -470), (2400, -470), (1200, -470), "2400.0",
        ext=[(0, -160, 0, -490), (2400, -160, 2400, -490)])
    dim(ax, (2610, 0), (2610, 2400), (2610, 1200), "2400.0", rot=90,
        ext=[(2450, 0, 2630, 0), (2450, 2400, 2630, 2400)])
    dim(ax, (150, -280), (450, -280), (300, -280), "300.0",
        ext=[(150, 150, 150, -300), (450, 150, 450, -300)])
    dim(ax, (0, 80), (150, 80), (75, 108), "150", fs=8.2)
    dim(ax, (910, 2690), (1490, 2690), (1200, 2690), "580.0",
        ext=[(910, 2550, 910, 2710), (1490, 2550, 1490, 2710)])
    dim(ax, (1660, 2250), (1660, 2550), (1660, 2400), "300.0", rot=90,
        ext=[(1490, 2250, 1680, 2250), (1490, 2550, 1680, 2550)])


def draw_labels(ax):
    leader(ax, PILLARS[1], (698, 2000), "立柱 Ø200×4\n高 400，遮挡视线", fs=8.5)
    leader(ax, (1200, 1820), (1508, 2032), "减速带 300×80\n低摩擦，限速 0.3 m/s", fs=8.5)
    leader(ax, (-60, 1430), (-372, 1560), "工位模块 580×300\n跨墙 150 / 150，赛前可整体换位", fs=8.5)
    leader(ax, (CX, CY - TURN_R), (2180, 640), "中央风车盘 Ø700\n放入一件自动转 90°", fs=8.5)
    leader(ax, (556, 556), (330, 700), "对角高速通道\n净宽 300，全程无遮挡", fs=8.5)


# ================================================================ 图 1
def fig_layout(path):
    fig, ax = plt.subplots(figsize=(11.6, 11.4))
    ax.set_aspect("equal")
    ax.axis("off")
    floor = draw_floor(ax)
    draw_corridors(ax, floor)
    draw_hazards(ax)
    draw_turn(ax)
    draw_pillars(ax)
    draw_starts(ax)
    draw_modules(ax)
    draw_dims(ax)
    draw_labels(ax)

    ax.text(0.5, 0.962, "《风车·四象》竞赛场地 · 布局图", transform=fig.transFigure,
            ha="center", va="center", fontsize=20, fontweight="bold", color="#1B1B1B")
    ax.text(0.5, 0.928, "2400 × 2400 mm ｜ 四角启停 · 四边工位 · 中央风车盘"
                        " ｜ 单位 mm，原点在场地左下角", transform=fig.transFigure,
            ha="center", va="center", fontsize=10.5, color="#5E5E5E")
    ax.text(0.5, 0.025, "四个边缘工位用同一套 580×300 接口；赛前互换位置并把本局排列写进任务牌二维码 —— 路径不能硬编码。",
            transform=fig.transFigure, ha="center", va="center", fontsize=9.2,
            color="#6A6A6A")

    ax.set_xlim(-560, 2820)
    ax.set_ylim(-560, 2830)
    fig.subplots_adjust(left=0.01, right=0.99, top=0.905, bottom=0.045)
    fig.savefig(path, dpi=200, facecolor="white")
    plt.close(fig)


# ================================================================ 图 2
ROUTE = [(330, 330), (452, 620), (450, 1200), (1200, 1950),
         (1950, 1200), (1200, 1650), (1650, 1200), (1200, 450), (330, 330)]
STOPS = [((450, 1200), "1"), ((1200, 1950), "2"), ((1950, 1200), "3"),
         ((1200, 1650), "4"), ((1650, 1200), "5"), ((1200, 450), "6")]
LEGEND = [("1", "读任务牌（西）", "拿到本局四工位排列"),
          ("2", "取原料（北）", "夹爪取 1 件，放入车载转盘"),
          ("3", "粗加工（东）", "压入夹具并原地转 90°"),
          ("4", "入盘（北槽）", "放入风车盘北侧刀槽"),
          ("5", "取成品（东槽）", "盘自动转 90°，东槽即成品"),
          ("6", "交付（南）", "送入成品暂存，回启停区 A")]


def fig_flow(path):
    fig, ax = plt.subplots(figsize=(10.9, 12.8))
    ax.set_aspect("equal")
    ax.axis("off")
    floor = draw_floor(ax)
    draw_corridors(ax, floor)
    draw_hazards(ax)
    draw_turn(ax)
    draw_pillars(ax)
    draw_starts(ax, label=False)
    draw_modules(ax, label=True, fs=8.4)
    for (name, x, y, d) in START_ZONES:
        ax.text(x + START / 2 - d[0] * 78, y + START / 2 - d[1] * 78, name,
                ha="center", va="center", color="white", fontsize=13,
                fontweight="bold", zorder=3.5)
    ax.text(300, 118, "起点 A", ha="center", va="center", fontsize=9.5,
            color="#6B6B6B", zorder=5.2,
            bbox=dict(boxstyle="round,pad=0.2", fc="white", ec="#DDDDDD", lw=0.5))

    pts = np.array(ROUTE, float)
    ax.plot(pts[:, 0], pts[:, 1], color="white", lw=10, alpha=0.95,
            solid_capstyle="round", solid_joinstyle="round", zorder=4.6)
    ax.plot(pts[:, 0], pts[:, 1], color=C_ROUTE, lw=3.6,
            solid_capstyle="round", solid_joinstyle="round", zorder=4.7)
    for i in range(len(pts) - 1):
        a, b = pts[i], pts[i + 1]
        if np.linalg.norm(b - a) < 10:
            continue
        m = a + (b - a) * 0.5
        u = (b - a) / np.linalg.norm(b - a)
        pv = np.array([-u[1], u[0]])
        ax.add_patch(Polygon([m + u * 48, m - u * 28 + pv * 27, m - u * 28 - pv * 27],
                             closed=True, facecolor=C_ROUTE, edgecolor="none",
                             zorder=4.75))
    for (p, num) in STOPS:
        ax.add_patch(Circle(p, 72, facecolor="white", edgecolor=C_ROUTE, lw=2.4,
                            zorder=4.9))
        ax.text(p[0], p[1], num, ha="center", va="center", fontsize=13.5,
                fontweight="bold", color=C_ROUTE, zorder=5.0)

    th = np.linspace(np.deg2rad(212), np.deg2rad(328), 60)
    ax.plot(CX + 204 * np.cos(th), CY + 204 * np.sin(th), color="#5C4000",
            lw=1.8, ls=(0, (4, 3)), zorder=5.1)
    ax.annotate("", xy=(CX + 204 * np.cos(np.deg2rad(212)), CY + 204 * np.sin(np.deg2rad(212))),
                xytext=(CX + 204 * np.cos(np.deg2rad(228)), CY + 204 * np.sin(np.deg2rad(228))),
                arrowprops=dict(arrowstyle="-|>", color="#5C4000", lw=1.8), zorder=5.1)
    ax.text(CX - 20, CY - 320, "盘转 90°", ha="center", va="center", fontsize=9,
            color="#5C4000", zorder=5.1,
            bbox=dict(boxstyle="round,pad=0.2", fc="#FFE9A8", ec="none"))

    # ---- 底部图例
    ax.add_patch(Rectangle((40, -830), 2320, 400, facecolor="#FAFAFA",
                           edgecolor="#D5D5D5", lw=1.0, zorder=6.0))
    for i, (num, t1, t2) in enumerate(LEGEND):
        col, row = i // 3, i % 3
        x0 = 130 + col * 1130
        y0 = -505 - row * 128
        ax.add_patch(Circle((x0, y0), 46, facecolor="white", edgecolor=C_ROUTE,
                            lw=2.0, zorder=6.2))
        ax.text(x0, y0, num, ha="center", va="center", fontsize=10,
                fontweight="bold", color=C_ROUTE, zorder=6.3)
        ax.text(x0 + 74, y0 + 28, t1, ha="left", va="center", fontsize=10,
                fontweight="bold", color="#1B1B1B", zorder=6.3)
        ax.text(x0 + 74, y0 - 26, t2, ha="left", va="center", fontsize=8.2,
                color="#707070", zorder=6.3)

    ax.text(0.5, 0.966, "《风车·四象》任务流线图（示例局）", transform=fig.transFigure,
            ha="center", va="center", fontsize=20, fontweight="bold", color="#1B1B1B")
    ax.text(0.5, 0.935, "本局排列：任务牌=西 ／ 原料=北 ／ 粗加工=东 ／ 交付=南"
                        " ｜ 换一局只换排列，路径程序不变",
            transform=fig.transFigure, ha="center", va="center", fontsize=10.5,
            color="#5E5E5E")
    ax.set_xlim(-380, 2700)
    ax.set_ylim(-900, 2760)
    fig.subplots_adjust(left=0.015, right=0.985, top=0.918, bottom=0.01)
    fig.savefig(path, dpi=200, facecolor="white")
    plt.close(fig)


if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__))
    fig_layout(os.path.join(here, "new_field_layout.png"))
    fig_flow(os.path.join(here, "new_field_flow.png"))
    print("done")


