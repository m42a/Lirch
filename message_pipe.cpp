#include "message_pipe.h"

bool message_pipe::has_message() const
{
	return !messages->queue.empty();
}

message message_pipe::peek() const
{
	if (has_message())
		return messages->queue.front();
	return message();
}

message message_pipe::read()
{
	message m=peek();
	if (has_message())
		messages->queue.pop();
	return m;
}

void message_pipe::write(const message &m)
{
	messages->queue.push(m);
}
