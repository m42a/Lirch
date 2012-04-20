// This is sometimes required to support the full curses API
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#ifdef __GNUC__
	// Enable printf and scanf argument checking
	#define GCC_PRINTF
	#define	GCC_SCANF
#endif

#include <curses.h>
#include <climits>
#include <cstdio>

#include <QString>
#include <QTextBoundaryFinder>

#include "lirch_constants.h"
#include "plugins/lirch_plugin.h"
#include "core/core_messages.h"

inline char CTRL(char c)
{
	//Leave only the lower 5 bits, so the return value is the numeric value
	//of CTRL+key (e.g. CTRL('c') gives you 3, which is the value you get
	//when hitting ctrl-c)
	return c&0x1f;
}

using namespace std;

template <class... args>
string strprintf(const string &format, args... a)
{
	//Thankfully, the C++ standard grabbed the definition from POSIX
	//instead of Windows, so I don't have to binary-search the correct
	//string size.
	int size=snprintf(NULL, 0, format.c_str(), a...);
	//Add padding for the terminating byte
	vector<char> s(size+1);
	//We can use sprintf instead of snprintf because we know the buffer is large enough
	sprintf(s.data(), format.c_str(), a...);
	return s.data();
}

//Same as wprintf, but handles unicode characters properly
template <class... args>
void wprintu(WINDOW *w, const string &format, args... a)
{
	string s=strprintf(format, a...);
	for (unsigned char c : s)
		waddch(w, c|(c>0x7f ? A_ALTCHARSET : 0));
}

//This wraps WINDOW pointers so they're be destroyed on exit
class WindowWrapper
{
public:
	WindowWrapper(WINDOW *ww=nullptr) : w(ww, delwin) {}
	WINDOW *get() const {return w.get();}
	operator WINDOW*() const {return get();}
	WINDOW& operator*() const {return *w;}
	WINDOW* operator->() const {return get();}
private:
	shared_ptr<WINDOW> w;
};

void runplugin(plugin_pipe &p, const string &name)
{
	QString input;
	int maxx, maxy;
	getmaxyx(stdscr, maxy, maxx);
	//10000 lines of scrollback should be enough for anyone
	WindowWrapper channel_output=newpad(10000, maxx);
	//Let the output scroll
	scrollok(channel_output, TRUE);
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
			if (key=='\r' || key=='\n')
			{
				wprintu(channel_output, "Message sent!\n");
				input="";
			}
			else
				input.push_back(QString::fromUcs4(&key, 1));
			wprintu(channel_output, "%s\n", input.toLocal8Bit().constData());
		}
		else if (rc==KEY_CODE_YES)
		{
			if (key==KEY_BACKSPACE)
			{
				QTextBoundaryFinder bounds(QTextBoundaryFinder::Grapheme, input);
				bounds.toEnd();
				int pos=bounds.toPreviousBoundary();
				if (pos!=-1)
				{
					input.remove(pos, INT_MAX);
				}
			}
			else if (key==KEY_ENTER)
			{
				wprintu(channel_output, "Message sent!\n");
				input="";
			}
			else
				break;
			wprintu(channel_output, "%s\n", input.toLocal8Bit().constData());
		}
		int x,y;
		getyx(channel_output, y, x);
		pnoutrefresh(channel_output, max(y-(maxy-1),0), 0, 0,0,maxy-2,maxx-1);
		wmove(stdscr, maxy-1,0);
		doupdate();
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
