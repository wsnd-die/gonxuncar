# -*- coding: utf-8 -*-
"""生成《智能搬运机器人竞赛 · 初赛／决赛规则对照表》DOCX"""
import os
from docx import Document
from docx.shared import Pt, Cm, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml.ns import qn
from docx.oxml import OxmlElement

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "搬运机器人竞赛规则_初赛决赛对照表.docx")

doc = Document()

# ---------------- 页面：A4 纵向，2.2cm 页边距
sec = doc.sections[0]
sec.page_width, sec.page_height = Cm(21.0), Cm(29.7)
sec.left_margin = sec.right_margin = Cm(2.2)
sec.top_margin = Cm(2.0)
sec.bottom_margin = Cm(2.0)

# ---------------- 默认字体
normal = doc.styles["Normal"]
normal.font.name = "宋体"
normal.font.size = Pt(10.5)
normal.element.rPr.rFonts.set(qn("w:eastAsia"), "宋体")


def set_run(run, name="宋体", size=9, bold=False, color=None):
    run.font.name = name
    run.font.size = Pt(size)
    run.bold = bold
    run._element.rPr.rFonts.set(qn("w:eastAsia"), name)
    if color:
        run.font.color.rgb = RGBColor(*color)


def para(text, name="宋体", size=10.5, bold=False, align=None, before=0, after=6,
         color=None):
    p = doc.add_paragraph()
    if align:
        p.alignment = align
    p.paragraph_format.space_before = Pt(before)
    p.paragraph_format.space_after = Pt(after)
    r = p.add_run(text)
    set_run(r, name, size, bold, color)
    return p


def shade(cell, hexcolor):
    tcPr = cell._tc.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), hexcolor)
    tcPr.append(shd)


def cell_text(cell, text, bold=False, size=9, name="宋体", align=WD_ALIGN_PARAGRAPH.LEFT):
    cell.text = ""
    p = cell.paragraphs[0]
    p.alignment = align
    p.paragraph_format.space_before = Pt(2)
    p.paragraph_format.space_after = Pt(2)
    p.paragraph_format.line_spacing = 1.05
    for i, line in enumerate(text.split("\n")):
        if i:
            p.add_run().add_break()
        r = p.add_run(line)
        set_run(r, name, size, bold)


# ---------------- 标题
t = doc.add_paragraph()
t.alignment = WD_ALIGN_PARAGRAPH.CENTER
t.paragraph_format.space_after = Pt(2)
set_run(t.add_run("智能搬运机器人竞赛 · 初赛／决赛规则对照表"), "黑体", 16, True)

sub = doc.add_paragraph()
sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
sub.paragraph_format.space_after = Pt(10)
set_run(sub.add_run("版本 v2.1　｜　依据新场地（2400×2400，四启停区，中央「四叶风车」作业区）"), "宋体", 9,
        False, (0x59, 0x59, 0x59))

# ---------------- 表格
ROWS = [
    ("比赛时间",
     "3 分钟调试 + 3 分钟比赛（调试余 1 分钟抽签定任务，余 30 s 停机待发）",
     "同上"),
    ("场地与地图",
     "2400×2400 灰色场地；中央为四片 R400 四分之一圆黄色区域，各贴于 1400×1400 方框四角",
     "2400×2400；灰、黄区域重新划分——中央改为「四叶风车」：四片 R400 四分之一圆角点内移至 "
     "Ø1600 圆上（45°/135°/225°/315°，距中心 800），尖端与 Ø800 圆相切，中心保留 Ø200 圆盘；"
     "外轮廓由方框改为 Ø1600 圆"),
    ("启停区",
     "3 处，300×300，位于场地角落",
     "4 处，300×300，四角各一；本队出发区由抽签指定，完赛须返回本队出发区 ★"),
    ("可行驶区域",
     "灰色 + 蓝色；进入其它颜色区域即终止比赛",
     "灰色 + 蓝色；中央黄色作业区（Ø1600）整体禁入，须沿灰色环带绕行 ★"),
    ("地形与障碍物",
     "平整地面；随机出现 Φ50×100 mm 黑色障碍物",
     "新增 4 条 300×80 低摩擦减速带（过带需主动降速）与 4 根 Ø200 固定立柱；"
     "随机障碍物加大至 Φ70×130 mm，出现数量增加 ★"),
    ("物料与夹爪",
     "常规圆柱／方块类物料",
     "圆锥与半球截取部分的组合体；保留圆心检测定位方式，需按物块外形设计专用夹爪；"
     "搬运途中仍不得以手爪夹持运送"),
    ("任务码",
     "两组三位数（如 123-321），红、绿、蓝对应 1、2、3，并全程显示",
     "同上"),
    ("搬运流程",
     "原料区（转盘 6~10 s/圈）→ 粗加工区 → 暂存区，两批各 3 件",
     "同上；第二批在暂存区须码垛于已有物料之上，且上下颜色一致、第一批放置正确"),
    ("外形尺寸",
     "正投影 ≤300×300 mm，高 ≤400 mm（含软质线缆）",
     "同上；折叠臂出发后可自行展开，展开后尺寸不受限制"),
    ("计分规则",
     "① 正确读取并显示任务码 4 分，全程保持显示 2 分；"
     "② 每正确抓取／装载 1 件 2 分；"
     "③ 平面放置按底面与色环中心距换算 1–6 环、非线性计分，码垛成功可获同等环数分；"
     "④ 完赛返回启停区 4 分",
     "分值不变；新场地色环位置与判定距离需重新标定 ★"),
    ("判罚规则",
     "推行物料移动、进入非灰／非蓝区域 → 立即终止比赛",
     "在初赛判罚基础上增加：碰撞立柱、撞倒或推移障碍物、越过黄色作业区 → 终止比赛 ★"),
    ("成绩评定",
     "两次运行取较优者",
     "同上；并列时依次比较现场运行成绩、用时，仍无法区分则抽签"),
]

