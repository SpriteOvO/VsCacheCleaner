#include "MainWindow.h"

#include <QDir>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QStorageInfo>
#include <QDesktopServices>

#include "Config.h"


namespace UserInterface
{
    std::atomic<bool> requireExit = false;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // CacheItemsHeaderViewT
    //

    CacheItemsHeaderViewT::CacheItemsHeaderViewT(Qt::Orientation orientation, QWidget *parent) :
        QHeaderView{orientation, parent}
    {
        // Disable sort by CheckBox column
        //
        _lastSortIndicator = qMakePair((int)ItemsTableColumnE::Size, Qt::SortOrder::DescendingOrder);
        connect(this, &QHeaderView::sortIndicatorChanged, this,
            [this](int logicalIndex, Qt::SortOrder order)
            {
                if (logicalIndex == (int)ItemsTableColumnE::CheckBox) {
                    setSortIndicator(_lastSortIndicator.first, _lastSortIndicator.second);
                }
                else {
                    _lastSortIndicator = qMakePair(logicalIndex, order);
                }
            }
        );
        setSortIndicator(_lastSortIndicator.first, _lastSortIndicator.second);

        // Handle "check all" CheckBox click action
        //
        connect(this, &QHeaderView::sectionClicked, this,
            [this](int logicalIndex)
            {
                if (logicalIndex != (int)ItemsTableColumnE::CheckBox) {
                    return;
                }

                int checkAllState = stateCheckAll();
                stateCheckAllChanged(checkAllState == Qt::Unchecked || checkAllState == Qt::PartiallyChecked);
                updateSection(logicalIndex);
            }
        );

        setSectionsClickable(true);
    }

    void CacheItemsHeaderViewT::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
    {
        painter->save();
        {
            QHeaderView::paintSection(painter, rect, logicalIndex);
        }
        painter->restore();

        // Draw "check all" CheckBox
        //
        if (logicalIndex == (int)ItemsTableColumnE::CheckBox)
        {
            QStyleOptionButton option;

            option.rect = rect;
            option.state = (!_isCheckBoxDisabled ? QStyle::State_Enabled : QStyle::State_None) | QStyle::State_Active;

            switch (stateCheckAll())
            {
            case Qt::Unchecked:
                option.state |= QStyle::State_Off;
                break;
            case Qt::PartiallyChecked:
                option.state |= QStyle::State_NoChange;
                break;
            case Qt::Checked:
                option.state |= QStyle::State_On;
                break;
            }

            style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
        }
    }

    void CacheItemsHeaderViewT::setCheckBoxDisabled(bool value)
    {
        _isCheckBoxDisabled = value;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // CacheItemsDelegateT
    //

    void CacheItemsDelegateT::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // This function is a HACK for aligning CheckBox and Focus to the center
        //

        if (index.column() != (int)ItemsTableColumnE::CheckBox) {
            QItemDelegate::paint(painter, option, index);
            return;
        }

        Q_ASSERT(index.isValid());

        QStyleOptionViewItem options = setOptions(index, option);

        painter->save();
        {
            Qt::CheckState checkState = Qt::Unchecked;
            QVariant value = index.data(Qt::CheckStateRole);
            if (value.isValid()) {
                checkState = (Qt::CheckState)(value.toInt());
            }

            drawBackground(painter, options, index);
            drawCheck(painter, options, options.rect, checkState);
            // drawFocus(painter, options, options.rect);
        }
        painter->restore();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // CacheItemsTableModelT
    //

    int CacheItemsTableModelT::rowCount(const QModelIndex &parent) const
    {
        // Note: When implementing a table based model, rowCount() should return 0 when the parent is valid.
        // See: https://doc.qt.io/qt-5/qabstractitemmodel.html#rowCount
        //
        if (parent.isValid()) {
            return 0;
        }
        return _cacheItems.size();
    }

    int CacheItemsTableModelT::columnCount(const QModelIndex &parent) const
    {
        // Same as above
        //
        if (parent.isValid()) {
            return 0;
        }
        return (int)ItemsTableColumnE::_Count;
    }

    QVariant CacheItemsTableModelT::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) {
            return QVariant{};
        }

        const auto &[itemData, itemView] = _cacheItems.at(index.row());

        switch ((ItemsTableColumnE)index.column())
        {
        case ItemsTableColumnE::CheckBox:
            if (role != Qt::CheckStateRole) {
                break;
            }
            return itemView.isChecked ? Qt::Checked : Qt::Unchecked;

        case ItemsTableColumnE::Path:
            if (role != Qt::DisplayRole) {
                break;
            }
            return itemData.FullPathName;

        case ItemsTableColumnE::AccessedDate:
            if (role != Qt::DisplayRole) {
                break;
            }
            return itemView.displayAccessedDate;

        case ItemsTableColumnE::Size:
            if (role != Qt::DisplayRole) {
                break;
            }
            return itemView.displaySize;
        }

