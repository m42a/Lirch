#include "plugins/lirch_plugin.h"

#include "plugins/quip.h"

#include "plugins/quip_messages.h"

#include "plugins/edict_messages.h"
#include "plugins/grinder_messages.h"

// TODO DOCUMENT
QuipPlugin::Quip::Quip(quip_request_data *request_data) {
	channel = request_data->channel;
	QProcess fortune_process;
	// TODO openmode QIODevice::WriteOnly?
	fortune_process.start("fortune -sn 240", QIODevice::ReadWrite);
	fortune_process.waitForFinished();
	fortune_process.closeWriteChannel();
	QByteArray result = fortune_process.readAll();
	text = QString(result);
}

message QuipPlugin::Quip::to_message() {
	// /quip messages appear in the same way /me messages do
	edict_message_subtype quip_subtype = edict_message_subtype::ME;
	// The format is <username> quips <fortune_text>
	QString quip_text = QObject::tr("quips ") + text;
	return edict_message::create(quip_subtype, channel, quip_text);
}

#define LIRCH_MESSAGE_TYPE_SHUTDOWN            "registration_status"
#define LIRCH_MESSAGE_TYPE_HANDLER_READY       "handler_ready"
#define LIRCH_MESSAGE_TYPE_REGISTRATION_STATUS "registration_status"
#define LIRCH_MESSAGE_TYPE_QUIP_REQUEST        "quip_request"
#include <cassert>

message QuipPlugin::forward_quip_request(QString command, QString channel) {
	// TODO what else to do with command?
	assert(command == QObject::tr("/quip"));
	quip_request_data request_data(channel);
	return request_data.to_message();
}


void QuipPlugin::handle_registration_reply(registration_status *status_data) {
	if (!status_data) {
		return;
	}
	int priority = status_data->priority;
	if (!status_data->status && priority > 0) {
		std::string message_type = status_data->type;
		pipe.write(registration_message::create(--priority, name, message_type));
	}
}

void QuipPlugin::handle_quip_request(quip_request_data *request_data) {
	if (!request_data) {
		return;
	}
	pipe.write(Quip(request_data).to_message());
}


void QuipPlugin::register_for_message_type(const QString &message_type, int with_priority) {
	pipe.write(registration_message::create(with_priority, name, message_type.toStdString()));
}

QuipPlugin::MessageType enumerate(const QString &message_type) {
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_SHUTDOWN)) {
		return QuipPlugin::MessageType::SHUTDOWN;
	}
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_REGISTRATION_STATUS)) {
		return QuipPlugin::MessageType::REGISTRATION_STATUS;
	}
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_QUIP_REQUEST)) {
		return QuipPlugin::MessageType::QUIP_REQUEST;
	}
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_HANDLER_READY)) {
		return QuipPlugin::MessageType::HANDLER_READY;
	}
	return QuipPlugin::MessageType::UNKNOWN;
}

// FIXME add doc and are we passing the right value? make lines shorter
bool QuipPlugin::handle_message(message incoming_message)
{
	bool propagate_message;
	// The type of a message determines what to do with it
	switch(enumerate(QString::fromStdString(incoming_message.type))) {
	// Shutdown messages immediately terminate the plugin
	case MessageType::SHUTDOWN:
		return false;
	// Registration statuses might need to be echoed
	case MessageType::REGISTRATION_STATUS:
		handle_registration_reply(dynamic_cast<registration_status *>(incoming_message.getdata()));
		// FIXME should this be true?
		propagate_message = false;
		break;
	// Quip requests are captured and not forwarded
	case MessageType::QUIP_REQUEST:
		handle_quip_request(dynamic_cast<quip_request_data *>(incoming_message.getdata()));
		propagate_message = false;
		break;
	// When the handler is ready, we want to register something
	case MessageType::HANDLER_READY:
		// Register the way to handle quip commands
		pipe.write(register_handler::create(QObject::tr("/quip"), forward_quip_request));
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

void run(plugin_pipe pipe, std::string internal_name)
{
	// Construct a plugin object (used throughout)
	QuipPlugin plugin(internal_name, pipe);
	// Register for these message types with the following priorities:
	plugin.register_for_message_type(QObject::tr(LIRCH_MESSAGE_TYPE_HANDLER_READY));
	plugin.register_for_message_type(QObject::tr(LIRCH_MESSAGE_TYPE_QUIP_REQUEST));
	// FIXME why is this here?
	//pipe.write(register_handler::create("/quip", generate_generate_quip_message));
	while (plugin.handle_message(pipe.blocking_read()));
	{
		// TODO remove this message (nothing to see here)
		pipe.write(done_message::create(internal_name));
	}
}
