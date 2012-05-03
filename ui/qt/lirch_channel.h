#ifndef LIRCH_CHANNEL_H
#define LIRCH_CHANNEL_H

#include <QAction>
#include <QColor>
#include <QFile>
#include <QListView>
#include <QObject>
#include <QPalette>
#include <QSet>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFormat>
#include <QTextStream>
#include <QTime>
#include <QWidget>

#include "ui/qt/ui_lirch_qt_interface.h"

static QTextBlockFormat block_format;

class LirchChannel : public QObject {
	Q_OBJECT
	// Representation:
	QString name;
	QWidget *tab;
	QAction *action;
	struct DisplayMessage {
		QString timestamp, text;
		bool ignored;
		// For convenience
		DisplayMessage(
			const QString &what = QString(),
			const QString &when = QTime::currentTime().toString(),
			bool hidden = false) :
			timestamp(when), text(what), ignored(hidden) { }
	};
	QList<DisplayMessage> messages;

	// These reference the UI
	QTabWidget *tabs;
	QListView *list;
	QMenu *menu;

	// These reference data models
	QTextBrowser *browser;
	QTextDocument *document;
	QStandardItemModel *users;

	// These are misc properties
	QTextCursor *cursor;
	QTextStream *stream;

	// Helper function to print a (stamped?) TextBlock at the cursor
	void show_message(const DisplayMessage &, bool = false);
public:
	explicit LirchChannel(const QString &, Ui::LirchQtInterface *);
	virtual ~LirchChannel();

	// Manipulate the data models
	void update_users(const QSet<QString> &);
	void add_message(const QString &, bool = false, bool = false);
	void prepare_persist(QFile *);

public slots:
	// Called by the action above
	void grab_tab_focus() const;
	void grab_user_list() const;
	// Called by the UI's reload signal
	void reload_messages(bool = false, bool = false);
	// Called by external processes
	void persist() const;

signals:
	// Used for feedback on persits
	void progress(int) const;
	void persisted() const;
};

#endif // LIRCH_CHANNEL_H
