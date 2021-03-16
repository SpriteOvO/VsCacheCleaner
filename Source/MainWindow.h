#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"

#include <QtCharts>
#include <QPair>
#include <QPainter>
#include <QMouseEvent>
#include <QItemDelegate>
#include <QAbstractItemView>
#include <atomic>
#include <future>
#include <thread>

#include "IEverything.h"


namespace UserInterface
{
    enum class CheckItemsRuleE : uint32_t
    {
        ByAccessedDate, // in days
        ByDataSize,     // in bytes
    };

    enum class ItemsTableColumnE : int
    {
        CheckBox,
        Path,
        AccessedDate,
        Size,

        _Count
    };

    struct ItemViewDataT
    {
        bool isChecked;
        QString displayAccessedDate;
        QString displaySize;
    };

    // Used to draw checkbox in header
    //
    class CacheItemsHeaderViewT : public QHeaderView
    {
        Q_OBJECT

    public:
        explicit CacheItemsHeaderViewT(Qt::Orientation orientation, QWidget *parent = nullptr);

        void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

    public Q_SLOTS:
        void setCheckBoxDisabled(bool value);

    Q_SIGNALS:
        int stateCheckAll() const;
        void stateCheckAllChanged(bool value);

    private:
        bool _isCheckBoxDisabled = false;
        QPair<int, Qt::SortOrder> _lastSortIndicator;

    };

    class CacheItemsDelegateT : public QItemDelegate
    {
        Q_OBJECT

    public:
        using QItemDelegate::QItemDelegate;

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    };

    class CacheItemsTableModelT : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        using QAbstractTableModel::QAbstractTableModel;

        int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
        int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

        static void sortCacheItems(
            QVector<QPair<Et::ItemDataT, ItemViewDataT>> &cacheItems,
            ItemsTableColumnE column,
            Qt::SortOrder order
        );

    public Q_SLOTS:
        QVector<QPair<Et::ItemDataT, ItemViewDataT>> getData();
        void updateData(const QVector<Et::ItemDataT> &items);
        void clearData();
        uint32_t checkByRule(CheckItemsRuleE rule, uint64_t argument);
        void checkPart(const QModelIndexList &indexList, bool state);
        void checkAll(bool state);
        int stateCheckAll() const;

    Q_SIGNALS:
        QPair<int, Qt::SortOrder> horizontalHeaderSortIndicator();

    private:
        QVector<QPair<Et::ItemDataT, ItemViewDataT>> _cacheItems;
        int _checkedCount = 0;
        int _stateCheckAll = Qt::Unchecked;

        void updateCheckAllState(bool updateState, int updateCount = 1);
        void resetCheckAllState();
    };

    // Rc = Right-click
    //
    class CacheItemsRcMenuT : public QMenu
    {
        Q_OBJECT

    public:
        CacheItemsRcMenuT(QWidget *parent = nullptr);

    Q_SIGNALS:
        QItemSelectionModel *itemsSelectionModel() const;

    public Q_SLOTS:
        void menuRequested(const QPoint &pos);

    private:
        enum class ActionIdE : uint32_t
        {
            OpenFolder,
            CheckSelected,
            UncheckSelected,
            CopyPath,
        };

        QAction *_openFolderAction;
        QAction *_copyPathAction;
        QAction *_checkSelectedAction;
        QAction *_uncheckSelectedAction;

        template <class ...ArgsT>
        QAction* addCommonAction(ArgsT &&...args)
        {
            QAction *action = addAction(std::forward<ArgsT>(args)...);
            parentWidget()->addAction(action);
            return action;
        }

        void connectCommonAction(QAction *action, ActionIdE actionId);

    private Q_SLOTS:
        void commonActionClicked(ActionIdE actionId);

    };

    class MainWindowT : public QMainWindow
    {
        Q_OBJECT

    public:
        enum class StatusTypeE : uint32_t
        {
            Info,
            Warning,
            Error
        };

        MainWindowT(QWidget *parent = Q_NULLPTR);
        ~MainWindowT();

    Q_SIGNALS:
        void setControlsDisabledSafety(bool isDisable);
        void showStatusMessageSafety(StatusTypeE statusType, const QString &text);
        QPair<int, Qt::SortOrder> horizontalHeaderSortIndicatorSafety();
        QString getPathTextSafety();
        QVector<QPair<Et::ItemDataT, UserInterface::ItemViewDataT>> getDataSafety();
        void updateDataSafety(const QVector<Et::ItemDataT> &items);
        void updateDiskStatsChartSafety(quint64 cache);
        void clearDataSafety();

    private:
        enum class HelpActionIdE : uint32_t
        {
            ReportIssue,
            CheckUpdate,
            About
        };

        Ui::VsCacheCleanerClass _ui;
        QAction *_pathClearAction;
        QLabel *_statusLabelType, *_statusLabelText;
        CacheItemsRcMenuT *_cacheItemsRcMenu;
        QBarSet *_barSetCache, *_barSetOther, *_barSetFree;

        std::future<void> _taskFuture;
        std::thread _observeThread;

        void DetectVsRunning();
        CacheItemsTableModelT* getCacheItemsTableModel();
        void setControlsDisabled(bool value);
        void initializeMenuActions();
        void asyncTask(std::function<void()> task, std::function<void()> observer = std::function<void()>{});
        void waitForEtInitialized();
        void showStatusMessage(StatusTypeE statusType, const QString &text);
        void updateDiskStatsChart(quint64 cache);
        void resetDiskStatsChart();
        void clearData();
        void showReady();

    private Q_SLOTS:
        void selectButtonClicked();
        void scanButtonClicked();
        void deleteButtonClicked();
        void helpMenuActionClicked(HelpActionIdE actionId);

    };

} // namespace UserInterface
