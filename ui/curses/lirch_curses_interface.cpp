// This is sometimes required to support the full curses API
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

// Enable printf and scanf argument checking
#define GCC_PRINTF
#define	GCC_SCANF

#include <curses.h>
#include <climits>

#include <QString>

#include "lirch_constants.h"
#include "plugins/lirch_plugin.h"
#include "core/core_messages.h"

using namespace std;

void runplugin(plugin_pipe &p, const string &name)
{
	string input;
	QString processed_input;
	//TODO: Check the value of ESCDELAY, and set it to 10 if it doesn't exist
	int maxx, maxy;
	getmaxyx(stdscr, maxy, maxx);
	auto all_output=newpad(1000, maxx);
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
			wprintw(all_output, "%s: %x: ", QString::fromUcs4(&key, 1).toLocal8Bit().constData(), key);
			for (auto &i : QString::fromUcs4(&key, 1).toLocal8Bit())
				wprintw(all_output, "%02x ", (unsigned char)i);
			wprintw(all_output, ": ");
			//addch(ACS_HLINE);
			for (auto &i : QString::fromUcs4(&key, 1).toLocal8Bit())
				waddch(all_output, (unsigned char)i|(A_ALTCHARSET*((unsigned char)i>0x7f)));
				//addch(i&~(/*A_ALTCHARSET|*/A_BLINK|A_BOLD/*|A_DIM|A_INVIS|A_PROTECT*/));
			waddch(all_output, '\n');
		}
		else if (rc==KEY_CODE_YES)
		{
			break;
		}
		else
		{
			wprintw(all_output, "No key pressed\n");
		}
		int x,y;
		getyx(all_output, y, x);
		prefresh(all_output, max(y-maxy,0), 0, 0,0,maxy-1,maxx-1);
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
