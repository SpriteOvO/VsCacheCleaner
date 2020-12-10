//
// Everything-SDK Wrapper
//

#pragma once

#include <QDateTime>
#include <QVector>
#include <QString>
#include <optional>

#include <Windows.h>

#include "Utils.h"


namespace Et
{
    enum class ItemTypeE : uint32_t
    {
        Unknown = 0,
        File,
        Folder
    };

    enum class SearchOptionE : uint32_t
    {
        // Require type
        //
        RequireFile            = (1 << 0),
        RequireFolder          = (1 << 1),

        // Require data
        //
        RequireAccessedDate    = (1 << 5),

        // Match rule
        //
        MatchCase              = (1 << 16),
        MatchWholeWord         = (1 << 17),
        MatchWholeFileName     = (1 << 18)
    };
    Q_DECLARE_FLAGS(SearchOptionsT, SearchOptionE)
    Q_DECLARE_OPERATORS_FOR_FLAGS(SearchOptionsT);

    struct ItemDataT
    {
        ItemTypeE Type;
        QString FullPathName;
        QString FileName;
        std::optional<uint64_t> DataSize;
        QDateTime AccessedDate;
    };

} // namespace Et

class IEverything
{
public:
    static IEverything& GetInstance();

    IEverything();
    ~IEverything();

    void LaunchBackend();
    void ExitBackend();

    bool IsInitialized();
    uint32_t GetErrorCode();

    std::optional<QVector<Et::ItemDataT>> Search(
        const QString &String,
        const Et::SearchOptionsT &SearchOptions
    );

private:
    std::atomic<bool> _IsLaunched = false;
    PROCESS_INFORMATION _ProcessInfo;

    void PrepareLaunch();
    bool SearchQuery(const QString &String, const Et::SearchOptionsT &SearchOptions);
    Et::ItemDataT GetResultData(uint32_t Index);

    void CheckErrorCausedByIPC();

    // Workaround to make the Everything-SDK support multiple instances
    //

    template <class CharT, class = std::enable_if_t<std::is_same_v<CharT, char> || std::is_same_v<CharT, wchar_t>>>
    using FnFindWindowT = HWND(__stdcall *)(const CharT *ClassName, const CharT *WindowName);

    template <class CharT, class = std::enable_if_t<std::is_same_v<CharT, char> || std::is_same_v<CharT, wchar_t>>>
    static inline FnFindWindowT<CharT> _FnOriginalFindWindow;

    template <class CharT, class = std::enable_if_t<std::is_same_v<CharT, char> || std::is_same_v<CharT, wchar_t>>>
    static HWND __stdcall DetourFindWindow(const CharT *ClassName, const CharT *WindowName);
};
