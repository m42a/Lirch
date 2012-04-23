#include "ui/qt/lirch_qtabwidget.h"

LirchQTabWidget::LirchQTabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
}

void LirchQTabWidget::closeTab(int index) {
    // TODO integrate this with .ui and hookup channels
    this->removeTab(index);
}
