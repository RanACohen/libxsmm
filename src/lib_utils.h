//
// Created by rancohen on 27/10/2020.
//

#ifndef LIBXSMM_LIB_UTILS_H
#define LIBXSMM_LIB_UTILS_H
#include <libxsmm_main.h>
#include <iostream>

typedef int (*BuildFunc)(const libxsmm_build_request* , libxsmm_code_pointer*);

#ifdef _WIN32
#include <libloaderapi.h>
#include <errhandlingapi.h>
#include <Windows.h>

typedef HMODULE LibraryHandle;

void getErrorText(int code, char *msg, int maxLen)
{
    LPTSTR errorText = NULL;

    DWORD dwFlags = // use system message tables to retrieve error text
            FORMAT_MESSAGE_FROM_SYSTEM
            // allocate buffer on local heap for error text
            | FORMAT_MESSAGE_ALLOCATE_BUFFER
            // Important! will fail otherwise, since we're not (and CANNOT) pass insertion parameters
            | FORMAT_MESSAGE_IGNORE_INSERTS;

    DWORD rc = FormatMessage(dwFlags, NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);

    if ( rc>0 )
    {
        strncpy_s(msg, maxLen, errorText, rc);
        LocalFree(errorText);
        return;
    }
    snprintf(msg, maxLen, "Cannot parse error code %d", code);
}
#else
// linux utility functions
#include <dlfcn.h>
typedef void* LibraryHandle;
#endif

#ifdef _WIN32

// todo: error handling...
LibraryHandle LoadSharedLib(const char *libraryName)
{
    HMODULE ret = LoadLibrary(libraryName);
    if (ret == NULL)
    {
        DWORD err = GetLastError();
        //std::string errText = getErrorText(err);
        //throw std::runtime_error(errText.c_str());
    }
    return ret;
}

BuildFunc GetBuildFunction(LibraryHandle libraryHandle)
{
    FARPROC f = GetProcAddress(libraryHandle, "libxsmm_build_target");
    if (f == NULL)
    {
        //throw std::exception("cannot find symbol 'libxsmm_build_target' in Library.");
    }
    return (BuildFunc)f;
}
#else
LibraryHandle LoadSharedLib(const char *libraryName)
{
    auto lib = dlopen("libhbn_plugin.so", RTLD_LAZY);
    if (!lib) {
        fprintf(stderr, "Issues with opening the library!");
        throw std::runtime_error(dlerror());
    }
    return lib;
}
BuildFunc GetBuildFunction(LibraryHandle libraryHandle)
{
    auto buildFunc =(BuildFunc) dlsym(libraryHandle, "libxsmm_build_target");
    if (!buildFunc) {
       dlclose(libraryHandle);
       fprintf(stderr, "Failed in dlsym, Error: %s\n", dlerror());
       throw std::runtime_error(dlerror());
    }
    return buildFunc;
}
#endif


#endif //LIBXSMM_LIB_UTILS_H
