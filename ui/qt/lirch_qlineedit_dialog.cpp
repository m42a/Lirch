#include "ui/qt/lirch_qlineedit_dialog.h"
#include "ui/qt/ui_lirch_qlineedit_dialog.h"

LirchQLineEditDialog::LirchQLineEditDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::LirchQLineEditDialog)
{
	ui->setupUi(this);
	QTimer::singleShot(0, ui->lineEdit, SLOT(selectAll()));
}

LirchQLineEditDialog::~LirchQLineEditDialog()
{
	delete ui;
}

void LirchQLineEditDialog::setLineEditText(const QString &text)
{
	ui->lineEdit->setText(text);
}

void LirchQLineEditDialog::setLabelText(const QString &text)
{
	ui->checkBox->setText(text);
}

void LirchQLineEditDialog::includeCheckbox(bool visible)
{
	ui->checkBox->setVisible(visible);
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

void LirchQLineEditDialog::accept()
{
	emit submitted(ui->lineEdit->text().trimmed(), ui->checkBox->checkState());
}