table = doc.add_table(rows=len(ROWS) + 1, cols=3)
table.style = "Table Grid"
table.alignment = WD_TABLE_ALIGNMENT.CENTER
table.autofit = False

WIDTHS = [Cm(2.6), Cm(6.6), Cm(7.2)]
for row in table.rows:
    for i, w in enumerate(WIDTHS):
        row.cells[i].width = w

hdr = table.rows[0]
for i, name in enumerate(("项　目", "现场初赛", "现场决赛")):
    cell_text(hdr.cells[i], name, bold=True, size=10, name="黑体",
              align=WD_ALIGN_PARAGRAPH.CENTER)
    shade(hdr.cells[i], "D9E2F3")
# 表头跨页重复
trPr = hdr._tr.get_or_add_trPr()
th = OxmlElement("w:tblHeader")
th.set(qn("w:val"), "true")
trPr.append(th)

for r, (a, b, c) in enumerate(ROWS, start=1):
    row = table.rows[r]
    for i in range(3):
        row.cells[i].width = WIDTHS[i]
    cell_text(row.cells[0], a, bold=True, size=9, name="黑体",
              align=WD_ALIGN_PARAGRAPH.CENTER)
    cell_text(row.cells[1], b, size=9)
    cell_text(row.cells[2], c, size=9)
    shade(row.cells[0], "F2F2F2")

# ---------------- 说明
doc.add_paragraph().paragraph_format.space_after = Pt(2)

h = doc.add_paragraph()
h.paragraph_format.space_before = Pt(8)
h.paragraph_format.space_after = Pt(4)
set_run(h.add_run("一、本次调整要点（依据新地图）"), "黑体", 11, True)

POINTS = [
    ("1. 中央黄区重构：",
     "原先四片彼此孤立、尖端悬空的四分之一圆，改为角点落在 Ø1600、尖端与 Ø800 相切的「四叶风车」，"
     "并明确为禁入区。机器人由「穿场直走」变为「沿灰色环带绕行」，路径规划与定位精度权重明显上升。"),
    ("2. 启停区 3 → 4：",
     "出发位置的不确定性增加，回位判定必须与「本队出发区」绑定，否则四角任一处都可能被判为未归位。"),
    ("3. 地形与障碍升级：",
     "新增低摩擦减速带与固定立柱，随机障碍物由 Φ50×100 放大到 Φ70×130，"
     "通行的安全余量变小，对避障与控制稳定性的要求同步提高。"),
    ("4. 物料形态改变：",
     "圆锥与半球组合体没有可用的平面或棱边作夹持面，夹爪需按物块外形重新设计，抓取姿态也需重新示教。"),
]
for k, v in POINTS:
    p = doc.add_paragraph()
    p.paragraph_format.space_after = Pt(4)
    p.paragraph_format.left_indent = Cm(0.4)
    set_run(p.add_run(k), "黑体", 10, True)
    set_run(p.add_run(v), "宋体", 10, False)

h2 = doc.add_paragraph()
h2.paragraph_format.space_before = Pt(6)
h2.paragraph_format.space_after = Pt(4)
set_run(h2.add_run("二、待确认条款（★ 标记项）"), "黑体", 11, True)

Q = [
    "立柱的判罚口径：撞倒即终止，还是仅扣分？建议与障碍物一致，按「撞倒或推移即终止」执行。",
    "减速带是否设硬性限速判罚：建议不设，靠摩擦自然惩罚，避免人为判罚争议。",
    "4 个启停区是否全部启用：若每场只用其中 2 个，回位判定需写明使用哪两个。",
    "新场地色环的环距与判定阈值：需按 Ø1600 作业区重新标定，直接沿用旧场地会普遍偏松或偏严。",
]
for q in Q:
    p = doc.add_paragraph(style="List Bullet")
    p.paragraph_format.space_after = Pt(2)
    set_run(p.add_run(q), "宋体", 10)

# ---------------- 页脚
foot = sec.footer.paragraphs[0]
foot.alignment = WD_ALIGN_PARAGRAPH.CENTER
set_run(foot.add_run("智能搬运机器人竞赛规则对照表 · v2.1"), "宋体", 8, False, (0x80, 0x80, 0x80))

doc.save(OUT)
print("saved", OUT)


