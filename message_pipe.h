#ifndef MESSAGE_PIPE_H_
#define MESSAGE_PIPE_H_

#include <memory>
#include <queue>
#include <mutex>

#include "message.h"

//A bidirectional pipe for passing messages
class message_pipe
{
public:
	message_pipe() : to_plugin(new std::queue<message>()), plugin_mutex(new std::mutex()), to_core(new std::queue<message>()), core_mutex(new std::mutex()) {}
	//Implicit copy constructor generation is deprecated in certain cases I
	//don't understand fully, so be cautious and explicitly declare them.
	message_pipe(const message_pipe &) = default;
	message_pipe(message_pipe &&) = default;
	message_pipe &operator=(const message_pipe &) = default;
	message_pipe &operator=(message_pipe &&) = default;

	bool plugin_has_message() const;
	message plugin_peek() const;
	message plugin_read();
	void plugin_write(const message &);

	bool core_has_message() const;
	message core_peek() const;
	message core_read();
	void core_write(const message &);
private:
	std::shared_ptr<std::queue<message>> to_plugin;
	std::shared_ptr<std::mutex> plugin_mutex;
	std::shared_ptr<std::queue<message>> to_core;
	std::shared_ptr<std::mutex> core_mutex;
};

#endif
