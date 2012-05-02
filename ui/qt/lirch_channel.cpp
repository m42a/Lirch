#include "ui/qt/lirch_channel.h"

// Makes a LirchChannel with the name channel_name managed by the UI *ui
LirchChannel::LirchChannel(const QString &channel_name, Ui::LirchQtInterface *ui) :
	name(channel_name),
	tab(new QWidget),
	tabs(ui->chatTabWidget),
	list(ui->chatUserList)
{
	// TODO Check to see if tab is duplicated
	int index = tabs->currentIndex();
	tabs->insertTab(index, tab, name);
	users = new QStandardItemModel(0, 1, tab);
	// TODO add QMenuItem with show action
	action = ui->menuViewTab->addAction(name, this, SLOT(grab_focus()));
	// Create a view and set its model
	QTextBrowser *browser = new QTextBrowser(tab);
	QPalette palette;
        QColor light_green(0xCC, 0xFF, 0xCC);
	palette.setColor(QPalette::Base, light_green);
	browser->setPalette(palette);
	document = new QTextDocument(browser);
	cursor = new QTextCursor(document);
	browser->setDocument(document);
	// Set up the layout with this view
	QBoxLayout *layout = new QBoxLayout(QBoxLayout::Direction::TopToBottom);
	layout->addWidget(browser);
	tab->setLayout(layout);
	// Select the new window
	action->trigger();
}

void LirchChannel::show_message(const DisplayMessage &message, bool show_timestamp) {
	if (show_timestamp) {
		cursor->insertText("[" + message.timestamp + "] ");
	}
	cursor->insertHtml(message.text);
	cursor->insertBlock(block_format);
}

void LirchChannel::update_users(const QSet<QString> &new_users) {
	users->clear();
	for (auto &new_user : new_users) {
		users->appendRow(new QStandardItem(new_user));
	}
}

// TODO make message reception do something to unfocused tabs, reset on focus
void LirchChannel::add_message(const QString& text, bool show_timestamp, bool ignore_message) {
	// Package the data so that we have it for reloads
	QString timestamp = QTime::currentTime().toString();
	DisplayMessage message(text, timestamp, ignore_message);
	messages.append(message);
	if (!ignore_message) {
		show_message(message, show_timestamp);
	}
}

void LirchChannel::grab_focus() const {
	int index = tabs->indexOf(tab);
	if (index != -1) {
		tabs->setCurrentIndex(index);
		list->setModel(users);
	} else {
		// TODO debug message
	}
}

// TODO make ignore list per-channel (need to edit add_message)
void LirchChannel::reload_messages(bool show_timestamps, bool show_ignored) {
	document->clear();
	for (auto &message : messages) {
		if (!message.ignored || show_ignored) {
			show_message(message, show_timestamps);
		}
	}
}

