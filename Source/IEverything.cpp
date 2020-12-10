#include "IEverything.h"

#include <Everything.h>
#include <MinHook.h>

#include <QDir>
#include <QCoreApplication>
#include <thread>

#include "Utils.h"

#if defined _WIN32
# pragma comment(lib, "Everything32.lib")
# pragma comment(lib, "MinHook.x86.lib")
#elif defined _WIN64
# pragma comment(lib, "Everything64.lib")
# pragma comment(lib, "MinHook.x64.lib")
#else
# error "Unknown compilation target."
#endif


IEverything& IEverything::GetInstance()
{
    static IEverything i;
    return i;
}

IEverything::IEverything()
{
    memset(&_ProcessInfo, 0, sizeof(_ProcessInfo));
}

IEverything::~IEverything()
{
    ExitBackend();
}

void IEverything::LaunchBackend()
{
    using namespace std::literals::chrono_literals;

    if (_IsLaunched) {
        return;
    }

    PrepareLaunch();

    // Launch Everything
    //
    STARTUPINFOW StartupInfo;
    memset(&StartupInfo, 0, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    QString CommandLine = "\"Everything\\Everything.exe\" -startup -instance \"VsCacheCleaner\"";

    // In VS debug mode, the API cannot find the relative path file in the first argument,
    // because it is relative to the VS solution path.
    //
    // This function can modify the contents of the second string.
    // Therefore, the parameter cannot be a pointer to read-only memory (such as a const variable or a literal string).
    // If the parameter is a constant string, the function may cause an access violation.
    //
    if (!CreateProcessW(
        nullptr,
        CommandLine.toStdWString().data(),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &StartupInfo,
        &_ProcessInfo))
    {
        FatalError("Launch Everything client failed.\nErrorCode: " + QString::number(GetLastError()));
    }

    // Wait Everything initialize IPC
    //

    uint32_t RetryCount = 0;
    while (Everything_GetBuildNumber() == 0 && RetryCount < 100)
    {
        std::this_thread::sleep_for(100ms);
        ++RetryCount;
    }

    if (Everything_GetBuildNumber() == 0) {
        FatalError("Connect to Everything failed.");
    }

    if (!Everything_IsAdmin()) {
        FatalError(QObject::tr("Everything client requires administrative privileges to index NTFS volumes.\nPlease try to reopen VsCacheCleaner as administrator."), false);
    }

    _IsLaunched = true;
}

void IEverything::ExitBackend()
{
    _IsLaunched = false;

    if (_ProcessInfo.hProcess != nullptr)
    {
        if (IsInitialized()) {
            Everything_SaveDB();
        }
        Everything_Exit();
        TerminateProcess(_ProcessInfo.hProcess, 0);
    }
}

bool IEverything::IsInitialized()
{
    return Everything_IsDBLoaded();
}

uint32_t IEverything::GetErrorCode()
{
    return Everything_GetLastError();
}

std::optional<QVector<Et::ItemDataT>> IEverything::Search(
    const QString &String,
    const Et::SearchOptionsT &SearchOptions
)
{
    std::optional<QVector<Et::ItemDataT>> Result; // HACK for NRVO

    do {
        QVector<Et::ItemDataT> Items;

        // Require nothing
        //
        
        if (!SearchOptions.testFlag(Et::SearchOptionE::RequireFile) && !SearchOptions.testFlag(Et::SearchOptionE::RequireFolder)) {
            Result = std::move(Items);
            break;
        }

        if (!SearchQuery(String, SearchOptions)) {
            Result = std::nullopt;
            break;
        }

        uint32_t TotalResults = Everything_GetNumResults();
        if (TotalResults == 0) {
            Result = std::move(Items);
            break;
        }

        Items.resize(TotalResults);

        for (uint32_t i = 0; i < TotalResults; ++i)
        {
            if (!_IsLaunched) {
                break;
            }

            Et::ItemDataT ItemData = GetResultData(i);

            // Check type
            //
            if (!(SearchOptions.testFlag(Et::SearchOptionE::RequireFile) && ItemData.Type == Et::ItemTypeE::File ||
                SearchOptions.testFlag(Et::SearchOptionE::RequireFolder) && ItemData.Type == Et::ItemTypeE::Folder))
            {
                FatalError(QString{"Searched data invalid.\nIndex: %1\nOptions: %2\nType: %3"}
                .arg(i).arg(SearchOptions).arg((uint32_t)ItemData.Type));
            }

            Items[i] = std::move(ItemData);
        }

        // Exclude trash files
        //
        Items.erase(std::remove_if(Items.begin(), Items.end(),
            [](const Et::ItemDataT &item) {
                return item.FullPathName.toUpper().indexOf("$RECYCLE.BIN") != -1;
            }
        ), Items.end());

        if (!_IsLaunched) {
            Result = std::nullopt;
            break;
        }

        Result = std::move(Items);

    } while (false);

    return Result;
}

void IEverything::PrepareLaunch()
{
    // We do not place the database in the running directory, but in the temporary directory.
    // This can prevent users from accidentally sharing the VsCacheCleaner client with the disk database.
    //

    // Create and cd to the new database folder
    //
    QDir DbFolder = QDir::temp();
    if (!DbFolder.exists("VsCacheCleaner")) {
        if (!DbFolder.mkdir("VsCacheCleaner")) {
            FatalError("Create database folder failed.\nTemp path: " + DbFolder.path());
        }
    }
    VCC_ASSERT(DbFolder.cd("VsCacheCleaner"));

    // Currently the Everything-SDK does not support multiple instances.
    // The following code is a workaround to make the Everything-SDK support multiple instances.
    //
    MH_STATUS Status = MH_Initialize();
    if (Status != MH_OK) {
        FatalError(QString{"MH_Initialize() failed.\nStatus: "} + MH_StatusToString(Status));
    }
    if (MH_CreateHookApi(L"User32.dll", "FindWindowA", &DetourFindWindow<char>, (void**)&_FnOriginalFindWindow<char>) != MH_OK ||
        MH_CreateHookApi(L"User32.dll", "FindWindowW", &DetourFindWindow<wchar_t>, (void**)&_FnOriginalFindWindow<wchar_t>) != MH_OK)
    {
        FatalError(QString{"MH_CreateHookApi() failed.\nStatus: "} + MH_StatusToString(Status));
    }
    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        FatalError(QString{"MH_EnableHook() failed.\nStatus: "} + MH_StatusToString(Status));
    }

    // The following code is a workaround to configure the Everything.
    // Currently the Everything-SDK does not support changing the Everything settings.
    // We have to modify the Everything.ini configuration file manually.
    //

    // Get Everything client directory
    //
    QDir EtFolder = QCoreApplication::applicationDirPath(); // QDir::currentPath() will return VS solution path in VS debug mode
    if (!EtFolder.cd("Everything") || !EtFolder.exists("Everything.exe")) {
        FatalError(QObject::tr("Everything client is missing.\nPlease try to reinstall VsCacheCleaner."), false);
    }

    QString ConfigFile = "Everything-VsCacheCleaner.ini";

    // Delete the old configuration file if it exists
    //
    if (EtFolder.exists(ConfigFile)) {
        if (!EtFolder.remove(ConfigFile)) {
            FatalError("Delete the old Everything configuration file failed.");
        }
    }

    // Write configuration data
    // The reason for not using QSettings is that QSettings will internally convert '\\' to '\\\\'
    //
    QString EtConfig = EtFolder.filePath(ConfigFile);
    if (!WritePrivateProfileStringW(L"Everything", L"db_location", QDir::toNativeSeparators(DbFolder.path()).toStdWString().c_str(), EtConfig.toStdWString().c_str()) ||
        !WritePrivateProfileStringW(L"Everything", L"index_date_accessed", L"1" /* true */, EtConfig.toStdWString().c_str()) ||
        !WritePrivateProfileStringW(L"Everything", L"index_folder_size", L"1" /* true */, EtConfig.toStdWString().c_str()))
    {
        FatalError("WritePrivateProfileStringW() failed.\nErrorCode: " + QString::number(GetLastError()));
    }
}

