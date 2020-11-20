#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VsCacheCleaner.h"

class VsCacheCleaner : public QMainWindow
{
    Q_OBJECT

public:
    VsCacheCleaner(QWidget *parent = Q_NULLPTR);

private:
    Ui::VsCacheCleanerClass ui;
};
