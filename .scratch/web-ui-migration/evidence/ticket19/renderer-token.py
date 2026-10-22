import ctypes as c
from ctypes import wintypes as w
import json, os, pathlib, subprocess, sys, time

parent_id = int(sys.argv[1])
output = pathlib.Path(sys.argv[2])
stage = pathlib.Path(sys.argv[3]).resolve()
kernel = c.WinDLL('kernel32', use_last_error=True)
advapi = c.WinDLL('advapi32', use_last_error=True)
kernel.OpenProcess.argtypes = [w.DWORD, w.BOOL, w.DWORD]
kernel.OpenProcess.restype = w.HANDLE
kernel.CloseHandle.argtypes = [w.HANDLE]
kernel.LocalFree.argtypes = [w.HLOCAL]
advapi.OpenProcessToken.argtypes = [w.HANDLE, w.DWORD, c.POINTER(w.HANDLE)]
advapi.GetTokenInformation.argtypes = [w.HANDLE, c.c_int, w.LPVOID, w.DWORD, c.POINTER(w.DWORD)]
advapi.IsTokenRestricted.argtypes = [w.HANDLE]
advapi.GetSidSubAuthorityCount.argtypes = [w.LPVOID]
advapi.GetSidSubAuthorityCount.restype = c.POINTER(w.BYTE)
advapi.GetSidSubAuthority.argtypes = [w.LPVOID, w.DWORD]
advapi.GetSidSubAuthority.restype = c.POINTER(w.DWORD)
advapi.ConvertSidToStringSidW.argtypes = [w.LPVOID, c.POINTER(w.LPWSTR)]

class SidAttributes(c.Structure):
    _fields_ = [('sid', w.LPVOID), ('attributes', w.DWORD)]

class Groups(c.Structure):
    _fields_ = [('count', w.DWORD), ('groups', SidAttributes * 1)]

def sid_text(sid):
    text = w.LPWSTR()
    if not advapi.ConvertSidToStringSidW(sid, c.byref(text)):
        raise c.WinError(c.get_last_error())
    try:
        return text.value
    finally:
        kernel.LocalFree(c.cast(text, w.HLOCAL))

def info(token, kind):
    needed = w.DWORD()
    advapi.GetTokenInformation(token, kind, None, 0, c.byref(needed))
    if not needed.value:
        raise c.WinError(c.get_last_error())
    buffer = c.create_string_buffer(needed.value)
    if not advapi.GetTokenInformation(token, kind, buffer, needed, c.byref(needed)):
        raise c.WinError(c.get_last_error())
    return buffer

def token_info(pid):
    process = kernel.OpenProcess(0x1000, False, pid)
    if not process:
        raise c.WinError(c.get_last_error())
    token = w.HANDLE()
    try:
        if not advapi.OpenProcessToken(process, 0x0008, c.byref(token)):
            raise c.WinError(c.get_last_error())
        label_buffer = info(token, 25)
        label = SidAttributes.from_buffer(label_buffer)
        count = advapi.GetSidSubAuthorityCount(label.sid)[0]
        rid = advapi.GetSidSubAuthority(label.sid, count - 1)[0]
        restricted_buffer = info(token, 11)
        restricted_count = w.DWORD.from_buffer(restricted_buffer).value
        array = (SidAttributes * restricted_count).from_address(c.addressof(restricted_buffer) + Groups.groups.offset)
        result = {'pid': pid, 'integritySid': sid_text(label.sid), 'integrityRid': rid,
                  'isTokenRestricted': bool(advapi.IsTokenRestricted(token)),
                  'restrictedSidCount': restricted_count,
                  'restrictedSids': [sid_text(item.sid) for item in array]}
        try:
            result['isAppContainer'] = bool(w.DWORD.from_buffer(info(token, 29)).value)
        except OSError as error:
            result['appContainerQueryError'] = str(error)
        return result
    finally:
        if token:
            kernel.CloseHandle(token)
        kernel.CloseHandle(process)

report = {'parentPid': parent_id, 'query': 'Read-only process/token queries. No elevation or launch flag changes.'}
try:
    shell = pathlib.Path(os.environ['SystemRoot']) / 'System32/WindowsPowerShell/v1.0/powershell.exe'
    query = ("Get-CimInstance Win32_Process -Filter \"Name='QtWebEngineProcess.exe'\" | "
             f"Where-Object {{$_.ParentProcessId -eq {parent_id} -and $_.CommandLine -match '--type=renderer'}} | "
             "Select-Object ProcessId,ParentProcessId,ExecutablePath,CommandLine | ConvertTo-Json -Depth 2")
    children = []
    end = time.monotonic() + 15
    while time.monotonic() < end and not children:
        text = subprocess.check_output([str(shell), '-NoProfile', '-Command', query], text=True).strip()
        if text:
            children = json.loads(text)
            if isinstance(children, dict):
                children = [children]
        else:
            time.sleep(0.25)
    if not children:
        raise RuntimeError('No exact owned renderer child appeared in the bounded observation.')
    child = children[0]
    if pathlib.Path(child['ExecutablePath']).resolve() != stage / 'QtWebEngineProcess.exe':
        raise RuntimeError('The owned renderer helper is outside the package.')
    report['rendererProcess'] = child
    report['parent'] = token_info(parent_id)
    report['renderer'] = token_info(child['ProcessId'])
    report['restrictedRendererObserved'] = (report['renderer']['isTokenRestricted']
        and report['renderer']['integrityRid'] <= 0x1000
        and report['renderer']['integrityRid'] < report['parent']['integrityRid'])
except Exception as error:
    report['observationError'] = str(error)
finally:
    output.write_text(json.dumps(report, indent=2))
print(json.dumps(report, indent=2))
sys.exit(0 if report.get('restrictedRendererObserved') else 1)
