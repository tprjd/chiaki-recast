"""Use Qt's flat archives with the Python 3.8 builder's pinned aqt release."""
from pathlib import Path
import runpy

from aqt.updater import Updater


update = Updater.update


@classmethod
def update_flat_sdk(cls, target, base_path, installed_desktop_arch_dir):
    base_path = Path(base_path)
    # shortcut: Python 3.8 needs old aqt; remove this adapter with a current builder.
    if (base_path / "mkspecs/qconfig.pri").is_file():
        assert target.version == "6.9.3" and target.os_name == "linux"
        contents = list(base_path.iterdir())
        prefix = base_path / "6.9.3/gcc_64"
        prefix.mkdir(parents=True)
        for item in contents:
            item.rename(prefix / item.name)
    return update(target, base_path, installed_desktop_arch_dir)


Updater.update = update_flat_sdk
if __name__ == "__main__":
    runpy.run_module("aqt", run_name="__main__")
