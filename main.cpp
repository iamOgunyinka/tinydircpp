#include <iostream>
#include "filehandler.h"

bool report_filename(std::string const& i)
{
    std::cout << i << std::endl;
    return true;
}

int main()
{
    using namespace tinydir;

    FileHandler a(".");

	if(a) a.breadth_first_traverse(report_filename, WithExtension(".cpp"));
	else  std::cout << "Unable to open directory/file" << std::endl;
	
	std::cout << "Changing directory" << std::endl;
	
	a = FileHandler("/tmp");
	if (a) a.visit_files(report_filename); //get filenames ONLY within the given directory
	else   std::cout << "Unable to open directory/file" << std::endl;
}
