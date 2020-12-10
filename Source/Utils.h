#pragma once

#include <QObject>
#include <QString>
#include <functional>

#include <Windows.h>


#if !defined TO_STRING && !defined __TO_STRING
# define __TO_STRING(value)      # value
# define TO_STRING(value)        __TO_STRING(value)
#endif

#define DEFER_EX(var, lambda)    DeferT var { lambda }
#define DEFER(lambda)            DEFER_EX(__DefererTemp_ ## __LINE__, lambda)

#define VCC_ASSERT(condition)    while (!(condition)) { FatalError("File: " __FILE__ "\nLine: " TO_STRING(__LINE__) "\nCode: " # condition, true); }


namespace Constant
{
    constexpr inline uint64_t SizeKb = 1024;
    constexpr inline uint64_t SizeMb = SizeKb * 1024;
    constexpr inline uint64_t SizeGb = SizeMb * 1024;
    constexpr inline uint64_t SizeTb = SizeGb * 1024;

} // namespace Constant

class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable &) = delete;
    NonCopyable& operator=(const NonCopyable &) = delete;
};

template <class FunctionT>
class DeferT final : NonCopyable
{
public:
    explicit inline DeferT(FunctionT Function) :
        _Function{std::move(Function)}
    {
    }

    inline ~DeferT()
    {
        if (!_IsCanceled) {
            _Function();
        }
    }

    inline void Cancel()
    {
        _IsCanceled = true;
    }

private:
    FunctionT _Function;
    bool _IsCanceled = false;

};

[[noreturn]] void FatalError(const QString &Message, bool Report = true);

namespace Text
{
    QString FormatDataSize(uint64_t DataSize);
    QString FormatTime(const QDateTime &DateTime);

} // namespace Text

namespace File
{
    void OpenFolder(const QString &Path);
    bool MoveToTrash(const QString &File);

} // namespace File

namespace Process
{
    struct InfoT
    {
        uint32_t Id;
        uint32_t ParentId;
        uint32_t ThreadsCount;
        uint32_t ThreadsBasePriority;
        QString Name;
    };

    bool EnumProcesses(const std::function<bool(const InfoT &Info)> &Callback);

} // namespace Process

namespace System
{
    QDateTime SystemTimeToQDateTime(const SYSTEMTIME &Time);
    bool SingleInstance(const QString &InstanceName);

} // namespace System
