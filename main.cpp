#include <iostream>
#include "filehandler.h"

int main()
{

    FileHandler a("/path/to/directory/");
	std::vector<std::string> vec;
	
	if(a.is_open()){
		a.recurseAll(); //get all Filenames down the "tree"
		vec = a.getExtension("cpp");  //automatically add a '.' before cpp
		for(const auto &i: vec){
			std::cout << i << std::endl;
		}
	} else {
		std::cout << "Unable to open directory/file" << std::endl;
		//may return EXIT_FAILURE or choose to continue
	}
	
	a.setFilename("\FilePath\for\Windows\");
	if(a.is_open()){
		a.getSingle(); //get filenames ONLY within the given directory
		vec = a.getFiles(); //all files "recovered"
		for(const auto &i: vec){
			std::cout << i << std::endl;
		}
	} else {
		std::cout << "Unable to open directory/file" << std::endl;
	}
	return 0;
}
