#ifndef MESSAGE_PIPE_H_
#define MESSAGE_PIPE_H_

#include <memory>
#include <queue>

#include "message.h"

class message_pipe
{
public:
	message_pipe() : messages(std::make_shared<pipe>()) {}
	message_pipe(const message_pipe &) = default;
	message_pipe(message_pipe &&) = default;
	message_pipe &operator=(const message_pipe &) = default;
	message_pipe &operator=(message_pipe &&) = default;

	bool has_message() const;
	message peek() const;
	message read();
	void write(const message &);
private:
	//Group all the variables together so we can't accidentally swap them
	//around independently.
	struct pipe
	{
		std::queue<message> queue;
	};
	std::shared_ptr<pipe> messages;
};

//A bidirectional pipe for passing messages
class bidirectional_message_pipe
{
public:
	bidirectional_message_pipe(const message_pipe &p, const message_pipe &c) : to_plugin(p), to_core(c) {}
	//Implicit copy constructor generation is deprecated in certain cases I
	//don't understand fully, so be cautious and explicitly declare them.
	bidirectional_message_pipe() = default;
	bidirectional_message_pipe(const bidirectional_message_pipe &) = default;
	bidirectional_message_pipe(bidirectional_message_pipe &&) = default;
	bidirectional_message_pipe &operator=(const bidirectional_message_pipe &) = default;
	bidirectional_message_pipe &operator=(bidirectional_message_pipe &&) = default;

	bool plugin_has_message() const {return to_plugin.has_message();}
	message plugin_peek() const {return to_plugin.peek();}
	message plugin_read() {return to_plugin.read();}
	void plugin_write(const message &m) {to_core.write(m);}

	bool core_has_message() const {return to_core.has_message();}
	message core_peek() const {return to_core.peek();}
	message core_read() {return to_core.read();}
	void core_write(const message &m) {to_plugin.write(m);}
private:
	message_pipe to_plugin;
	message_pipe to_core;
};

#endif
