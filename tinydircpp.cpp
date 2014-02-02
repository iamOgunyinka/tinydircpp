#include "tinydircpp.h"

#include <tuple>
#include <algorithm>

//According to the msdn documentation, _WIN32 is defined for both 32- and 64bit machines
#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma warning (disable : 4996)
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

namespace tinydir
{
    namespace {
        constexpr unsigned int TINYDIR_PATH_MAX     = 4096;
#ifdef _WIN32
        constexpr unsigned int TINYDIR_PATH_EXTRA   = 2;
#else
        constexpr unsigned int TINYDIR_PATH_EXTRA   = 0;
#endif
        constexpr unsigned int TINYDIR_FILENAME_MAX = 256;
    }

    bool operator< (const file &a, const file &b)
    {
        return std::tie(a.path, a.name) < std::tie(b.path, b.name);
    }
    bool operator==(const file &a, const file& b)
    {
        return std::tie(a.path, a.name) == std::tie(b.path, b.name);
    }

#ifdef _WIN32
    struct directory::NativeImpl
    {
        HANDLE h = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA f = { 0 };
    };
#else
    struct directory::NativeImpl
    {
        DIR *d = nullptr;
        struct dirent *e = nullptr;
    };
#endif

    directory::directory(const std::string &fileName, bool sorted) 
        : _pimpl(new NativeImpl)
    {
        if (fileName.empty() || fileName.find('\0') != std::string::npos)
            throw e_inval();
        if ((fileName.size() + TINYDIR_PATH_EXTRA) >= TINYDIR_PATH_MAX)
            throw e_nametoolong();

        path = fileName;

#ifdef _WIN32
        path += std::string("\\*");
        _pimpl->h = FindFirstFile(path.c_str(), &f);
        path.resize(path.size() - 2);
        if (_pimpl->h == INVALID_HANDLE_VALUE)
#else
        _pimpl->d = opendir(fileName.c_str());
        if (_pimpl->d == nullptr)
#endif
        {
            throw e_noent();
        }

        /* read first file */
#ifdef _WIN32
        has_next = true;
#else
        _pimpl->e = readdir(_pimpl->d);
        has_next = !!_pimpl->e;
#endif
        if (sorted)
        {
            read_remaining();
            std::sort(files.begin(), files.end());
        }
    }

    directory::directory(directory&& d) 
       : path(std::move(d.path)),
         has_next(d.has_next),
         files(std::move(d.files)),
         _pimpl(std::move(d._pimpl))
    {
    }

    directory::~directory() { 
        close();
    }

    void directory::read_remaining()
    {
        for (; has_next; next())
            files.push_back(readfile());
    }

    void directory::close()
    {
        path = "";
        has_next = false;
        if (!files.empty())
        {
            files.clear();
        }
#ifdef _WIN32
        if (_pimpl->h != INVALID_HANDLE_VALUE)
        {
            FindClose(_pimpl->h);
        }
        _pimpl->h = INVALID_HANDLE_VALUE;
#else
        if (_pimpl->d)
        {
            closedir(_pimpl->d);
        }
        _pimpl->d = nullptr;
        _pimpl->e = nullptr;
#endif
    }

    void directory::next()
    {
        if (!has_next)
        {
            throw e_noent();
        }

#ifdef _WIN32
        if (FindNextFile(_pimpl->h, &f) == 0)
#else
        _pimpl->e = readdir(_pimpl->d);
        if (_pimpl->e == nullptr)
#endif
        {
            has_next = false;
#ifdef _WIN32
            if (GetLastError() != ERROR_SUCCESS &&
                GetLastError() != ERROR_NO_MORE_FILES)
            {
                close();
                throw e_io();
            }
#endif
        }
    }

    file directory::readfile() const
    {
#ifdef _WIN32
        if (_pimpl->h == INVALID_HANDLE_VALUE)
#else
        if (_pimpl->e == nullptr)
#endif
        {
            throw e_noent();
        }
        if (path.size() +
            std::string(
#ifdef _WIN32
                f.cFileName
#else
                _pimpl->e->d_name
#endif
            ).size() + 1 + TINYDIR_PATH_EXTRA >= TINYDIR_PATH_MAX)
        {
            /* the path for the file will be too long */
            throw e_nametoolong();
        }
        if (std::string(
#ifdef _WIN32
                f.cFileName
#else
                _pimpl->e->d_name
#endif
            ).size() >= TINYDIR_FILENAME_MAX)
        {
            throw e_nametoolong();
        }

        file result;
        result.path = path;
        result.path += std::string(SLASH);

        result.name = std::string(
#ifdef _WIN32
            f.cFileName
#else
            _pimpl->e->d_name
#endif
        );
        result.path += result.name;
#ifndef _WIN32
        struct stat s;
        if (stat((result.path).c_str(), &s) == -1)
            throw e_noent();
#endif
        result.is_dir =
#ifdef _WIN32
            !!(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
            S_ISDIR(s.st_mode);
#endif
        result.is_reg =
#ifdef _WIN32
            !!(f.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ||
            (
                !(f.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) &&
                !(f.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                !(f.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
#ifdef FILE_ATTRIBUTE_INTEGRITY_STREAM
                !(f.dwFileAttributes & FILE_ATTRIBUTE_INTEGRITY_STREAM) &&
#endif
#ifdef FILE_ATTRIBUTE_NO_SCRUB_DATA
                !(f.dwFileAttributes & FILE_ATTRIBUTE_NO_SCRUB_DATA) &&
#endif
                !(f.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) &&
                !(f.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY));
#else
            S_ISREG(s.st_mode);
#endif

        return result;
    }

    directory directory::open_subdir_n(size_t i)
    {
        return directory(files.at(i).path, true); // TODO FIXME why sorted?
    }
}
