#ifndef TINYDIRCPP_H
#define TINYDIRCPP_H

#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

namespace tinydir
{
    struct e_noent       : virtual std::runtime_error { e_noent() : std::runtime_error("No such file or directory.") {} };
    struct e_inval       : virtual std::runtime_error { e_inval() : std::runtime_error("Invalid argument.") {} };
    struct e_nametoolong : virtual std::runtime_error { e_nametoolong() : std::runtime_error("Filename too long.") {} };
    struct e_io          : virtual std::runtime_error { e_io()    : std::runtime_error("Input/output error.") {} };

    struct file
    {
        friend bool operator< (const file &a, const file &b);
        friend bool operator==(const file &a, const file& b);

        std::string path;
        std::string name;
        bool is_dir;
        bool is_reg;
    };

    struct directory
    {
        std::string path;

        bool has_next; // TODO FIXME
        std::vector<file> files;

        struct NativeImpl;
        std::unique_ptr<NativeImpl> _pimpl;

        directory(const std::string &fileName, bool sorted = false);
        directory(directory&&);
        ~directory();
      private:
        void read_remaining();
        void close();
        void next();
        file readfile() const;
        directory open_subdir_n(size_t i);
    };
}
#endif
