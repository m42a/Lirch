#include <thread>
#include <string>

#ifdef _WIN32
#	include <windows.h>
#else //POSIX
#	include <dlfcn.h>
#endif

#include "plugin_loader.h"

using namespace std;

//We don't *really* need this to be its own separate file, but i refuse to let
//this code near the main product.
bool load_plugin(const string &fname, const plugin_pipe &p)
{
#ifdef _WIN32
	HMODULE obj=LoadLibrary(fname.c_str());
	if (HMODULE==NULL)
		return false;

	FARPROC ver_func=GetProcAddress(obj,"plugin_version");
	if (ver_func==NULL)
	{
		FreeLibrary(obj);
		return false;
	}

	int ver=(*(int __cdecl (*)())(ver_func))();
	if (ver!=0)
	{
		FreeLibrary(obj);
		return false;
	}

	FARPROC run_func=GetProcAddress(obj,"plugin_init");
	if (run_func==NULL)
	{
		FreeLibrary(obj);
		return false;
	}
	//FARPROC returns an int * by default, so cast it to void.  This has
	//more parentheses than Lisp.
	thread(*(void __cdecl (*)(plugin_pipe))(run_func), p).detach();
#else //POSIX
	//Open the object file but don't resolve its symbols, and give it its
	//own local symbol table.  This is faster than resolving symbols, but
	//if a library has unresolved symbols we won't know until it encounters
	//them at runtime.
	void *obj=dlopen(fname.c_str(), RTLD_LAZY | RTLD_LOCAL);
	if (obj==NULL)
		return false;
	//The POSIX standard says void * must convert to a function pointer,
	//but the C standard does not, so add a cast to shut up the compiler.
	auto ver_func=(int (*)())dlsym(obj, "plugin_version");
	if (ver_func==NULL)
	{
		dlclose(obj);
		return false;
	}
	//Plugin had the wrong version, so we can't load it
	int ver=(*ver_func)();
	if (ver!=0)
	{
		dlclose(obj);
		return false;
	}

	auto run_func=(void (*)(plugin_pipe))dlsym(obj, "plugin_init");
	if (run_func==NULL)
	{
		dlclose(obj);
		return false;
	}
	//Finally initialize the plugin
	thread(*run_func,p).detach();
#endif
	return true;
}
