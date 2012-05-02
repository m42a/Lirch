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

#include "ui/qt/lirch_qt_interface.h"
#include "ui/qt/ui_lirch_qt_interface.h"
#include "ui/qt/lirch_qlineedit_dialog.h"
#include "ui/qt/lirch_qtabwidget.h"
#include "ui/qt/lirch_setup_wizard.h"

class Ui::Channel {
	// A channel has a name and an associated tab
	QString name;
	QWidget *tab;
	// These reference the UI
	QTabWidget *tabs;
	QListView *list;
	// These reference data models
	QTextDocument *messages;
	QTextCursor *cursor;
	QTextFrameFormat frame_format;
	QStandardItemModel *users;
public:
	Channel(const QString &channel_name, Ui::LirchQtInterface *ui) :
		name(channel_name),
		tabs(ui->chatTabWidget),
		list(ui->chatUserList)
	{
		// TODO Check to see if tab is duplicated
		tab = new QWidget(tabs);
		tab->setWindowTitle("#" + name);
		int index = tabs->currentIndex();
		tabs->insertTab(index, tab, name);
		// TODO add QMenuItem with show action
		users = new QStandardItemModel(0, 1, tab);
		// Create a view and set its model
		QTextBrowser *browser = new QTextBrowser(tab);
		messages = new QTextDocument(browser);
		cursor = new QTextCursor(messages);
		browser->setDocument(messages);
		// Set up the layout with this view
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
		layout->addWidget(browser);
		tab->setLayout(layout);
	}
	void update_users(const QSet<QString> &new_users) {
		users->clear();
		for (auto &new_user : new_users) {
			users->appendRow(new QStandardItem(new_user));
		}
	}
	// TODO make message reception do something to the tabs
	void add_message(const QString& message, bool show_timestamp = false, bool ignore_message = false) {
		qDebug() << message + " (stamp:" + show_timestamp + ",ignore:" + ignore_message + ")";
		// TODO package data into frames FIXME subclass QTextFrame to track hidden fields
		if (!ignore_message) {
			QTextFrame *message_frame = cursor->insertFrame(frame_format);
			QString timestamp = "[ " + QTime::currentTime().toString() + "] ";
			if (show_timestamp) {
				message_frame->firstCursorPosition().insertText(timestamp + message);
			} else {
				message_frame->firstCursorPosition().insertText(message);
			}
		}
	}
	void grab_focus() const {
		int index = tabs->indexOf(tab);
		if (index != -1) {
			tabs->setCurrentIndex(index);
			list->setModel(users);
		} else {
			// TODO debug message
		}
	}
	void reload_messages(bool show_ignored = false, bool show_timestamps = false) {
		// TODO once hidden fields are stored, we can turn them off and on	
	}
};

// QT UI

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
    client_pipe = nullptr;

    // Add in the default channel where it is meant to go
    QString default_channel_name = tr(LIRCH_DEFAULT_CHANNEL);
    Ui::Channel default_channel(default_channel_name, ui);
    channels.insert(default_channel_name, default_channel);

    // Setup system tray TODO settings and QIcon for this?
    system_tray_icon = nullptr;
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        system_tray_icon = new QSystemTrayIcon(this);
    }

    // Load up the settings and kick things off
    loadSettings();
}

LirchQtInterface::~LirchQtInterface()
{
    // Save settings on destruction
    saveSettings();
    // Cleanup the UI
    delete ui;
}

// settings-related (IMPORTANT: always edit these functions as a pair)

