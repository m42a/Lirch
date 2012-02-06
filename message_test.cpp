#include <iostream>
#include <string>

#include "message.h"
#include "message_pipe.h"

using namespace std;

const string tests[]={"toplugin", "tocore", "readempty", "cycle", "order"};

int dotest(string test)
{
	message_pipe mp;
	if (test=="toplugin")
	{
		mp.core_write(message{"Hello, plugin"});
		if (mp.plugin_read().text!="Hello, plugin")
			return 1;
		return 0;
	}
	if (test=="tocore")
	{
		mp.plugin_write(message{"Hello, core"});
		if (mp.core_read().text!="Hello, core")
			return 1;
		return 0;
	}
	if (test=="readempty")
	{
		if (mp.plugin_read().text!="")
			return 1;
		if (mp.core_read().text!="")
			return 1;
		return 0;
	}
	if (test=="cycle")
	{
		mp.plugin_write(message{"Whee!"});
		mp.core_write(mp.core_read());
		mp.plugin_write(mp.plugin_read());
		if (mp.core_read().text!="Whee!")
			return 1;
		return 0;
	}
	if (test=="order")
	{
		mp.plugin_write(message{"Message 1"});
		mp.core_write(message{"Message 2"});
		mp.plugin_write(message{"Message 3"});
		if (mp.core_read().text!="Message 1")
			return 1;
		if (mp.plugin_read().text!="Message 2")
			return 1;
		if (mp.core_read().text!="Message 3")
			return 1;
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
