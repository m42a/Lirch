#include <string>
#include <QSettings>

#include "lirch_plugin.h"

using namespace std;

void run(plugin_pipe p, string name)
{
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "The Addams Family", "Lirch");
}
