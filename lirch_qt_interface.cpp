#include "client.h"
#include "ui_client.h"

LirchQtInterface::LirchQtInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Client)
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
                         tr("'%1' could not be sent.").arg("[use ui to fetch msgTextArea.getText()]"),
                         QMessageBox::Ok);
}

void LirchQtInterface::on_actionAbout_triggered()
{
    QMessageBox::information(this,
                             tr("About Lirch %1").arg(LIRCH_VERSION_STRING),
                             tr("// TODO insert Copyright and License info"));
}
