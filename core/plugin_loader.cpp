#include <string>

#ifdef _WIN32
#	include <windows.h>
#else //POSIX
#	include <dlfcn.h>
#endif

#include "plugin_loader.h"

//We don't *really* need this to be its own separate file, but i refuse to let
//this code near the main product.
bool load_plugin(std::string fname, const plugin_pipe &p)
{
#ifdef _WIN32
	HMODULE obj=LoadLibrary(fname.c_str());
	if (HMODULE==NULL)
		return false;
	FARPROC func=GetProcAddress(obj,"plugin_init");
	if (func==NULL)
		return false;
	//FARPROC returns an int * by default, so cast it to void.  This has
	//more parentheses than Lisp.
	(*(void __cdecl (*)(plugin_pipe))(func))(p);
#else //POSIX
	//Open the object file whenever you get around to it, and with its own
	//local symbol table.
	void *obj=dlopen(fname.c_str(), RTLD_LAZY | RTLD_LOCAL);
	if (obj==NULL)
		return false;
	//The POSIX standard says void * must convert to a function pointer,
	//but the C standard does not, so add a cast to shut up the compiler.
	auto func=(void (*)(plugin_pipe))dlsym(obj, "plugin_init");
	if (func==NULL)
		return false;
	//Finally initialize the plugin
	(*func)(p);
	//We should close the object here, but can't for technical reasons.
	//They're described in detail at:
	//https://github.com/m42a/Lirch/wiki/Stuff-That-Should-Be-Written-Down-Somewhere

#endif
	return true;
}
