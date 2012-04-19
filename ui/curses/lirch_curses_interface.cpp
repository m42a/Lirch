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
	int maxx, maxy;
	getmaxyx(stdscr, maxy, maxx);
	//10000 lines of scrollback should be enough for anyone
	auto all_output=newpad(10000, maxx);
	//Let the output scroll
	scrollok(all_output, TRUE);
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
	//Set the delay when hitting escape to 10 milliseconds, unless it was
	//already set.  The ESCDELAY variable is not supported in all curses
	//implementations, but should not cause problems in implementations
	//that ignore it.
	setenv("ESCDELAY", "10", 0);
	//Initialize curses
	initscr();
	//Don't buffer typed characters
	cbreak();
	//Wain no more than a tenth of a second for input
	timeout(100);
	noecho();
	//Makes enter return \r instead of \n
	nonl();
	//Flush the input buffer if an interrupt key is pressed.  This ensures
	//we don't miss keystrokes in the event of a SIGSTOP
	intrflush(stdscr, FALSE);
	//Enable keycodes
	keypad(stdscr, TRUE);
	runplugin(p, name);
	//Make sure to always restore the terminal to a sane configuration
	endwin();
	return;
}
