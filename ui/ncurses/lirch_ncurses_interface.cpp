#include <ncurses.h>

#include "lirch_constants.h"
#include "plugins/lirch_plugin.h"

void run(plugin_pipe& p, std::string name)
{
	// Step one...
	p.write(registration_message::create(LIRCH_MSG_PRI_REG_MAX, name, "display"));
	return;
}
