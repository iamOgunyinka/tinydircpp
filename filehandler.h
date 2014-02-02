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

class Folder
{
public:
	Folder(const std::string &filepath);
	Folder& operator=(const Folder &) = delete;
	Folder(Folder &&);
	Folder(const Folder &) = delete;
	~Folder() { }

	typedef std::vector<std::string>::size_type size_type;
	void recurseDownDirectory();
	void getSingle(); //looking for new API name, getSingle() does not describe what this function does
	void open(const std::string &path, std::vector<std::string> &fileContainer, std::stack<std::string> &stackDirectory);
	std::vector<std::string> getFiles();
	size_type getNumberOfFiles();
	std::vector<std::string> getExtension(const std::string &extension);
	bool isDirectory(const std::string &path_to_directory);
	bool is_open();
	void setFilename(const std::string &new_dirpath);
	std::vector<std::string>::iterator begin();
	std::vector<std::string>::iterator end();
	operator bool() const;
	
	bool isFile(const std::string &filepath);
private:
    bool okay;
    std::vector<std::string> allFiles;
    std::stack<std::string> directories;
    std::string filename;
    void removeSlashFrom(std::string &filepath);
    char stringToChar(const std::string &);
};

#endif
