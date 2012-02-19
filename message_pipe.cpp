#include "message_pipe.h"

//mutex_lock_guard locks a mutex upon construction, and unlocks
//it upon destruction
typedef std::lock_guard<std::recursive_mutex> mutex_lock_guard;

//To be thread-safe, we need to lock every function that accesses containers,
//since the STL is not guaranteed to be reentrant.
bool message_pipe::plugin_has_message() const
{
	mutex_lock_guard l(*plugin_mutex);
	return !to_plugin->empty();
}

message message_pipe::plugin_peek() const
{
	mutex_lock_guard l(*plugin_mutex);
	//Do sanity checks so we don't exhibit undefined behavior
	if (plugin_has_message())
		return to_plugin->front();
	else
		return message();
}

message message_pipe::plugin_read()
{
	mutex_lock_guard l(*plugin_mutex);
	message m;
	if (plugin_has_message())
	{
		m=to_plugin->front();
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
	mutex_lock_guard l(*core_mutex);
	to_core->push(m);
}

bool message_pipe::core_has_message() const
{
	mutex_lock_guard l(*core_mutex);
	return !to_core->empty();
}

message message_pipe::core_peek() const
{
	mutex_lock_guard l(*core_mutex);
	//Do sanity checks so we don't exhibit undefined behavior
	if (core_has_message())
		return to_core->front();
	else
		return message();
}

message message_pipe::core_read()
{
	mutex_lock_guard l(*core_mutex);
	message m;
	if (core_has_message())
	{
		m=to_core->front();
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
	mutex_lock_guard l(*plugin_mutex);
	to_plugin->push(m);
}
