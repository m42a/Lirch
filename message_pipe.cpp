#include "message_pipe.h"

//mutex_lock_guard locks a mutex upon construction, and unlocks
//it upon destruction
typedef std::lock_guard<std::recursive_mutex> mutex_lock_guard;

//To be thread-safe, we need to lock every function that accesses containers,
//since the STL is not guaranteed to be reentrant.
bool message_pipe::has_message() const
{
	mutex_lock_guard l(messages->mutex);
	return !messages->queue.empty();
}

message message_pipe::peek() const
{
	mutex_lock_guard l(messages->mutex);
	//Do sanity checks so we don't exhibit undefined behavior
	if (has_message())
		return messages->queue.front();
	return message();
}

message message_pipe::read()
{
	mutex_lock_guard l(messages->mutex);
	message m=peek();
	if (has_message())
		messages->queue.pop();
	return m;
}

void message_pipe::write(const message &m)
{
	mutex_lock_guard l(messages->mutex);
	messages->queue.push(m);
}
/*
bool message_pipe::core_has_message() const
{
	mutex_lock_guard l(messages->mutex);
	return !to_core->empty();
}

message message_pipe::core_peek() const
{
	mutex_lock_guard l(messages->mutex);
	//Do sanity checks so we don't exhibit undefined behavior
	if (core_has_message())
		return to_core->front();
	else
		return message();
}

message message_pipe::core_read()
{
	mutex_lock_guard l(messages->mutex);
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
	mutex_lock_guard l(messages->mutex);
	to_plugin->push(m);
=======
>>>>>>> message-passing-tentative
}*/
