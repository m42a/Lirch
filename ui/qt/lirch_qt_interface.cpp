#include "lirch_qt_interface.h"
#include "ui_lirch_qt_interface.h"

// TODO (not in order of importance)
// 1) Ugh, fix the layout somehow?
// 2) Setup Wizard
//    a) Nick menu
//    b) Ignore/Block menu
// 3) Tab Management (use QTabWidget or QWidget container?)
//    !) Representation of each chatView
//    ?) Default presentation
//    a) Integration with Menus
//    b) Integration with User Lists
// 4) Message Pipe interactions
//    a) Channel creation
//    b) Polling for participants

#include "plugins/lirch_plugin.h"
#include "plugins/edict_messages.h"

void run(plugin_pipe p, std::string name) {
    p.write(registration_message::create(LIRCH_MSG_PRI_REG_STAT, name, "display"));
    // TODO register for recieved messages
    while (true) {
        // Fetch a message from the pipe
        message m = p.blocking_read();
        // Determine what type of message it is
        if (m.type == LIRCH_MSG_TYPE_SHUTDOWN) {
            return;
        } else if (m.type == LIRCH_MSG_TYPE_REG_STAT) {
            // Recieved a registration status message
            auto reg = dynamic_cast<registration_status *>(m.getdata());
            // Or not...
            if (!reg) {
                continue;
            }
            // Try again on failure
            if (!reg->status) {
                if (reg->priority > 30000) {
                  // FIXME??? reg->decrement_priority(); instead of -1
                  p.write(registration_message::create(reg->priority - 1, name, reg->type));
                } else {
                  return;
                }
            }
        } else {
            // By default, echo the message with decremented priority
            p.write(m.decrement_priority());
        }
    }
};

LirchClientPipe::LirchClientPipe()
{
    // FIXME is has_connection necessary?
    static bool has_connection = false;
    hole = nullptr;
    if (!has_connection) {
        has_connection = true;
        hole = new plugin_pipe();
    }
}

LirchClientPipe::~LirchClientPipe()
{
    if (hole != nullptr) {
        delete hole;
    }
}

void LirchClientPipe::start()
{
    run(*hole, LIRCH_QT_INTERFACE_ID);
    emit stop("core_processor");
}

// TODO for Tor (by priority)
// Get QTUI to interact with Core (message display)
// Get QTUI to interact with Core (nick requests, blocking and polling)
// Write Wizard/nickchange widgets

LirchQtInterface::LirchQtInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LirchQtInterface),
    settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME)
{
    // Initialize the UI
    ui->setupUi(this);
    // Add a variety of UI enhancements (select on focus and quit action)
    ui->msgTextBox->installEventFilter(this);
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    // client_pipe facilitates communication with the core
    client_pipe = new LirchClientPipe();
    if (client_pipe->hole == nullptr) {
        emit fatal_error("client_pipe");
    } else {
        // Setup core_processor to run client_pipe
        QThread *core_processor = new QThread();
        client_pipe->moveToThread(core_processor);
        connect(core_processor, SIGNAL(started()), client_pipe, SLOT(start()));
        // When the client_pipe is done, quit the thread
        connect(client_pipe, SIGNAL(stop(QString)), core_processor, SLOT(quit()));
        // The first of these is required for cleanup, second can be done otherwise
        connect(client_pipe, SIGNAL(stop(QString)), client_pipe, SLOT(deleteLater()));
        connect(client_pipe, SIGNAL(stop(QString)), core_processor, SLOT(deleteLater()));
        // Kill the UI if core_processor receives shutdown (TODO should reconnect?)
        connect(client_pipe, SIGNAL(stop(QString)), this, SLOT(fatal_error(QString)));

        // Load up the settings and kick things off
        loadSettings();
        core_processor->start();
    }
}

