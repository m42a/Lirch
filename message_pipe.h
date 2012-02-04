#ifndef MESSAGE_PIPE_H_
#define MESSAGE_PIPE_H_

#include <memory>
#include <queue>

#include "message.h"

//A bidirectional pipe for passing messages
class message_pipe
{
public:
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
	std::shared_ptr<std::queue<message>> to_core;
};

#endif
