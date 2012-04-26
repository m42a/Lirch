#include "plugins/lirch_plugin.h"
#include "plugins/grinder_messages.h"
#include "plugins/edict_messages.h"


Quip::Quip(quip_request *request_data) {
	
}

Quip::execute_fortune_process() {
	QProcess fortune_process;
	// TODO connect shit together
	
	// TODO openmode QIODevice::WriteOnly?
	fortune_process.start("fortune -sn 240", QIODevice::ReadWrite);
}


message Quip::to_message() {
	edict_message_subtype::ME;
	message quip_message = edict_message::create()
	pipe.write(quip_message);
	return edict_message::create(edict_message_subtype::ME, );
}



void QuipPlugin::forward_quip_request(QString command, QString channel) {
	// TODO what else to do with command?
	assert(command == tr("/quip"));
	quip_request_data request_data(channel);
	pipe.write(request_data.to_message());
}


void QuipPlugin::handle_registration_reply(registration_status *status_data) {
	if (!status_data) {
		return;
	}
	int priority = status_data->priority;
	if (!status_data->status && priority > 0) {
		std::string registration_type = status_data->type;
		pipe.write(registration_message(--priority, name, registration_type));
	}
}

void QuipPlugin::handle_quip_request(quip_request *request_data) {
	if (!request) {
		return;
	}
	pipe.write(Quip(request_data).to_message());
}


void QuipPlugin::register_for_message_type(const QString &message_type, int with_priority) {
	pipe.write(registration_method::create(with_priority, name, message_type));
}

message_type enumerate(const QString &message_type) {
	if (message_type == tr(LIRCH_MSG_TYPE_SHUTDOWN)) {
		return message_type::SHUTDOWN;
	}
	if (message_type == tr(LIRCH_MSG_TYPE_REGISTRATION_STATUS)) {
		return message_type::REGISTRATION_STATUS;
	}
	if (message_type == tr(LIRCH_MSG_TYPE_QUIP_REQUEST)) {
		return message_type::QUIP_REQUEST;
	}
	if (message_type == tr(LIRCH_MSG_TYPE_HANDLER_READY)) {
		return message_type::HANDLER_READY;
	}
	return message_type::UNKNOWN;
}

// FIXME add doc and are we passing the right value?
bool QuipPlugin::handle_message(message incoming_message)
{
	bool propagate_message;
	QString incoming_message_type(incoming_message.type);
	// The type of a message determines what to do with it
	switch(enumerate(incoming_message_type)) {
	// Shutdown messages immediately terminate the plugin
	case SHUTDOWN:
		return false;
	// Registration statuses might need to be echoed
	case REGISTRATION_STATUS:
		handle_registration_reply(dynamic_cast<registration_status *>(m.getdata()));
		// FIXME should this be true?
		propagate_message = false;
		break;
	// Quip requests are captured and not forwarded
	case QUIP_REQUEST:
		handle_quip_request(dynamic_cast<quip_request_data *>(m.getdata()));
		propagate_message = false;
		break;
	// When the handler is ready, we want to register something
	case HANDLER_READY:
		// Register the way to handle quip commands
		pipe.write(register_handler::create("/quip", forward_quip_request));
		propagate_message = true;
		break;
	// Forward messages along by default
	default:
		propagate_message = true;
		break;
	}
	// This message might need to be forwarding
	if (propagate_message) {
		incoming_message.decrement_priority();
		pipe.write(incoming_message);
	}
	return true;
}

// OBLIGATORY PLUGIN FUNCTION

void run(plugin_pipe pipe, string plugin_name)
{
	// Construct a plugin object (used throughout)
	QuipPlugin plugin(name, pipe);
	// Register for these message types with the following priorities:
	plugin.register_for_message_type(tr(LIRCH_MSG_TYPE_HANDLER_READY));
	plugin.register_for_message_type(tr(LIRCH_MSG_TYPE_QUIP_REQUEST));
	// FIXME why is this here? pipe.write(register_handler::create("/quip", generate_generate_quip_message));
	while (plugin.handle_message(pipe.blocking_read()));
	{
		// TODO remove this message (nothing to see here)
		pipe.write(done_message::create(plugin_name));
	}
}
