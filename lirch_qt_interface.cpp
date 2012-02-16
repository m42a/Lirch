#include "lirch_qt_interface.h"
#include "ui_lirch_qt_interface.h"

LirchQtInterface::LirchQtInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LirchQtInterface)
{
    ui->setupUi(this);
}

LirchQtInterface::~LirchQtInterface()
{
    delete ui;
}

void LirchQtInterface::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void LirchQtInterface::on_msgSendButton_clicked()
{
    QMessageBox::warning(this,
                         tr("Unimplemented Feature"),
                         tr("'%1' could not be sent.").arg("// TODO get message text"),
                         QMessageBox::Ok);
}

void LirchQtInterface::on_actionAbout_triggered()
{
    QMessageBox::information(this,
                             tr("About Lirch %1").arg(LIRCH_VERSION_STRING),
                             tr("// TODO insert Copyright and License info"));
}
