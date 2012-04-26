#include <string>

#include <QString>
#include <QProcess>

#include "core/message.h"
#include "core/message_view.h"

class Quip {
	QString text;

	// Helper functions called by the constructor
	void execute_fortune_process();
private:
	Quip(quip_request *request_data);
	message to_message();
}

class QuipPlugin {
	// A QuipPlugin wraps its internal name and a pipe (used for communication)
	std::string name;
	plugin_pipe pipe;

	// This is given to the meatgrinder (/quip handler)
	void forward_quip_request(QString command, QString channel);

	// TODO should we add this to the project globally?
	enum class MessageType {
		SHUTDOWN,
		REGISTRATION_STATUS,
		QUIP_REQUEST,
		HANDLER_READY,
		UNKNOWN;
	};

	// Helper functions for handle_message (see below)
	MessageType enumerate(const QString &message_type) const;
	void handle_registration_reply(registration status *status_data);
	void handle_quip_request(quip_request *request_data);
public:
	// When the plugin runs, we create an instance to manage behavior
	QuipPlugin(const std::string &quip_plugin_name, plugin_pipe &quip_plugin_pipe) :
		name(quip_plugin_name),
		pipe(quip_plugin_pipe) { }

	// Plugins have exactly two roles: 1) register for messages; 2) handle messages
	void register_for_message_type(const QString &message_type, int with_priority = 0);
	void handle_message(message incoming_message);
};
