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



bool loadFileList(const std::string& masterPath, FileList& out, std::string& error);


std::vector<char> packFileList(const FileList& list);
FileList          unpackFileList(const char* data, std::size_t size);

}
