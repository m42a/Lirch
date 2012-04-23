#include <fstream>
#include <map>
#include <string>
#include <memory>

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
	pipe.write(registration_message::create(0, name, LIRCH_MSG_TYPE_DISPLAY));

	// Maintain a record of open files (so we can close them later)
	map<QString, unique_ptr<ofstream>> open_files;

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
				if (open_files.count(channelname)!=0) {
					// TODO can some of this be cached? That way, we won't load it every single time
					// FIXME get the log directory to find the user's home
					QString directory = settings.value("root_directory", LIRCH_DEFAULT_LOG_DIR).value<QString>();
					// Open a new file in the desired directory
					string filepath = directory.append(channelname).append(".log").toStdString();
					// Append to any pre-existing file
					auto filestream = new ofstream(filepath.c_str(), fstream::app);
					// TODO what if open fails? or insert fails?
					auto p = open_files.insert(make_pair(channelname, unique_ptr<ofstream>(filestream)));
					if (p.second) {
						// Obligatory eight-tilda salute
						*filestream << "~~~~~~~~" << endl;
					}
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

	// FIXME is this proper usage?
	settings.endGroup();

	// TODO should the logger notify when it exits?
	//done_message::create(name);
}
