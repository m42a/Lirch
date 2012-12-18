// TODO left:
// 2) We need to be able to click on links
// 4) Make /q and /quit should exit without prompt
// 6) New ignores fire a redraw

#include "ui/qt/lirch_qt_interface.h"
#include "ui/qt/ui_lirch_qt_interface.h"
#include "ui/qt/lirch_qlineedit_dialog.h"
#include "ui/qt/lirch_setup_wizard.h"

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
	connect(ui->chatTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tab_changed(int)));

	files = new QFileSystemModel(this);
	// client_pipe facilitates communication with the core
	client_pipe = nullptr;

	// Change some colors
	QPalette palette;
	QColor light_yellow(0xFF, 0xFF, 0x99);
	palette.setColor(QPalette::Base, light_yellow);
	ui->msgTextBox->setPalette(palette);
	QColor light_blue(0xCC, 0xCC, 0xFF);
	palette.setColor(QPalette::Base, light_blue);
	ui->chatUserList->setPalette(palette);

	// Add in the default channel where it is meant to go
	QString default_channel_name = tr(LIRCH_DEFAULT_CHANNEL);
	LirchChannel *channel = new LirchChannel(default_channel_name, ui);
	channels.insert(default_channel_name, channel);

	// Setup system tray TODO settings and QIcon for this?
	system_tray_icon = nullptr;
	if (QSystemTrayIcon::isSystemTrayAvailable()) {
		system_tray_icon = new QSystemTrayIcon(this);
		// System Tray Menu
		QMenu *tray_icon_menu = new QMenu(this);
		QString label = tr("Minimize To Tray");
		QKeySequence sequence(Qt::CTRL + Qt::Key_M);
		QAction *show_hide = tray_icon_menu->addAction(label, this, SLOT(setVisible(bool)), sequence);
		// TODO work on getting menu functional
		show_hide->setCheckable(true);
		//show_hide->setChecked(true);
		tray_icon_menu->addAction(ui->actionQuit);
		system_tray_icon->setContextMenu(tray_icon_menu);
		// Assorted options
		system_tray_icon->setIcon(this->windowIcon());
		system_tray_icon->setToolTip(tr("%1 %2").arg(LIRCH_PRODUCT_NAME, LIRCH_VERSION_STRING));
		//system_tray_icon->show();
	}

	// Load up the settings and kick things off
	loadSettings();
}

LirchQtInterface::~LirchQtInterface()
{
	// Save settings on destruction
	saveSettings();
	// Remove all the tabs
	for (auto &channel : channels) {
		delete channel;
	}
	// Cleanup the UI
	delete ui;
}

// settings-related (IMPORTANT: always edit these functions as a pair)

