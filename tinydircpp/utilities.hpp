#pragma once
#include <stdexcept>

#ifndef PRIVATE_IMPL
#define PRIVATE_IMPL
#endif // !PRIVATE_IMPL

namespace tinydircpp
{
    namespace fs {
        class path;
        namespace details PRIVATE_IMPL {
            struct invalid_filename : public std::runtime_error {
                invalid_filename( char const * error_message ) : std::runtime_error{ error_message }{}
            };
            struct name_too_long : public std::runtime_error {
                name_too_long( char const * error_message ) : std::runtime_error{ error_message }{}
            };
        }
        class filesystem_error : public std::system_error {
        public:
            filesystem_error( std::string const & what, std::error_code ec ) : filesystem_error{ what, {}, ec }{}
            filesystem_error( std::string const & what, path const & p, std::error_code ec ) :
                filesystem_error{ what, p, {}, ec } {}
            filesystem_error( std::string const & what, path const & p1, path const & p2, std::error_code ec ):
                std::system_error{ ec, what }, first_path{ p1 }, second_path{ p2 }{}

            path const & path1() const { return first_path;  }
            path const & path2() const { return second_path; }
            using std::system_error::what;

        private:
            path const first_path{};
            path const second_path{};
        };
    }

    enum class file_type : int {
        none = 0,
        not_found = -1,
        regular = 1,
        directory,
        symlink,
        block,
        character,
        fifo,
        socket,
        unknown
    };

    enum class copy_options : int {
        none = 0,
        skip_existing = 0x1,
        overwrite_existing = 0x2,
        update_existing = 0x4,
        recursive = 0x8,
        copy_symlinks = 0x10,
        skip_symlinks = 0x20,
        directories_only = 0x40,
        create_symlinks = 0x80,
        create_hardlinks = 0x100
    };

    enum class perms : int {
        none = 0,
        owner_read = 256,
        owner_write = 128,
        owner_exec = 64,
        owner_all = owner_read | owner_write | owner_exec,
        group_read = 32,
        group_write = 16,
        group_exec = 8,
        group_all = group_read | group_write | group_exec,
        others_read = 4,
        others_write = 2,
        others_exec = 1,
        others_all = others_read | others_write | others_exec,
        all = 511,
        set_uid = 2048,
        set_gid = 1024,
        mask = 512,
        unknown = 0xFFFF,
        symlink_perms = 0x4000
    };

    enum class permissions_status : int {
        replace_bits = 0,
        add_bits = 1,
        remove_bits = 2,
        follow_symlinks = 4
    };
}