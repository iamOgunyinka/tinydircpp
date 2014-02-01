#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "tinydircpp.h"
#include <stack>
#include <vector>

#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

class FileHandler
{
public:
	FileHandler(const std::string &);
	FileHandler& operator=(const FileHandler &) = delete;
	FileHandler(const FileHandler &) = delete;
	~FileHandler() { }

	typedef std::vector<std::string>::size_type size_type;
	void recurseAll();
	void getSingle();
	void open(const std::string &, std::vector<std::string> &, std::stack<std::string> &);
	std::vector<std::string> getFiles();
	size_type getFileNumbers();
	std::vector<std::string> getExtension(std::string );
	bool isDirectory(std::string);
	bool is_open();
	void setFilename(std::string);
	
	bool isFile(std::string fileName);
private:
    std::vector<std::string> allFiles;
    std::stack<std::string> directories;
	std::string filename;
	char stringToChar(std::string);
};

#endif
