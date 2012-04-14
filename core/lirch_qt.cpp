#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#include <QApplication>
#include <QObject>
#include <QString>

#include "lirch_constants.h"
#include "core/core_messages.h"
#include "ui/lirch_client_pipe.h"
#include "ui/qt/lirch_qt_interface.h"

LirchClientPipe mediator;

void run_core(const vector<message> &vm);

extern bool verbose;

#define USE_TCLAP
#ifdef USE_TCLAP
#include <tclap/CmdLine.h>

static struct lirch_options {
  bool list_preloads, no_preloads, verbose;
  vector<string> plugins;
} session;

void parse_args(int argc, char *argv[]) {
	try {
		// Init the options object
		QString id = QObject::tr("%1 %2 core (%3)").arg(
						LIRCH_PRODUCT_NAME,
						LIRCH_VERSION_STRING,
						LIRCH_BUILD_HASH);
		TCLAP::CmdLine options(id.toStdString(), ' ', LIRCH_VERSION_STRING);
		// Specify switches TODO functionalize?
		TCLAP::SwitchArg verboseSwitch("v", "verbose",
			"Enable loud output (all messages)", options, false);
		TCLAP::SwitchArg noPreloadsSwitch("n", "no_preloads",
			"Don't load the preloaded plugins", options, false);
		TCLAP::SwitchArg listPreloadsSwitch("l", "list_preloads",
			"List the preloaded plugins", options, false);
		TCLAP::UnlabeledMultiArg<string> plugin_pairs("plugin_pairs",
			"Bare strings, in pairs like: antenna lib/libantenna.so", false,
			"plugin_name plugin_file", "pair<string, string>");
                options.add(plugin_pairs);
		// Parse all the options
		options.parse(argc, argv);
		session.verbose = verboseSwitch.getValue();
		session.no_preloads = noPreloadsSwitch.getValue();
		session.list_preloads = listPreloadsSwitch.getValue();
		session.plugins = plugin_pairs.getValue();
	} catch (TCLAP::ArgException &e) {
		cerr << e.error() << endl;
	}
}
#endif // USE_TCLAP

int main(int argc, char *argv[])
{
	// TODO review QtCore's QtApplication documentation
	QApplication lirch(argc, argv);
	// TODO sometimes being necessary to avoid string conversion issues?
	setlocale(LC_NUMERIC, "C");
	// The window is constructed here, show()'n later (see plugin header)
	LirchQtInterface main_window;
        // A small intermediate object is used to mediate
        QObject::connect(&mediator,    SIGNAL(alert(QString, QString)),
                         &main_window, SLOT(display(const QString &, const QString &)));
        QObject::connect(&mediator,    SIGNAL(shutdown(QString)),
                         &main_window, SLOT(die(const QString &)));
        QObject::connect(&mediator,    SIGNAL(run(LirchClientPipe *)),
                         &main_window, SLOT(use(LirchClientPipe *)));

	vector<pair<string, string>> plugins;
	// Prepare the list of plugins specified in build (see lirch_constants.h)
	for (auto &p : preloads)
	{
		plugins.push_back(make_pair<string, string>(p.name, p.filename));
	}

	// TODO figure out what happens to unparsed args
	cerr << "Before parse: ";
	for (int i = 0; i < argc; ++i) {
		cerr << "'" << argv[i] << "' ";
	}
	cerr << "(end)" << endl;
	parse_args(argc, argv);
	verbose = session.verbose;
	cerr << "After parse: ";
	for (int i = 0; i < argc; ++i) {
		cerr << "'" << argv[i] << "' ";
	}
	cerr << "(end)" << endl;

	// Handle a request to display these
	if (session.list_preloads) {
		for (auto &p : plugins) {
			cerr << p.first << ": " << p.second << endl;
		}
	}

	// Honor a request to ignore these
	if (session.no_preloads) {
		plugins.clear();
	}

	// Add in plugins specified by the command line
	auto pit = session.plugins.begin();
	auto pitend = session.plugins.end();
	pair<string, string> p;
	while (pit != pitend) {
		p.first = *pit; ++pit;
		if (pit == pitend) {
			break;
		}
		p.second = *pit; ++pit;
		plugins.push_back(p);
	}

	// Run the core in a separate thread
	vector<message> add_messages;
	for (auto &p : plugins) {
		add_messages.push_back(plugin_adder::create(p.first, p.second));
	}
	thread core_thread(run_core, add_messages);
	core_thread.detach();

	// When the event loop terminates, so will we
	return lirch.exec();
}
