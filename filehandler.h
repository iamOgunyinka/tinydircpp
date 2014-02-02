#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "tinydircpp.h"
#include <functional>

namespace tinydir
{

    struct WithExtension
    {
        WithExtension(std::string const& ext) : ext(ext) {
            if(this->ext[0] != '.')
                this->ext.insert(this->ext.begin(), '.');
        }
        bool operator()(std::string const& filename) const;
    private:
        std::string ext;
    };

    class FileHandler
    {
        struct TruePredicate {
            bool operator()(std::string const&) const { return true; }
        };
    public:
        using Handler = std::function<bool(std::string const& filename)>;

        FileHandler(const std::string &);
        FileHandler& operator=(const FileHandler &) = default;
        FileHandler(const FileHandler &) = default;
        ~FileHandler() { }

        bool breadth_first_traverse(Handler handler, Handler filter = TruePredicate(), bool recurse = true);

        // visit_files is equivalent to calling breadth_first_traverse with recurse = false
        bool visit_files           (Handler handler, Handler filter = TruePredicate());

        explicit operator bool() const;
        
    private:
        bool is_open();

        bool okay;
        std::string filename;
    };

}
#endif
