#include "ui/qt/lirch_qt_tab_widget.h"

LirchQtTabWidget::LirchQtTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

void LirchQtTabWidget::closeTab(int index) {
    this->removeTab(index);
}
