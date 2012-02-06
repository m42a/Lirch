#include <iostream>
#include <string>

#include "message.h"
#include "message_pipe.h"

using namespace std;

const string tests[]={"toplugin", "tocore", "readempty"};

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
		if (ret==1)
			cout << "Test " << argv[n] << " failed\n";
		else if (ret==2)
			cout << "Unsupported test " << argv[n] << endl;
	}
	return ret;
}