bool IEverything::SearchQuery(const QString &String, const Et::SearchOptionsT &SearchOptions)
{
    uint32_t RequestFlags = EVERYTHING_REQUEST_FILE_NAME | EVERYTHING_REQUEST_PATH | EVERYTHING_REQUEST_SIZE;
    RequestFlags |= SearchOptions.testFlag(Et::SearchOptionE::RequireAccessedDate) ? EVERYTHING_REQUEST_DATE_ACCESSED : 0;
    Everything_SetRequestFlags(RequestFlags);

    Everything_SetMatchPath(false);
    Everything_SetMatchCase(SearchOptions.testFlag(Et::SearchOptionE::MatchCase));
    Everything_SetMatchWholeWord(SearchOptions.testFlag(Et::SearchOptionE::MatchWholeWord));

    QString Modifiers;

    // Currently the Everything-SDK does not provide APIs to set these options, so a workaround is to use String argument
    // See more: https://www.voidtools.com/support/everything/command_line_options/#searching
    //
    if (SearchOptions.testFlag(Et::SearchOptionE::MatchWholeFileName)) {
        Modifiers += "wholefilename:";
    }

    if (!(SearchOptions.testFlag(Et::SearchOptionE::RequireFile) && SearchOptions.testFlag(Et::SearchOptionE::RequireFolder)))
    {
        if (SearchOptions.testFlag(Et::SearchOptionE::RequireFile)) {
            Modifiers += "files:";
        }
        else {
            Modifiers += "folders:";
        }
    }

    Everything_SetSearchW((Modifiers + "\"" + String + "\"").toStdWString().c_str());

    if (!Everything_QueryW(true)) {
        CheckErrorCausedByIPC();
        return false;
    }

    return true;
}

