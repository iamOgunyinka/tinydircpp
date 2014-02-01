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
	std::vector<std::string> getExtension(const std::string &);
	bool isDirectory(const std::string &);
	bool is_open();
	void setFilename(const std::string &);
	std::vector<std::string>::iterator begin();
	std::vector<std::string>::iterator end();
	operator bool() const;
	
	bool isFile(const std::string &);
private:
    bool okay;
    std::vector<std::string> allFiles;
    std::stack<std::string> directories;
	std::string filename;
	char stringToChar(const std::string &);
};

#endif
