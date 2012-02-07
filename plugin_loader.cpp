#include <string>

#ifdef WIN32
#	include <windows.h>
#else //POSIX
#	include <dlfcn.h>
#endif

bool load_plugin(std::string fname)
{
#ifdef WIN32
	HMODULE obj=LoadLibrary(fname.c_str());
	if (HMODULE==NULL)
		return false;
	FARPROC func=GetProcAddress(obj,"init");
	if (func==NULL)
		return false;
	//FARPROC returns an int * by default, so cast it to void.  This has
	//more parentheses than Lisp.
	(*(void (*)())(func))();
#else //POSIX
	//Open the object file whenever you get around to it, and with its own
	//local symbol table.
	void *obj=dlopen(fname.c_str(), RTLD_LAZY | RTLD_LOCAL);
	if (obj==NULL)
		return false;
	//The POSIX standard says void * must convert to a function pointer,
	//but the C standard does not, so add a cast to shut up the compiler.
	void (*func)()=(void (*)())dlsym(obj, "init");
	if (func==NULL)
		return false;
	//Finally initialize the plugin
	(*func)();
#endif
	return true;
}
