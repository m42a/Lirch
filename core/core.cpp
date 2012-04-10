#include <algorithm>
#include <unordered_map>
#include <string>
#include <thread>
#include <iostream>
#include <csignal>

#include "message.h"
#include "message_pipe.h"
#include "registry.h"
#include "plugin_loader.h"
#include "required_messages.h"
#include "core_messages.h"

using namespace std;

static unordered_map<string, message_pipe> out_pipes;
static unordered_map<string, registry> message_registrations;
static message_pipe in_pipe;

static bool verbose;

void handle_sigint(int)
{
	//Quit nicely when we get a ctrl-c, but not if we get it twice.  We
	//need to unregister before we write because we don't want to lock the
	//mutex twice.  This should only be a problem under heavy load (which
	//is exactly when we don't want to block SIGINT).
	signal(SIGINT, SIG_DFL);
	in_pipe.write(core_quit_message::create());
}

ostream &operator<<(ostream &out, const message &m)
{
	return out << '(' << m.type << ',' << m.priority << ',' << m.getdata() << ')';
}

static void to_plugin(message m)
{
	if (message_registrations.count(m.gettype())==0)
	{
		//This message type has no registration, so discard it
		if (verbose)
			cerr << " was not registered" << endl;
		return;
	}
	auto plugin=message_registrations[m.gettype()].get(m.getpriority());
	if (plugin.second=="")
	{
		if (verbose)
			cerr << " has no more registered plugins" << endl;
		return;
	}
	if (out_pipes.count(plugin.second)==0)
	{
		//This type does not exist
		if (verbose)
			cerr << " is registered to the non-existent plugin " << plugin.second << endl;
		return;
	}
	//Change the message priority to the registered priority
	out_pipes[plugin.second].write(m.change_priority(plugin.first));
	if (verbose)
		cerr << " was sent to " << plugin.second << " as " << m << endl;
}

static void add_plugin(const message &m)
{
	auto pa=dynamic_cast<plugin_adder *>(m.getdata());
	if (!pa)
		//The message wasn't a plugin_adder
		return;
	if (out_pipes.count(pa->name)!=0)
		//There's already a plugin with this name
		return;
	message_pipe mp;
	thread t1(load_plugin, pa->filename, plugin_pipe(bidirectional_message_pipe(mp, in_pipe)));
	t1.detach();
	out_pipes[pa->name]=mp;
	mp.write(hello_message::create(pa->name));
	if (verbose)
		cerr << " loaded plugin " << pa->name << " from " << pa->filename << endl;
}

static void remove_plugin(const message &m)
{
	auto d=dynamic_cast<done_message *>(m.getdata());
	if (!d)
		return;
	for (auto &i : message_registrations)
		i.second.removeall(d->name);
	decltype(message_registrations.begin()) i;
	//Who says C++ is verbose?
	while ((i=std::find_if(message_registrations.begin(), message_registrations.end(), [](decltype(*i) &p) {return p.second.empty();}))!=message_registrations.end())
			message_registrations.erase(i);
	out_pipes.erase(d->name);
	if (verbose)
		cerr << " removed plugin " << d->name << endl;
}

static void add_registration(const message &m)
{
	auto r=dynamic_cast<registration_message *>(m.getdata());
	if (!r)
		return;
	if (out_pipes.count(r->getname())==0)
		//We can't talk to this plugin, so ignore its request
		return;

	bool b=message_registrations[r->getmessage()].add(r->getpriority(), r->getname());
	//Tell the plugin whether it failed or not
	out_pipes[r->getname()].write(registration_status::create(b, r->getpriority(), r->getmessage()));
	if (verbose)
		cerr << " registered " << r->message_type << " to " << r->plugin_name << endl;
}

static void target_plugin(const message &m)
{
	auto i=dynamic_cast<targeted_message *>(m.getdata());
	if (!i)
		return;
	if (out_pipes.count(i->name)==0)
		return;
	out_pipes[i->name].write(i->mess);
	if (verbose)
		cerr << " targeted " << i->mess << " towards " << i->name << endl;
}

static void initiate_shutdown()
{
	if (verbose)
		cerr << " initiated a shutdown" << endl;
	for (auto &i : out_pipes)
		i.second.write(shutdown_message::create());
}

static void process(const message &m)
{
	if (verbose)
		cerr << "Message " << m;
	if (m.gettype()=="add_plugin")
		add_plugin(m);
	else if (m.gettype()=="register")
		add_registration(m);
	else if (m.type=="done")
		remove_plugin(m);
	else if (m.type=="target")
		target_plugin(m);
	else if (m.type=="core_quit")
		initiate_shutdown();
	else
		to_plugin(m);
}

static void run_core(const vector<message> &vm)
{
	for (auto m : vm)
		process(m);
	while (!out_pipes.empty())
	{
		process(in_pipe.blocking_read());
	}
}

int main(int argc, char *argv[])
{
	setlocale(LC_ALL,"");
	signal(SIGINT, handle_sigint);
	vector<message> vm;
	for (int i=1; i<argc-1; i+=2)
	{
		if (argv[i]==string("-v") || argv[i]==string("--verbose"))
		{
			verbose=true;
			--i;
		}
		else
			vm.push_back(plugin_adder::create(argv[i],argv[i+1]));
	}
	run_core(vm);
}
