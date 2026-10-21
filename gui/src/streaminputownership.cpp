#include "streaminputownership.h"

#include <QTimer>
#include <QVariant>
#include <QJSValue>

namespace {
bool flag(QObject *object, const char *name)
{
    return object && object->property(name).toBool();
}
QObject *objectProperty(QObject *object, const char *name)
{
    if (!object) return nullptr;
    const auto value = object->property(name);
    if (value.metaType() == QMetaType::fromType<QJSValue>()) return value.value<QJSValue>().toQObject();
    return value.value<QObject *>();
}
}

StreamInputOwnership::StreamInputOwnership(QWindow *host, QQuickWindow *content, QObject *parent)
    : QObject(parent), host(host), content(content)
{
}

bool StreamInputOwnership::blocked() const
{
    for (const auto &presentation : presentations)
        if (presentation.captured) return true;
    return false;
}

void StreamInputOwnership::changed(bool previous)
{
    ++captureGeneration;
    if (previous != blocked()) emit blockedChanged();
    emit captureChanged(blocked());
}

QString StreamInputOwnership::present(QObject *owner, const QString &role, QQuickItem *focusTarget)
{
    if (!owner) return {};
    const auto existing = current.value(owner);
    if (presentations.contains(existing) && presentations.value(existing).active) return existing;
    const bool previous = blocked();
    ++focusGeneration;
    const auto id = QString::number(++serial);
    auto *window = focusTarget ? focusTarget->window() : nullptr;
    if (!window) {
        if (auto *item = qobject_cast<QQuickItem *>(owner)) window = item->window();
    }
    if (!window) {
        if (auto *context = qobject_cast<QQuickItem *>(objectProperty(owner, "context"))) window = context->window();
    }
    if (!window || (content && (role == "display" || role == "renderer" || role == "color-mapping" || role == "keyboard-mouse")))
        window = content;
    Presentation presentation;
    presentation.owner = owner;
    presentation.role = role;
    presentation.epoch = epoch;
    presentation.returnFocus = role == "notice" && noticeFocus.contains(owner)
        ? noticeFocus.take(owner).data() : window ? window->activeFocusItem() : nullptr;
    if (!presentation.returnFocus && window) {
        auto *focused = window->contentItem();
        while (focused && focused->scopedFocusItem()) focused = focused->scopedFocusItem();
        if (focused != window->contentItem()) presentation.returnFocus = focused;
    }
    presentations.insert(id, presentation);
    current.insert(owner, id);
    if (!watched.contains(owner)) {
        watched.insert(owner);
        connect(owner, &QObject::destroyed, this, [this, owner] { retire(owner); });
    }
    QPointer<QQuickItem> target = focusTarget;
    changed(previous);
    if (target && presentations.contains(id) && current.value(owner) == id) {
        if (role == "notice") {
            const auto generation = focusGeneration;
            QPointer<QQuickItem> target = focusTarget;
            QTimer::singleShot(0, this, [this, id, generation, target] {
                if (generation != focusGeneration || !presentations.contains(id)
                        || !presentations.value(id).active || presentations.value(id).epoch != epoch || !target) return;
                if (auto *window = target->window(); window && window->isVisible()) window->requestActivate();
                else if (host) host->requestActivate();
                target->forceActiveFocus(Qt::TabFocusReason);
            });
        } else target->forceActiveFocus(Qt::TabFocusReason);
    }
    return id;
}

void StreamInputOwnership::dismiss(const QString &id)
{
    auto found = presentations.find(id);
    if (found == presentations.end() || !found->active || current.value(found->owner) != id) return;
    const bool previous = blocked();
    found->active = false;
    const bool captured = found->captured;
    found->captured = false;
    if (captured) changed(previous);
    found = presentations.find(id);
    if (found == presentations.end() || current.value(found->owner) != id) return;
    if (found->role == "notice") {
        auto *owner = found->owner.data();
        auto *backend = objectProperty(owner, "backend");
        if (objectProperty(backend, "session") && (flag(owner, "blockedContent") || flag(owner, "videoDisabled"))
                && !returnEligible(id)) noticeFocus.insert(owner, found->returnFocus);
        restore(id, true);
    } else if (found->role != "connection" && found->role != "psn" && found->role != "legacy-error"
            && found->role != "pin" && !(found->role == "menu" && flag(found->owner, "separate"))) {
        restore(id, false);
    }
}

