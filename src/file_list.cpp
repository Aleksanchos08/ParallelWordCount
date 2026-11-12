#include "file_list.h"
#include <cstring>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace wc {

static std::string trim(const std::string& s) {
    const std::size_t b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    const std::size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

bool loadFileList(const std::string& masterPath, FileList& out, std::string& error) {
    out = FileList();
    std::ifstream in(masterPath);
    if (!in) { error = "cannot open master list: " + masterPath; return false; }

    const fs::path base = fs::path(masterPath).parent_path();
    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        fs::path p(line);
        if (p.is_relative()) p = base / p;

        std::error_code ec;
        const auto size = fs::file_size(p, ec);
        if (ec) { error = "cannot stat " + p.string() + ": " + ec.message(); return false; }

        FileInfo f;
        f.path = p.string();
        f.size = static_cast<std::uint64_t>(size);
        out.totalSize += f.size;
        out.files.push_back(std::move(f));
    }
    if (out.files.empty()) { error = "master list is empty: " + masterPath; return false; }
    return true;
}


std::vector<char> packFileList(const FileList& list) {
    std::vector<char> buf;
    for (const FileInfo& f : list.files) {
        const char* s = reinterpret_cast<const char*>(&f.size);
        buf.insert(buf.end(), s, s + sizeof(f.size));
        buf.insert(buf.end(), f.path.begin(), f.path.end());
        buf.push_back('\0');
    }
    return buf;
}

FileList unpackFileList(const char* data, std::size_t size) {
    FileList list;
    std::size_t i = 0;
    while (i < size) {
        FileInfo f;
        std::memcpy(&f.size, data + i, sizeof(f.size));
        i += sizeof(f.size);
        const std::size_t len = std::strlen(data + i);
        f.path.assign(data + i, len);
        i += len + 1;
        list.totalSize += f.size;
        list.files.push_back(std::move(f));
    }
    return list;
}

}
