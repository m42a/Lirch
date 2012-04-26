#ifndef QUIP_PLUGIN_H
#define QUIP_PLUGIN_H

#include <string>
#include <QString>
#include <QProcess>
#include "core/message.h"
#include "core/message_view.h"
#include "plugins/quip_constants.h"
#include "plugins/quip_messages.h"

// Encapsulates the internal name and a plugin's pipe (used for communication)
class QuipPlugin {
	std::string name;
	plugin_pipe pipe;

	// A QuipPlugin manages objects of the following type
	class Quip {
		// A Quip encapsuates the destined channel and fortune text
		QString channel;
		QString text;
	public:
		Quip(quip_request_data *request_data);
		message to_message();
	};

	// This is given to the meatgrinder (/quip handler)
	static message forward_quip_request(QString command, QString channel);
public:
	// TODO should we add something like this to the project globally?
	enum class MessageType {
		SHUTDOWN, REGISTRATION_STATUS, QUIP_REQUEST, HANDLER_READY, UNKNOWN
	};

	// When a quip plugin runs, we create an instance to manage its behavior
	QuipPlugin(const std::string &quip_plugin_name, plugin_pipe &quip_plugin_pipe) :
		name(quip_plugin_name),
		pipe(quip_plugin_pipe) { }

	// Plugins have exactly two roles: 1) register for messages; 2) handle messages
	void register_for_message_type(const QString &message_type, int with_priority = 0);
	bool handle_message(message incoming_message);
private:
	// These are helper functions for handle_message (above)
	MessageType enumerate(const QString &message_type) const;
	void handle_registration_reply(registration_status *status_data);
	void handle_quip_request(quip_request_data *request_data);
};

#endif // QUIP_PLUGIN_H
