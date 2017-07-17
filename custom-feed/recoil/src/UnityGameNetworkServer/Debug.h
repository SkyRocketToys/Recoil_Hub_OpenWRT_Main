#include <iostream>
#include <string>

#ifdef DEBUG_BUILD

//TODO, fix this
#define DEBUG_LOG(x) (std::wcout << (x) << std::endl)

#define DEBUG_LOGWARNING(x) (std::wcout << (x) << std::endl)

#define DEBUG_LOGERROR(x) (std::wcerr << (x) << std::endl)


//#define DEBUG_LOG(x) (std::cout << (x))
//
//#define DEBUG_LOGWARNING(x) (std::cout << "Warning: " << (x))
//
//#define DEBUG_LOGERROR(x) (std::cerr << (x))

#else

#define DEBUG_LOG(x) 

#define DEBUG_LOGWARNING(x)

#define DEBUG_LOGERROR(x)

#endif