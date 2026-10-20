const fs = require("node:fs");
const vm = require("node:vm");
const assert = require("node:assert/strict");
const path = require("node:path");
const root = path.resolve(__dirname, "../..");
const prototype = path.join(root, "gui/prototype/settings");
const data = vm.runInNewContext(
  fs.readFileSync(path.join(prototype, "settings-data.js"), "utf8") +
  fs.readFileSync(path.join(prototype, "advanced-data.js"), "utf8") +
  ";({sections,keyMappings})"
);
const rows = Object.values(data.sections).flatMap(groups => groups.flatMap(([,rows])=>rows));
const refs = new Set(rows.flatMap(row => [row[4]?.ref, ...(row[4]?.refs || [])]).filter(Boolean));
for (const consoleType of ["PS4","PS5"]) for (const route of ["Local","Remote"]) {
  for (const field of ["resolution","fps","bitrate",...(consoleType==="PS5"?["codec"]:[])]) refs.add(field+route+consoleType);
}
const files = ["SettingsDialog.qml","DisplaySettingsDialog.qml","PlaceboSettingsDialog.qml","PlaceboColorMappingDialog.qml"];
const inventory = {};
for (const file of files) {
  const qml = fs.readFileSync(path.join(root,"gui/src/qml",file),"utf8");
  const writes = [...new Set([...qml.matchAll(/Chiaki\.settings\.(\w+)\s*=(?!=|>)/g)].map(match=>match[1]))];
  const missing = writes.filter(ref=>!refs.has(ref));
  assert.deepEqual(missing, [], `Missing ${file} settings`);
  inventory[file] = writes.length;
}
const nativeSettings = fs.readFileSync(path.join(root,"gui/src/settings.cpp"),"utf8");
const keyBlock = nativeSettings.split("QMap<int, Qt::Key> Settings::GetControllerMapping()")[1].split("// Then fill in")[0];
assert.equal(data.keyMappings.length, [...keyBlock.matchAll(/Qt::Key::Key_/g)].length);
const advancedInventory = JSON.parse(fs.readFileSync(path.join(__dirname,"advanced-inventory.json"),"utf8"));
for (const [file, controls] of Object.entries(advancedInventory)) {
  const visibleControls = controls.filter(c=>c.props.visible!=="false");
  const rendered = rows.filter(row=>row[4]?.source?.startsWith(file+":"));
  assert.equal(rendered.length, visibleControls.length, `${file} control count`);
  for (const c of visibleControls) {
    const row = rendered.find(row=>row[4].source===c.source);
    assert.ok(row, c.source);
    if (c.kind==="ComboBox") assert.deepEqual(Array.from(row[1]), c.options, c.label);
    if (c.kind==="Slider") {
      assert.equal(row[4].min, Number(c.props.from), c.label);
      assert.equal(row[4].max, Number(c.props.to), c.label);
      assert.equal(row[4].step, Number(c.props.stepSize), c.label);
    }
  }
}
const result = {status:"passed", nativeSettingWrites:inventory, keyboardMappings:data.keyMappings.length,
  prototypeRows:rows.length, advancedControls:73,
  excluded:"Permanently hidden documentation URL field; native operating-system pickers and login/registration flows are entry points."};
fs.writeFileSync(path.join(__dirname,"inventory-check.json"),JSON.stringify(result,null,2)+"\n");
console.log(JSON.stringify(result,null,2));
