// This is sometimes required to support the full curses API
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

// Enable printf and scanf argument checking
#define GCC_PRINTF
#define	GCC_SCANF

#include <curses.h>

#include <QString>

#include "lirch_constants.h"
#include "plugins/lirch_plugin.h"
#include "core/core_messages.h"

using namespace std;

void runplugin(plugin_pipe &p, const string &name)
{
	string input;
	QString processed_input;
	//p.write(registration_message::create(-30000, name, "display"));
	while (true)
	{
		/*while (p.has_message())
		{
			message m=p.read();
		}*/
		wint_t key;
		int rc=get_wch(&key);
		if (rc==OK)
		{
			//add_wch(key);
			printw("%s: %x\n", QString::fromUcs4(&key, 1).toLocal8Bit().constData(), key);
			for (auto &i : QString::fromUcs4(&key, 1).toLocal8Bit())
				printw("%02x ", (unsigned char)i);
			printw(": ");
			//addch(ACS_HLINE);
			for (auto &i : QString::fromUcs4(&key, 1).toLocal8Bit())
				addch((unsigned char)i|(A_ALTCHARSET*((unsigned char)i>0x7f)));
				//addch(i&~(/*A_ALTCHARSET|*/A_BLINK|A_BOLD/*|A_DIM|A_INVIS|A_PROTECT*/));
			printw("\n");
		}
		else if (rc==KEY_CODE_YES)
		{
			break;
		}
		else
		{
			printw("No key pressed\n");
		}
	}
	p.write(core_quit_message::create());
}

void run(plugin_pipe p, string name)
{
	// Step one...
	initscr();
	cbreak();
	//Wain no more than a tenth of a second for input
	//halfdelay(1);
	cbreak();
	timeout(100);
	//noecho();
	noecho();
	scrollok(stdscr, TRUE);
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	runplugin(p, name);
	endwin();
	return;
}
