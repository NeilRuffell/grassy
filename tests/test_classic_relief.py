"""Check the source-level classic-relief settings and bundled presets."""
import configparser
from pathlib import Path
import re
import unittest
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
SCHEMA = ROOT / "libbreezecommon/breezesettingsdata.kcfg"
UI = ROOT / "kdecoration/config/ui/breezeconfigurationui.ui"
CONFIG = ROOT / "kdecoration/config/breezeconfigwidget.cpp"
NS = {"k": "http://www.kde.org/standards/kcfg/1.0"}


class ClassicReliefTests(unittest.TestCase):
    def setUp(self):
        self.entries = ET.parse(SCHEMA).findall(".//k:entry", NS)

    def test_classic_settings_are_opt_in_and_wired(self):
        ui = ET.parse(UI)
        config = CONFIG.read_text(encoding="utf-8")
        classic = [e for e in self.entries if e.attrib["name"].startswith("Classic")]
        self.assertEqual(len(classic), 14)
        for entry in classic:
            name = entry.attrib["name"]
            field = name[0].lower() + name[1:]
            if entry.attrib["type"] == "Bool":
                self.assertEqual(entry.find("k:default", NS).text, "false")
            self.assertIsNotNone(ui.find(f".//widget[@name='{field}']"))
            self.assertIn(f"m_internalSettings->{field}()", config)
            self.assertIn(f"m_internalSettings->set{name}(", config)

    def test_classic_renderer_uses_procedural_primitives(self):
        renderer = (ROOT / "kdecoration/classicrelief.h").read_text(encoding="utf-8")
        for forbidden in (".png", "QPixmap", "QImage", "QBitmap", "QSvgRenderer", "drawPixmap", "drawImage"):
            self.assertNotIn(forbidden, renderer)
        for required in ("QLinearGradient", "QPainterPath", "steppedPath", "glyph", "fillPath"):
            self.assertIn(required, renderer)
        for source, method in (("breezedecoration.cpp", "ClassicRelief::steppedPath"),
                               ("breezebutton.cpp", "ClassicRelief::glyph")):
            self.assertIn(method, (ROOT / "kdecoration" / source).read_text(encoding="utf-8"))

    def test_gloss_is_one_four_stop_fill(self):
        renderer = (ROOT / "kdecoration/classicrelief.h").read_text(encoding="utf-8")
        gloss = renderer.split("inline void gloss(", 1)[1].split("inline void titlebar(", 1)[0]
        self.assertEqual(gloss.count("QLinearGradient "), 1)
        self.assertEqual(gloss.count("gradient.setColorAt("), 4)
        self.assertEqual(gloss.count("p->fillRect("), 1)
        self.assertIn("p->fillRect(r, gradient)", gloss)
        self.assertIn("gradient.setColorAt(0.499, shade(base, b))", gloss)
        self.assertIn("gradient.setColorAt(0.501, shade(base, c))", gloss)

    def test_classic_geometry_uses_native_icon_size(self):
        decoration = (ROOT / "kdecoration/breezedecoration.cpp").read_text(encoding="utf-8")
        sizes = decoration.split("void Decoration::calculateIconSizes()", 1)[1].split("qreal Decoration::captionHeight", 1)[0]
        classic = sizes.index("if (m_internalSettings->classicGeometry()")
        self.assertLess(sizes.index("switch (m_internalSettings->iconSize())"), classic)
        self.assertLess(sizes.index("switch (m_internalSettings->systemIconSize())"), classic)
        self.assertIn("baseSize / (1.8 * gridUnit)", sizes)
        self.assertIn("scaleBackgroundPercent() / 112.0", sizes)
        for expression in ("24 * backgroundFactor", "16 * iconFactor", "20 * backgroundFactor"):
            self.assertIn(expression, sizes)
        button = (ROOT / "kdecoration/breezebutton.cpp").read_text(encoding="utf-8")
        self.assertIn("glyphSize = m_iconSize.width() * (10.0 / 16.0)", button)
        self.assertNotIn("glyphSize = qMin(m_iconSize.width(), 10", button)

    def test_human_button_outline_and_glyphs(self):
        renderer = (ROOT / "kdecoration/classicrelief.h").read_text(encoding="utf-8")
        button = renderer.split("inline void button(", 1)[1]
        self.assertIn("outerPath.subtracted(innerPath)", button)
        self.assertIn("overlay(Qt::black, 0.45)", button)
        self.assertIn("innerPath.subtracted(corePath)", button)
        self.assertIn("overlay(Qt::white, 0.40)", button)
        self.assertIn("overlay(Qt::black, 0.20)", button)
        self.assertNotIn("pressed ? dark : light", button)
        glyph = renderer.split("inline bool glyph(", 1)[1].split("inline void gloss(", 1)[0]
        self.assertIn("relief.createStroke(symbol).united(symbol)", glyph)
        self.assertIn("stroker.setWidth(2.3)", glyph)
        self.assertNotIn("p->translate(0.6, 0.8)", glyph)
        source = (ROOT / "kdecoration/breezebutton.cpp").read_text(encoding="utf-8")
        self.assertIn("backgroundSize * 1.20", source)
        self.assertIn("backgroundSize * 0.90", source)

    def test_maximize_restore_use_closed_vector_outlines(self):
        renderer = (ROOT / "kdecoration/classicrelief.h").read_text(encoding="utf-8")
        glyph = renderer.split("inline bool glyph(", 1)[1].split("inline void gloss(", 1)[0]
        windows = glyph.split("if (type == DecorationButtonType::Maximize) {", 1)[1].split(
            "// Keep the already accepted minimize and close glyphs unchanged.", 1)[0]
        self.assertIn("windows.addRect(QRectF(1.8, 1.8, 6.4, 6.4))", windows)
        self.assertIn("windows.addRect(QRectF(1.5, 3.6, 4.9, 4.9))", windows)
        self.assertIn("if (!checked)", windows)
        self.assertEqual(windows.count("p->drawPath(windows)"), 2)
        self.assertIn("p->setBrush(Qt::NoBrush)", windows)
        self.assertNotIn("simplified()", windows)
        self.assertNotIn("united(", windows)
        self.assertIn("p->restore();", windows)

    def test_classic_background_respects_native_checked_state(self):
        source = (ROOT / "kdecoration/breezebutton.cpp").read_text(encoding="utf-8")
        classic = source.split("void Button::paintSmallSizedButtonBackground(", 1)[1].split(
            "qreal penWidth =", 1)[0]
        self.assertIn("bool reliefPressed = isPressed()", classic)
        self.assertIn("bool reliefHovered = hovered()", classic)
        self.assertIn("if (!reliefPressed && isChecked() && isCheckable())", classic)
        self.assertIn("buttonStateChecked(active) == InternalSettings::EnumButtonStateChecked::Hover", classic)
        self.assertIn("reliefPressed = !reliefHovered", classic)
        self.assertIn("active, reliefHovered, reliefPressed", classic)
        checkable = source.split("bool Button::isCheckable() const", 1)[1].split("bool Button::hovered()", 1)[0]
        self.assertNotIn("DecorationButtonType::Maximize", checkable)

    def test_human_presets_use_valid_keys_and_enums(self):
        keys = {k: None for k in ("KwinBorderSize", "KwinButtonsOnLeft", "KwinButtonsOnRight")}
        for entry in self.entries:
            name = entry.attrib["name"]
            names = [name]
            if "$(" in name:
                names = [
                    re.sub(r"\$\([^)]*\)", value.text, name)
                    for value in entry.findall("k:parameter/k:values/k:value", NS)
                ]
            for key in names:
                keys[key] = entry
        path = ROOT / "kdecoration/config/presets/Human.klpw"
        preset = configparser.ConfigParser(interpolation=None)
        preset.optionxform = str
        preset.read(path, encoding="utf-8")
        group = preset["Windeco Preset Human"]
        for key, value in group.items():
            self.assertIn(key, keys, key)
            entry = keys[key]
            if entry is not None:
                self.assertNotEqual(entry.attrib["type"], "Color", key)
                if entry.attrib["type"] == "Enum":
                    self.assertIn(value, [c.attrib["name"] for c in entry.findall("k:choices/k:choice", NS)])
            self.assertFalse(key.startswith("ButtonOverrideColors"), key)
        for name in ("ClassicGradientActive", "ClassicGradientInactive",
                     "ClassicButtonBevel", "ClassicFrameBevel",
                     "ClassicTitleShadow", "ClassicGeometry", "ClassicGlyphs"):
            self.assertTrue(group.getboolean(name))
        self.assertFalse(group.getboolean("ClassicCustomColors"))
        self.assertEqual(group.getint("ClassicFrameWidth"), 5)
        self.assertEqual(group["ButtonIconColorsActive"], "TitleBarText")
        self.assertEqual(group["ButtonIconColorsInactive"], "TitleBarText")
        self.assertFalse(group.getboolean("UseTitleBarColorForAllBorders"))

    def test_retired_bundled_presets_not_reimported(self):
        directory = ROOT / "kdecoration/config/presets"
        model = (ROOT / "libbreezecommon/presetsmodel.cpp").read_text(encoding="utf-8")
        for filename in ("Human_Jaunty.klpw", "Human_Orange.klpw"):
            self.assertFalse((directory / filename).exists())
            self.assertIn(f'presetFile == QStringLiteral("{filename}")', model)
        self.assertIn(":classic-relief-3", model)
        self.assertIn("if (isPresetFromFilePresent(presetsConfig, presetFile, presetName))", model)


if __name__ == "__main__":
    unittest.main()
