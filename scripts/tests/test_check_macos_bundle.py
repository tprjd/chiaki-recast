import importlib.util
import os
import sys
sys.dont_write_bytecode = True
from pathlib import Path
import plistlib
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("bundle_check", Path(__file__).parents[1] / "check-macos-bundle.py")
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


class BundleCheck(unittest.TestCase):
    def test_native_notice_copy_rejects_missing_text(self):
        spec = importlib.util.spec_from_file_location("notices", Path(__file__).parents[1] / "macos-notices.py")
        notices = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(notices)
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "keg"
            source.mkdir()
            with self.assertRaisesRegex(ValueError, "No license texts"):
                notices.copy_notices(source, Path(tmp) / "notices")
            (source / "COPYING.txt").write_text("exact dependency notice")
            notices.copy_notices(source, Path(tmp) / "notices")
            self.assertEqual((Path(tmp) / "notices/COPYING.txt").read_text(), "exact dependency notice")

    def make_bundle(self, root):
        app = root / "chiaki.app"
        paths = [
            checker.HELPER + "/MacOS/QtWebEngineProcess",
            "Contents/MacOS/chiaki", "Contents/MacOS/libSDL3.dylib", "Contents/MacOS/libSDL2-2.0.0.dylib",
            "Contents/PlugIns/platforms/libqcocoa.dylib", "Contents/Resources/qt.conf",
            "Contents/Resources/licenses/frontend/THIRD-PARTY-NOTICES.txt",
            "Contents/Resources/vulkan/icd.d/MoltenVK_icd.json", "Contents/Resources/vulkan/icd.d/libMoltenVK.dylib",
            "Contents/Frameworks/libvulkan.1.dylib",
        ]
        paths += ["Contents/Frameworks/" + n + ".framework/" + n for n in checker.FRAMEWORKS]
        paths += ["Contents/Frameworks/QtWebEngineCore.framework/Resources/" + n for n in (
            *checker.RESOURCES, "v8_context_snapshot.x86_64.bin", "v8_context_snapshot.arm64.bin", "qtwebengine_locales/en-US.pak")]
        for relative in paths:
            p = app / relative
            p.parent.mkdir(parents=True, exist_ok=True)
            p.write_text("fixture")
            p.chmod(0o755)
        with (app / "Contents/Info.plist").open("wb") as stream:
            plistlib.dump(dict(NSMicrophoneUsageDescription="Mic", NSSupportsAutomaticGraphicsSwitching=True), stream)
        (app / checker.HELPER / "Frameworks").symlink_to(os.path.relpath(app / "Contents/Frameworks", app / checker.HELPER), target_is_directory=True)
        (app / "Contents/Frameworks/vulkan").symlink_to("libvulkan.1.dylib")
        return app

    def test_complete_layout_for_both_architectures(self):
        with tempfile.TemporaryDirectory() as tmp:
            app = self.make_bundle(Path(tmp))
            checker.check_bundle(app, "x86_64")
            checker.check_bundle(app, "arm64")
            with self.assertRaisesRegex(ValueError, "Unsupported"):
                checker.check_bundle(app, "i386")

    def test_missing_resources_and_unsafe_links_are_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            app = self.make_bundle(Path(tmp))
            for relative in ["Resources/qtwebengine_resources.pak", "Resources/qtwebengine_locales/en-US.pak", "Resources/v8_context_snapshot.arm64.bin"]:
                path = app / "Contents/Frameworks/QtWebEngineCore.framework" / relative
                path.unlink()
                with self.assertRaisesRegex(ValueError, "Missing"):
                    checker.check_bundle(app, "arm64")
                path.write_text("fixture")
            helper = app / checker.HELPER / "MacOS/QtWebEngineProcess"
            helper.chmod(0o644)
            with self.assertRaisesRegex(ValueError, "not executable"):
                checker.check_bundle(app)
            helper.chmod(0o755)
            link = app / checker.HELPER / "Frameworks"
            link.unlink()
            link.symlink_to(Path(tmp), target_is_directory=True)
            with self.assertRaisesRegex(ValueError, "symlink"):
                checker.check_bundle(app)

    def test_missing_webengine_helper_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            app = Path(tmp) / "chiaki.app"
            app.mkdir()
            with self.assertRaisesRegex(ValueError, "QtWebEngineProcess"):
                checker.check_bundle(app)


if __name__ == "__main__":
    unittest.main()
