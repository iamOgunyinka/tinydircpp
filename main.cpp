#include <iostream>
#include "filehandler.h"

int main()
{
    FileHandler a(".");
	std::vector<std::string> vec;
	
	if(a.is_open()){ //explicitly requesting if it is "openable"
		a.recurseAll(); //get all Filenames down the "tree"
		vec = a.getExtension("cpp");  //automatically prefix with a '.', return std::vector of strings
		for(const auto &i: vec){
			std::cout << i << std::endl;
		}
	} else {
		std::cout << "Unable to open directory/file" << std::endl;
		//You may choose to EXIT_FAILURE
	}
	
	std::cout << "Changing directory" << std::endl;
	
	a.setFilename("/tmp");
	if(a){
		a.getSingle(); //get filenames ONLY within the given directory
		for(const auto &i: a){ //FileHandler objects are iterable
			std::cout << i << std::endl;
		}
	} else {
		std::cout << "Unable to open directory/file" << std::endl;
	}
	return 0;
}