void LirchQtInterface::loadSettings()
{
    // Make sure to document any changes in the settings schema (change below and on wiki)
    settings.beginGroup("UserData");
    default_nick = settings.value("nick", LIRCH_DEFAULT_NICK).value<QString>();
    settings.endGroup();
    // Load persisted view state
    settings.beginGroup("QtMainWindow");
    resize(settings.value("size", QSize(640, 480)).toSize());
    move(settings.value("position", QPoint(100, 100)).toPoint());
    ui->chatLayout->restoreState(settings.value("splitter").toByteArray());
    ui->actionViewSendButton->setChecked(settings.value("show_msgSendButton", true).value<bool>());
    ui->actionViewUserList->setChecked(settings.value("show_chatUserList", true).value<bool>());
    settings.endGroup();
    // Load persisted model settings
    settings.beginGroup("ChatView");
    settings.beginGroup("Messages");
    show_ignored_messages = settings.value("show_ignored_messages", false).value<bool>();
    show_message_timestamps = settings.value("show_message_timestamps", true).value<bool>();
    settings.endGroup();
    settings.endGroup();
}

void LirchQtInterface::saveSettings()
{
    // Make sure to document any changes in the settings schema (change above and on wiki)
    settings.beginGroup("UserData");
    settings.setValue("nick", default_nick);
    settings.endGroup();
    settings.beginGroup("QtMainWindow");
    settings.setValue("size", size());
    settings.setValue("position", pos());
    settings.setValue("splitter", ui->chatLayout->saveState());
    settings.setValue("show_msgSendButton", ui->actionViewSendButton->isChecked());
    settings.setValue("show_chatUserList", ui->actionViewUserList->isChecked());
    settings.endGroup();
    settings.beginGroup("ChatView");
    settings.beginGroup("Messages");
    settings.setValue("show_ignored_messages", show_ignored_messages);
    settings.setValue("show_message_timestamps", show_message_timestamps);
    settings.endGroup();
    settings.endGroup();
}

// EVENT RELATED

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

bool LirchQtInterface::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->msgTextBox) {
        // Filter FocusIn events on the text box to select all text therein
        if (event->type() == QEvent::FocusIn) {
            // FIXME there has to be a better way to do this
            QTimer::singleShot(0, object, SLOT(selectAll()));
            return true;
        }
        return false;
    }
    return QMainWindow::eventFilter(object, event);
}

void LirchQtInterface::on_msgSendButton_clicked()
{
	// Get the text to send
	QString text = ui->msgTextBox->text();
	// Ignore empty case
	if (text.isEmpty()) {
		return;
	}
	// TODO modify to have connected state
	request_edict_send(text, true);
	// TODO here is the failure condition, when does this happen?
	// QMessageBox::warning(this,
	//                     tr("Unimplemented Feature"),
	//                     tr("'%1' could not be sent.").arg(text),
	//                     QMessageBox::Ok);
	// Don't forget to clear the text from the box
	ui->msgTextBox->clear();
}

// FILE MENU

void LirchQtInterface::on_actionConnect_triggered(bool checked)
{
	// TODO actual delegation to core (core forwards to antenna)
	if (checked) {
		// FIXME send leave message
	} else {
		// FIXME send join message
	}
}

// new-related

void LirchQtInterface::on_actionNewChannel_triggered()
{
	// FIXME get channel name interactively, send /join
	QString name = QTime::currentTime().toString();
	Ui::Channel channel(name, ui);
	channels.insert(name, channel);
	channel.grab_focus();
}

void LirchQtInterface::on_actionNewTransfer_triggered()
{
    alert_user(tr("The %1 feature is forthcoming.").arg("New > File Transfer"));
}

// log-related (TODO make a UI for this)

void LirchQtInterface::on_actionSaveLog_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Log File"), "./", tr("Logs (*.log)"));
}

void LirchQtInterface::on_actionOpenLog_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Log File"), "./", tr("Logs (*.log)"));
}

// EDIT MENU (TODO these UIs need work)

void LirchQtInterface::on_actionEditNick_triggered()
{
	LirchQLineEditDialog nick_dialog;
	connect(&nick_dialog, SIGNAL(submit(QString, bool)),
		this, SLOT(request_nick_change(QString, bool)));
	nick_dialog.exec();
}

void LirchQtInterface::on_actionEditIgnored_triggered()
{
	LirchQLineEditDialog ignore_dialog;
	connect(&ignore_dialog, SIGNAL(submit(QString, bool)),
		this, SLOT(request_block_ignore(QString, bool)));
	ignore_dialog.exec();
}

