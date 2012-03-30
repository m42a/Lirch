//Note: this header is included by plugins, not by code that wants to interact
//with plugins.  Code that wants to interact with plugins should call
//load_plugin().
#ifndef LIRCH_PLUGIN_H_
#define LIRCH_PLUGIN_H_

#include <string>

#include "core/message_view.h"
#include "core/required_messages.h"

//Why windows, why?
#if defined(_WIN32)
#	if defined(_WIN64)
#		pragma comment(linker, "/EXPORT:plugin_version")
#		pragma comment(linker, "/EXPORT:plugin_init")
#	else
#		pragma comment(linker, "/EXPORT:plugin_version=_plugin_version")
#		pragma comment(linker, "/EXPORT:plugin_init=_plugin_init")
#	endif
#endif

void run(plugin_pipe, std::string);

extern "C"
{
	int
#ifdef _WIN32
		//We need this to ensure semi-consistent name mangling and APIs
		__cdecl
#endif
		plugin_version()
	{
		return 0;
	}

	void
#ifdef _WIN32
		__cdecl
#endif
		plugin_init(plugin_pipe p)
	{
		message m=p.blocking_read();
		if (m.type!="hello")
			return;
		auto d=dynamic_cast<hello_message *>(m.getdata());
		if (!d)
			return;
		try
		{
			//This calls the plugin-defined code
			run(p, d->name);
		}
		catch (...)
		{
			//Catch every exception, since uncaught exceptions will terminate the program
			//We might want to put some logging here
		}
		p.write(done_message::create(d->name));
	}
}

#endif
