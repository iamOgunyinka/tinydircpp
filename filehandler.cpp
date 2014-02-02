#include "filehandler.h"
#include <algorithm>
#include <stack>

#ifdef _WIN32
#define SLASH '\\'
#else
#define SLASH '/'
#endif

namespace tinydir
{
    namespace /* file static */
    {
        bool bfs_impl(const std::string &source, FileHandler::Handler handler, std::stack<std::string>& dirStack)
        {
            tinydir::directory dir(source, true);

            for (auto const& file: dir.files)
            {
                if (file.name == "." || file.name == "..")
                    continue;
                if (file.is_dir)
                    dirStack.push(source + SLASH + file.name);
                else
                {
                    if (!handler(source + SLASH + file.name)) // the handler can cancel traversal
                        return false;
                }
            }

            return true;
        }

    }

    FileHandler::FileHandler(const std::string &_filename): okay(false), filename(_filename)
    {
        if (!filename.empty())
        {
            auto slashpos = filename.find_last_not_of(SLASH);
            if (std::string::npos != slashpos)
                filename.erase(slashpos + 1);
        }

        okay = is_open();
    }

    bool FileHandler::is_open()
    {
        // TODO fixme: do accessibility check without exceptions for control flow
        try { return tinydir::directory(filename).has_next; } 
        catch(...) { return false; }
    }

    bool FileHandler::breadth_first_traverse(Handler handler, Handler filter, bool recurse){
    
        std::stack<std::string> directories;
        do
        {
            std::string newpath = filename;

            if(!directories.empty()){
                newpath = directories.top();
                directories.pop();
            }

            if (!bfs_impl(newpath, 
                    [&](std::string const& filename) { return !filter(filename) || handler(filename); }, 
                    directories))
            {
                return false;
            }
        } while(recurse && !directories.empty());

        return true;
    }

    bool FileHandler::visit_files(Handler handler, Handler filter){
        return breadth_first_traverse(handler, filter, false);
    }

    bool WithExtension::operator()(std::string const& filename) const
    {
        return filename.size()>=ext.size() 
            && filename.find(ext) == filename.size()-ext.size();
    }

    FileHandler::operator bool() const
    { 
        return okay; 
    }
}
