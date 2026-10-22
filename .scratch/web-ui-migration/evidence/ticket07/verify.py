"""Check final source against tested revisions and record artifact identities."""
from pathlib import Path
import hashlib
import json
import re
import subprocess
import sys

EVIDENCE = Path(__file__).resolve().parent
REPO = EVIDENCE.parents[3]

def git(*args):
    return subprocess.check_output(["git", *args], cwd=REPO).decode("utf-8").replace("\r\n", "\n")

def digest(data):
    return hashlib.sha256(data).hexdigest()

def source(path, revision=None):
    if revision:
        return git("show", revision + ":" + path)
    return (REPO / path).read_text(encoding="utf-8")

def definition(text, marker, indent=""):
    begin = text.index(marker)
    end = text.index("\n" + indent + "}\n", begin)
    return text[begin:end + len(indent) + 3]

checks = []
def compare(label, path, revision, transform=lambda text: text):
    actual = transform(source(path))
    expected = transform(source(path, revision))
    checks.append({"label": label, "path": path, "tested_source": revision,
        "actual_sha256": digest(actual.encode()), "expected_sha256": digest(expected.encode()), "equal": actual == expected})

for marker in ["QmlRegist::QmlRegist(", "QmlRegist::~QmlRegist()", "ChiakiErrorCode QmlRegist::start(",
        "void QmlRegist::log_cb(", "void QmlRegist::regist_cb(",
        "QVariantMap QmlBackend::createRegistrationContext(", "bool QmlBackend::registrationContextCurrent(",
        "bool QmlBackend::registerHost(", "QString QmlBackend::startAccountLookup(", "bool QmlBackend::cancelAccountLookup("]:
    compare(marker, "gui/src/qmlbackend.cpp", "c152f87b", lambda text, marker=marker: definition(text, marker))
for marker in ["QVariantMap SetupBridge::openRegistration(", "bool SetupBridge::registrationAccountCurrent(",
        "QVariantMap SetupBridge::beginRegistrationAccount(", "QVariantMap SetupBridge::lookupRegistrationAccount(",
        "QVariantMap SetupBridge::readRegistrationAccount(", "QVariantMap SetupBridge::endRegistrationAccount(",
        "QVariantMap SetupBridge::connectRegistration("]:
    compare(marker, "gui/src/setupbridge.cpp", "c152f87b", lambda text, marker=marker: definition(text, marker))
compare("corrected registration context", "gui/src/setupbridge.cpp", "c7a5c7d1",
    lambda text: definition(text, "QVariantMap SetupBridge::submitRegistration("))

def projection(text):
    start = text.index("    QVariantMap registration{{")
    end = min(pos for token in ["    const auto nativeAttempt =", "    state = {{"] if (pos := text.find(token, start)) >= 0)
    return text[start:end]
compare("registration and lookup snapshot allowlists", "gui/src/setupbridge.cpp", "c152f87b", projection)
compare("registration and account profile invalidation", "gui/src/qmlbackend.cpp", "c152f87b",
    lambda text: definition(text, "void QmlBackend::profileChanged()").split("    // Stop replies before")[0])
compare("registration transport defaults", "gui/include/registrationrequests.h", "c152f87b")
compare("native sign-in UI", "gui/web/src/PsnAccount.svelte", "c152f87b")
compare("final pairing UI", "gui/web/src/Pairing.svelte", "add8c676")
compare("pairing UI before focus fallback", "gui/web/src/Pairing.svelte", "c7a5c7d1",
    lambda text: "\n".join(line for line in text.splitlines() if "opener?.isConnected" not in line))

for name, revision in [("registrationLookup", "c152f87b"), ("registrationLookupTimeout", "c152f87b"),
        ("registrationAccountSources", "c152f87b"), ("registrationNativeValidation", "c152f87b"),
        ("registrationStaleTargets_data", "c152f87b"), ("registrationStaleTargets", "c152f87b"),
        ("psnSignIn", "c152f87b"), ("consoleTargetGuards", "c152f87b"),
        ("registrationAddressCorrection", "c7a5c7d1"), ("registrationForm", "c7a5c7d1"),
        ("registrationProgress", "c7a5c7d1"), ("registrationHandoff", "add8c676")]:
    # Only per-instance request arguments changed after ticket08 introduced native discovery control.
    def scenario(text, name=name):
        body = definition(text, "    void " + name + "(", "    ")
        body = re.sub(r"^        QmlNativeServices services\([^\n]+\);$", "        QmlNativeServices services(TRANSPORT_ARGUMENTS);", body, flags=re.M)
        # The later user scope sets a 1024 CSS-pixel floor. No feature assertion changes.
        body = "\n".join(line for line in body.splitlines() if "setup.resize(" not in line
            and "setup.view()->setZoomFactor(" not in line and not line.strip().startswith('capture("'))
        return body.replace("innerWidth >= 1024 && ", "")
    compare("scenario " + name, "gui/tests/embeddedsetup.cpp", revision, scenario)

# The TypeScript request names and adapters still match the tested pairing interface.
names = ["beginRegistrationAccount", "lookupRegistrationAccount", "readRegistrationAccount", "endRegistrationAccount",
    "openRegistration", "submitRegistration", "connectRegistration", "accountBegin:", "accountLookup:",
    "accountRead:", "accountEnd:", "registrationOpen:", "registrationSubmit:", "registrationConnect:"]
