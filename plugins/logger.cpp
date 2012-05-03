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
	pipe.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, LIRCH_MSG_TYPE_LOGGING));

	// Maintain a record of open files (close on destruction)
	map<QString, unique_ptr<ofstream>> open_files;
	// Maintain a record of channel names to log/not log (ephemeral)
	map<QString, bool> channel_greylist;
	
	// Fetch the settings
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
	settings.beginGroup("Logging");
	// Set the log directory
	QString log_directory = settings.value("root_directory", LIRCH_DEFAULT_LOG_DIR).value<QString>();
	bool logging_disabled = settings.value("disabled", false).value<bool>();

	// Determine the default log directory
	QFileInfo probe(log_directory);
	if (!probe.isDir()) {
		// Setup a log folder in the settings dir
		probe.setFile(settings.fileName());
		QDir directory = probe.dir();
		// Won't fail if log already exists
		directory.mkdir(LIRCH_PRODUCT_NAME);
		directory.cd(LIRCH_PRODUCT_NAME);
		log_directory = directory.canonicalPath();
		if (log_directory.isEmpty()) {
			log_directory = LIRCH_DEFAULT_LOG_DIR;
		}
	}
	// TODO consider this (it drags in a QtGui dependency)
	// QDesktopServices::storageLocation(QDesktopServices::DataLocation)

	// TODO figure out a better way to do this?
	QChar double_exclamation_mark(0x203C);
	QChar interrobang(0x203D);
	QString buffer;
	buffer += double_exclamation_mark;
	buffer += interrobang;
	buffer += " ";
	QByteArray notify_prefix = buffer.toUtf8();

	while (true) {
		message msg = pipe.blocking_read();
		//handle shutdown condition
		if (msg.type == LIRCH_MSG_TYPE_SHUTDOWN)
		{
			// Cleanup happens below
			break;
		}

		//capture registration messages
		else if (msg.type == LIRCH_MSG_TYPE_REG_STAT)
		{
			registration_status * internals = dynamic_cast<registration_status *>(msg.getdata());
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
		else if (msg.type == LIRCH_MSG_TYPE_DISPLAY)
		{
			display_message * internals = dynamic_cast<display_message *>(msg.getdata());
			//if this is truly a display message, then we can use it
			if (internals) {
				//pass the message back along
				pipe.write(msg.decrement_priority());

				bool log = !logging_disabled;

				//see if this channel is listed
				auto entry = channel_greylist.find(internals->channel);
				if (entry != channel_greylist.end()) {
					// If so, this overrides the default setting
					log = entry->second;
				}

				//check that we are supposed to be logging
				if (!log || internals->channel.isEmpty()) {
					continue;
				}

				//convert the message contents into something logable
				QString channelname = QUrl::toPercentEncoding(internals->channel);

				//find the corresponding filestream
				if (!open_files.count(channelname)) {
					// Open a new file in the desired directory
					QDir directory_path(log_directory);
					// Append to any pre-existing file
					auto filestream = new ofstream(directory_path.absoluteFilePath(channelname + ".log").toUtf8().constData(), fstream::app);
					// If open fails, just skip it
					if (filestream->is_open()) {
						open_files[channelname] = unique_ptr<ofstream>(filestream);
						// Session separator
						*filestream << "~~~~~~~~" << endl;
					} else {
						delete filestream;
						continue;
					}
				}

				auto nick = internals->nick.toUtf8();
				auto contents = internals->contents.toUtf8();
				auto &stream = *open_files[channelname];
				//log the message contents in a manner which matches the message type
				switch (internals->subtype) {
				case display_message_subtype::NORMAL:
					stream << "<" << nick.constData() << "> " << contents.constData();
					break;
				case display_message_subtype::ME:
					stream << "* " << nick.constData() << " " << contents.constData();
					break;
				case display_message_subtype::NOTIFY:
					stream << notify_prefix.constData() << contents.constData();
					break;
				default:
					stream << "? " << contents.constData();
				}
				stream << endl;
			}
		}

		//change settings
		else if (msg.type == LIRCH_MSG_TYPE_LOGGING) {
			logging_message * internals = dynamic_cast<logging_message *>(msg.getdata());
			if (internals) {
				// SET_NONE tells us to disregard this
				if (internals->has_option(logging_message::logging_option::SET_NONE)) {
					continue;
				}
				// SET_DIRECTORY tells us to change the log directory on restart
				if (internals->has_option(logging_message::logging_option::SET_DIRECTORY)) {
					// Just setting log_directory won't update open_files
					settings.setValue("root_directory", internals->get_directory());
				}
				// SET_MODE sets the logger's mode (on/off/default)
				if (internals->has_option(logging_message::logging_option::SET_MODE)) {
					// Changing mode clears the channel list
					channel_greylist.clear();
					switch (internals->get_mode()) {
					case logging_message::logging_mode::ON:
						logging_disabled = false;
						break;
					case logging_message::logging_mode::OFF:
						logging_disabled = true;
						break;
					default:
						logging_disabled = true;
						channel_greylist[LIRCH_DEFAULT_CHANNEL] = true;
					}
				}
				// SET_FORMAT sets the logger's format
				if (internals->has_option(logging_message::logging_option::SET_FORMAT)) {
					// FIXME save this once we have more formats
					//log_format = internals->get_format();
				}
				// SET_CHANNEL enables/disables logging for specific channels
				if (internals->has_option(logging_message::logging_option::SET_CHANNELS)) {
					channel_greylist.insert(internals->begin(), internals->end());
				}
			}
		}

		else
		{
			pipe.write(msg.decrement_priority());
		}
	}

	settings.setValue("disabled", logging_disabled);
	settings.endGroup();
}
