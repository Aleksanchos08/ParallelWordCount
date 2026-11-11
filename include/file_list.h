// file_list.h - master file list ("which files to count") and its MPI-friendly packing
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace wc {

struct FileInfo {
    std::string   path;
    std::uint64_t size = 0;  // bytes
};

struct FileList {
    std::vector<FileInfo> files;
    std::uint64_t         totalSize = 0;  // sum of all file sizes
};

// Reads the master list (one path per line, '#' = comment; relative paths are resolved
// against the directory of the list) and stats every file. Only rank 0 calls this.
bool loadFileList(const std::string& masterPath, FileList& out, std::string& error);

// Flat byte representation, used for MPI_Bcast.
std::vector<char> packFileList(const FileList& list);
FileList          unpackFileList(const char* data, std::size_t size);

}  // namespace wc
