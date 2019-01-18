#include <iostream>
#include "filehandler.h"

int main()
{

    Folder a("/path/to/folder");
	std::vector<std::string> vec;
	
	if(a.is_open()){ //explicitly requesting if is "openable"
		a.recurseDownDirectory(); //get all Filenames down the "tree"
		vec = a.getExtension("cc");  //automatically prefix with a '.', return vector of strings
		for(const auto &i: vec){
			std::cout << i << std::endl;
		}
	} else {
		std::cout << "Unable to open directory/file" << std::endl;
	}

	std::cout << "Changing directory to \"C:\\New\\FilePath\"" << std::endl;
	
	a.setFilename("C:\\WindowsPathName\\");
	if(a){
		a.getSingle(); //get filenames ONLY within the given directory
		for(const auto &i: a){ //Folder objects are iterable
			std::cout << i << std::endl;
		}
	} else {
		std::cout << "Unable to open directory/file" << std::endl;
	}
	return 0;
}
