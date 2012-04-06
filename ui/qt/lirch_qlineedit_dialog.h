#ifndef LIRCH_QLINEEDIT_DIALOG_H
#define LIRCH_QLINEEDIT_DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
    class LirchQLineEditDialog;
}

class LirchQLineEditDialog : public QDialog {
    Q_OBJECT
public:
    LirchQLineEditDialog(QWidget *parent = 0);
    ~LirchQLineEditDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::LirchQLineEditDialog *ui;

signals:
    void submit(QString, bool);

private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
};

#endif // LIRCH_QLINEEDIT_DIALOG_H
