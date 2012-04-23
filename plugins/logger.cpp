#include <fstream>
#include <map>
#include <string>
using namespace std;

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
	pipe.write(registration_message::create(0, name, LIRCH_MSG_TYPE_DISPLAY));

	// Maintain a record of open files (so we can close them later)
	map<QString, ofstream*> open_files;
	
	// Fetch the settings
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, LIRCH_PRODUCT_NAME);
	settings.beginGroup("Logging");
	
	while (true) {
		message front = pipe.blocking_read();
		if(front.type == LIRCH_MSG_TYPE_SHUTDOWN)
		{
			break;
		}
		else if(front.type == LIRCH_MSG_TYPE_REG_STAT)
		{
			registration_status * internals=dynamic_cast<registration_status *>(front.getdata());
			if (internals)
			{
				//This is a registration_status message, just like it said
				if (!internals->status)
					pipe.write(registration_message::create(internals->priority-1, name, internals->type));
			}
		}

		//logs the display messages
		else if (front.type == LIRCH_MSG_TYPE_DISPLAY)
		{
			display_message * internals = dynamic_cast<display_message *>(front.getdata());
			//if this is truly a display message, then we can use it
			if(internals)
			{
				//pass the message back along FIXME do this first? or last?
				pipe.write(front.decrement_priority());

				//convert the message contents into something logable
				QString channelname = QUrl::toPercentEncoding(internals->channel);

				//find the corresponding filestream
				ofstream *filestream;
				auto entry = open_files.find(channelname);
				//create one if it doesn't already exist
				if (entry == open_files.end()) {
					// TODO can some of this be cached? That way, we won't load it every single time
					// FIXME get the log directory to find the user's home
					QString directory = settings.value("root_directory", LIRCH_DEFAULT_LOG_DIR).value<QString>();
					// Open a new file in the desired directory
					string filepath = directory.append(channelname).append(".log").toStdString();
					// Append to any pre-existing file
					filestream = new ofstream(filepath.c_str(), fstream::app);
					// TODO what if open fails? or insert fails?
					auto p = open_files.insert(make_pair(channelname, filestream));
					if (p.second) {
						// Obligatory eight-tilda salute
						*filestream << "~~~~~~~~" << endl;
					}
				}

				//actually get the representation we can log FIXME wstring?
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
		else if(front.type == LIRCH_MSG_TYPE_LOGGING) {
			logging_message * internals = dynamic_cast<logging_message *>(front.getdata());
			if(internals)
			{
				settings.setValue("root_directory", internals->root_directory);
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
	settings.endGroup();

	// TODO should the logger notify when it exits?
	//done_message::create(name);
}
