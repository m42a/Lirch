#include <fstream>
#include <map>
#include <string>
#include <memory>

#include <QDir>
#include <QFileInfo>
#include <QSettings> 
#include <QString>
#include <QUrl>

#include "lirch_constants.h"
#include "plugins/display_messages.h"
#include "plugins/lirch_plugin.h"
#include "plugins/logger_messages.h"

using namespace std;

void run(plugin_pipe pipe, std::string name)
{
	// Register for certain message types
	pipe.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_DISPLAY));

	// Maintain a record of open files (close on destruction)
	map<QString, unique_ptr<ofstream>> open_files;
	// Maintain a record of channel names not to log (ephemeral)
	map<QString, bool> channel_blacklist;
	
	// Fetch the settings
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
	settings.beginGroup("Logging");

	// Determine the default log directory
	QDir default_log_directory;
	// FIXME current method: use the build-configured directory if possible; otherwise, make a subdir in settings
	QFileInfo probe(LIRCH_DEFAULT_LOG_DIR);
	if (probe.isDir() && probe.isWritable()) {
		default_log_directory = QDir(probe.absoluteFilePath());
	} else {
		// Setup a log folder in the settings dir
		probe.setFile(settings.fileName());
		default_log_directory = probe.dir();
		// Won't fail if log already exists
		default_log_directory.mkdir(LIRCH_PRODUCT_NAME);
		default_log_directory.cd(LIRCH_PRODUCT_NAME);
	}
	// TODO consider this (it drags in a QtGui dependency)
	// QDesktopServices::storageLocation(QDesktopServices::DataLocation)

	// Set the log directory
	QString log_directory = settings.value("root_directory", default_log_directory.canonicalPath()).value<QString>();
	
	while (true) {
		message front = pipe.blocking_read();
		//handle shutdown condition
		if(front.type == LIRCH_MSG_TYPE_SHUTDOWN)
		{
			// Cleanup happens below
			break;
		}

		//capture registration messages
		else if(front.type == LIRCH_MSG_TYPE_REG_STAT)
		{
			registration_status * internals = dynamic_cast<registration_status *>(front.getdata());
			// Make sure this is an affirmative response
			if (internals && !internals->status)
			{
				if (internals->priority < LIRCH_MSG_PRI_REG_MIN) {
					break;
				}
				// Retry a few thousand times, with lower priority
				pipe.write(registration_message::create(internals->priority-1, name, internals->type));
			}
		}

		//logs the display messages, provided logging is enabled
		else if (front.type == LIRCH_MSG_TYPE_DISPLAY)
		{
			display_message * internals = dynamic_cast<display_message *>(front.getdata());
			//if this is truly a display message, then we can use it
			if(internals)
			{
				//pass the message back along FIXME do this first? or last?
				pipe.write(front.decrement_priority());

				//so long as it is not on the blocklist, we will log it
				auto entry = channel_blacklist.find(internals->channel);
				if (entry != channel_blacklist.end() && entry->second) {
					// Skip this message, its channel was blacklisted
					continue;
				}

				//convert the message contents into something logable
				QString channelname = QUrl::toPercentEncoding(internals->channel);

				//find the corresponding filestream
				if (!open_files.count(channelname)) {
					// Open a new file in the desired directory
					string filepath = log_directory.append(channelname).append(".log").toStdString();
					// Append to any pre-existing file
					auto filestream = new ofstream(filepath.c_str(), fstream::app);
					// TODO what if open fails? or insert fails?
					open_files[channelname] = unique_ptr<ofstream>(filestream);
					*filestream << "~~~~~~~~" << endl;
				}

				auto nick = internals->nick.toUtf8();
				auto contents = internals->contents.toUtf8();
				//log the message contents in a manner which matches the message type
				switch (internals->subtype) {
					case display_message_subtype::NORMAL:
					*open_files[channelname] << "<" << nick.constData() << "> " << contents.constData();
					break;
					case display_message_subtype::ME:
					*open_files[channelname] << "* " << nick.constData() << " " << contents.constData();
					break;
					case display_message_subtype::NOTIFY:
					*open_files[channelname] << QString("‼‽ ").toUtf8().constData() << contents.constData();
					break;
					default:
					*open_files[channelname] << "? " << contents.constData();
				}
				//flush the stream
				*open_files[channelname] << endl;
			}
		}

		//change settings
		else if (front.type == LIRCH_MSG_TYPE_LOGGING) {
			logging_message * internals = dynamic_cast<logging_message *>(front.getdata());
			if (internals) {
				// SET_NONE tells us to disregard this
				if (internals->has_option(logging_message::logging_option::SET_NONE)) {
					continue;
				}
				// SET_LDIR tells us to change the log directory on restart
				if (internals->has_option(logging_message::logging_option::SET_LDIR)) {
					// FIXME just setting log_directory won't update open_files
					settings.setValue("root_directory", internals->get_directory());
				}
				// SET_MODE sets the logger's mode (on/off)
				if (internals->has_option(logging_message::logging_option::SET_MODE)) {
					// FIXME handle this
				}
				// SET_FORM sets the logger's format
				if (internals->has_option(logging_message::logging_option::SET_FORM)) {
					// FIXME handle this
				}
				// SET_CHAN enables/disables logging for specific channels
				if (internals->has_option(logging_message::logging_option::SET_CHAN)) {
					channel_blacklist.insert(internals->begin(), internals->end());
				}
			}
		}

		//pass it on
		else
		{
			pipe.write(front.decrement_priority());
		}
	}

	// FIXME is this proper usage?
	settings.setValue("root_directory", log_directory);
	settings.endGroup();
}
