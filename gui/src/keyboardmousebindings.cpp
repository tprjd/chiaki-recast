#include "keyboardmousebindings.h"
#include <QKeySequence>
#include <QCoreApplication>
#include <QSet>
#include <Qt>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

KeyboardMouseBindings DefaultKeyboardMouseBindings()
{
    return {{{Qt::Key_W, 0}, {Qt::Key_S, 0}, {Qt::Key_A, 0}, {Qt::Key_D, 0},
        {Qt::Key_Space, 0}, {Qt::Key_Control, 0}, {Qt::Key_R, 0}, {Qt::Key_F, 0},
        {Qt::Key_Q, 0}, {Qt::Key_E, 0}, {-int(Qt::RightButton), Qt::Key_Z},
        {-int(Qt::LeftButton), Qt::Key_X}, {Qt::Key_Shift, 0}, {Qt::Key_C, 0},
        {Qt::Key_Up, 0}, {Qt::Key_Down, 0}, {Qt::Key_Left, 0}, {Qt::Key_Right, 0},
        {Qt::Key_Return, 0}, {Qt::Key_Backspace, 0}, {Qt::Key_Tab, 0}, {Qt::Key_F1, 0}}};
}

int KeyboardMouseNativeKey(int input)
{
#ifdef Q_OS_WIN
    if (input >= Qt::Key_F1 && input <= Qt::Key_F24) return VK_F1 + input - Qt::Key_F1;
    const struct { int key; int native; } keys[] = {
        {Qt::Key_Backspace, VK_BACK}, {Qt::Key_Tab, VK_TAB}, {Qt::Key_Return, VK_RETURN},
        {Qt::Key_Insert, VK_INSERT}, {Qt::Key_Delete, VK_DELETE}, {Qt::Key_Pause, VK_PAUSE},
        {Qt::Key_Print, VK_SNAPSHOT}, {Qt::Key_Home, VK_HOME}, {Qt::Key_End, VK_END},
        {Qt::Key_Left, VK_LEFT}, {Qt::Key_Up, VK_UP}, {Qt::Key_Right, VK_RIGHT}, {Qt::Key_Down, VK_DOWN},
        {Qt::Key_PageUp, VK_PRIOR}, {Qt::Key_PageDown, VK_NEXT}, {Qt::Key_Shift, VK_SHIFT},
        {Qt::Key_Control, VK_CONTROL}, {Qt::Key_Alt, VK_MENU}, {Qt::Key_Meta, VK_LWIN},
        {Qt::Key_CapsLock, VK_CAPITAL}, {Qt::Key_NumLock, VK_NUMLOCK}, {Qt::Key_ScrollLock, VK_SCROLL},
        {Qt::Key_Menu, VK_APPS}
    };
    for (const auto &key : keys) if (key.key == input) return key.native;
    if (input >= Qt::Key_Space && input <= 0xffff) {
        const SHORT native = VkKeyScanW(WCHAR(input));
        if (native != -1) return native & 0xff;
    }
#endif
    return 0;
}

int NormalizeKeyboardMouseKey(int input)
{
    if (input == Qt::Key_Enter) return Qt::Key_Return;
    if (input == Qt::Key_Backtab) return Qt::Key_Tab;
#ifdef Q_OS_WIN
    if (input >= Qt::Key_Space && input <= 0xffff) {
        const int native = KeyboardMouseNativeKey(input);
        const auto base = MapVirtualKeyW(native, MAPVK_VK_TO_CHAR) & 0xffff;
        if (base >= Qt::Key_Space) return QChar(ushort(base)).toUpper().unicode();
    }
#endif
    return input;
}

bool ValidKeyboardMouseInput(int input)
{
    if (!input) return true;
    if (input < 0) return input == -int(Qt::LeftButton) || input == -int(Qt::RightButton)
        || input == -int(Qt::MiddleButton) || input == -int(Qt::BackButton) || input == -int(Qt::ForwardButton);
    if (input == Qt::Key_F8 || input == Qt::Key_F11 || input == Qt::Key_Escape
            || input == Qt::Key_Alt || input == Qt::Key_Meta) return false;
#ifdef Q_OS_WIN
    return KeyboardMouseNativeKey(input) != 0;
#else
    return (input >= Qt::Key_Space && input <= 0xffff)
        || (input >= Qt::Key_F1 && input <= Qt::Key_F24)
        || (input >= Qt::Key_Backspace && input <= Qt::Key_ScrollLock);
#endif
}

bool ValidKeyboardMouseBindings(const KeyboardMouseBindings &bindings)
{
    QSet<int> used;
    for (const auto &action : bindings) for (int input : action) {
        if (NormalizeKeyboardMouseKey(input) != input || !ValidKeyboardMouseInput(input) || (input && used.contains(input))) return false;
        if (input) used.insert(input);
    }
    return true;
}

QString KeyboardMouseInputName(int input)
{
    switch (input) {
    case 0: return QCoreApplication::translate("KeyboardMouseBindings", "Unassigned");
    case -int(Qt::LeftButton): return QCoreApplication::translate("KeyboardMouseBindings", "Left click");
    case -int(Qt::RightButton): return QCoreApplication::translate("KeyboardMouseBindings", "Right click");
    case -int(Qt::MiddleButton): return QCoreApplication::translate("KeyboardMouseBindings", "Middle click");
    case -int(Qt::BackButton): return QCoreApplication::translate("KeyboardMouseBindings", "Mouse button 4");
    case -int(Qt::ForwardButton): return QCoreApplication::translate("KeyboardMouseBindings", "Mouse button 5");
    default: return QKeySequence(input).toString(QKeySequence::NativeText);
    }
}
