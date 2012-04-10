/*
 * At some point, the logger needs to have a better default location to put the files
 * as well as being able to read a custom path from the .ini
 */

#include <fstream>
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include "core/message.h"
#include "edict_messages.h"
#include "display_messages.h"
#include "lirch_plugin.h"

using namespace std;

void openLog(QString channel, map<QString, ofstream*> &open_files);

void run(plugin_pipe pipe, std::string name)
{
	bool shutdown = false;
	pipe.write(registration_message::create(0, name, "display"));
	pipe.write(registration_message::create(0, name, "me_display"));
	map<QString, ofstream*> open_files;
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
					openLog(channelname,open_files);
				string nick(internals->nick.toUtf8().data());
				string contents(internals->contents.toUtf8().data());
				display_message_subtype::Enum subtype=internals->subtype;

				//log the message contents in a manner which matches the message type
				string output ="";
				if(subtype==display_message_subtype::NORMAL)
				{
					output = "<"+nick+"> "+contents+"\n";
				}
				else if(subtype==display_message_subtype::ME)
				{
					output = "* "+nick+" "+contents+"\n";
				}
				else if(subtype==display_message_subtype::NOTIFY)
				{
					output = "‼‽"+contents+"\n";
				}

				//actually writes the message to the log file
				*open_files[channelname]<<output<<endl;
			}
		}
		else
		{
			pipe.write(front.decrement_priority());
		}
	}
	//done_message::create(name);
}


//opens a file and adds it to the open_files map.  also adds 8 tilde to demarkate the beginning of a session
void openLog(QString channel, map<QString, ofstream*> &open_files)
{
	string filename(channel.toUtf8().data());
	filename += ".txt";
	ofstream * newFile = new ofstream();
	newFile->open(filename.c_str(), fstream::app);
	*newFile <<"~~~~~~~~"<<endl;
	open_files[channel]=newFile;
}
