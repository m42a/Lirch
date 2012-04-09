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

void openLog(QString channel, map<QString, unique_ptr<ofstream>> &open_files);

void run(plugin_pipe pipe, std::string name)
{
	bool shutdown = false;
	pipe.write(registration_message::create(0, name, "display"));
	pipe.write(registration_message::create(0, name, "me_display"));
	map<QString, unique_ptr<ofstream>> open_files;
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
					openLog(channelname,open_files);
				string nick(internals->nick.toUtf8().data());
				string contents(internals->contents.toUtf8().data());
				string output = "<"+nick+"> "+contents+"\n";
				open_files[channelname]->write(output.c_str(), output.size());
			}
		}
		else if (front.type == "me_display")
		{
			me_display_message * internals = dynamic_cast<me_display_message *>(front.getdata());
			if(internals)
			{
				pipe.write(front.decrement_priority());
				QString channelname = internals->channel;
				if(!open_files.count(channelname))
					openLog(channelname,open_files);
				string nick(internals->nick.toUtf8().data());
				string contents(internals->contents.toUtf8().data());
				string output = "* "+nick+" "+contents+"\n";
				open_files[channelname]->write(output.c_str(), output.size());
			}
		}
		else if (front.type == "notify_display")
		{
			notify_display_message * internals = dynamic_cast<notify_display_message *>(front.getdata());
			if(internals)
			{
				pipe.write(front.decrement_priority());
				QString channelname = internals->channel;
				if(!open_files.count(channelname))
					openLog(channelname,open_files);
				string contents(internals->contents.toUtf8().data());
				string output = "#"+contents+"\n";
				open_files[channelname]->write(output.c_str(), output.size());
			}
		}
		else
		{
			pipe.write(front.decrement_priority());
		}
	}
	//done_message::create(name);
}

void openLog(QString channel, map<QString, unique_ptr<ofstream>> &open_files)
{
	string filename(channel.toUtf8().data());
	filename += ".txt";
	open_files[channel] = unique_ptr<ofstream>(new ofstream());
	open_files[channel]->open(filename.c_str(), fstream::app);
	open_files[channel]->write("~~~~~~~~\n", 9);
}
