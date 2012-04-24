#include <fstream>
#include <map>
#include <string>
using namespace std;

#include <QDir>
#include <QFileInfo>
#include <QSettings> 
#include <QString>
#include <QUrl>

#include "lirch_constants.h"
#include "plugins/display_messages.h"
#include "plugins/lirch_plugin.h"
#include "plugins/logger_messages.h"

void run(plugin_pipe pipe, std::string name)
{
	// Register for certain message types
	pipe.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_DISPLAY));

	// Maintain a record of open files (so we can close them later)
	map<QString, ofstream*> open_files;
	// Maintain a record of channel names not to log (ephemeral)
	map<QString, bool> channel_blacklist;
	
	// Fetch the settings
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
	settings.beginGroup("Logging");
	bool logging_disabled = settings.value("disabled", false).value<bool>();
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
		else if (front.type == LIRCH_MSG_TYPE_DISPLAY && !logging_disabled)
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

				//find the corresponding filestream TODO what about QMap<QString, QFile>?
				ofstream *filestream;
				//create one if it doesn't already exist
				if (!open_files.count(channelname)) {
					// Open a new file in the desired directory
					string filepath = log_directory.append(channelname).append(".log").toStdString();
					// Append to any pre-existing file
					filestream = new ofstream(filepath.c_str(), fstream::app);
					// TODO what if open fails? or insert fails?
					open_files[channelname] = filestream;
					*filestream << "~~~~~~~~" << endl;
				}

				//actually get the representation we can log FIXME wstring? QFile?
				string nick = internals->nick.toStdString();
				string contents = internals->contents.toStdString();
				//log the message contents in a manner which matches the message type
				switch (internals->subtype) {
					case display_message_subtype::NORMAL:
					*filestream << "<" << nick << "> " << contents;
					break;
					case display_message_subtype::ME:
					*filestream << "* " << nick << " " << contents;
					break;
					case display_message_subtype::NOTIFY:
					// FIXME make this not raw UTF-8:
					//*filestream << "‼‽ " + contents;
					*filestream << "! " << contents;
					break;
					default:
					*filestream << "? " << contents;
				}
				//flush the stream
				*filestream << endl;
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
				// SET_LGBL disables logging globally
				if (internals->has_option(logging_message::logging_option::SET_LGBL)) {
					logging_disabled = internals->is_disabled();
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

	// Don't forget to close open files
	for (auto &key_pair : open_files) {
		key_pair.second->close();
		delete key_pair.second;
	}

	// FIXME is this proper usage?
	settings.setValue("disabled", logging_disabled);
	settings.setValue("root_directory", log_directory);
	settings.endGroup();

	// TODO should the logger notify when it exits?
	//done_message::create(name);
}