bool StreamInputOwnership::returnEligible(const QString &id) const
{
    const auto found = presentations.constFind(id);
    if (found == presentations.cend() || !found->owner || current.value(found->owner) != id || found->epoch != epoch) return false;
    for (auto other = presentations.cbegin(); other != presentations.cend(); ++other)
        if (other.key() != id && other->active) return false;
    if (found->role == "notice")
        return flag(found->owner, "noticesReady") && !flag(found->owner, "actionNoticeVisible");
    if (found->role == "menu") {
        auto *owner = found->owner.data();
        auto *view = objectProperty(owner, "context");
        return !flag(owner, "open") && owner->property("settingsTarget").toString().isEmpty()
            && !flag(owner, "disconnectPending") && !flag(view, "settingsOpen")
            && !flag(view, "sessionLoading") && !flag(view, "actionNoticeVisible");
    }
    return true;
}

void StreamInputOwnership::restoreNow(const QString &id)
{
    if (!returnEligible(id)) return;
    const auto presentation = presentations.value(id);
    QPointer<QQuickItem> item = presentation.returnFocus;
    if (presentation.role == "notice" || presentation.role == "menu") {
        if (item && item->window() && item->window()->isVisible()) item->window()->requestActivate();
        else if (host) host->requestActivate();
    }
    if (item && item->isVisible() && (presentation.role != "pin" || item->isEnabled()))
        item->forceActiveFocus(Qt::TabFocusReason);
    else if (presentation.role == "pin") {
        if (auto *navigation = objectProperty(presentation.owner, "navigation"))
            QMetaObject::invokeMethod(navigation, "restoreMainFocus");
    }
}

void StreamInputOwnership::restore(const QString &id, bool deferred)
{
    if (!deferred) { restoreNow(id); return; }
    const auto generation = focusGeneration;
    const auto captureEpoch = epoch;
    QTimer::singleShot(0, this, [this, id, generation, captureEpoch] {
        if (generation == focusGeneration && captureEpoch == epoch) restoreNow(id);
        if (presentations.contains(id) && presentations.value(id).settled) forget(id);
    });
}

void StreamInputOwnership::settled(const QString &id)
{
    auto found = presentations.find(id);
    if (found == presentations.end() || found->active || found->settled) return;
    found->settled = true;
    const auto role = found->role;
    const bool separate = flag(found->owner, "separate");
    if (role == "pin") restore(id, true);
    else if (role == "menu" && separate) restore(id, false);
    if (role != "pin" && role != "notice") forget(id);
}

void StreamInputOwnership::forget(const QString &id)
{
    const auto found = presentations.find(id);
    if (found == presentations.end()) return;
    const auto presentation = found.value();
    auto *ownerItem = qobject_cast<QQuickItem *>(presentation.owner.data());
    for (auto &other : presentations) {
        auto *target = other.returnFocus.data();
        if (ownerItem && target && (target == ownerItem || ownerItem->isAncestorOf(target)))
            other.returnFocus = presentation.returnFocus;
    }
    if (current.value(presentation.owner) == id) current.remove(presentation.owner);
    presentations.erase(found);
}

void StreamInputOwnership::retire(QObject *owner)
{
    const bool previous = blocked();
    bool captured = false;
    for (const auto &presentation : presentations)
        if (!presentation.owner && presentation.captured) captured = true;
    watched.remove(owner);
    // QPointer clears before destroyed is emitted; the current identity still identifies its record.
    const auto id = current.take(owner);
    presentations.remove(id);
    for (auto it = presentations.begin(); it != presentations.end();) {
        if (!it->owner) it = presentations.erase(it); else ++it;
    }
    noticeFocus.remove(owner);
    if (captured) changed(previous);
}

void StreamInputOwnership::sessionChanged(bool active, bool pendingPin, bool psn)
{
    const bool previous = blocked();
    ++focusGeneration;
    const bool preserve = pendingPin || (active && (sessionActive || psn));
    sessionActive = active;
    if (!preserve) {
        ++epoch;
        noticeFocus.clear();
        for (auto &presentation : presentations) {
            presentation.captured = presentation.active && presentation.role == "connection";
            if (presentation.active) presentation.epoch = epoch;
        }
    }
    changed(previous);
}