Et::ItemDataT IEverything::GetResultData(uint32_t Index)
{
    Et::ItemDataT Result;

    // Get type
    //
    if (Everything_IsFileResult(Index)) {
        Result.Type = Et::ItemTypeE::File;
    }
    else if (Everything_IsFolderResult(Index)) {
        Result.Type = Et::ItemTypeE::Folder;
    }
    else {
        Result.Type = Et::ItemTypeE::Unknown;
    }

    // Get full path name
    //
    uint32_t FullPathNameCount = Everything_GetResultFullPathNameW(Index, nullptr, 0);
    if (FullPathNameCount == 0) {
        FatalError("Everything_GetResultFullPathNameW() first returned 0.");
    }

    Result.FullPathName.resize(FullPathNameCount);
    uint32_t FullPathNameCopiedCount = Everything_GetResultFullPathNameW(Index, (wchar_t*)Result.FullPathName.data(), Result.FullPathName.size() + 1);
    if (FullPathNameCopiedCount == 0) {
        FatalError("Everything_GetResultFullPathNameW() second returned 0.");
    }

    // Get file name
    //
    const wchar_t *FileName = Everything_GetResultFileNameW(Index);
    if (FileName == nullptr) {
        FatalError("Everything_GetResultFileNameW() returned nullptr.");
    }
    Result.FileName = QString::fromWCharArray(FileName);

    // Get size
    //
    LARGE_INTEGER Size;
    if (!Everything_GetResultSize(Index, &Size)) {
        Result.DataSize = std::nullopt;
    }
    else {
        Result.DataSize = Size.QuadPart;
    }

    // Get accessed date
    //
    FILETIME AccessedDate;
    SYSTEMTIME ConvertedAccessedDate;
    if (Everything_GetResultDateAccessed(Index, &AccessedDate) && FileTimeToSystemTime(&AccessedDate, &ConvertedAccessedDate)) {
        Result.AccessedDate = System::SystemTimeToQDateTime(ConvertedAccessedDate);
    }

    return Result;
}

void IEverything::CheckErrorCausedByIPC()
{
    if (_IsLaunched && GetErrorCode() == EVERYTHING_ERROR_IPC) {
        FatalError(QObject::tr("Everything client is not running.\nPlease try to reopen VsCacheCleaner."), false);
    }
}

// Workaround begin
//

template <class CharT, class>
HWND IEverything::DetourFindWindow(const CharT *ClassName, const CharT *WindowName)
{
    QString RealClassName;
    if constexpr (std::is_same_v<CharT, char>) {
        RealClassName = QString{ClassName};
    }
    else {
        RealClassName = QString::fromWCharArray(ClassName);
    }

    // Append the instance name
    //
    if (RealClassName == "EVERYTHING_TASKBAR_NOTIFICATION") {
        RealClassName += "_(VsCacheCleaner)";
    }

    if constexpr (std::is_same_v<CharT, char>) {
        return _FnOriginalFindWindow<char>(RealClassName.toStdString().c_str(), WindowName);
    }
    else {
        return _FnOriginalFindWindow<wchar_t>(RealClassName.toStdWString().c_str(), WindowName);
    }
}