compare("pairing WebChannel request adapters", "gui/web/src/bridge.ts", "add8c676",
    lambda text: "\n".join(line for line in text.splitlines() if any(name in line for name in names)))

signatures = {}
for path in ["gui/include/qmlbackend.h", "gui/include/qmlnativeservices.h"]:
    signatures[path] = "RegistrationRequests registration = {}, DiscoveryRequests discovery = {}" in source(path)
for path in ["gui/src/qmlbackend.cpp", "gui/src/qmlnativeservices.cpp"]:
    signatures[path] = "RegistrationRequests registration, DiscoveryRequests discovery" in source(path)
signatures["native service forwards both requests"] = "std::move(registration), std::move(discovery)" in source("gui/src/qmlnativeservices.cpp")
signatures["backend binds discovery before construction"] = "discovery_manager(nullptr, std::move(discovery))" in source("gui/src/qmlbackend.cpp")
signatures["backend retains native registration requests"] = "registration_requests(std::move(registration))" in source("gui/src/qmlbackend.cpp")
signatures["backend retains launcher"] = "new QmlSettings(settings, this, std::move(launcher))" in source("gui/src/qmlbackend.cpp")
signatures["raw session default remains production"] = "startSession = chiaki_session_start" in source("gui/include/psnrequests.h")

paths = ["gui/include/registrationrequests.h", "gui/include/qmlbackend.h", "gui/include/qmlnativeservices.h", "gui/include/setupbridge.h",
    "gui/src/qmlbackend.cpp", "gui/src/qmlnativeservices.cpp", "gui/src/setupbridge.cpp", "gui/include/psnrequests.h",
    "gui/include/discoverymanager.h", "gui/src/discoverymanager.cpp", "gui/include/streamsession.h", "gui/src/streamsession.cpp",
    "gui/web/src/Pairing.svelte", "gui/web/src/PsnAccount.svelte", "gui/web/src/App.svelte", "gui/web/src/bridge.ts",
    "gui/web/src/ConnectAction.svelte", "gui/tests/embeddedsetup.cpp", "gui/tests/CMakeLists.txt", "gui/tests/README.md", "gui/web/package-lock.json"]
files = {path: {"worktree_sha256": digest((REPO/path).read_bytes()),
    "normalized_sha256": digest(source(path).encode()), "git_blob_sha256": digest(git("show", "HEAD:" + path).encode())} for path in paths}
artifacts = {}
for name, path in {
    "application": "C:/Users/david/.cache/chiaki-msvc-native/ticket07-build/gui/chiaki.exe",
    "embedded_tests": "C:/Users/david/.cache/chiaki-msvc-native/ticket07-build/gui/tests/chiaki-embedded-setup-tests.exe",
    "javascript": "C:/Users/david/.cache/chiaki-msvc-native/ticket07-build/gui/web/app.js",
    "css": "C:/Users/david/.cache/chiaki-msvc-native/ticket07-build/gui/web/app.css"}.items():
    artifacts[name] = {"path": path, "sha256": digest(Path(path).read_bytes())}

retired = ["gui/src/qml/RegistDialog.qml", "gui/src/qml/PSNLoginDialog.qml", "gui/tests/qml/tst_RegistrationForm.qml",
    "gui/tests/qml/tst_RegistrationResults.qml", "gui/tests/qml/tst_RegistrationAccount.qml", "gui/tests/qml/fixtures/RegistrationHarness.qml",
    "gui/tests/qml/fixtures/RegistrationAccountHarness.qml"]
retired_absent = all(not (REPO / path).exists() for path in retired)
reports = {}
for path in sorted(EVIDENCE.glob("*result.txt")):
    text = path.read_text(encoding="utf-8")
    match = re.search(r"Totals: (\d+) passed, (\d+) failed, (\d+) skipped", text)
    if match:
        reports[path.name] = {"passed": int(match[1]), "failed": int(match[2]), "skipped": int(match[3]),
            "failures": re.findall(r"FAIL!  : EmbeddedSetupTest::([^()]*)", text)}
all_pass = all(check["equal"] for check in checks) and all(signatures.values()) and retired_absent
all_pass = all_pass and all(item["normalized_sha256"] == item["git_blob_sha256"] for item in files.values())
result = {"source": git("rev-parse", "HEAD").strip(), "gui_tree": git("rev-parse", "HEAD:gui").strip(),
    "comparisons": checks, "constructor_and_default_checks": signatures, "source_files": files, "artifacts": artifacts,
    "retired_routes_absent": retired_absent, "reports": reports, "all_source_checks_pass": all_pass,
    "evidence_sha256": {path.name: digest(path.read_bytes()) for path in sorted(EVIDENCE.iterdir())
        if path.is_file() and path.name != "identity.json"}}
if "--check" in sys.argv:
    recorded = json.loads((EVIDENCE / "identity.json").read_text(encoding="utf-8"))
    all_pass = all_pass and result["source_files"] == recorded["source_files"]
    all_pass = all_pass and result["artifacts"] == recorded["artifacts"]
    all_pass = all_pass and result["evidence_sha256"] == recorded["evidence_sha256"]
else:
    (EVIDENCE / "identity.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
print(json.dumps({"comparisons": len(checks), "source_files": len(files), "all_pass": all_pass}))
raise SystemExit(0 if all_pass else 1)