// VIEW MENU

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

// TODO make these use a helper function

void LirchQtInterface::on_actionViewTimestamps_toggled(bool checked)
{
	show_message_timestamps = checked;
	auto itr = channels.find(ui->chatTabWidget->tabText(ui->chatTabWidget->currentIndex()));
	if (itr != channels.end()) {
		auto &channel = itr.value();
		channel.reload_messages(show_message_timestamps, show_ignored_messages);
	}
}

void LirchQtInterface::on_actionViewIgnored_toggled(bool checked)
{
	show_ignored_messages = checked;
	auto itr = channels.find(ui->chatTabWidget->tabText(ui->chatTabWidget->currentIndex()));
	if (itr != channels.end()) {
		auto &channel = itr.value();
		channel.reload_messages(show_message_timestamps, show_ignored_messages);
	}
}

// tab-related

void LirchQtInterface::on_actionViewDefault_triggered()
{
	auto itr = channels.find(tr(LIRCH_DEFAULT_CHANNEL));
	if (itr != channels.end()) {
		auto &channel = itr.value();
		channel.grab_focus();
	}
}

void LirchQtInterface::on_actionViewTransfers_triggered()
{
	alert_user(tr("The %1 feature is forthcoming.").arg("View > File Tranfers"));
}

// ABOUT MENU

void LirchQtInterface::on_actionWizard_triggered()
{
	LirchSetupWizard setup_wizard;
	if (setup_wizard.exec()) {
		QString nick = setup_wizard.get_nick();
		if (setup_wizard.nick_is_default()) {
			default_nick = nick;
		}
		logging_message::logging_options options;
		options |= logging_message::logging_option::SET_DIRECTORY;
		options |= logging_message::logging_option::SET_MODE;
		logging_message log_data(options);
		log_data.set_directory(setup_wizard.get_logging_directory());
		log_data.set_mode(setup_wizard.get_logging_mode());
		client_pipe->send(logging_message::create(log_data));
	}
}

void LirchQtInterface::on_actionAbout_triggered()
{
	QMessageBox about_box;
	about_box.setIcon(QMessageBox::Information);
	about_box.setWindowTitle(tr("About %1 %2").arg(
		LIRCH_PRODUCT_NAME,
		LIRCH_VERSION_STRING));
	about_box.setText(tr("%1 %2 (%3) is Copyright (c) %4, %5 (see README)").arg(
		LIRCH_PRODUCT_NAME,
		LIRCH_VERSION_STRING,
		LIRCH_BUILD_HASH,
		LIRCH_COPYRIGHT_YEAR,
		LIRCH_COMPANY_NAME));
	about_box.setInformativeText("Do you want to report a bug?");
	about_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	about_box.setDefaultButton(QMessageBox::No);
	if (about_box.exec() == QMessageBox::Yes) {
		QString date, time, title, body, url;
		date = QDate::currentDate().toString();
		time = QTime::currentTime().toString();
		title = tr("Auto-Generated Bug Report [%1 at %2]").arg(date, time);
		body = tr("I am experiencing the following issue with %1 %2 (%3):").arg(
			LIRCH_PRODUCT_NAME,
			LIRCH_VERSION_STRING,
			LIRCH_BUILD_HASH);
		url = tr("https://github.com/m42a/Lirch/issues/new?title=%1&body=%2");
		QDesktopServices::openUrl(QUrl(url.arg(title, body)));
	}
}

// INTERNAL SLOTS (re-implemented protected functions)

void LirchQtInterface::showEvent(QShowEvent *e)
{
	// TODO first-time QWizard to determine if these happen:
	// The first tab is always the default channel
	ui->actionViewDefault->trigger();
	// Fire the connect action
	ui->actionConnect->setChecked(true);
	// Get the textbox to focus on any Show
	ui->msgTextBox->setFocus(Qt::ActiveWindowFocusReason);
	// Propagate
	e->ignore();
}

