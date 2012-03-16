#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "message.h"
#include "message_pipe.h"

using namespace std;

int dotest(string test)
{
	bidirectional_message_pipe mp;
	if (test=="toplugin")
	{
		mp.core_write(message{"Hello, plugin"});
		if (mp.plugin_read().type!="Hello, plugin")
			return 1;
		return 0;
	}
	if (test=="tocore")
	{
		mp.plugin_write(message{"Hello, core"});
		if (mp.core_read().type!="Hello, core")
			return 1;
		return 0;
	}
	if (test=="readempty")
	{
		if (mp.plugin_read().type!="")
			return 1;
		if (mp.core_read().type!="")
			return 1;
		return 0;
	}
	if (test=="cycle")
	{
		mp.plugin_write(message{"Whee!"});
		mp.core_write(mp.core_read());
		mp.plugin_write(mp.plugin_read());
		if (mp.core_read().type!="Whee!")
			return 1;
		return 0;
	}
	if (test=="exhaust")
	{
		mp.plugin_write(message{"Yukkuri shite itte ne"});
		if (mp.core_read().type!="Yukkuri shite itte ne")
			return 1;
		if (mp.core_read().type!="")
			return 1;
		if (mp.plugin_read().type!="")
			return 1;
		return 0;
	}
	if (test=="sharing")
	{
		message_pipe shared;
		bidirectional_message_pipe mp1(message_pipe(), shared);
		bidirectional_message_pipe mp2(message_pipe(), shared);
		mp1.plugin_write(message{"I said hey!"});
		if (mp1.core_peek().type!="I said hey!")
			return 1;
		if (mp2.core_read().type!="I said hey!")
			return 1;
		if (mp1.core_read().type!="")
			return 1;
		mp1.core_write(message{"HEY!"});
		if (mp2.plugin_read().type!="")
			return 1;
		if (mp1.plugin_read().type!="HEY!")
			return 1;
		return 0;
	}
	if (test=="order")
	{
		mp.plugin_write(message{"Message 1"});
		mp.core_write(message{"Message 2"});
		mp.plugin_write(message{"Message 3"});
		if (mp.core_read().type!="Message 1")
			return 1;
		if (mp.plugin_read().type!="Message 2")
			return 1;
		if (mp.core_read().type!="Message 3")
			return 1;
		return 0;
	}
	if (test=="copying")
	{
		mp.plugin_write(message{"open the pod bay doors hal"});
		bidirectional_message_pipe mp2=mp;
		if (mp2.core_read().type!="open the pod bay doors hal")
			return 1;
		cout << "part1\n";
		mp2.core_write(message{"I'm sorry Dave"});
		mp.core_write(message{"I'm afraid I can't do that"});
		if (mp2.plugin_read().type!="I'm sorry Dave")
			return 1;
		bidirectional_message_pipe mp3=mp2;
		if (mp3.plugin_read().type!="I'm afraid I can't do that")
			return 1;
		mp2.plugin_write(message{"whats the problem"});
		if (mp3.core_read().type!="whats the problem")
			return 1;
		mp3.core_write(message{"Your grammar is too terrible, Dave"});
		if (mp.plugin_read().type!="Your grammar is too terrible, Dave")
			return 1;
		return 0;
	}
	if (test=="threadseq")
	{
		thread t1([&mp]()
		{
			mp.plugin_write(message{"na"});
		});
		thread t2([&mp]()
		{
			mp.plugin_write(message{"na"});
		});
		t1.join();
		t2.join();
		if (mp.core_read().type!="na")
			return 1;
		if (mp.core_read().type!="na")
			return 1;
		if (mp.core_read().type!="")
			return 1;
		return 0;
	}
	if (test=="0bread")
	{
		mp.core_write(message{"wa pan nashi!"});
		if (mp.plugin_blocking_read().type!="wa pan nashi!")
			return 1;
		return 0;
	}
	if (test=="1bread")
	{
		thread t([&mp]()
		{
			this_thread::sleep_for(chrono::milliseconds(500));
			mp.plugin_write(message{"Threading"});
		});
		if (mp.core_blocking_read().type!="Threading")
		{
			t.join();
			return 1;
		}
		t.join();
		return 0;
	}
	if (test=="2bread")
	{
		thread t([&mp]()
		{
			string s=mp.plugin_blocking_read().type;
			this_thread::sleep_for(chrono::milliseconds(250));
			if (s!="Batman")
				mp.plugin_write(message{"Aquaman"});
			else
				mp.plugin_write(message{"Superman"});
		});
		mp.plugin_write(message{"Spiderman"});
		mp.core_write(message{"Batman"});
		if (mp.core_blocking_read().type!="Spiderman")
		{
			t.join();
			return 1;
		}
		if (mp.core_blocking_read().type!="Superman")
		{
			t.join();
			return 1;
		}
		t.join();
		return 0;
	}
	if (test=="shared2bread")
	{
		thread t1([&mp]()
		{
			if (mp.plugin_blocking_read().type!="rm")
				mp.plugin_write(message{"/"});
			else
				mp.plugin_write(message{"-rf"});
		});
		thread t2([&mp]()
		{
			if (mp.plugin_blocking_read().type!="rm")
				mp.plugin_write(message{"/"});
			else
				mp.plugin_write(message{"-rf"});
		});
		this_thread::sleep_for(chrono::milliseconds(250));
		mp.core_write(message{"rm"});
		if (mp.core_blocking_read().type!="-rf")
		{
			mp.core_write(message{""});
			mp.core_write(message{""});
			t1.join();
			t2.join();
			return 1;
		}
		if (mp.core_read().type!="")
		{
			mp.core_write(message{""});
			mp.core_write(message{""});
			t1.join();
			t2.join();
			return 1;
		}
		mp.core_write(message{"rm"});
		if (mp.core_blocking_read().type!="-rf")
		{
			mp.core_write(message{""});
			mp.core_write(message{""});
			t1.join();
			t2.join();
			return 1;
		}
		t1.join();
		t2.join();
		return 0;
	}
	return 2;
}

int main(int argc, char *argv[])
{
	if (argc<2)
		return 0;
	bool verbose=false;
	int n=1;
	while (argv[n]!=NULL && argv[n][0]=='-')
	{
		for (int o=1;argv[n][o]!='\0';++o)
		{
			if (argv[n][o]=='v')
				verbose=true;
			else if (argv[n][o]=='q')
				verbose=false;
			else
			{
				cout << "Unsupported option '" << argv[n][o] << "'\n";
				return 3;
			}
		}
		++n;
	}
	if (argv[n]==NULL)
	{
		cout << "No test specified\n";
		return 3;
	}
	int ret=dotest(argv[n]);
	if (verbose)
	{
		if (ret==0)
			cout << "Test " << argv[n] << " was successful\n";
		else if (ret==1)
			cout << "Test " << argv[n] << " failed\n";
		else if (ret==2)
			cout << "Unsupported test " << argv[n] << endl;
	}
	return ret;
}
