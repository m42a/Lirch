#include <fstream>
#include <string>
#include <map>
#include <memory>
#include "core/message.h"
#include "edict_messages.h"
#include "lirch_plugin.h"

using namespace std;

void run(plugin_pipe pipe, std::string name)
{
	bool shutdown = false;
	pipe.write(registration_message::create(0, name, "display"));
	map<QString, unique_ptr<ofstream> > open_files;
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
		else if (front.type == "display")
		{
			display_message * internals = dynamic_cast<display_message *>(front.getdata());
			if(internals)
			{
				pipe.write(front.decrement_priority());
				QString channelname = internals->channel;
				if(!open_files.count(channelname))
				{
					string filename(channelname.toUtf8().data());
					filename += ".txt";
					open_files[channelname] = unique_ptr<ofstream>(new ofstream());
					open_files[channelname]->open(filename.c_str(), fstream::app);
				}
				string output(internals->contents.toUtf8().data());
				open_files[channelname]->write(output.c_str(), sizeof output);
			}
		}
		else
			pipe.write(front.decrement_priority());
	}
	//done_message::create(name);
}
