// This is a plugin for Lirch (see us on GitHub!)
#include "plugins/lirch_plugin.h"
#include "plugins/quip.h"
#include "plugins/quip_messages.h"
#include "plugins/edict_messages.h"
#include "plugins/grinder_messages.h"

// QUIP CLASS FUNCTIONS (constructor and message wrapper)

QuipPlugin::Quip::Quip(quip_request_data *request_data)
{
	// Fill in the destined channel name
	channel = request_data->channel;
	// Execute a subprocess (get a fortune)
	QProcess fortune_process;
	QString fortune_command = QObject::tr("fortune -sn 240");
	fortune_process.start(fortune_command, QIODevice::ReadOnly);
	fortune_process.waitForFinished();
	fortune_process.closeWriteChannel();
	text = QString(fortune_process.readAll());
	// Be at least a bit witty
	if (text.isEmpty())
	{
		text = QObject::tr("I should install fortune!");
	}
}

message QuipPlugin::Quip::to_message()
{
	// /quip messages appear in the same way /me messages do
	edict_message_subtype quip_subtype = edict_message_subtype::ME;
	// The format is <username> quips <fortune_text>
	QString quip_text = QObject::tr("quips\n") + text;
	return edict_message::create(quip_subtype, channel, quip_text);
}

// QUIPPLUGIN CLASS FUNCTIONS (static)

message QuipPlugin::forward_quip_request(QString command, QString channel)
{
	if (command == QObject::tr("/quip"))
	{
		return quip_request_data(channel).to_message();
	}
	return empty_message::create();
}

// QUIPPLUGIN CLASS FUNCTIONS (private)

void QuipPlugin::handle_registration_reply(registration_status *status_data)
{
	if (!status_data)
	{
		return;
	}
	int priority = status_data->priority;
	// Provided we failed to register, but have high enough priority, retry
	if (!status_data->status && priority > LIRCH_MESSAGE_PRIORITY_MIN)
	{
		std::string message_type = status_data->type;
		pipe.write(registration_message::create(--priority, name, message_type));
	}
}

void QuipPlugin::handle_quip_request(quip_request_data *request_data)
{
	if (!request_data)
	{
		return;
	}
	// Make a Quip and forward it
	Quip quip(request_data);
	message quip_message = quip.to_message();
	pipe.write(quip_message);
}

// QUIPPLUGING PUBLIC FUNCTIONS (public)

bool QuipPlugin::has_not_received_shutdown_command()
{
	message incoming_message = pipe.blocking_read();
	// Pull out some internals for later use
	std::unique_ptr<message_data> message_data = std::move(incoming_message.data);
	QString message_type = QString::fromStdString(incoming_message.type);
	// The type of a message determines what to do with it
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_SHUTDOWN))
	{
		// Shutdown messages immediately terminate the plugin
		return false;
	}
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_REGISTRATION_STATUS))
	{
		// Registration statuses might need to be echoed
		handle_registration_reply(dynamic_cast<registration_status *>(message_data.get()));
		return true;
	}
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_QUIP_REQUEST))
	{
		// Quip requests are captured and not forwarded
		handle_quip_request(dynamic_cast<quip_request_data *>(message_data.get()));
		return true;
	}
	if (message_type == QObject::tr(LIRCH_MESSAGE_TYPE_HANDLER_READY))
	{
		// When the handler is ready, we want to tell it we know how to handle /quip
		pipe.write(register_handler::create(QObject::tr("/quip"), forward_quip_request));
	}
	// Put back what you stole
	incoming_message.data = std::move(message_data);
	// This message might need to be forwarded (do so by default)
	incoming_message.decrement_priority();
	pipe.write(incoming_message);
	return true;
}

void QuipPlugin::register_for_message_type(const QString &message_type, int with_priority)
{
	std::string type_string = message_type.toStdString();
	pipe.write(registration_message::create(with_priority, name, type_string));
}

// MAIN PLUGIN FUNCTION

void run(plugin_pipe pipe, std::string internal_name)
{
	// TODO remove this line once race condition is fixed
	pipe.write(register_handler::create(QObject::tr("/quip"), QuipPlugin::forward_quip_request));
	// Construct a plugin object (used throughout)
	QuipPlugin plugin(internal_name, pipe);
	// Register for these message types with the following priorities:
	plugin.register_for_message_type(QObject::tr(LIRCH_MESSAGE_TYPE_HANDLER_READY));
	plugin.register_for_message_type(QObject::tr(LIRCH_MESSAGE_TYPE_QUIP_REQUEST));
	// Begin a loop that will only exit when sent the shutdown command
	while (plugin.has_not_received_shutdown_command());
}
