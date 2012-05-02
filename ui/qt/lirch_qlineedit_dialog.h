#ifndef LIRCH_QLINEEDIT_DIALOG_H
#define LIRCH_QLINEEDIT_DIALOG_H

#include <QDialog>
#include <QString>
#include <QTimer>

namespace Ui {
	class LirchQLineEditDialog;
}

class LirchQLineEditDialog : public QDialog {
	Q_OBJECT
public:
	explicit LirchQLineEditDialog(QWidget * = 0);
	virtual ~LirchQLineEditDialog();
	void setLineEditText(const QString &);
	void setLabelText(const QString &);
	void includeCheckbox(bool);

protected:
	void changeEvent(QEvent *);

private:
	Ui::LirchQLineEditDialog *ui;

signals:
	void submitted(QString, bool);

private slots:
	void accept();
};

#endif // LIRCH_QLINEEDIT_DIALOG_H
