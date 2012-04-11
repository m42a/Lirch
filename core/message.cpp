#include "message_view.h"

bool message_pipe::locked_has_message() const
{
	return !messages->queue.empty();
}

message message_pipe::locked_peek() const
{
	if (locked_has_message())
		return messages->queue.front();
	return message();
}

message message_pipe::locked_read()
{
	message m=locked_peek();
	if (locked_has_message())
		messages->queue.pop();
	return m;
}

void message_pipe::locked_write(const message &m)
{
	messages->queue.push(m);
}

//mutex_lock_guard locks a mutex upon construction, and unlocks
//it upon destruction
typedef std::unique_lock<std::mutex> mutex_lock_guard;

//To be thread-safe, we need to lock every function that accesses containers,
//since the STL is not guaranteed to be reentrant.
bool message_pipe::has_message() const
{
	mutex_lock_guard l(messages->mutex);
	return locked_has_message();
}

message message_pipe::peek() const
{
	mutex_lock_guard l(messages->mutex);
	return locked_peek();
}

message message_pipe::read()
{
	mutex_lock_guard l(messages->mutex);
	return locked_read();
}

message message_pipe::blocking_read()
{
	mutex_lock_guard l(messages->mutex);
	messages->cond.wait(l, [this](){return this->locked_has_message();});
	return locked_read();
}

void message_pipe::write(const message &m)
{
	{
		mutex_lock_guard l(messages->mutex);
		locked_write(m);
	}
	//Signal that there's a message after we've unlocked the mutex to avoid
	//useless contention.
	messages->cond.notify_one();
}

bool bidirectional_message_pipe::plugin_has_message() const {return to_plugin.has_message();}
message bidirectional_message_pipe::plugin_peek() const {return to_plugin.peek();}
message bidirectional_message_pipe::plugin_read() {return to_plugin.read();}
message bidirectional_message_pipe::plugin_blocking_read() {return to_plugin.blocking_read();}
void bidirectional_message_pipe::plugin_write(const message &m) {to_core.write(m);}

bool bidirectional_message_pipe::core_has_message() const {return to_core.has_message();}
message bidirectional_message_pipe::core_peek() const {return to_core.peek();}
message bidirectional_message_pipe::core_read() {return to_core.read();}
message bidirectional_message_pipe::core_blocking_read() {return to_core.blocking_read();}
void bidirectional_message_pipe::core_write(const message &m) {to_plugin.write(m);}

bool plugin_pipe::has_message() const {return pipe.plugin_has_message();}
message plugin_pipe::peek() const {return pipe.plugin_peek();}
message plugin_pipe::read() {return pipe.plugin_read();}
message plugin_pipe::blocking_read() {return pipe.plugin_blocking_read();}
void plugin_pipe::write(const message &m) {pipe.plugin_write(m);}

bool core_pipe::has_message() const {return pipe.core_has_message();}
message core_pipe::peek() const {return pipe.core_peek();}
message core_pipe::read() {return pipe.core_read();}
message core_pipe::blocking_read() {return pipe.core_blocking_read();}
void core_pipe::write(const message &m) {pipe.core_write(m);}
