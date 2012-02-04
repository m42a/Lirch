#include "message_pipe.h"

bool message_pipe::plugin_has_message() const
{
	return !to_plugin->empty();
}

message message_pipe::plugin_peek() const
{
	//Do sanity checks so we don't exhibit undefined behavior
	if (plugin_has_message())
		return to_plugin->front();
	else
		return message();
}

message message_pipe::plugin_read()
{
	message m;
	if (plugin_has_message())
	{
		m=plugin_peek();
		to_plugin->pop();
	}
	else
	{
		m=message();
	}
	return m;
}

void message_pipe::plugin_write(const message &m)
{
	to_core->push(m);
}

bool message_pipe::core_has_message() const
{
	return !to_core->empty();
}

message message_pipe::core_peek() const
{
	//Do sanity checks so we don't exhibit undefined behavior
	if (core_has_message())
		return to_core->front();
	else
		return message();
}

message message_pipe::core_read()
{
	message m;
	if (core_has_message())
	{
		m=core_peek();
		to_core->pop();
	}
	else
	{
		m=message();
	}
	return m;
}

void message_pipe::core_write(const message &m)
{
	to_plugin->push(m);
}
