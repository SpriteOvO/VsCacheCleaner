#include "Utils.h"

#include <QUrl>
#include <QLocale>
#include <QDateTime>
#include <QDesktopServices>

#include <tlhelp32.h>

#include "Config.h"


void FatalError(const QString &Message, bool Report)
{
    QString Content;

    if (Report)
    {
        Content = QObject::tr(
            "An error has occurred!\n"
            "Please help us fix this problem.\n"
            "--------------------------------------------------\n"
            "\n"
            "%1\n"
            "\n"
            "--------------------------------------------------\n"
            "Click \"Abort\" or \"Ignore\" will pop up GitHub issue tracker page.\n"
            "You can submit this information to us there.\n"
            "Thank you very much!"
        ).arg(Message);
    }
    else {
        Content = Message;
    }

    int Result;
    do {
        Result = MessageBoxW(
            nullptr,
            Content.toStdWString().c_str(),
            QObject::tr("VsCacheCleaner fatal error").toStdWString().c_str(),
            MB_ABORTRETRYIGNORE | MB_ICONERROR
        );
#if defined _DEBUG
        if (Result == IDRETRY) {
            __debugbreak();
        }
#endif
    } while (Result == IDRETRY);

#if !defined _DEBUG
    if (Report) {
        QDesktopServices::openUrl(QUrl{VCC_URL_ISSUES});
    }
#endif

    exit(EXIT_FAILURE);
}

namespace Text
{
    QString FormatDataSize(uint64_t DataSize)
    {
        // return QLocale{}.formattedDataSize(DataSize, 2, QLocale::DataSizeIecFormat);

        QString ValueStr, UnitStr;

        if (DataSize < Constant::SizeKb) {
            ValueStr = QString::number(DataSize);
            UnitStr = "B";
        }
        else if (DataSize >= Constant::SizeKb && DataSize < Constant::SizeMb) {
            ValueStr = QString::number(DataSize / Constant::SizeKb);
            UnitStr = "KB";
        }
        else if (DataSize >= Constant::SizeMb && DataSize < Constant::SizeGb) {
            ValueStr = QString::number((float)DataSize / (float)Constant::SizeMb, 'f', 2);
            UnitStr = "MB";
        }
        else if (DataSize >= Constant::SizeGb && DataSize < Constant::SizeTb) {
            ValueStr = QString::number((float)DataSize / (float)Constant::SizeGb, 'f', 2);
            UnitStr = "GB";
        }
        else {
            ValueStr = QString::number((float)DataSize / (float)Constant::SizeTb, 'f', 2);
            UnitStr = "TB";
        }

        return ValueStr + " " + UnitStr;
    }

    QString FormatTime(const QDateTime &DateTime)
    {
        return DateTime.toString(QLocale{}.dateFormat(QLocale::FormatType::ShortFormat));
    }

} // namespace Text

namespace File
{
    void OpenFolder(const QString &Path)
    {
        ShellExecuteW(nullptr, nullptr, L"explorer.exe", ("/select,\"" + Path + "\"").toStdWString().c_str(), nullptr, SW_SHOWDEFAULT);
    }

    bool MoveToTrash(const QString &File)
    {
        // Double null termination needed, so can't use QString::utf16
        //
        QVarLengthArray<wchar_t, MAX_PATH + 1> FromFile(File.length() + 2);
        File.toWCharArray(FromFile.data());
        FromFile[File.length()] = wchar_t{};
        FromFile[File.length() + 1] = wchar_t{};

        SHFILEOPSTRUCTW Operation;
        Operation.hwnd = nullptr;
        Operation.wFunc = FO_DELETE;
        Operation.pFrom = FromFile.constData();
        Operation.pTo = nullptr;
        Operation.fFlags = FOF_ALLOWUNDO | FOF_NO_UI;
        Operation.fAnyOperationsAborted = false;
        Operation.hNameMappings = nullptr;
        Operation.lpszProgressTitle = nullptr;

        return SHFileOperationW(&Operation) == 0;
    }

} // namespace File

namespace Process
{
    bool EnumProcesses(const std::function<bool(const InfoT &Info)> &Callback)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return false;
        }

        PROCESSENTRY32W ProcessEntry;
        ProcessEntry.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &ProcessEntry))
        {
            do {
                InfoT Info;

                Info.Id = ProcessEntry.th32ProcessID;
                Info.ParentId = ProcessEntry.th32ParentProcessID;
                Info.ThreadsCount = ProcessEntry.cntThreads;
                Info.ThreadsBasePriority = ProcessEntry.pcPriClassBase;
                Info.Name = QString::fromWCharArray(ProcessEntry.szExeFile);

                if (!Callback(Info)) {
                    break;
                }

            } while (Process32NextW(hSnapshot, &ProcessEntry));
        }

        CloseHandle(hSnapshot);
        return true;
    }

} // namespace Process

namespace System
{
    QDateTime SystemTimeToQDateTime(const SYSTEMTIME &Time)
    {
        return QDateTime{
            QDate{Time.wYear, Time.wMonth, Time.wDay},
            QTime{Time.wHour, Time.wMinute, Time.wSecond}
        };
    }

    bool SingleInstance(const QString &InstanceName)
    {
        HANDLE hMutex = CreateMutexW(nullptr, false, ("Global\\" + InstanceName + "_InstanceMutex").toStdWString().c_str());
        uint32_t LastErrorCode = GetLastError();

        if (hMutex == NULL) {
            FatalError("Create instance mutex failed.\nErrorCode: " + QString::number(LastErrorCode));
        }

        // No need to close the handle
        //
        return LastErrorCode != ERROR_ALREADY_EXISTS;
    }

} // namespace System
