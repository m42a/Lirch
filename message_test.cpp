#include <iostream>
#include <string>

#include "message.h"
#include "message_pipe.h"

using namespace std;

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
}

int main(int argc, char *argv[])
{
	if (argc<=2)
		return 0;
	int ret=dotest(argv[1]);
	if (ret==1)
		cout << "Test " << argv[1] << " failed\n";
	else if (ret==2)
		cout << "Unsupported test " << argv[1] << endl;
	return ret;
}