void LirchQtInterface::loadSettings()
{
	// Load persisted view state
	settings.beginGroup("QtMainWindow");
	resize(settings.value("size", QSize(640, 480)).toSize());
	move(settings.value("position", QPoint(100, 100)).toPoint());
	ui->chatLayout->restoreState(settings.value("splitter").toByteArray());
	ui->actionViewSendButton->setChecked(settings.value("show_msgSendButton", true).value<bool>());
	ui->actionViewUserList->setChecked(settings.value("show_chatUserList", true).value<bool>());
	first_time_run = settings.value("first_time_run", true).value<bool>();
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
	settings.beginGroup("QtMainWindow");
	settings.setValue("size", size());
	settings.setValue("position", pos());
	settings.setValue("splitter", ui->chatLayout->saveState());
	settings.setValue("show_msgSendButton", ui->actionViewSendButton->isChecked());
	settings.setValue("show_chatUserList", ui->actionViewUserList->isChecked());
	settings.setValue("first_time_run", false);
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
	// FIXME there has to be a better way to do this
	if (object == ui->msgTextBox) {
		// Filter FocusIn events on the text box to select all text therein
		if (event->type() == QEvent::FocusIn) {
			QTimer::singleShot(0, object, SLOT(selectAll()));
		}
	} else {
		// FIXME intercept link clicks
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
	// FIXME modify to have connected state
	request_edict_send(text, ui->actionConnect->isChecked());
	// Don't forget to clear the text from the box
	ui->msgTextBox->clear();
}

// FILE MENU

void LirchQtInterface::on_actionConnect_triggered(bool checked)
{
	auto end = channels.end();
	if (checked) {
		for (auto itr = channels.begin(); itr != end; ++itr) {
			client_pipe->send(message::create<set_channel_message>(itr.key()));
		}
	} else {
		for (auto itr = channels.begin(); itr != end; ++itr) {
			client_pipe->send(message::create<leave_channel_message>(itr.key()));
			// TODO better message
			itr.value()->add_message(tr("Disconnected"), true, false);
		}
	}
}

// new-related

void LirchQtInterface::on_actionNewChannel_triggered()
{
	LirchQLineEditDialog channel_dialog;
	channel_dialog.setWindowTitle(tr("New Channel"));
	channel_dialog.includeCheckbox(false);
	connect(&channel_dialog, SIGNAL(submitted(QString, bool)),
		this, SLOT(request_new_channel(QString, bool)));
	channel_dialog.exec();
}

void LirchQtInterface::on_actionNewTransfer_triggered()
{
	alert_user(tr("The %1 feature is forthcoming.").arg("New > File Transfer"));
}

// log-related

void LirchQtInterface::on_actionSaveLog_triggered()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Log File"), "./", tr("Logs (*.log)"));
	if (!filename.isEmpty()) {
		// Begin the save process, get all necessary data
		QFile *file = new QFile(filename);
		auto tab_widget = ui->chatTabWidget;
		auto itr = channels.find(tab_widget->tabText(tab_widget->currentIndex()));
		if (itr != channels.end() && file->open(QIODevice::WriteOnly | QIODevice::Text)) {
			auto channel = itr.value();
			// Give the user some feedback with a widget
			QWidget *feedback = new QWidget(this, Qt::Dialog);
			feedback->setWindowModality(Qt::ApplicationModal);
			QProgressBar *progress_bar = new QProgressBar(feedback);
			QBoxLayout *layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
			layout->addWidget(progress_bar);
			feedback->setLayout(layout);
			// While we save the file in a thread
			QThread *saving_thread = new QThread(this);
			channel->prepare_persist(file);
			channel->moveToThread(saving_thread);
			// Begin, feeding back progress 
			connect(channel, SIGNAL(progress(int)), progress_bar, SLOT(setValue(int)));
			connect(saving_thread, SIGNAL(started()), channel, SLOT(persist()));
			connect(saving_thread, SIGNAL(started()), feedback, SLOT(show()));
			connect(channel, SIGNAL(persisted()), saving_thread, SLOT(quit()));
			connect(saving_thread, SIGNAL(finished()), feedback, SLOT(close()));
			connect(saving_thread, SIGNAL(finished()), file, SLOT(deleteLater()));
			connect(saving_thread, SIGNAL(finished()), saving_thread, SLOT(deleteLater()));
			saving_thread->start();
		} else {
			QMessageBox::information(this, tr("Error"), tr("Cannot save file: '%1'").arg(filename));
			delete file;
		}
	}
}

void LirchQtInterface::on_actionOpenLog_triggered()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Log File"), "./", tr("Logs (*.log)"));
	if (!filename.isEmpty()) {
		QFile file(filename);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			// Pop up a simple viewer
			QWidget *viewer = new QWidget(this, Qt::Dialog);
			viewer->setWindowTitle(tr("Log Viewer"));
			QTextEdit *notepad = new QTextEdit(viewer);
			// Pull UTF-8 from the file
			QTextStream reader(&file);
			reader.setCodec("UTF-8");
			while (!reader.atEnd()) {
				notepad->append(reader.readLine());
			}
			// Stick it all in a box
			QBoxLayout *layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
			layout->addWidget(notepad);
			viewer->setLayout(layout);
			viewer->show();
		} else {
			QMessageBox::information(this, tr("Error"), tr("Cannot read file: '%1'").arg(filename));
		}
	}
}

// EDIT MENU

