#ifndef MESSAGE_VIEW_H_
#define MESSAGE_VIEW_H_

#include "message_pipe.h"

class plugin_pipe
{
public:
	plugin_pipe(const bidirectional_message_pipe &p) : pipe(p) {}

	plugin_pipe() = default;
	plugin_pipe(const plugin_pipe &) = default;
	plugin_pipe(plugin_pipe &&) = default;
	plugin_pipe &operator=(const plugin_pipe &) = default;
	plugin_pipe &operator=(plugin_pipe &&) = default;

	bool has_message() const;
	message peek() const;
	message read();
	message blocking_read();
	void write(const message &m);
private:
	bidirectional_message_pipe pipe;
};

class core_pipe
{
public:
	core_pipe(const bidirectional_message_pipe &p) : pipe(p) {}

	core_pipe() = default;
	core_pipe(const core_pipe &) = default;
	core_pipe(core_pipe &&) = default;
	core_pipe &operator=(const core_pipe &) = default;
	core_pipe &operator=(core_pipe &&) = default;

	bool has_message() const;
	message peek() const;
	message read();
	message blocking_read();
	void write(const message &m);
private:
	bidirectional_message_pipe pipe;
};

#endif
