/*
 * At some point, the logger needs to have a better default location to put the files
 * as well as being able to read a custom path from the .ini
 */

#include <fstream>
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include <QSettings> 

#include "core/message.h"
#include "edict_messages.h"
#include "display_messages.h"
#include "lirch_plugin.h"
#include "lirch_constants.h"
#include "logger.h"

using namespace std;

void openLog(QString channel, map<QString, ofstream*> &open_files, QSettings &settings);

void run(plugin_pipe pipe, std::string name)
{
	bool shutdown = false;
	pipe.write(registration_message::create(0, name, "display"));
	map<QString, ofstream*> open_files;
	
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, LIRCH_COMPANY_NAME, "Lirch");
	//settings.beginGroup("Logger");
	//settings.setValue("Logger/root_directory", "/home/danieo2/Lirch/logs/");
	
	while(!shutdown)
	{
		message front = pipe.blocking_read();
		if(front.type == "shutdown")
		{
			shutdown = true;
		}
		else if(front.type == "registration_status")
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
		else if (front.type == "display")
		{
			display_message * internals = dynamic_cast<display_message *>(front.getdata());
			//if this is truly a display message, then we can use it
			if(internals)
			{
				pipe.write(front.decrement_priority());

				//convert the message contents into something logable
				QString channelname = internals->channel;
				if(!open_files.count(channelname))
					openLog(channelname,open_files, settings);
				string nick(internals->nick.toUtf8().data());
				string contents(internals->contents.toUtf8().data());
				display_message_subtype subtype=internals->subtype;

				//log the message contents in a manner which matches the message type
				string output ="";
				if(subtype==display_message_subtype::NORMAL)
				{
					output = "<"+nick+"> "+contents;
				}
				else if(subtype==display_message_subtype::ME)
				{
					output = "* "+nick+" "+contents;
				}
				else if(subtype==display_message_subtype::NOTIFY)
				{
					output = "‼‽ "+contents;
				}

				//actually writes the message to the log file
				*open_files[channelname]<<output<<endl;
			}
			else if(front.type == "set logger directory")
			{
				set_logger_directory_message * internals = dynamic_cast<set_logger_directory_message *>(front.getdata());
				if(internals)
				{
					settings.setValue("Logger/root_directory", internals->directory_root);
				}
			}
		}
		else
		{
			pipe.write(front.decrement_priority());
		}
	}
	//setting.endGroup();
	//done_message::create(name);
}

/*void sanatize(string & input)
{
	for(int i = 0; i<input.size(); i++)
	{
		if(input[i] == "/")
			input[i] = "_";
	}
}*/

//opens a file and adds it to the open_files map.  also adds 8 tilde to demarkate the beginning of a session
void openLog(QString channel, map<QString, ofstream*> &open_files, QSettings &settings)
{
	string root(settings.value("Logger/root_directory", LIRCH_DEFAULT_LOG_DIR).toString().toUtf8().data());
	root += "/";
	string filename(QUrl::toPercentEncoding(channel, "\0").data());
	//sanatize(filename);
	filename += ".txt";
	filename = root.append(filename);
	ofstream * newFile = new ofstream();
	newFile->open(filename.c_str(), fstream::app);
	*newFile <<"~~~~~~~~"<<endl;
	open_files[channel]=newFile;
}