        return QVariant{};
    }

    bool CacheItemsTableModelT::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (!index.isValid()) {
            return false;
        }

        switch ((ItemsTableColumnE)index.column())
        {
        case ItemsTableColumnE::CheckBox:
        {
            if (role != Qt::CheckStateRole || value.type() != QVariant::Int) {
                break;
            }

            bool &isChecked = _cacheItems[index.row()].second.isChecked;
            bool newChecked = value.toInt() == Qt::Checked;

            if (isChecked != newChecked) {
                isChecked = newChecked;
                updateCheckAllState(newChecked);
                dataChanged(index, index, {role});
            }
            return true;
        }
        }

        return false;
    }

    QVariant CacheItemsTableModelT::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Orientation::Horizontal) {
            return QVariant{};
        }

        switch ((ItemsTableColumnE)section)
        {
        case ItemsTableColumnE::CheckBox:
            // Do not show content for CheckBox column
            //
            return QVariant{};

        case ItemsTableColumnE::Path:
            if (role != Qt::DisplayRole) {
                break;
            }
            return tr("Path");

        case ItemsTableColumnE::Size:
            if (role != Qt::DisplayRole) {
                break;
            }
            return tr("Size");

        case ItemsTableColumnE::AccessedDate:
            if (role != Qt::DisplayRole) {
                break;
            }
            return tr("AccessedDate");
        }

        return QVariant{};
    }

    Qt::ItemFlags CacheItemsTableModelT::flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);

        if (!index.isValid()) {
            return flags;
        }

        switch ((ItemsTableColumnE)index.column())
        {
        case ItemsTableColumnE::CheckBox:
            flags |= Qt::ItemIsUserCheckable;
            flags &= ~Qt::ItemIsSelectable;
            return flags;

        default:
            break;
        }

        return flags;
    }

    void CacheItemsTableModelT::sort(int column, Qt::SortOrder order)
    {
        beginResetModel();
        {
            sortCacheItems(_cacheItems, (ItemsTableColumnE)column, order);
        }
        endResetModel();
    }

    void CacheItemsTableModelT::sortCacheItems(
        QVector<QPair<Et::ItemDataT, ItemViewDataT>> &cacheItems,
        ItemsTableColumnE column,
        Qt::SortOrder order
    )
    {
        switch (column)
        {
        case ItemsTableColumnE::CheckBox:
            // Do not sort for CheckBox column
            //
            break;

        case ItemsTableColumnE::Path:
            qSort(cacheItems.begin(), cacheItems.end(),
                [order](const auto &value1, const auto &value2)
                {
                    if (order == Qt::AscendingOrder) {
                        return value1.first.FullPathName < value2.first.FullPathName;
                    }
                    else {
                        return value2.first.FullPathName < value1.first.FullPathName;
                    }
                }
            );
            break;

        case ItemsTableColumnE::AccessedDate:
            qSort(cacheItems.begin(), cacheItems.end(),
                [order](const auto &value1, const auto &value2)
                {
                    const auto &date1 = value1.first.AccessedDate;
                    const auto &date2 = value2.first.AccessedDate;

                    if (!date1.isValid() || !date2.isValid()) {
                        return false;
                    }

                    if (order == Qt::AscendingOrder) {
                        return date1 < date2;
                    }
                    else {
                        return date2 < date1;
                    }
                }
            );
            break;

        case ItemsTableColumnE::Size:
            qSort(cacheItems.begin(), cacheItems.end(),
                [order](const auto &value1, const auto &value2)
                {
                    const auto &size1 = value1.first.DataSize;
                    const auto &size2 = value2.first.DataSize;

                    if (!size1.has_value() || !size2.has_value()) {
                        return false;
                    }

                    if (order == Qt::AscendingOrder) {
                        return size1.value() < size2.value();
                    }
                    else {
                        return size2.value() < size1.value();
                    }
                }
            );
            break;
        }
    }

    QVector<QPair<Et::ItemDataT, ItemViewDataT>> CacheItemsTableModelT::getData()
    {
        return _cacheItems;
    }

    void CacheItemsTableModelT::updateData(const QVector<Et::ItemDataT> &items)
    {
        QVector<QPair<Et::ItemDataT, ItemViewDataT>> data;

        data.resize(items.size());

        for (int i = 0; i < data.size(); ++i)
        {
            // Qt slot function does not support && argument.
            //
            Et::ItemDataT item = items[i];
            ItemViewDataT viewData;

            viewData.isChecked = false;
            viewData.displayAccessedDate = item.AccessedDate.isValid() ? Text::FormatTime(item.AccessedDate) : QString{};
            viewData.displaySize = item.DataSize.has_value() ? Text::FormatDataSize(item.DataSize.value()) : QString{};

            data[i] = qMakePair(std::move(item), std::move(viewData));
        }

        beginResetModel();
        {
            _cacheItems = std::move(data);
            resetCheckAllState();

            // Sort by horizontal header current rule
            //
            const auto &[sortIndex, sortOrder] = horizontalHeaderSortIndicator();
            sort(sortIndex, sortOrder);
        }
        endResetModel();
    }

    void CacheItemsTableModelT::clearData()
    {
        beginResetModel();
        {
            _cacheItems.clear();
        }
        endResetModel();
    }

    int CacheItemsTableModelT::stateCheckAll() const
    {
        return _stateCheckAll;
    }

    uint32_t CacheItemsTableModelT::checkByRule(CheckItemsRuleE rule, uint64_t argument)
    {
        if (_cacheItems.empty()) {
            return 0;
        }

        uint32_t checkedCount = 0;

        switch (rule)
        {
        case CheckItemsRuleE::ByAccessedDate:
        {
            QDateTime agoDateTime = QDateTime::currentDateTime().addDays(-(int64_t)argument);
            beginResetModel();
            {
                checkAll(false);

                for (auto &[itemData, itemView] : _cacheItems)
                {
                    if (!itemData.AccessedDate.isValid()) {
                        continue;
                    }

                    if (itemData.AccessedDate < agoDateTime) {
                        itemView.isChecked = true;
                        ++checkedCount;
                    }
                }
                updateCheckAllState(true, checkedCount);
            }
            endResetModel();
            return checkedCount;
        }
        case CheckItemsRuleE::ByDataSize:
        {
            beginResetModel();
            {
                checkAll(false);

                for (auto &[itemData, itemView] : _cacheItems)
                {
                    if (!itemData.DataSize.has_value()) {
                        continue;
                    }

                    if (itemData.DataSize.value() >= argument) {
                        itemView.isChecked = true;
                        ++checkedCount;
                    }
                }
                updateCheckAllState(true, checkedCount);
            }
            endResetModel();
            return checkedCount;
        }
        }

        return 0;
    }

    void CacheItemsTableModelT::checkPart(const QModelIndexList &indexList, bool state)
    {
        beginResetModel();
        {
            int changedCount = 0;
            for (const QModelIndex &index : indexList)
            {
                if (!index.isValid() || index.column() != (int)ItemsTableColumnE::CheckBox) {
                    continue;
                }

                bool &isChecked = _cacheItems[index.row()].second.isChecked;

                if (isChecked != state) {
                    isChecked = state;
                    ++changedCount;
                }
            }
            updateCheckAllState(state, changedCount);
        }
        endResetModel();
    }

    void CacheItemsTableModelT::checkAll(bool state)
    {
        if (_cacheItems.empty()) {
            return;
        }

        beginResetModel();
        {
            for (auto &[itemData, itemView] : _cacheItems) {
                itemView.isChecked = state;
            }
            updateCheckAllState(state, _cacheItems.size());
        }
        endResetModel();
    }

    void CacheItemsTableModelT::updateCheckAllState(bool updateState, int updateCount)
    {
        if (updateCount == 0) {
            return;
        }
        _checkedCount = std::clamp(_checkedCount + (updateState ? updateCount : -updateCount), 0, _cacheItems.size());
        _stateCheckAll = _checkedCount == 0 ? Qt::Unchecked : (_checkedCount == _cacheItems.size() ? Qt::Checked : Qt::PartiallyChecked);
    }

    void CacheItemsTableModelT::resetCheckAllState()
    {
        _checkedCount = 0;
        _stateCheckAll = Qt::Unchecked;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // CacheItemsRcMenuT
    //

    CacheItemsRcMenuT::CacheItemsRcMenuT(QWidget *parent) :
        QMenu{parent}
    {
        _openFolderAction = addCommonAction(tr("Open folder"));
        _copyPathAction = addCommonAction(tr("Copy path"));
        _checkSelectedAction = addCommonAction(tr("Check selected items"));
        _uncheckSelectedAction = addCommonAction(tr("Uncheck selected items"));

        _openFolderAction->setShortcut(QKeySequence::Open);
        _copyPathAction->setShortcut(QKeySequence::Copy);

        connectCommonAction(_openFolderAction, ActionIdE::OpenFolder);
        connectCommonAction(_copyPathAction, ActionIdE::CopyPath);
        connectCommonAction(_checkSelectedAction, ActionIdE::CheckSelected);
        connectCommonAction(_uncheckSelectedAction, ActionIdE::UncheckSelected);
    }

    void CacheItemsRcMenuT::menuRequested(const QPoint &pos)
    {
        QItemSelectionModel *selectionModel = itemsSelectionModel();
        if (!selectionModel->hasSelection()) {
            return;
        }

        QModelIndexList indexList = selectionModel->selectedRows((int)ItemsTableColumnE::CheckBox);
        bool isOnlyOneSelected = indexList.size() == 1;

        _openFolderAction->setVisible(isOnlyOneSelected);
        _copyPathAction->setVisible(isOnlyOneSelected);

        move(cursor().pos());
        show();
    }

    void CacheItemsRcMenuT::connectCommonAction(QAction *action, ActionIdE actionId)
    {
        connect(action, &QAction::triggered, this, std::bind(&CacheItemsRcMenuT::commonActionClicked, this, actionId));
    }

    void CacheItemsRcMenuT::commonActionClicked(ActionIdE actionId)
    {
        QItemSelectionModel *selectionModel = itemsSelectionModel();
        if (!selectionModel->hasSelection()) {
            return;
        }

        auto *cacheItemsTableModel = (CacheItemsTableModelT*)selectionModel->model();

        switch (actionId)
        {
        case CacheItemsRcMenuT::ActionIdE::OpenFolder:
        {
            QModelIndexList indexList = selectionModel->selectedRows((int)ItemsTableColumnE::Path);
            if (indexList.size() != 1) {
                break;
            }

            QVariant data = indexList.front().data();
            if (data.type() == QVariant::String) {
                File::OpenFolder(data.toString());
            }
            break;
        }
        case CacheItemsRcMenuT::ActionIdE::CheckSelected:
        {
            QModelIndexList indexList = selectionModel->selectedRows((int)ItemsTableColumnE::CheckBox);
            cacheItemsTableModel->checkPart(indexList, true);
            break;
        }
        case CacheItemsRcMenuT::ActionIdE::UncheckSelected:
        {
            QModelIndexList indexList = selectionModel->selectedRows((int)ItemsTableColumnE::CheckBox);
            cacheItemsTableModel->checkPart(indexList, false);
            break;
        }
        case CacheItemsRcMenuT::ActionIdE::CopyPath:
        {
            QModelIndexList indexList = selectionModel->selectedRows((int)ItemsTableColumnE::Path);
            if (indexList.size() != 1) {
                break;
            }

            QVariant data = indexList.front().data();
            if (data.type() == QVariant::String) {
                QApplication::clipboard()->setText(data.toString());
            }
            break;
        }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // MainWindowT
    //

    MainWindowT::MainWindowT(QWidget *parent)
        : QMainWindow(parent)
    {
        _ui.setupUi(this);

        setWindowIcon(QIcon{VCC_RESOURCE_ICON});

        DetectVsRunning();

        IEverything::GetInstance().LaunchBackend();

        // static const char clearButtonActionNameC[] = "_q_qlineeditclearaction";
        // Defined in qtbase\src\widgets\widgets\qlineedit.cpp
        //
        // See more: https://forum.qt.io/topic/94478/enable-the-clear-button-action-in-a-read-only-qlineedit
        //
        _pathClearAction = _ui.lineEditPath->findChild<QAction*>("_q_qlineeditclearaction");
        VCC_ASSERT(_pathClearAction != nullptr);

        // Initialize table view
        //
        auto *cacheItemsTableModel = new CacheItemsTableModelT{_ui.tableViewItems};
        auto *cacheItemsDelegate = new CacheItemsDelegateT{_ui.tableViewItems};
        auto *cacheItemsHeader = new CacheItemsHeaderViewT{Qt::Orientation::Horizontal, _ui.tableViewItems};
        _cacheItemsRcMenu = new CacheItemsRcMenuT{this};

        _ui.tableViewItems->setModel(cacheItemsTableModel);
        _ui.tableViewItems->setItemDelegate(cacheItemsDelegate);
        _ui.tableViewItems->setHorizontalHeader(cacheItemsHeader);
        _ui.tableViewItems->setContextMenuPolicy(Qt::CustomContextMenu);

        cacheItemsHeader->setSectionResizeMode((int)ItemsTableColumnE::CheckBox, QHeaderView::ResizeToContents);
        cacheItemsHeader->setSectionResizeMode((int)ItemsTableColumnE::Path, QHeaderView::Stretch);
        cacheItemsHeader->setSectionResizeMode((int)ItemsTableColumnE::AccessedDate, QHeaderView::ResizeToContents);
        cacheItemsHeader->setSectionResizeMode((int)ItemsTableColumnE::Size, QHeaderView::ResizeToContents);

        // Initialize status bar
        //
        _statusLabelType = new QLabel{this};
        _statusLabelText = new QLabel{this};
        _ui.statusBar->insertPermanentWidget(0, _statusLabelType);
        _ui.statusBar->insertPermanentWidget(1, _statusLabelText, 1 /* Make it left-aligned */);

        // Initialize disk stats chart
        //
        auto *chart = new QChart;
        auto *chartView = new QChartView{chart, this};
        auto *percentBar = new QHorizontalPercentBarSeries{chart};
        _barSetCache = new QBarSet{tr("Cache"), percentBar};
        _barSetOther = new QBarSet{tr("Other"), percentBar};
        _barSetFree = new QBarSet{tr("Free"), percentBar};

        chart->addSeries(percentBar);
        chart->setMargins(QMargins{0, -10, 0, 0});
        chart->setAnimationOptions(QChart::AnimationOption::AllAnimations);
        chart->setBackgroundVisible(false);
        // chart->legend()->setAlignment(Qt::AlignBottom);
        chart->layout()->setContentsMargins(0, 0, 0, 0);

        chartView->setFixedHeight(40);

        _barSetCache->setColor(QColor{255, 22, 93});
        _barSetOther->setColor(QColor{255, 154, 0});
        _barSetOther->setColor(QColor{255, 154, 0});
        _barSetFree->setColor(QColor{62, 193, 211});

        _barSetCache->append(1);
        _barSetOther->append(1);
        _barSetFree->append(1);

        percentBar->append({_barSetCache, _barSetOther, _barSetFree});
        _ui.verticalLayout->addWidget(chartView);

        // Disable all controls
        //
        setControlsDisabled(true);

        // Connect thread-safety functions
        // The reason for not using "QMetaObject::invokeMethod" is that it does not support non-slot functions
        //
        qRegisterMetaType<StatusTypeE>("StatusTypeE");
        qRegisterMetaType<QVector<Et::ItemDataT>>("QVector<Et::ItemDataT>");

        connect(this, &MainWindowT::setControlsDisabledSafety, this, &MainWindowT::setControlsDisabled);
        connect(this, &MainWindowT::showStatusMessageSafety, this, &MainWindowT::showStatusMessage);
        connect(this, &MainWindowT::horizontalHeaderSortIndicatorSafety, this,
            [=]() {
                return qMakePair(cacheItemsHeader->sortIndicatorSection(), cacheItemsHeader->sortIndicatorOrder());
            },
            Qt::BlockingQueuedConnection // For return value
        );
        connect(this, &MainWindowT::getPathTextSafety, this,
            [this]() {
                return _ui.lineEditPath->text();
            },
            Qt::BlockingQueuedConnection // For return value
        );
        connect(this, &MainWindowT::updateDataSafety, this,
            [this](const QVector<Et::ItemDataT> &items) {
                getCacheItemsTableModel()->updateData(items);
            }
        );
        connect(this, &MainWindowT::updateDiskStatsChartSafety, this, &MainWindowT::updateDiskStatsChart);
        connect(this, &MainWindowT::getDataSafety, this,
            [this]() {
                return getCacheItemsTableModel()->getData();
            },
            Qt::BlockingQueuedConnection // For return value
        );
        connect(this, &MainWindowT::clearDataSafety, this, &MainWindowT::clearData);

        // Connect normal functions
        //
        connect(cacheItemsHeader, &CacheItemsHeaderViewT::stateCheckAll, cacheItemsTableModel, &CacheItemsTableModelT::stateCheckAll);
        connect(cacheItemsHeader, &CacheItemsHeaderViewT::stateCheckAllChanged, cacheItemsTableModel, &CacheItemsTableModelT::checkAll);
        connect(cacheItemsTableModel, &CacheItemsTableModelT::horizontalHeaderSortIndicator, this,
            [=]() {
                return qMakePair(cacheItemsHeader->sortIndicatorSection(), cacheItemsHeader->sortIndicatorOrder());
            }
        );
        connect(_cacheItemsRcMenu, &CacheItemsRcMenuT::itemsSelectionModel, this,
            [this] {
                return _ui.tableViewItems->selectionModel();
            }
        );
        connect(_pathClearAction, &QAction::triggered, this,
            [this] {
                if (_pathClearAction->isEnabled() && !_ui.lineEditPath->text().isEmpty()) {
                    clearData();
                }
            }
        );
        connect(_ui.pushButtonSelect, &QPushButton::clicked, this, &MainWindowT::selectButtonClicked);
        connect(_ui.pushButtonScan, &QPushButton::clicked, this, &MainWindowT::scanButtonClicked);
        connect(_ui.pushButtonDelete, &QPushButton::clicked, this, &MainWindowT::deleteButtonClicked);
        connect(_ui.tableViewItems, &QTableView::customContextMenuRequested, _cacheItemsRcMenu, &CacheItemsRcMenuT::menuRequested);

        initializeMenuActions();
        waitForEtInitialized();
    }

    MainWindowT::~MainWindowT()
    {
        requireExit = true;

        IEverything::GetInstance().ExitBackend();

        if (_observeThread.joinable()) {
            _observeThread.join();
        }

        if (_taskFuture.valid()) {
            _taskFuture.wait();
        }
    }

    void MainWindowT::DetectVsRunning()
    {
#if !defined _DEBUG
        QVector<Process::InfoT> vsProcesses;

        do {
            vsProcesses.clear();

            bool isSuccessed = Process::EnumProcesses(
                [&](const Process::InfoT &info)
                {
                    if (info.Name.toLower() == "devenv.exe") {
                        vsProcesses.append(info);
                    }
                    return true;
                }
            );

            // Ignore the error directly
            //
            if (!isSuccessed) {
                return;
            }

            if (!vsProcesses.isEmpty())
            {
                QString processesStr;

                for (const Process::InfoT &info : vsProcesses) {
                    processesStr += " - " + info.Name + " (ID " + QString::number(info.Id) + ")\n";
                }

                QMessageBox::StandardButton button = QMessageBox::warning(this, "VsCacheCleaner",
                    tr("Detected the Visual Studio is running.\n\n%1\nPlease close the above processes and try again.").arg(processesStr),
                    QMessageBox::Abort | QMessageBox::Retry
                );

                if (button == QMessageBox::Abort) {
                    exit(EXIT_FAILURE);
                }
            }

        } while (!vsProcesses.isEmpty());
#endif
    }

    CacheItemsTableModelT* MainWindowT::getCacheItemsTableModel()
    {
        return (CacheItemsTableModelT*)_ui.tableViewItems->model();
    }

    void MainWindowT::setControlsDisabled(bool value)
    {
        _pathClearAction->setDisabled(value);
        _ui.tableViewItems->setDisabled(value);
        _ui.pushButtonSelect->setDisabled(value);
        _ui.pushButtonScan->setDisabled(value);
        _ui.pushButtonDelete->setDisabled(value);
        _ui.menuRule->setDisabled(value);
        ((CacheItemsHeaderViewT*)_ui.tableViewItems->horizontalHeader())->setCheckBoxDisabled(value);
    }

    void MainWindowT::initializeMenuActions()
    {
        auto *cacheItemsTableModel = getCacheItemsTableModel();

        const auto &connectRuleAction = [&](QAction *action, CheckItemsRuleE selectRule, uint64_t selectArgument)
        {
            connect(action, &QAction::triggered, cacheItemsTableModel,
                [=]() {
                    uint32_t checkedCount = cacheItemsTableModel->checkByRule(selectRule, selectArgument);
                    showStatusMessage(StatusTypeE::Info, tr("Checked %1 items.").arg(checkedCount));
                }
            );
        };

        connectRuleAction(_ui.actionMenuCheckItemsAccessed7DaysAgo, CheckItemsRuleE::ByAccessedDate, 7);
        connectRuleAction(_ui.actionMenuCheckItemsAccessed1MonthAgo, CheckItemsRuleE::ByAccessedDate, 30);
        connectRuleAction(_ui.actionMenuCheckItemsAccessed3MonthsAgo, CheckItemsRuleE::ByAccessedDate, 90);
        connectRuleAction(_ui.actionMenuCheckItemsAccessed6MonthsAgo, CheckItemsRuleE::ByAccessedDate, 180);
        connectRuleAction(_ui.actionMenuCheckItemsAccessed1YearAgo, CheckItemsRuleE::ByAccessedDate, 365);

        connectRuleAction(_ui.actionMenuCheckItemsGreaterThan50MbInSize, CheckItemsRuleE::ByDataSize, 50 * Constant::SizeMb);
        connectRuleAction(_ui.actionMenuCheckItemsGreaterThan100MbInSize, CheckItemsRuleE::ByDataSize, 100 * Constant::SizeMb);
        connectRuleAction(_ui.actionMenuCheckItemsGreaterThan500MbInSize, CheckItemsRuleE::ByDataSize, 500 * Constant::SizeMb);
        connectRuleAction(_ui.actionMenuCheckItemsGreaterThan1GbInSize, CheckItemsRuleE::ByDataSize, 1 * Constant::SizeGb);
        connectRuleAction(_ui.actionMenuCheckItemsGreaterThan3GbInSize, CheckItemsRuleE::ByDataSize, 3 * Constant::SizeGb);

        connect(_ui.actionReportIssue, &QAction::triggered, this, std::bind(&MainWindowT::helpMenuActionClicked, this, HelpActionIdE::ReportIssue));
        connect(_ui.actionCheckUpdate, &QAction::triggered, this, std::bind(&MainWindowT::helpMenuActionClicked, this, HelpActionIdE::CheckUpdate));
        connect(_ui.actionAbout, &QAction::triggered, this, std::bind(&MainWindowT::helpMenuActionClicked, this, HelpActionIdE::About));
    }

    void MainWindowT::asyncTask(std::function<void()> task, std::function<void()> observer_)
    {
        using namespace std::literals::chrono_literals;

        if (_observeThread.joinable()) {
            _observeThread.join();
        }

        _taskFuture = std::async(std::launch::async, std::move(task));

        if (!observer_) {
            return;
        }

        _observeThread = std::thread{
            [this, observer{std::move(observer_)}]
            {
                while (_taskFuture.wait_for(200ms) != std::future_status::ready)
                {
                    if (requireExit) {
                        return;
                    }
                    observer();
                }
            }
        };
    }

    void MainWindowT::waitForEtInitialized()
    {
        using namespace std::literals::chrono_literals;

        // Wait for "Everything" index disk
        //
        asyncTask(
            [this]()
            {
                auto &everything = IEverything::GetInstance();

                while (!everything.IsInitialized())
                {
                    if (requireExit) {
                        return;
                    }
                    std::this_thread::sleep_for(100ms);
                }

                setControlsDisabledSafety(false);
                showReady();
            },
            [this]()
            {
                static size_t waitedCount = 0;
                showStatusMessageSafety(
                    StatusTypeE::Info,
                    tr("Everything client is initializing, please wait%1 (First time may be a little slow)").arg(QString{'.'}.repeated(waitedCount % 7))
                );
                ++waitedCount;
            }
        );
    }

    void MainWindowT::showStatusMessage(StatusTypeE statusType, const QString &text)
    {
        QString type;

        switch (statusType)
        {
        case StatusTypeE::Info:
            type = tr("Info");
            break;
        case StatusTypeE::Warning:
            type = tr("Warning");
            break;
        case StatusTypeE::Error:
            type = tr("Error");
            break;
        }

        _statusLabelType->setText(type);
        _statusLabelText->setText(text);
    }

    void MainWindowT::updateDiskStatsChart(quint64 cache)
    {
        // Get disk(s) space information
        //
        uint64_t total = 0, free = 0, other = 0;
        QString scanPath = _ui.lineEditPath->text();

        if (scanPath.isEmpty())
        {
            // All disks
            //
            QList<QStorageInfo> storages = QStorageInfo::mountedVolumes();
            VCC_ASSERT(!storages.isEmpty());

            for (const QStorageInfo &storage : storages)
            {
                qint64 storageTotal = storage.bytesTotal();
                qint64 storageFree = storage.bytesFree();

                if (storageTotal == -1 || storageFree == -1) {
                    continue;
                }
                total += storageTotal;
                free += storageFree;
            }
        }
        else
        {
            // Single disk
            //
            QFileInfo file{scanPath};
            VCC_ASSERT(file.isDir());

            QStorageInfo storage{file.dir()};

            qint64 storageTotal = storage.bytesTotal();
            qint64 storageFree = storage.bytesFree();

            if (storageTotal != -1 && storageFree != -1) {
                total = storageTotal;
                free = storageFree;
            }
        }

        if (total == 0) {
            resetDiskStatsChart();
            return;
        }

        VCC_ASSERT(cache + free <= total);
        other = total - cache - free;

        _barSetCache->setLabel(tr("Cache (%1)").arg(Text::FormatDataSize(cache)));
        _barSetOther->setLabel(tr("Other (%1)").arg(Text::FormatDataSize(other)));
        _barSetFree->setLabel(tr("Free (%1)").arg(Text::FormatDataSize(free)));

        _barSetCache->replace(0, cache);
        _barSetOther->replace(0, other);
        _barSetFree->replace(0, free);
    }

    void MainWindowT::resetDiskStatsChart()
    {
        _barSetCache->setLabel(tr("Cache"));
        _barSetOther->setLabel(tr("Other"));
        _barSetFree->setLabel(tr("Free"));

        _barSetCache->replace(0, 1);
        _barSetOther->replace(0, 1);
        _barSetFree->replace(0, 1);
    }

    void MainWindowT::clearData()
    {
        resetDiskStatsChart();
        getCacheItemsTableModel()->clearData();
    }

    void MainWindowT::showReady()
    {
        showStatusMessageSafety(StatusTypeE::Info, tr("Ready."));
    }

    void MainWindowT::selectButtonClicked()
    {
        QString path = QFileDialog::getExistingDirectory(this, tr("Select Path"), QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (path.isEmpty()) {
            return;
        }

        _ui.lineEditPath->setText(QDir::toNativeSeparators(path));
        clearData();
        showReady();
    }

    void MainWindowT::scanButtonClicked()
    {
        // Wait for "Everything" scan
        //
        asyncTask(
            [this]()
            {
                auto &everything = IEverything::GetInstance();

                setControlsDisabledSafety(true);
                DEFER(
                    [this]() {
                        setControlsDisabledSafety(false);
                    }
                );

                clearDataSafety();

                QString scanPath = getPathTextSafety(), scanString;
                bool isScanPathEmpty = scanPath.isEmpty();

                if (!isScanPathEmpty)
                {
                    if (!QFileInfo{scanPath}.isDir()) {
                        showStatusMessageSafety(StatusTypeE::Error, tr("Scan path is not a directory."));
                        return;
                    }

                    // Add a directory separator at the end if it does not exist
                    //
                    if (scanPath.back() != '\\') {
                        scanPath += '\\';
                    }

                    scanString = scanPath + "*\\.vs";
                }
                else {
                    scanString = "*\\.vs";
                }

                // Scan .vs folders (not including the .vs folder in the current directory of "scanPath", if any)
                //
                std::optional<QVector<Et::ItemDataT>> opItems = IEverything::GetInstance().Search(
                    scanString,
                    Et::SearchOptionE::RequireFolder | Et::SearchOptionE::RequireAccessedDate | Et::SearchOptionE::MatchWholeFileName
                );

                if (!opItems.has_value()) {
                    showStatusMessageSafety(StatusTypeE::Error, tr("Scan failed. ErrorCode: %1-%2").arg(1).arg(IEverything::GetInstance().GetErrorCode()));
                    return;
                }

                if (!isScanPathEmpty)
                {
                    // Scan the .vs folder in the current directory
                    //
                    scanString = scanPath + ".vs";

                    std::optional<QVector<Et::ItemDataT>> opCurrentItem = IEverything::GetInstance().Search(
                        scanString,
                        Et::SearchOptionE::RequireFolder | Et::SearchOptionE::RequireAccessedDate | Et::SearchOptionE::MatchWholeFileName
                    );

                    if (!opCurrentItem.has_value()) {
                        showStatusMessageSafety(StatusTypeE::Error, tr("Scan failed. ErrorCode: %1-%2").arg(2).arg(IEverything::GetInstance().GetErrorCode()));
                        return;
                    }

                    if (!opCurrentItem->empty()) {
                        VCC_ASSERT(opCurrentItem->size() == 1);
                        opItems->push_front(std::move(opCurrentItem->front()));
                    }
                }

                // Filter out "...\Microsoft Visual Studio\2019\Community\Common7\IDE\.vs" "C:\Users\XXX\.vs" and count cache size
                //
                static QVector<QString> filterList = {
                    "\\ide\\.vs",
                    QDir::toNativeSeparators(QDir::home().filePath(".vs")).toLower()
                };

                QVector<Et::ItemDataT> &items = opItems.value();
                quint64 cacheTotal = 0;

                items.erase(std::remove_if(items.begin(), items.end(),
                    [&](const Et::ItemDataT &item)
                    {
                        for (const QString &filterPath : filterList) {
                            if (item.FullPathName.toLower().indexOf(filterPath) != -1) {
                                return true;
                            }
                        }

                        if (item.DataSize.has_value()) {
                            cacheTotal += item.DataSize.value();
                        }
                        return false;
                    }
                ), items.end());

                updateDiskStatsChartSafety(cacheTotal);
                updateDataSafety(opItems.value());
                showStatusMessageSafety(StatusTypeE::Info, tr("Scan completed. %1 results.").arg(opItems->size()));
            },
            [this]() {
                static size_t waitedCount = 0;
                showStatusMessageSafety(StatusTypeE::Info, tr("Scanning%1").arg(QString{'.'}.repeated(waitedCount % 7)));
                ++waitedCount;
            }
        );
    }

    void MainWindowT::deleteButtonClicked()
    {
        asyncTask(
            [this]()
            {
                setControlsDisabledSafety(true);
                DEFER(
                    [this]() {
                        setControlsDisabledSafety(false);
                    }
                );

                int total = 0, successed = 0, failed = 0;
                quint64 deletedSize = 0;
                QVector<QPair<Et::ItemDataT, ItemViewDataT>> cacheItems = getDataSafety();

                // Count the items to be deleted
                //
                for (const auto &[itemData, itemView] : cacheItems) {
                    if (itemView.isChecked) {
                        ++total;
                    }
                }

                if (total == 0) {
                    showStatusMessageSafety(StatusTypeE::Warning, tr("No items are checked."));
                    return;
                }

                // Sort by path descending order
                // This can make subfolders be deleted first if the folders have parent-child relationships
                //
                CacheItemsTableModelT::sortCacheItems(cacheItems, ItemsTableColumnE::Path, Qt::DescendingOrder);

                // Delete caches
                //
                cacheItems.erase(std::remove_if(cacheItems.begin(), cacheItems.end(),
                    [&](const QPair<Et::ItemDataT, ItemViewDataT> &item)
                    {
                        const auto &[itemData, itemView] = item;

                        if (!itemView.isChecked || requireExit) {
                            return false;
                        }

                        bool isSuccessed = File::MoveToTrash(itemData.FullPathName);

                        if (!isSuccessed) {
                            ++failed;
                        }
                        else {
                            ++successed;
                            if (itemData.DataSize.has_value()) {
                                deletedSize += itemData.DataSize.value();
                            }
                        }

                        static size_t waitedCount = 0;
                        showStatusMessageSafety(
                            StatusTypeE::Info,
                            tr("(%1/%2) Deleting caches%3 (%4)")
                            .arg(successed + failed).arg(total)
                            .arg(QString{'.'}.repeated(waitedCount % 7))
                            .arg(Text::FormatDataSize(deletedSize)));
                        ++waitedCount;

                        return isSuccessed;
                    }
                ), cacheItems.end());

                showStatusMessageSafety(
                    StatusTypeE::Info,
                    tr("Delete completed. %1 cache cleared. (%2 successed, %3 failed)")
                    .arg(Text::FormatDataSize(deletedSize))
                    .arg(successed).arg(failed));

                // Update data
                //
                QVector<Et::ItemDataT> newData;
                quint64 newCacheTotal = 0;

                newData.resize(cacheItems.size());
                for (int i = 0; i < cacheItems.size(); ++i)
                {
                    newData[i] = std::move(cacheItems[i].first);
                    if (newData[i].DataSize.has_value()) {
                        newCacheTotal += newData[i].DataSize.value();
                    }
                }
                updateDiskStatsChartSafety(newCacheTotal);
                updateDataSafety(newData);
            }
        );
    }

    void MainWindowT::helpMenuActionClicked(HelpActionIdE actionId)
    {
        switch (actionId)
        {
        case HelpActionIdE::ReportIssue:
            QDesktopServices::openUrl(QUrl{VCC_URL_ISSUES});
            break;

        case HelpActionIdE::CheckUpdate:
            QDesktopServices::openUrl(QUrl{VCC_URL_RELEASES});
            break;

        case HelpActionIdE::About:
        {
            QString content = tr(
                "<h3>VsCacheCleaner</h3>"
                "<p>Visual Studio Solution Cache Cleaner</p>"
                "<hr>"
                "<p>Version %1 (<a href=\"%2\">Change log</a>)</p>"
                "<p>Open source on <a href=\"%3\">%4</a></p>"
                "<p>Licensed under the <a href=\"%5\">%6</a></p>"
                "<p>Folder search is based on <a href=\"%7\">%8</a></p>"
                "%9")
                .arg(VCC_VERSION_STRING).arg(VCC_URL_CURRENT_RELEASE)
                .arg(VCC_URL_REPO).arg("GitHub")
                .arg(VCC_URL_LICENSE).arg(VCC_LICENSE)
                .arg("https://www.voidtools.com/").arg("Everything")
                .arg("<p>Copyright © 2020-2021 SpriteOvO. All rights reserved.</p>");

            QMessageBox::about(this, tr("About VsCacheCleaner"), content);
            break;
        }
        }
    }

} // namespace UserInterface

////////////////////////////////////////////////////////////////////////////////////////////////////
// main
//

int main(int argc, char *argv[])
{
    if (!System::SingleInstance("VsCacheCleaner")) {
        return 0;
    }

    QApplication application(argc, argv);

    // Install translator, ignore errors
    //
    QTranslator translator;
    QDir translationFolder = QCoreApplication::applicationDirPath();
    translationFolder.cd("translations");
    translator.load(QLocale{}, "vcc", "_", translationFolder.path());
    application.installTranslator(&translator);

    UserInterface::MainWindowT window;
    window.show();
    return application.exec();
}
