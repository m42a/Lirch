#ifndef LIRCH_QTABWIDGET_H
#define LIRCH_QTABWIDGET_H

#include <QTabWidget>

class LirchQTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit LirchQTabWidget(QWidget *parent = 0);

signals:

public slots:
    void closeTab(int index);
};

#endif // LIRCH_QTABWIDGET_H
