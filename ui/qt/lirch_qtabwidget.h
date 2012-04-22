#ifndef LIRCH_QT_TAB_WIDGET_H
#define LIRCH_QT_TAB_WIDGET_H

#include <QTabWidget>

class LirchQtTabWidget : public QTabWidget
{
Q_OBJECT
public:
    explicit LirchQtTabWidget(QWidget *parent = 0);

signals:

public slots:
    void closeTab(int index);
};

#endif // LIRCH_QT_TAB_WIDGET_H