void LirchQtInterface::on_actionEditNick_triggered()
{
	LirchQLineEditDialog nick_dialog;
	nick_dialog.setWindowTitle(tr("Edit Nick"));
	nick_dialog.setLineEditText(default_nick);
	nick_dialog.setLabelText(tr("Default"));
	connect(&nick_dialog, SIGNAL(submitted(QString, bool)),
		this, SLOT(request_nick_change(QString, bool)));
	nick_dialog.exec();
}

void LirchQtInterface::on_actionEditIgnored_triggered()
{
	LirchQLineEditDialog ignore_dialog;
	ignore_dialog.setWindowTitle("Add Ignore/Block");
	ignore_dialog.setLabelText(tr("Block"));
	connect(&ignore_dialog, SIGNAL(submitted(QString, bool)),
		this, SLOT(request_block_ignore(QString, bool)));
	ignore_dialog.exec();
}

void LirchQtInterface::on_actionEditShown_triggered()
{
	LirchQLineEditDialog unignore_dialog;
	unignore_dialog.setWindowTitle("Remove Ignore/Block");
	unignore_dialog.setLabelText(tr("Unblock"));
	connect(&unignore_dialog, SIGNAL(submitted(QString, bool)),
		this, SLOT(request_unblock_unignore(QString, bool)));
	unignore_dialog.exec();
}

// VIEW MENU

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
	for (auto &channel : channels) {
		channel->reload_messages(show_message_timestamps, show_ignored_messages);
	}
}

void LirchQtInterface::on_actionViewIgnored_toggled(bool checked)
{
	show_ignored_messages = checked;
	for (auto &channel : channels) {
		channel->reload_messages(show_message_timestamps, show_ignored_messages);
	}
}

void LirchQtInterface::on_actionViewTransfers_triggered()
{
	int index = ui->chatTabWidget->indexOf(ui->fileTransfersTab);
	if (index != -1) {
		ui->chatTabWidget->setCurrentIndex(index);
	}
}

// ABOUT MENU

