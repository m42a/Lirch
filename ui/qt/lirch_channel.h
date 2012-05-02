#ifndef LIRCH_CHANNEL_H
#define LIRCH_CHANNEL_H

#include <QAction>
#include <QListView>
#include <QObject>
#include <QSet>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFormat>
#include <QTextFrame>
#include <QTextFrameFormat>
#include <QTime>
#include <QWidget>

#include "ui/qt/ui_lirch_qt_interface.h"

static QTextFrameFormat frame_format;

class LirchChannel : public QObject {
	Q_OBJECT
	// Representation:
	QString name;
	QWidget tab;
	struct DisplayMessage;
	QList<DisplayMessage> messages;

	// These reference the UI
	QTabWidget *tabs;
	QListView *list;
	// These reference data models
	QTextDocument *document;
	QStandardItemModel *users;

	// These are misc properties
	QAction *action;
	QTextCursor *cursor;
	// Helper struct and function
	struct DisplayMessage {
		QString timestamp, text;
		bool ignored;
		DisplayMessage(const QString &what = QString(), const QString &when = QTime::currentTime().toString(), bool hidden = false) :
			timestamp(when), text(what), ignored(hidden) { }
	};
	void show_message(const DisplayMessage &, bool);
public:
	explicit LirchChannel(const QString &, Ui::LirchQtInterface *);
	// Manipulate the data models
	void update_users(const QSet<QString> &);
	void add_message(const QString &, bool = false, bool = false);
public slots:
	// Called by the action above
	void grab_focus() const;
	// Called by the UI's reload signal
	void reload_messages(bool = false, bool = false);
};

#endif // LIRCH_CHANNEL_H
