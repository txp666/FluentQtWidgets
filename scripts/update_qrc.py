import pathlib

root = pathlib.Path(__file__).resolve().parents[1] / "resources"
qss = sorted(root.glob("qss/**/*.qss"))
icons = sorted(root.glob("images/icons/*.svg"))
i18n = sorted(root.glob("i18n/*.qm"))

lines = ["<RCC>", '    <qresource prefix="/qfluentwidgets">']
for path in qss + icons + i18n:
    lines.append(f"        <file>{path.relative_to(root).as_posix()}</file>")
lines += ["    </qresource>", "</RCC>", ""]

(root / "fluentqtwidgets.qrc").write_text("\n".join(lines), encoding="utf-8")
print(f"qrc updated: {len(qss)} qss, {len(icons)} icons, {len(i18n)} qm")