void LirchQtInterface::on_actionWizard_triggered()
{
	LirchSetupWizard setup_wizard;
	if (setup_wizard.exec()) {
		// Configure the nick
		QString nick = setup_wizard.get_nick();
		bool permanent = setup_wizard.nick_is_default();
		if (permanent) {
			default_nick = nick;
		}
		request_nick_change(nick, permanent);
		// Also configure the logger
		logging_message::logging_options options;
		options |= logging_message::logging_option::SET_DIRECTORY;
		options |= logging_message::logging_option::SET_MODE;
		logging_message log_data(options);
		log_data.set_directory(setup_wizard.get_logging_directory());
		log_data.set_mode(setup_wizard.get_logging_mode());
		client_pipe->send(message::create<logging_message>(log_data));
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
	about_box.setInformativeText(tr("Do you want to report a bug?"));
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
	if (first_time_run) {
		ui->actionWizard->trigger();
	}
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

void LirchQtInterface::tab_changed(int index) {
	if (index != -1) {
		auto tab_widget = ui->chatTabWidget;
		QMap<QString, LirchChannel *>::iterator itr, end = channels.end();
		// Set the userlist's model when we achieve focus
		if (tab_widget->currentWidget() == ui->fileTransfersTab) {
			ui->chatUserList->setModel(files);
		} else if ((itr = channels.find(tab_widget->tabText(index))) != end) {
			// Or be a little more clever
			auto channel = itr.value();
			channel->grab_user_list();
		} else {
			// FIXME should never happen
		}
	}
}

// For loading a particular client pipe
void LirchQtInterface::use(LirchClientPipe *pipe)
{
	if (pipe != nullptr && pipe->ready()) {
		client_pipe = pipe;
		this->show();
	} else {
		QString plugin_name = (pipe) ? pipe->name() : tr("(null)");
		this->die(tr("failure between core and %1").arg(plugin_name), false);
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
	// FIXME send a display message for logger with reason?
	this->close();
}

// Occurs when display messages are received
void LirchQtInterface::display(QString channel_name, QString nick, QString text)
{
	// Any displays that would get sent to a blank channel get sent to every open channel
	if (channel_name.isEmpty()) {
		auto end = channels.end();
		for (auto itr = channels.begin(); itr != end; ++itr) {
			display(itr.key(), nick, text);
		}
		return;
	}
	bool ignore_message = (ignored_users.find(nick) != ignored_users.end());
	// Find the tab's QTextBrowser's document (model) we desire
	auto itr = channels.find(channel_name);
	if (itr == channels.end() || !ui->actionConnect->isChecked()) {
		return;
	}
	auto &channel = itr.value();
	// These are for sanitation
	text.replace(QRegExp("&(?!amp;)"), "&amp;");
	text.replace(QRegExp("<"), "&lt;");
	text.replace(QRegExp(">"), "&gt;");
	text.replace(QRegExp("\\n"), "<br />");
	// This regex wraps links sent in display messages.  The nbsp prevents links from traversing line boundaries (I don't know why that happens, but this fixes it)
	text.replace(QRegExp("([]A-Za-z0-9._~:/?#@!$&'()*+,;%=[-]+:[]A-Za-z0-9._~:/?#@!$&'()*+,;%=[-]+)", Qt::CaseSensitive, QRegExp::RegExp2), "<a href=\"\\1\">\\1</a>&nbsp;");
	// Show the message in the view
	channel->add_message(text, show_message_timestamps, ignore_message);
}

// Occurs when userlist messages are received
void LirchQtInterface::userlist(QMap<QString, QSet<QString>> data)
{
	// For every channel
	for (auto datum = data.begin(); datum != data.end(); ++datum) {
		// Find the model if it exists
		auto itr = channels.find(datum.key());
		if (itr != channels.end()) {
			QSet<QString> users;
			// Tag users
			for (auto &name : datum.value()) {
				if (ignored_users.contains(name)) {
					users.insert(name + tr(" (ignored)"));
				} else {
					users.insert(name);
				}
			}
			// Update the channel
			auto &channel = *itr;
			channel->update_users(users);
		}
    	}
}

// Occurs when changed_nick messages are received
void LirchQtInterface::nick(QString new_nick, bool permanent)
{
	// Only change the nick when this change is meant to be permanent
	if (permanent)
	{
		default_nick = new_nick;
	}
}

void LirchQtInterface::focus(QString channel) {
	auto itr = channels.find(channel);
	if (itr != channels.end()) {
		// Give the desired tab focus
		auto channel = itr.value();
		channel->grab_tab_focus();
	} else {
		// Make a new channel, otherwise
		request_new_channel(channel, ui->actionConnect->isChecked());
	}
}

void LirchQtInterface::leave(QString channel) {
	auto itr = channels.find(channel);
	if (itr != channels.end()) {
		LirchChannel *channel = itr.value();
		channels.erase(itr);
		// Causes cleanup after removal from the tab widget
		delete channel;
	}
}

// MESSAGE EMITTERS

void LirchQtInterface::request_new_channel(QString name, bool connected)
{
	if (connected) {
		// FIXME field is not validated
		LirchChannel *channel = new LirchChannel(name, ui);
		channels.insert(name, channel);
		client_pipe->send(message::create<set_channel_message>(name));
	}
}

void LirchQtInterface::request_edict_send(QString text, bool connected)
{
	if (connected) {
		auto tab_widget = ui->chatTabWidget;
		QString channel_name = tab_widget->tabText(tab_widget->currentIndex());
		// The core will pass this raw edict to the meatgrinder
		client_pipe->send(message::create<raw_edict_message>(text, channel_name));
	}
}

void LirchQtInterface::request_nick_change(QString new_nick, bool make_default)
{
	// The core will pass this request to the userlist
	client_pipe->send(message::create<nick_message>(new_nick, make_default));
}

// FIXME do this more elegantly

void LirchQtInterface::request_block_ignore(QString name, bool block)
{
	block_message_subtype request_type = block_message_subtype::ADD;
	if (block) {
		// FIXME field is not validated
		client_pipe->send(message::create<block_message>(request_type, QHostAddress(name)));
	} else {
		ignored_users.insert(name);
	}
}

void LirchQtInterface::request_unblock_unignore(QString name, bool block)
{
	block_message_subtype request_type = block_message_subtype::REMOVE;
	if (block) {
		// FIXME field is not validated
		client_pipe->send(message::create<block_message>(request_type, QHostAddress(name)));
	} else {
		ignored_users.remove(name);
	}
}

