#include "filehandler.h"

FileHandler::FileHandler(const std::string &_filename): okay(false), filename(_filename)
{
	while(filename[filename.size()-1] == stringToChar(SLASH)){
		filename.erase((filename.rbegin() + 1).base(), filename.end());
	}
	if(is_open()){
		okay = false;
	}
}

bool FileHandler::is_open()
{
	tinydir_dir dir;
	int isOpen = tinydir_open(&dir, filename);
	if(isOpen !=0){
		tinydir_close(&dir);
		return false;
	}
	tinydir_close(&dir);
	return true;
}

void FileHandler::setFilename(const std::string &new_file)
{
	filename = new_file;
	okay = false;
	allFiles.clear();
	while(!directories.empty()){
		directories.pop();
	}
	if(is_open()) okay = true;
}

void FileHandler::recurseAll(){
	do
	{
		std::string newpath { filename };
		if(!directories.empty()){
			newpath = directories.top();
			directories.pop();
		}
		open(newpath, allFiles, directories);
	} while(!directories.empty());
}

inline void FileHandler::getSingle(){
	open(filename, allFiles, directories);
}

void FileHandler::open(const std::string &source, std::vector<std::string> &vec, std::stack<std::string> &stack)
{
	tinydir_dir dir;
	std::string stack_string { };
	tinydir_open_sorted(&dir, source.c_str());
	for (unsigned int i = 0; i < dir.n_files; ++i)
	{
		tinydir_file file;
		tinydir_readfile_n(&dir, &file, i);

		if (file.is_dir)
		{
			stack_string = source + std::string(SLASH) + file.name;
			if(!(stack_string[stack_string.size() - 1] == '.')){
				stack.push(stack_string);
			}
		} else {
			auto found = stack_string.find("..");
			if(found != std::string::npos){
					stack_string.erase(found, 2);
			}
			vec.push_back(source + std::string(SLASH) + file.name);
		}
	}
	tinydir_close(&dir);
}

inline std::vector<std::string> FileHandler::getFiles(){
	return allFiles;
}

inline FileHandler::size_type FileHandler::getFileNumbers(){
	return allFiles.size();
}

inline char FileHandler::stringToChar(const std::string &s){
	char c = s[0];
	return c;
}

std::vector<std::string> FileHandler::getExtension(const std::string &_ext)
{
	std::string ext { _ext};
	if(ext[0] != '.'){
		ext.insert(ext.begin(), '.');
	}
	std::vector<std::string> temp(allFiles.cbegin(), allFiles.cend());
	auto partition = std::stable_partition(temp.begin(), temp.end(),
			[=](std::string s)->bool{ return s.find(ext) != std::string::npos; } );
	temp.erase(partition, temp.end());
	return temp;
}

bool FileHandler::isDirectory(const std::string &fileName)
{
	tinydir_dir dir;
	tinydir_open(&dir, fileName.c_str());
	tinydir_file file { };
	tinydir_readfile(&dir, &file);

	if (file.is_dir){
		tinydir_close(&dir);
		return true;
	}
	tinydir_close(&dir);
	return false;
}

std::vector<std::string>::iterator FileHandler::begin() {
	return allFiles.begin();
}

std::vector<std::string>::iterator FileHandler::end() {
	return allFiles.end();
}

FileHandler::operator bool() const{
	return okay;
}