LirchQtInterface::~LirchQtInterface()
{
    // Save settings on destruction, right? Right.
    saveSettings();

    // TODO free the client_pipe? Think this is handled above.
    // delete client_pipe;

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

void LirchQtInterface::loadSettings()
{
    settings.beginGroup("UserData");
    // TODO mitigate need for default nick
    nick = settings.value("nick", "").value<QString>();
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    resize(settings.value("size", QSize(640, 480)).toSize());
    move(settings.value("position", QPoint(100, 100)).toPoint());
    settings.endGroup();
    settings.beginGroup("ChatView");
    settings.beginGroup("Messages");
    show_ignored_messages = settings.value("show_ignored_messages", false).value<bool>();
    show_message_timestamps = settings.value("show_message_timestamps", true).value<bool>();
    settings.endGroup();
    settings.endGroup();
}

void LirchQtInterface::saveSettings()
{
    // TODO document schema for settings
    settings.beginGroup("UserData");
    settings.setValue("nick", nick);
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    settings.setValue("size", size());
    settings.setValue("position", pos());
    settings.endGroup();
    settings.beginGroup("ChatView");
    settings.beginGroup("Messages");
    settings.setValue("show_ignored_messages", show_ignored_messages);
    settings.setValue("show_message_timestamps", show_message_timestamps);
    settings.endGroup();
    settings.endGroup();
}

void LirchQtInterface::on_actionConnect_triggered(bool checked)
{
    // TODO actual delegation to core (core forwards to antenna)
    if (checked) {
        QMessageBox::information(this,
                                 tr("Alert"),
                                 tr("You connected!"));
    } else {
        QMessageBox::information(this,
                                 tr("Alert"),
                                 tr("You disconnected!"));
    }
}

bool LirchQtInterface::eventFilter(QObject *object, QEvent *event)
{
    // Filter FocusIn events on the text box to select all text therein
    if (event->type() == QEvent::FocusIn) {
        if (object == ui->msgTextBox) {
            // TODO there has to be a better way to do this
            QTimer::singleShot(0, object, SLOT(selectAll()));
            return false;
        }
    }
    return QObject::eventFilter(object, event);
}

void LirchQtInterface::on_msgSendButton_clicked()
{
    // Get the text to send
    QString text = ui->msgTextBox->text();
    // Ignore empty case
    if (text.isEmpty()) {
        return;
    }

    // All text is prefixed with the nick
    QString prefix = "<" + nick + "> ";
    // And potentially a timestamp
    if (show_message_timestamps) {
        prefix += "[" + QTime::currentTime().toString() + "] ";
    }

    // The core will pass this raw edict to the meatgrinder
    message edict = raw_edict_message::create(text, LIRCH_DEFAULT_CHANNEL_ID);
    client_pipe->hole->write(edict);

    // And send back a response
    message echo = client_pipe->hole->blocking_read();

    // FIXME cast echo to recieved{,me}
    //dynamic_cast<edict_message *>();
    //dynamic_cast<me_edict_message *>();
    // FIXME Update the proper model, default:
    //ui->chatViewArea->append(prefix);

    // FIXME here is the failure condition
    // QMessageBox::warning(this,
    //                     tr("Unimplemented Feature"),
    //                     tr("'%1' could not be sent.").arg(text),
    //                     QMessageBox::Ok);

    // Don't forget to clear the text from the box
    ui->msgTextBox->clear();
}

void LirchQtInterface::on_actionAbout_triggered()
{
    // TODO does this need to be its own widget?
    QMessageBox::information(this,
                             tr("About Lirch %1").arg(LIRCH_VERSION_STRING),
                             tr("Lirch %1 (%2) is Copyright (c) %3, %4 (see README)").arg(
                                     LIRCH_VERSION_STRING,
                                     LIRCH_BUILD_HASH,
                                     LIRCH_COPYRIGHT_YEAR,
                                     LIRCH_COMPANY_NAME));
}

// TODO is there some built-in stuff for things like these?

void LirchQtInterface::on_actionViewUserList_toggled(bool checked)
{
    if (checked) {
        ui->chatUserList->show();
    } else {
        ui->chatUserList->hide();
    }
}

void LirchQtInterface::on_actionViewSendButton_toggled(bool checked)
{
    if (checked) {
        ui->msgSendButton->show();
    } else {
        ui->msgSendButton->hide();
    }
}

void LirchQtInterface::on_actionViewTimestamps_toggled(bool checked)
{
    show_message_timestamps = checked;
}

void LirchQtInterface::on_actionViewIgnored_toggled(bool checked)
{
    show_ignored_messages = checked;
}

void LirchQtInterface::fatal_error(QString msg)
{
    QMessageBox::information(this,
                             tr("Fatal Error"),
                             tr("Details: '%1'").arg(msg));
    emit close();
}

void LirchQtInterface::on_actionNewChannel_triggered()
{
    QMessageBox::information(this,
                             tr("Unimplemented Feature"),
                             tr("The %1 feature is forthcoming.").arg("New > Private Channel"));
}

void LirchQtInterface::on_actionNewTransfer_triggered()
{
    QMessageBox::information(this,
                             tr("Unimplemented Feature"),
                             tr("The %1 feature is forthcoming.").arg("New > File Tranfer"));
}

void LirchQtInterface::on_actionViewDefault_triggered()
{
    int index = ui->chatTabWidget->indexOf(ui->defaultChannelTab);
    if (index != -1) {
        ui->chatTabWidget->setCurrentIndex(index);
    }
}

void LirchQtInterface::on_actionViewTransfers_triggered()
{
    QMessageBox::information(this,
                             tr("Unimplemented Feature"),
                             tr("The %1 feature is forthcoming.").arg("View > File Tranfers"));
}

void LirchQtInterface::on_actionWizard_triggered()
{
    QMessageBox::information(this,
                             tr("Unimplemented Feature"),
                             tr("The %1 feature is forthcoming.").arg("Help > Setup Wizard"));
}

void LirchQtInterface::on_actionSaveLog_triggered()
{
    QMessageBox::information(this,
                             tr("Confirmation"),
                             tr("Log saved: %1/default.log").arg(LIRCH_DEFAULT_LOG_DIR));
}

void LirchQtInterface::on_actionOpenLog_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Log File"), "./", tr("Logs (*.log)"));
    ui->chatLogArea->clear();
    ui->chatLogArea->append(filename);
    int index = ui->chatTabWidget->indexOf(ui->logChannelTab);
    if (index != -1) {
        ui->chatTabWidget->setCurrentIndex(index);
    }
}
