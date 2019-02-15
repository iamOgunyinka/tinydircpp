README
####

It was my desired goal to follow N3505 closely in this implementation but as time went by, I have decided to digress 
a bit and now implement Python's `os.path` module and its counterparts.

tinydircpp::filesystem

class fs::path::
    abspath(...)
    basename(...)
    common_path(...)
    common_prefix(...)
    directory_name(...)
    exists(...)
    lexists(...)
    expand_user(...)
    expand_vars(...)
    get_access_time()
    get_creation_time()
    get_writing_time()
    get_size()
    is_abs()
    is_file()
    is_dir()
    is_link()
    is_fifo()
    is_mount()
    join(path, paths...)
    normcase()
    normpath()
    realpath()
    relpath()
    same_file()
    same_open_file()
    same_stat()
    split()
    split_drive()
    split_ext()
    split_unc()
    supports_unicode_filenames()

namespace fs::
    access(...) // test to see if the invoking user has the specified access to the path
    chdir()
    chmod()
    chflags()
    chown()
    chroot()
    fchdir()
    getcwd()
    lchflags()
    lchmod()
    lchown()
    link()
    listdir()
    lstat()
    mkdir()
    makedirs()
    make_fifo()
    make_nod()
    major()
    minor()
    makedev()
    pathconf()
    pathconf_names()
    readlink()
    remove()
    remove_dirs()
    rename()
    renames()
    replace()
    rmdir()
    scandir() ==> yields a DirectoryEntry with the following
    class DirectoryEntry::
        name()
        path()
        inode()
        is_dir()
        is_file()
        is_symlink()
        stat()
    stat()
    class stat_result::
        st_mode
        st_ino
        st_dev
        st_nlink
        st_uid
        st_gid
        st_size
        st_atime
        st_mtime
        st_ctime
        st_atime_ns
        st_mtime_ns
        st_ctime_ns
        st_blocks
        st_blksize 
        st_rdev 
        st_flags 
        st_gen 
        st_birthtime 
        st_rsize 
        st_creator 
        st_type 
        st_file_attributes 
    symlink()
    sync()
    truncate()
    unlink()
    utime()
    walk()
