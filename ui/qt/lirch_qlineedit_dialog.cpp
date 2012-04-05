#include "lirch_qlineedit_dialog.h"
#include "ui_lirch_qlineedit_dialog.h"

LirchQLineEditDialog::LirchQLineEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LirchQLineEditDialog)
{
    ui->setupUi(this);
}

LirchQLineEditDialog::~LirchQLineEditDialog()
{
    delete ui;
}

void LirchQLineEditDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void LirchQLineEditDialog::on_buttonBox_accepted()
{
    emit submit(ui->lineEdit->text(), ui->checkBox->checkState());
}

void LirchQLineEditDialog::on_buttonBox_rejected()
{
    // Reject, don't submit
}
