import os
from pathlib import Path
import tempfile
import unittest
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from stage_grassy import ROOT_FILES, SOURCE_DIRS, stage
from check_install_manifest import checked_paths


class IsolationTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.source = Path(self.tmp.name) / "source"
        self.source.mkdir()
        for name in ROOT_FILES:
            (self.source / name).write_text("Klassy\n", encoding="utf-8")
        for name in SOURCE_DIRS:
            (self.source / name).mkdir()
        self.destination = Path(self.tmp.name) / "grassy-source"

    def test_identity_and_preset_compatibility(self):
        settings = self.source / "libbreezecommon" / "klassy.kcfg"
        settings.write_text(
            'klassy/klassyrc\nnamespace Klassy\nKLASSYCOMMON_EXPORT\n'
            'org.kde.Klassy.Style\nKlassyIconThemeInherits\n'
            'Klassy Window Decoration Preset File\n', encoding="utf-8")
        (self.source / "colors" / "OpalFruitsLight.colors").write_text(
            "ColorScheme=OpalFruitsLight\n", encoding="utf-8")
        stage(self.source, self.destination)
        generated = self.destination / "libbreezecommon" / "grassy.kcfg"
        self.assertEqual(generated.read_text(), (
            'grassy/grassyrc\nnamespace Grassy\nGRASSYCOMMON_EXPORT\n'
            'org.kde.Grassy.Style\nKlassyIconThemeInherits\n'
            'Klassy Window Decoration Preset File\n'))
        self.assertTrue((self.destination / "GrassyConfig.cmake.in").exists())
        self.assertTrue((self.destination / "colors" / "GrassyOpalFruitsLight.colors").exists())
        self.assertTrue(settings.exists())
        timestamp = generated.stat().st_mtime_ns
        stage(self.source, self.destination)
        self.assertEqual(timestamp, generated.stat().st_mtime_ns)
        settings.unlink()
        stage(self.source, self.destination)
        self.assertFalse(generated.exists())

    def test_symlink_and_binary_asset(self):
        asset = self.source / "kdecoration" / "Klassy_logo.svg"
        asset.write_text("<svg/>", encoding="utf-8")
        link = self.source / "kstyle" / "Klassy_logo.svg"
        link.symlink_to("../kdecoration/Klassy_logo.svg")
        binary = self.source / "icons" / "klassy.png"
        binary.write_bytes(b"\x00\xffklassy")
        stage(self.source, self.destination)
        generated = self.destination / "kstyle" / "Grassy_logo.svg"
        self.assertEqual(os.readlink(generated), "../kdecoration/Grassy_logo.svg")
        self.assertTrue(generated.exists())
        self.assertEqual((self.destination / "icons" / "grassy.png").read_bytes(), binary.read_bytes())

    def test_unowned_destination_refused(self):
        self.destination.mkdir()
        with self.assertRaises(RuntimeError):
            stage(self.source, self.destination)

    def test_manifest(self):
        prefix = Path(self.tmp.name) / "usr"
        prefix.mkdir()
        manifest = Path(self.tmp.name) / "manifest"
        good = prefix / "lib" / "org.kde.grassy.so"
        manifest.write_text(str(good) + "\n")
        self.assertEqual(checked_paths(manifest, prefix), [good])
        for invalid in (
            prefix / "lib" / "org.kde.klassy.so",
            prefix / "share" / "OpalFruitsLight.colors",
            prefix / "grassy" / ".." / "klassy",
            Path("/outside/grassy.so"),
        ):
            manifest.write_text(str(good) + "\n" + str(invalid) + "\n")
            with self.assertRaises(RuntimeError):
                checked_paths(manifest, prefix)


if __name__ == "__main__":
    unittest.main()
