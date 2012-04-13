#include <iostream>
#include <string>
#include <thread>
#include <vector>
using namespace std;

#include <QApplication>
#include <QObject>

#include "lirch_constants.h"
#include "core/core_messages.h"
#include "ui/lirch_client_pipe.h"
#include "ui/qt/lirch_qt_interface.h"

LirchClientPipe mediator;

void run_core(const vector<message> &vm);

extern bool verbose;

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

	vector<message> vm;
	// Preload a variety of plugins specified in build (see lirch_constants.h)
	for (auto &p : preloads)
	{
		vm.push_back(plugin_adder::create(string(p.name), string(p.filename)));
	}

	// Load plugins specified on command line
	// TODO can we parse the args with QApplication?
	if (argc > 1) {
		int i = 1, j = 0;
		// The first argument might specify verbose mode
		if (argv[i] == string("-v") || argv[i] == string("--verbose")) {
			cerr << "Lirch " << LIRCH_VERSION_STRING << " Core (" << LIRCH_BUILD_HASH << ")" << endl;
			cerr << "preloads: ";
			for (auto &p : preloads) {
				cerr << p.name << " ";
			}
			cerr << ";" << endl;
			for (auto &p : preloads) {
				cerr << ++j << ") " << p.filename << endl;
			}
			verbose = true;
			++i;
		}
		// The rest are plugin (name, filename) pairs (need two at a time)
		string name, filename;
		while (i + 1 < argc) {
			name = argv[i++];
			filename = argv[i++];
			vm.push_back(plugin_adder::create(name, filename));
		}
	}

	// Run the core in a separate thread
	thread core_thread(run_core, vm);
	core_thread.detach();
	// When the event loop terminates, so will we
	return lirch.exec();
}
