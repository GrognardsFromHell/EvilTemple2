
#include <QScopedPointer>

#include "basepathfinder.h"

// Make SURE it's unicode based
#if !defined(UNICODE)
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace EvilTemple {

    static const HKEY RegistryRoot = HKEY_LOCAL_MACHINE;

    static const wchar_t *RegistryKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
                                L"\\Uninstall\\{AD80F06B-0F21-4EEE-934D-BEF0D21E6383}";

    static const wchar_t *RegistryValue = L"InstallLocation";

    static QDir defaultDir() {
        return QDir::current();
    }

    QDir BasepathFinder::find() {
        HKEY key;
        LONG result = RegOpenKeyEx(RegistryRoot, RegistryKey, 0, KEY_READ|KEY_WOW64_32KEY, &key);

        if (result != ERROR_SUCCESS) {
            qWarning("Unable to open registry key for ToEE installation directory: %#lx", result);
            return defaultDir();
        }

        DWORD dataType;
        DWORD dataLength;

        result = RegQueryValueEx(key, RegistryValue, NULL, &dataType, NULL, &dataLength);

        if (result != ERROR_SUCCESS) {
            qWarning("Unable to query registry key for ToEE installation directory: %#lx", result);
            return defaultDir();
        }

        if (dataType != REG_SZ) {
            qWarning("InstallPath has wrong data type: %#lx", dataType);
            return defaultDir();
        }

        // Allocate a large enough buffer
        QScopedPointer<wchar_t, QScopedPointerPodDeleter> data((wchar_t*)malloc(dataLength + sizeof(wchar_t)));
        memset(data.data(), 0, dataLength + sizeof(wchar_t)); // Make sure it's correctly null-terminated

        result = RegQueryValueEx(key, RegistryValue, NULL, &dataType,
                                 reinterpret_cast<BYTE*>(data.data()), &dataLength);

        if (result != ERROR_SUCCESS) {
            qWarning("Unable to read registry key for ToEE installation directory: %#lx", result);
            return defaultDir();
        }

        QDir dir = QDir(QString::fromWCharArray(data.data()));

        result = RegCloseKey(key);

        if (result != ERROR_SUCCESS) {
            qWarning("Unable to close registry key for ToEE installation directory: %#lx", result);
        }

        return dir;
    }

}

