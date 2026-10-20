"""Read the fixed QML form structure for the browser inventory; no QML execution."""
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
QML = ROOT / "gui/src/qml"


def extract(filename):
    lines = (QML / filename).read_text(encoding="utf-8").splitlines()
    controls = []
    group = "Display"
    label = ""
    i = 0
    while i < len(lines):
        line = lines[i]
        comment = re.fullmatch(r"\s*// ([A-Za-z -]+)\s*", line)
        if comment:
            group = comment[1]
        start = re.match(r"(\s*)(?:C\.)?(Label|ComboBox|CheckBox|Slider|TextField|Button)\s*\{", line)
        if not start:
            i += 1
            continue
        indent, kind = len(start[1]), start[2]
        end = i + 1
        depth = 1
        props = {}
        while end < len(lines):
            entry = lines[end]
            m = re.match(r"\s*([\w.]+): (.*)", entry)
            if m and depth == 1:
                props[m[1]] = m[2]
            clean = re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//.*', "", entry)
            depth += clean.count("{") - clean.count("}")
            if depth == 0:
                break
            end += 1
        block = "\n".join(lines[i:end + 1])
        strings = lambda s: re.findall(r'qsTr\("([^"]*)"\)', s)
        text = strings(props.get("text", ""))
        if kind == "Label":
            if text and text[0].startswith("(") and controls:
                controls[-1]["default"] = text[0][1:-1]
            elif text and not text[0].startswith("%"):
                label = text[0].rstrip(":")
        else:
            own = text[0] if text else ""
            refs = sorted(set(re.findall(r"Chiaki.settings\.(\w+)", block)))
            controls.append(dict(label=label or own, kind=kind, group=group, text=own,
                                 options=strings(props.get("model", "")), props=props,
                                 refs=refs, source=f"{filename}:{i + 1}", default=""))
        i = end + 1
    return controls


if __name__ == "__main__":
    inventory = {name: extract(name) for name in [
        "DisplaySettingsDialog.qml", "PlaceboSettingsDialog.qml", "PlaceboColorMappingDialog.qml"
    ]}
    for name, controls in inventory.items():
        source = (QML / name).read_text(encoding="utf-8")
        expected = len(re.findall(r"^\s*(?:C\.)?(?:ComboBox|CheckBox|Slider|TextField|Button)\s*\{", source, re.M))
        assert len(controls) == expected, (name, len(controls), expected)
    (Path(__file__).parent / "advanced-inventory.json").write_text(json.dumps(inventory, indent=2), encoding="utf-8")
    pages = {"DisplaySettingsDialog.qml": "Display", "PlaceboSettingsDialog.qml": "Renderer", "PlaceboColorMappingDialog.qml": "Color mapping"}
    names = {}
    for controls in inventory.values():
        for c in controls:
            for key in ["value", "checked", "currentIndex"]:
                m = re.fullmatch(r"Chiaki.settings\.(\w+)", c["props"].get(key, ""))
                if m:
                    names[m[1]] = c["label"]
    def condition(value):
        value = value.replace("kneeadaptation.visible", "Tone-mapping Function is Spline, St2094-40 or St-2094-10")
        value = value.replace("targetPeakChoice.currentIndex == 1", "Target Peak is Numeric Value")
        value = value.replace("targetContrastChoice.currentIndex == 2", "Target Contrast is Numeric Value")
        value = re.sub(r"Chiaki.settings\.(\w+)", lambda m: names.get(m[1], m[1]), value)
        value = re.sub(r"PlaceboColorMappingDialog\.\w+\.(\w+)", lambda m: re.sub(r"([a-z])([A-Z])", r"\1 \2", m[1]), value)
        value = value.replace(" == 0", " is Custom").replace(" == ", " is ").replace("&&", "and").replace("||", "or")
        return "When " + value.replace("(", "").replace(")", "") if value else ""
    result = {}
    for filename, controls in inventory.items():
        groups = {}
        for c in controls:
            p = c["props"]
            # The native URL field is permanently hidden; the copy command is visible.
            if p.get("visible") == "false":
                continue
            meta = {"source": c["source"], "refs": c["refs"], "defaultText": c["default"]}
            if p.get("visible") and "options_url" not in p["visible"]:
                meta["condition"] = condition(p["visible"])
            if c["kind"] == "ComboBox":
                kind = c["options"]
                initial = next((v for v in kind if v == c["default"] or c["default"].startswith(v + ",")), kind[0])
            elif c["kind"] == "CheckBox":
                kind, initial = "toggle", c["default"] == "Checked"
            elif c["kind"] == "Slider":
                kind = "number"
                initial = float(re.match(r"-?[\d.]+", c["default"])[0])
                for source, key in [("from","min"),("to","max"),("stepSize","step")]:
                    meta[key] = float(p[source])
                assert meta["min"] <= initial <= meta["max"], c["label"]
            else:
                kind, initial = "action", c["text"]
            if c["label"] == "Color Mapping Settings":
                meta["page"] = "Color mapping"
            description = "nits" if c["label"] == "Target Peak Value" else ""
            label = c["label"]
            if c["text"] == "Click to Re-Copy URL":
                label = "Copy documentation URL again"
            groups.setdefault(c["group"], []).append([label,kind,initial,description,meta])
        result[pages[filename]] = list(groups.items())
    target = ROOT / "gui/prototype/settings/advanced-data.js"
    target.write_text("// Extracted from native QML for this disposable prototype.\nObject.assign(sections, " + json.dumps(result, ensure_ascii=False, indent=2) + ");\n", encoding="utf-8")
    for name, controls in inventory.items():
        print(name, len(controls))
        for c in controls:
            print(c["source"], c["label"], c["kind"], c["default"], c["refs"])