void LirchQtInterface::closeEvent(QCloseEvent *e)
{
	// Any close request is accepted when the core is shutdown
	if (client_pipe == nullptr || !client_pipe->ready()) {
		// FIXME race condition on /q and /quit
		e->accept();
	}
	// Confirm the close (potentially ignore)
	auto result = QMessageBox::question(this,
		tr("Close Prompt"),
		tr("Are you sure you want to quit %1?").arg(LIRCH_PRODUCT_NAME),
		QMessageBox::Yes,
		QMessageBox::No,
		QMessageBox::NoButton);
        if (result == QMessageBox::Yes) {
		e->accept();
	} else {
		e->ignore();
	}
}

// UTILITY FUNCTIONS

// Provides a method for conveniently alerting the user
void LirchQtInterface::alert_user(QString msg)
{
	// Empty alerts trigger fatal errors to encourage proper alerts
	if (msg.isEmpty()) {
		die(tr("Empty user alert message."), false);
	} else {
		QMessageBox::information(this, tr("Alert"), msg);
	}
}

// For loading a particular client pipe
void LirchQtInterface::use(LirchClientPipe *pipe)
{
	if (pipe != nullptr && pipe->ready()) {
		client_pipe = pipe;
		this->show();
	} else {
		QString fatal_msg = (pipe) ? pipe->name() : tr("(null)");
		this->die(tr("failure between core and %1").arg(fatal_msg), false);
	}
}

// For killing the GUI nicely 
void LirchQtInterface::die(QString msg, bool silent_but_deadly)
{
	if (!silent_but_deadly) {
		QMessageBox::information(this,
			tr("Fatal"),
			tr("Details: '%1'").arg(msg));
	}
	// TODO send a display message for logger with reason
	this->close();
}

// Occurs when display messages are received
void LirchQtInterface::display(QString nick, QString channel_name, QString text) {
	bool ignore_message = (ignored_users.find(nick) == ignored_users.end());
	// Find the tab's QTextBrowser's document (model) we desire
	auto itr = channels.find(channel_name);
	if (itr == channels.end()) {
		return;
	}
	auto &channel = itr.value();
	// This regex wraps links sent in display messages TODO fix
	text.replace(QRegExp("\\b(http://.*)\\b"), "<a href=\"\\1\">\\1</a>");
	// Show the message in the view
	channel.add_message(text, show_message_timestamps, ignore_message);
}

// Occurs when userlist messages are received
void LirchQtInterface::userlist(QMap<QString, QSet<QString>> data) {
	// For every channel
	for (auto datum = data.begin(); datum != data.end(); ++datum) {
		// Find the model if it exists
		auto channel = channels.find(datum.key());
		if (channel != channels.end()) {
			// Copy all the items over
			channel->update_users(datum.value());
		}
    	}
}

// Occurs when changed_nick messages are received
void LirchQtInterface::nick(QString new_nick, bool permanent)
{
	if (permanent)
	{
		default_nick = new_nick;
	}
}

// MESSAGE EMITTERS

void LirchQtInterface::request_edict_send(QString text, bool current)
{
	if (current) {
		QString channel_name = ui->chatTabWidget->tabText(ui->chatTabWidget->currentIndex());
		// The core will pass this raw edict to the meatgrinder
		client_pipe->send(raw_edict_message::create(text, channel_name));
	}
}

void LirchQtInterface::request_nick_change(QString new_nick, bool permanent) {
	// The core will pass this request to the userlist
	client_pipe->send(nick_message::create(new_nick, permanent));
}

void LirchQtInterface::request_block_ignore(QString name, bool block)
{
	block_message_subtype request_type = block_message_subtype::ADD;
	if (block) {
		// TODO cleanly convert the input into an IP address
		// FIXME should be able to lookup from last userlist
		client_pipe->send(block_message::create(request_type, QHostAddress(name)));
	} else {
		// TODO emit signal to redraw?
		ignored_users.insert(name);
	}
}

