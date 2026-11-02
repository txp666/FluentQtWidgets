"""Measure qfluentwidgets.multimedia (PyQt-Fluent-Widgets 1.11.3) for tests/tst_media.cpp.

The media classes need Qt Multimedia, which the default C++ build doesn't have, so they are not in
python_sizes.json (tst_python_parity runs in every build). This prints the sizes and child geometries that
tst_media.cpp checks; run it with the same environment as measure_python.py:

    <venv>/bin/python -I tools/parity/measure_python_media.py
"""
import json
import os
import sys

os.environ["QT_QPA_PLATFORM"] = "offscreen"
os.environ.pop("QT_SCALE_FACTOR", None)

from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont
from PyQt5.QtTest import QTest
from PyQt5.QtWidgets import QApplication, QWidget

QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, False)
app = QApplication(sys.argv[:1])
app.setStyle("Fusion")
font = QFont()
font.setFamilies(["Segoe UI", "Microsoft YaHei", "PingFang SC"])
font.setPixelSize(14)
app.setFont(font)

from qfluentwidgets import Theme, setTheme  # noqa: E402
from qfluentwidgets.multimedia import SimpleMediaPlayBar, StandardMediaPlayBar, VideoWidget  # noqa: E402
from qfluentwidgets.multimedia.media_play_bar import VolumeView  # noqa: E402

setTheme(Theme.LIGHT)


def rect(w, parent):
    p = w.mapTo(parent, w.rect().topLeft())
    return [p.x(), p.y(), w.width(), w.height()]


def shown(widget, size=None):
    if size:
        widget.resize(*size)
    widget.show()
    QTest.qWaitForWindowExposed(widget)
    QTest.qWait(50)
    return widget


out = {}

bar = shown(SimpleMediaPlayBar(), (600, 48))
out["SimpleMediaPlayBar"] = {
    "sizeHint": [bar.sizeHint().width(), bar.sizeHint().height()],
    "minimumSize": [bar.minimumSize().width(), bar.minimumSize().height()],
    "playButton": rect(bar.playButton, bar),
    "progressSlider": rect(bar.progressSlider, bar),
    "volumeButton": rect(bar.volumeButton, bar),
}

bar = shown(StandardMediaPlayBar(), (600, 102))
out["StandardMediaPlayBar"] = {
    "sizeHint": [bar.sizeHint().width(), bar.sizeHint().height()],
    "minimumSize": [bar.minimumSize().width(), bar.minimumSize().height()],
    **{name: rect(getattr(bar, name), bar) for name in (
        "progressSlider", "currentTimeLabel", "remainTimeLabel", "volumeButton", "skipBackButton",
        "playButton", "skipForwardButton")},
}

view = VolumeView()
view.setVolume(30)
shown(view)
out["VolumeView"] = {
    "size": [view.width(), view.height()],
    "muteButton": rect(view.muteButton, view),
    "volumeSlider": rect(view.volumeSlider, view),
    "volumeLabel": rect(view.volumeLabel, view),
}

video = shown(VideoWidget(), (800, 450))
out["VideoWidget"] = {"playBar": rect(video.playBar, video)}

print(json.dumps(out, indent=1))
