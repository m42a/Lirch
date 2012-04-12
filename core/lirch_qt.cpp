#include <string>
#include <thread>
#include <vector>
using namespace std;

#include <QApplication>
#include <QObject>

#include "lirch_constants.h"
#include "core/core_messages.h"
#include "ui/qt/lirch_qt_interface.h"

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
        // A small, static interconnect object is used to mediate
        extern LirchClientPipe interconnect;
        QObject::connect(&interconnect, SIGNAL(show(const QString &, const QString &)),
                         &main_window,   SLOT(display(const QString &, const QString &)));
        QObject::connect(&interconnect, SIGNAL(shutdown(const QString &)),
                         &main_window,   SLOT(die(const QString &)));
        QObject::connect(&interconnect, SIGNAL(run(LirchClientPipe *)),
                         &main_window,   SLOT(use(LirchClientPipe *)));
	// TODO can we parse the args with QApplication?
	vector<message> vm;
	// Preload a variety of plugins specified in build (see lirch_constants.h)
	extern const preload_data preloads[LIRCH_NUM_PRELOADS];
	for (int i = 1; i < LIRCH_NUM_PRELOADS; ++i)
	{
		vm.push_back(
			plugin_adder::create(
				string(preloads[i - 1].name),
				string(preloads[i - 1].filename)
			)
		);
	}
	// Load plugins specified on command line
	for (int i=1; i<argc-1; i+=2)
	{
		if (argv[i]==string("-v") || argv[i]==string("--verbose"))
		{
			verbose=true;
			--i;
		}
		else
			vm.push_back(plugin_adder::create(argv[i],argv[i+1]));
	}
	thread core_thread(run_core, vm);
	core_thread.detach();
	return lirch.exec();
}
