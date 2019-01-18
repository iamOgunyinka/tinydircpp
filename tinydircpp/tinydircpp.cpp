// tinydircpp.cpp : Defines the exported functions for the static library.
//

#include "tinydircpp.h"

// This is an example of an exported variable
int ntinydircpp=0;

// This is an example of an exported function.
int fntinydircpp(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see tinydircpp.h for the class definition
Ctinydircpp::Ctinydircpp()
{
    return;
}
