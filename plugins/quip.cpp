// This is a plugin for Lirch (see us on GitHub!)
#include "plugins/lirch_plugin.h"
#include "plugins/quip.h"
#include "plugins/quip_messages.h"
#include "plugins/edict_messages.h"
#include "plugins/grinder_messages.h"

// QUIP CLASS FUNCTIONS (constructor and message wrapper)

QuipPlugin::Quip::Quip(quip_request_data *request_data) {
	// Fill in the destined channel name
	channel = request_data->channel;
	// Execute a subprocess (get a fortune)
	QProcess fortune_process;
	QString fortune_command = QObject::tr("fortune -sn 240");
	fortune_process.start(fortune_command, QIODevice::WriteOnly);
	fortune_process.waitForFinished();
	fortune_process.closeWriteChannel();
	text = QString(fortune_process.readAll());
	// Be at least a bit witty
	if (text.isEmpty()) {
		text = QObject::tr("I should install fortune!");
	}
}

message QuipPlugin::Quip::to_message() {
	// /quip messages appear in the same way /me messages do
	edict_message_subtype quip_subtype = edict_message_subtype::ME;
	// The format is <username> quips <fortune_text>
	QString quip_text = QObject::tr("quips ") + text;
	return edict_message::create(quip_subtype, channel, quip_text);
}

// QUIPPLUGIN CLASS FUNCTIONS (static)
#include <cassert>
message QuipPlugin::forward_quip_request(QString command, QString channel) {
	// TODO what else to do with command?
	assert(command == QObject::tr("/quip"));
	return quip_request_data(channel).to_message();
}

// QUIPPLUGIN CLASS FUNCTIONS (private)

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

// QUIPPLUGING PUBLIC FUNCTIONS (public)

#include <QDebug>

bool QuipPlugin::handle_message(message incoming_message)
{
	// Some message stop here and others go on
	bool propagate_message;
	// Pull out some internals for later use
	message_data *data = incoming_message.data.get();
	QString message_type = QString::fromStdString(incoming_message.type);
	// TODO remove this and #include above
	qDebug() << QObject::tr("Received '%1' message tag").arg(message_type);
	// The type of a message determines what to do with it
	switch(enumerate(message_type)) {
	// Shutdown messages immediately terminate the plugin
	case MessageType::SHUTDOWN:
		return false;
	// Registration statuses might need to be echoed
	case MessageType::REGISTRATION_STATUS:
		handle_registration_reply(dynamic_cast<registration_status *>(data));
		propagate_message = true;
		break;
	// Quip requests are captured and not forwarded
	case MessageType::QUIP_REQUEST:
		handle_quip_request(dynamic_cast<quip_request_data *>(data));
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
	// This message might need to be forwarded
	if (propagate_message) {
		incoming_message.decrement_priority();
		pipe.write(incoming_message);
	}
	return true;
}

void QuipPlugin::register_for_message_type(const QString &message_type, int with_priority) {
	pipe.write(registration_message::create(with_priority, name, message_type.toStdString()));
}

// MAIN PLUGIN FUNCTION

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
		pipe.write(done_message::create(internal_name));
	}
}
