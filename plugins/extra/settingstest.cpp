#include <string>

#include <QSettings>

#include "plugins/lirch_plugin.h"
#include "core/required_messages.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "The Addams Family", "Lirch");
	p.write(done_message::create(name));
}
