#include <vector>
#include <csignal>
#include <string>

#include "message.h"
#include "message_pipe.h"
#include "core_messages.h"
#include "lirch_constants.h"

using namespace std;

extern message_pipe in_pipe;
extern bool verbose;

void run_core(const vector<message> &vm);

void handle_sigint(int)
{
	//Quit nicely when we get a ctrl-c, but not if we get it twice.  We
	//need to unregister before we write because we don't want to lock the
	//mutex twice.  This should only be a problem under heavy load (which
	//is exactly when we don't want to block SIGINT).
	signal(SIGINT, SIG_DFL);
	in_pipe.write(core_quit_message::create());
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL,"");
	if (signal(SIGINT, handle_sigint)==SIG_IGN)
		signal(SIGINT, SIG_IGN);
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

	// Loop until core shutdown
	run_core(vm);
}
