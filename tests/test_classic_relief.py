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
