#include <cstdio>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lirch_plugin.h"
#include "grinder_messages.h"
#include "edict_messages.h"

using namespace std;

class generate_quip_message : public message_data
{
public:
	virtual std::unique_ptr<message_data> copy() const {return std::unique_ptr<message_data>(new generate_quip_message(*this));}
	static message create(QString _channel) {return message_create("generate_quip", new generate_quip_message(_channel));}

	generate_quip_message(QString _channel) : channel(_channel) {}

	QString channel;
};

message generate_generate_quip_message(QString, QString channel)
{
	return generate_quip_message::create(channel);
}

class file_descriptor_handler
{
public:
	file_descriptor_handler(int _file_descriptor) : file_descriptor(_file_descriptor) {}
	~file_descriptor_handler() {close(file_descriptor);}
private:
	int file_descriptor;
};

void execute_fortune_process(int pipe_write_file_descriptor)
{
	if (dup2(pipe_write_file_descriptor, STDOUT_FILENO)==-1)
		_exit(1);
	execlp("fortune", "fortune", "-s", "-n", "240", NULL);
	_exit(1);
}

bool generate_quip(plugin_pipe &pipe, generate_quip_message *possible_generate_quip_message)
{
	if (possible_generate_quip_message==nullptr)
		return true;
	int pipe_file_descriptors[2];
	if (::pipe(pipe_file_descriptors)==-1)
		return true;
	pid_t fork_return_value=fork();
	if (fork_return_value==0)
	{
		close(pipe_file_descriptors[0]);
		execute_fortune_process(pipe_file_descriptors[1]);
	}
	file_descriptor_handler read_pipe_file_descriptor(pipe_file_descriptors[0]);
	close(pipe_file_descriptors[1]);
	if (fork_return_value==-1)
		return true;
	waitpid(fork_return_value, NULL, 0);
	char fortune_output_buffer[252]="quips\n";
	int fortune_output_buffer_offset=strlen(fortune_output_buffer);
	int read_return_value=12;
	while (fortune_output_buffer_offset!=sizeof(fortune_output_buffer) && read_return_value!=0 && (read_return_value!=-1 || errno==EINTR))
	{
		read_return_value=read(pipe_file_descriptors[0], fortune_output_buffer+fortune_output_buffer_offset, sizeof(fortune_output_buffer)-fortune_output_buffer_offset);
		fortune_output_buffer_offset+=read_return_value*(read_return_value>0);
	}
	while (fortune_output_buffer_offset!=0 && fortune_output_buffer[--fortune_output_buffer_offset]=='\n');
	{
		++fortune_output_buffer_offset;
	}
	QString quip_recieved_from_fortune=QString::fromLocal8Bit(fortune_output_buffer, fortune_output_buffer_offset);
	if (fortune_output_buffer_offset!=0)
		pipe.write(edict_message::create(edict_message_subtype::ME, possible_generate_quip_message->channel, quip_recieved_from_fortune));
	return true;
}

bool possibly_deal_with_registration_status_message(plugin_pipe &pipe, registration_status *possible_registration_message, const string &plugin_name)
{
	if (possible_registration_message==nullptr)
		return true;
	if (possible_registration_message->status)
		return true;
	if (possible_registration_message->priority>28000)
		pipe.write(registration_message::create(possible_registration_message->priority-1, plugin_name, possible_registration_message->type));
	return true;
}

bool deal_with_message(plugin_pipe &pipe, message incoming_message, const string &plugin_name)
{
	if (incoming_message.type=="shutdown")
		return false;
	if (incoming_message.type=="registration_status")
		return possibly_deal_with_registration_status_message(pipe, dynamic_cast<registration_status *>(incoming_message.getdata()), plugin_name);
	if (incoming_message.type=="generate_quip")
		return generate_quip(pipe, dynamic_cast<generate_quip_message *>(incoming_message.getdata()));
	if (incoming_message.type=="handler_ready")
		pipe.write(register_handler::create("/quip", generate_generate_quip_message));
	incoming_message.decrement_priority();
	pipe.write(incoming_message);
	return true;
}

void run(plugin_pipe pipe, string plugin_name)
{
	pipe.write(registration_message::create(30000, plugin_name, "handler_ready"));
	pipe.write(registration_message::create(30000, plugin_name, "generate_quip"));
	pipe.write(register_handler::create("/quip", generate_generate_quip_message));
	while (deal_with_message(pipe, pipe.blocking_read(), plugin_name));
	{
	}
}
