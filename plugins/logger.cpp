#include "lirch_plugin.h"
#include "core/message.h"
#include <fstream>
#include "unistd.h"
#include "edict_messages.h"

void run(plugin_pipe pipe, std::string name)
{
	bool shutdown = false;
	pipe.write(registration_message::create(0, name, "display"));
	while(!shutdown)
	{
		if(pipe.has_message())
		{
			message front = pipe.read();
			if(front.type == "shutdown")
			{
				shutdown = true;
			}
			else if(front.type == "registration_status")
			{
				registration_status * internals=(registration_status *)(front.getdata());
				if (internals)
				{
					//This is a registration_status message, just like it said
					if (!internals->status)
						pipe.write(registration_message::create(internals->priority-1, name, internals->type));
				}	
			}
			else if (front.type == "display")
			{
				display_message * internals = (display_message *)(front.getdata());
				if(internals)
				{
					ofstream channel_log;
					string filename = internals->channel.ToStdString();
					filename += ".txt";
					channel_log.open(filename, fstream::app);
					channel_log.write(internals->content.ToStdString());
					channel_log.close();
				}
			}
		}
		else
			sleep(50);
	}
	done_message::create(name);
}
