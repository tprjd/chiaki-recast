import hashlib,json,subprocess
from pathlib import Path
root=Path.cwd()
base='6faf8ff3ebded10c6de035d5b2cc5a4a78efa227'
current=subprocess.check_output(['git','rev-parse','HEAD'],text=True).strip()
def read(ref,path): return subprocess.check_output(['git','show',f'{ref}:{path}']).decode('utf-8')
def function(ref,path,name,indent=''):
    source=read(ref,path)
    start=source.index(name)
    return source[start:source.index('\n'+indent+'}\n',start)+len(indent)+3]
def digest(value): return hashlib.sha256(value.encode()).hexdigest()
checks=[]
for path,names in {
 'gui/src/qmlbackend.cpp':['QmlBackend::createSession(', 'QmlBackend::resolveConnectionTarget(', 'QmlBackend::retryConnection(', 'QmlBackend::createRegistrationContext(', 'QmlBackend::registrationContextCurrent(', 'QmlBackend::registerHost(', 'QmlBackend::autoRegisterHost(', 'QmlBackend::autoRegister(', 'QmlBackend::finishAutoRegister(', 'QmlBackend::connectToHost('],
 'gui/src/setupbridge.cpp':['SetupBridge::autoRegisterConsole(', 'SetupBridge::openRegistration(', 'SetupBridge::submitRegistration(', 'SetupBridge::connectConsole('],
}.items():
    for name in names:
        old=function(base,path,name); new=function(current,path,name); actual_new=new
        if name == 'SetupBridge::connectConsole(':
            new=new.replace('    if (!controllerOperation.isEmpty()) return {{"accepted", false}};\n', '')
        checks.append(dict(file=path,name=name,equal=old==new,tested_sha256=digest(old),merged_sha256=digest(actual_new),
            allowed_delta='Ticket16 controller-operation rejection. The ticket09 fixture has no mapping operation.' if new != actual_new else None))
for path in ['gui/src/applicationcontroller.cpp','gui/web/src/ConnectAction.svelte','gui/web/src/Pairing.svelte']:
    old=read(base,path); new=read(current,path)
    checks.append(dict(file=path,equal=old==new,tested_sha256=digest(old),merged_sha256=digest(new)))
for name in ['void automaticRegistration_data()','void automaticRegistration()']:
    old=function(base,'gui/tests/embeddedsetup.cpp',name,'    '); new=function(current,'gui/tests/embeddedsetup.cpp',name,'    ')
    checks.append(dict(file='gui/tests/embeddedsetup.cpp',name=name,equal=old==new,tested_sha256=digest(old),merged_sha256=digest(new)))
for path in ['gui/src/qmlcontroller.cpp','gui/src/setupwindow.cpp','gui/web/src/navigation.ts','gui/web/src/App.svelte','gui/include/qmlnativeservices.h']:
    old=read('56acff7791d1bb49ad4ade2e063f281a6b4bbbc4',path); new=read(current,path)
    checks.append(dict(file=path,source='ticket16 integration',equal=old==new,tested_sha256=digest(old),merged_sha256=digest(new)))
bridge=read(current,'gui/src/setupbridge.cpp')
assert '{"controllerSetup", controllerSetup}' in bridge
assert '{"connectionBusy", backend->registrationBusy() || backend->qmlWindow()' in bridge
assert 'if (nativeAttempt.value("registrationOnly").toBool() && backend->qmlWindow())' in bridge
assert subprocess.check_output(['git','rev-parse','0f720321:gui']) == subprocess.check_output(['git','rev-parse',f'{current}:gui'])
assert all(check['equal'] for check in checks), [check for check in checks if not check['equal']]
print(json.dumps(dict(tested_source=base,merged_source=current,checks=checks,passed=len(checks)),indent=2))
