from pathlib import Path
import configparser
import json
import sys

run = Path(sys.argv[1])
assert json.loads((run / 'contracts.json').read_text())['allPassed']
before = configparser.ConfigParser(interpolation=None, strict=False)
after = configparser.ConfigParser(interpolation=None, strict=False)
before.read(run / 'baseline.ini', encoding='utf-8')
after.read(run / 'Chiaki' / 'Chiaki.ini', encoding='utf-8')
sections = ['registered_hosts', 'manual_hosts', 'controller_mappings']
for section in sections:
    assert dict(before[section]) == dict(after[section]), section
preferences = ['disconnect_action', 'auto_connect_mac', 'auto_discovery', 'remote_play_ask', 'add_steam_shortcut_ask']
for key in preferences:
    assert before['settings'].get(key) == after['settings'].get(key), key
print(json.dumps({'unchangedSections': sections, 'unchangedPreferences': preferences, 'allPassed': True}))
