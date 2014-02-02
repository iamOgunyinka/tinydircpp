#include "filehandler.h"

Folder::Folder(const std::string &_filename): okay{false}, filename{_filename}
{
	removeSlashFrom(filename);
	if(is_open()){
		okay = true;
	}
}

bool Folder::is_open()
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

void Folder::setFilename(const std::string &new_file)
{
	filename = new_file;
	removeSlashFrom(filename);
	okay = false;
	allFiles.clear();
	while(!directories.empty()){
		directories.pop();
	}
	if(is_open()) okay = true;
}

void Folder::recurseDownDirectory(){
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

inline void Folder::getSingle(){
	open(filename, allFiles, directories);
}

void Folder::open(const std::string &source, std::vector<std::string> &vec, std::stack<std::string> &stack)
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

inline std::vector<std::string> Folder::getFiles(){
	return allFiles;
}

inline Folder::size_type Folder::getNumberOfFiles(){
	return allFiles.size();
}

inline char Folder::stringToChar(const std::string &s){
	char c = s[0];
	return c;
}

std::vector<std::string> Folder::getExtension(const std::string &_ext)
{
	std::string ext { _ext};
	if(ext[0] != '.'){
		ext.insert(ext.begin(), '.');
	}
	std::vector<std::string> temp(allFiles.cbegin(), allFiles.cend());
	std::remove_if(temp.begin(), temp.end(), 
	        [=](const std::string &s)->bool{ return s.find(ext) != std::string::npos; } );
	return temp;
}

bool Folder::isDirectory(const std::string &fileName)
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

std::vector<std::string>::iterator Folder::begin() {
	return allFiles.begin();
}

std::vector<std::string>::iterator Folder::end() {
	return allFiles.end();
}

Folder::operator bool() const{
	return okay;
}

void Folder::removeSlashFrom(std::string &filepath)
{
	if(filepath.size() > 0){
		while(filepath[filepath.size()-1] == stringToChar(SLASH)){
			filepath.erase((filepath.rbegin() + 1).base(), filepath.end());
		}
	}
}
