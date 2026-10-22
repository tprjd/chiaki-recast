from pathlib import Path
import hashlib, json, os, pickle, subprocess, sys, uuid, winreg
stage=Path(sys.argv[1]).resolve()
output=Path(sys.argv[2]).resolve(); output.mkdir(parents=True,exist_ok=True)
root=r'Software\Chiaki'
def snapshot():
 result={}
 def visit(path):
  with winreg.OpenKey(winreg.HKEY_CURRENT_USER,path) as key:
   children,values,_=winreg.QueryInfoKey(key)
   result[path]=sorted([winreg.EnumValue(key,n) for n in range(values)],key=lambda x:x[0])
   for child in sorted(winreg.EnumKey(key,n) for n in range(children)): visit(path+'\\'+child)
 try: visit(root)
 except FileNotFoundError: pass
 return result
def restore(saved):
 current=snapshot()
 for path in sorted(set(current)-set(saved),key=lambda p:p.count('\\'),reverse=True): winreg.DeleteKey(winreg.HKEY_CURRENT_USER,path)
 for path,values in saved.items():
  with winreg.CreateKey(winreg.HKEY_CURRENT_USER,path) as key:
   keep={x[0] for x in values}
   for name,_,_ in current.get(path,[]):
    if name not in keep: winreg.DeleteValue(key,name)
   for name,value,kind in values: winreg.SetValueEx(key,name,0,kind,value)
def digest(state): return hashlib.sha256(pickle.dumps(state,protocol=4)).hexdigest()
before=snapshot()
backup=Path('C:/Users/david/AppData/Local/Temp/chiaki-web-ui-migration/ticket19-private-settings'); backup.mkdir(exist_ok=True)
(backup/('native-settings-before-'+uuid.uuid4().hex+'.pkl')).write_bytes(pickle.dumps(before,protocol=4))
profile='package-check-'+uuid.uuid4().hex
with winreg.CreateKey(winreg.HKEY_CURRENT_USER,root+'\\Chiaki-'+profile+'\\settings') as key:
 for name in ('auto_discovery','remote_play_ask','add_steam_shortcut_ask'): winreg.SetValueEx(key,name,0,winreg.REG_DWORD,0)
env={key:value for key,value in os.environ.items() if not key.upper().startswith(('QT_','QTWEBENGINE_','QML_','QML2_','CHIAKI_')) and key.upper()!='QTDIR'}
env['PATH']=str(stage)+';'+os.environ['SystemRoot']+'\\System32;'+os.environ['SystemRoot']
command=[str(stage/'chiaki.exe'),'--profile',profile]
(output/'normal-start-command.json').write_text(json.dumps({'command':command,'cwd':str(stage),'PATH':env['PATH'],'qt_overrides':{k:v for k,v in env.items() if k.upper().startswith(('QT_','QTWEBENGINE_','QML_'))},'safety':'Fresh profile with no consoles or credentials; full native settings privately backed up and restored. Ordinary production LAN discovery is not isolated.'},indent=2))
code=1
try:
 with (output/'normal-start-runtime.txt').open('w') as log:
  process=subprocess.Popen(command,cwd=stage,env=env,stdout=log,stderr=subprocess.STDOUT)
  (output/'normal-start-pid.txt').write_text(str(process.pid))
  try: code=process.wait(timeout=120)
  except subprocess.TimeoutExpired:
   process.terminate(); code=process.wait(timeout=15)
   (output/'normal-start-timeout.txt').write_text('Closed the owned app after the observation timeout.')
finally:
 restore(before)
 after=snapshot()
 (output/'normal-start-settings.json').write_text(json.dumps({'restored':before==after,'beforeSha256':digest(before),'afterSha256':digest(after),'backup':'Private external backup retained; not committed.'},indent=2))
 if before!=after: raise RuntimeError('Native settings restoration did not match the original state.')
(output/'normal-start-exit.txt').write_text(str(code))
sys.exit(code)
