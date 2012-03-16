//Note: this header is included by plugins, not by code that wants to interact
//with plugins.  Code that wants to interact with plugins should call
//load_plugin().
#ifndef LIRCH_PLUGIN_H_
#define LIRCH_PLUGIN_H_

#include "message_pipe.h"

//Why windows, why?
#if defined(_WIN32)
#	if defined(_WIN64)
#		pragma comment(linker, "/EXPORT:plugin_version")
#		pragma comment(linker, "/EXPORT:plugin_init")
#	else
#		pragma comment(linker, "/EXPORT:plugin_version=_plugin_version")
#		pragma comment(linker, "/EXPORT:plugin_init=_plugin_init")
#	endif
#else

void init(message_pipe);

extern "C"
{
	int
#ifdef _WIN32
		//We need this to ensure semi-consistent name mangling
		__cdecl
#endif
		plugin_version()
	{
		return 0;
	}

	void
#ifdef _WIN32
		//We need this to ensure semi-consistent name mangling
		__cdecl
#endif
		plugin_init(bidirectional_message_pipe p)
	{
		//Generic initialization code will go here

		//This calls the plugin-defined initialization code
		init(p);
	}
}

#endif
