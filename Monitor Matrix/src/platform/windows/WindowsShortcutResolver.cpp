#include "WindowsShortcutResolver.h"

#include <qdir.h>
#include <qfileinfo.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#include <shobjidl.h>

namespace {

QString hresultText(HRESULT hr)
{
    return QStringLiteral("0x%1")
        .arg(static_cast<unsigned long>(hr), 8, 16, QLatin1Char('0'));
}

void setError(QString* errorMessage, const QString& message)
{
    if (errorMessage) {
        *errorMessage = message;
    }
}

class ComApartment {
private:
    HRESULT m_result = E_FAIL;
    bool m_uninitialize = false;

public:
    ComApartment()
    {
        m_result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        if (m_result == S_OK || m_result == S_FALSE) {
            m_uninitialize = true;
        }
    }

    ~ComApartment()
    {
        if (m_uninitialize) {
            CoUninitialize();
        }
    }

    bool isUsable() const
    {
        return SUCCEEDED(m_result) || m_result == RPC_E_CHANGED_MODE;
    }

    HRESULT result() const
    {
        return m_result;
    }
};

} // namespace

namespace WindowsShortcutResolver {

bool resolveExecutableTarget(
    const QString& shortcutPath,
    QString* executablePath,
    QString* errorMessage)
{
    if (executablePath) {
        executablePath->clear();
    }

    const QFileInfo shortcutInfo(shortcutPath);

    if (!shortcutInfo.exists() ||
        !shortcutInfo.isFile() ||
        shortcutInfo.suffix().compare(QStringLiteral("lnk"), Qt::CaseInsensitive) != 0) {
        setError(errorMessage, QStringLiteral("Shortcut file is invalid."));
        return false;
    }

    ComApartment com;

    if (!com.isUsable()) {
        setError(
            errorMessage,
            QStringLiteral("Cannot initialize COM for shortcut resolution: %1")
                .arg(hresultText(com.result()))
        );
        return false;
    }

    IShellLinkW* shellLink = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_ShellLink,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&shellLink)
    );

    if (FAILED(hr) || !shellLink) {
        setError(
            errorMessage,
            QStringLiteral("Cannot create ShellLink resolver: %1")
                .arg(hresultText(hr))
        );
        return false;
    }

    IPersistFile* persistFile = nullptr;
    hr = shellLink->QueryInterface(IID_PPV_ARGS(&persistFile));

    if (FAILED(hr) || !persistFile) {
        shellLink->Release();
        setError(
            errorMessage,
            QStringLiteral("Cannot read shortcut file interface: %1")
                .arg(hresultText(hr))
        );
        return false;
    }

    const std::wstring shortcutNative =
        shortcutInfo.absoluteFilePath().toStdWString();

    hr = persistFile->Load(shortcutNative.c_str(), STGM_READ);

    if (FAILED(hr)) {
        persistFile->Release();
        shellLink->Release();
        setError(
            errorMessage,
            QStringLiteral("Cannot load shortcut: %1")
                .arg(hresultText(hr))
        );
        return false;
    }

    wchar_t targetPath[MAX_PATH] = {};
    WIN32_FIND_DATAW findData = {};

    hr = shellLink->GetPath(
        targetPath,
        MAX_PATH,
        &findData,
        SLGP_UNCPRIORITY
    );

    persistFile->Release();
    shellLink->Release();

    if (FAILED(hr) || targetPath[0] == L'\0') {
        setError(
            errorMessage,
            QStringLiteral("Shortcut does not point to a filesystem executable.")
        );
        return false;
    }

    const QFileInfo targetInfo(QString::fromWCharArray(targetPath));

    if (!targetInfo.exists() ||
        !targetInfo.isFile() ||
        targetInfo.suffix().compare(QStringLiteral("exe"), Qt::CaseInsensitive) != 0) {
        setError(
            errorMessage,
            QStringLiteral("Shortcut target is not an executable.")
        );
        return false;
    }

    if (executablePath) {
        *executablePath = QDir::toNativeSeparators(targetInfo.absoluteFilePath());
    }

    return true;
}

} // namespace WindowsShortcutResolver
