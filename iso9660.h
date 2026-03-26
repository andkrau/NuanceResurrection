// Minimal ISO9660 reader — reads files from ISO image without mounting
// Supports reading from a file within a FUSE-mounted ZIP
#ifndef ISO9660_H
#define ISO9660_H

#include <cstdio>

// Large file support for 32-bit builds
#if defined(__i386__) || defined(_M_IX86)
#define ISO_FOPEN fopen64
#define ISO_FSEEK(f, off, w) fseeko64(f, (off64_t)(off), w)
#else
#define ISO_FOPEN fopen
#define ISO_FSEEK(f, off, w) fseek(f, (long)(off), w)
#endif
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>

struct ISO9660Entry {
    std::string name;
    uint32_t lba;      // logical block address
    uint32_t size;
    bool isDir;
};

class ISO9660Reader {
    FILE* fp;
    uint32_t rootLBA;
    uint32_t rootSize;

    static uint32_t read32le(const uint8_t* p) { return p[0]|(p[1]<<8)|(p[2]<<16)|(p[3]<<24); }

    std::vector<ISO9660Entry> readDir(uint32_t lba, uint32_t size) {
        std::vector<ISO9660Entry> entries;
        std::vector<uint8_t> buf(size);
        ISO_FSEEK(fp, (long long)lba * 2048, SEEK_SET);
        fread(buf.data(), 1, size, fp);

        uint32_t pos = 0;
        while (pos < size) {
            uint8_t recLen = buf[pos];
            if (recLen == 0) { pos = ((pos / 2048) + 1) * 2048; continue; }
            if (pos + recLen > size) break;

            uint8_t nameLen = buf[pos + 32];
            uint32_t eLBA = read32le(&buf[pos + 2]);
            uint32_t eSize = read32le(&buf[pos + 10]);
            bool isDir = (buf[pos + 25] & 0x02) != 0;

            char name[256] = {};
            memcpy(name, &buf[pos + 33], nameLen);

            // Skip . and ..
            if (nameLen == 1 && (name[0] == 0 || name[0] == 1)) { pos += recLen; continue; }

            // Remove ;1 version suffix
            char* semi = strchr(name, ';');
            if (semi) *semi = '\0';
            // Remove trailing dot only if no extension (e.g. "DIRNAME.")
            size_t nlen = strlen(name);
            if (nlen > 0 && name[nlen-1] == '.' && !strchr(name, '.')) name[nlen-1] = '\0';

            fprintf(stderr, "ISO9660 readDir: entry '%s' (len=%u, bytes=%02x%02x%02x%02x) lba=%u size=%u dir=%d\n",
                name, nameLen, (uint8_t)name[0], nameLen>1?(uint8_t)name[1]:0, nameLen>2?(uint8_t)name[2]:0, nameLen>3?(uint8_t)name[3]:0,
                eLBA, eSize, isDir);
            entries.push_back({name, eLBA, eSize, isDir});
            pos += recLen;
        }
        return entries;
    }

public:
    ISO9660Reader() : fp(nullptr), rootLBA(0), rootSize(0) {}
    ~ISO9660Reader() { close(); }

    bool open(const char* isoPath) {
        fp = ISO_FOPEN(isoPath, "rb");
        if (!fp) { fprintf(stderr, "ISO9660: cannot open %s (errno=%d)\n", isoPath, errno); return false; }

        // Read primary volume descriptor at sector 16
        uint8_t pvd[2048];
        ISO_FSEEK(fp, 16 * 2048, SEEK_SET);
        if (fread(pvd, 1, 2048, fp) != 2048) { fprintf(stderr, "ISO9660: cannot read PVD\n"); close(); return false; }
        if (pvd[0] != 1 || memcmp(&pvd[1], "CD001", 5) != 0) { fprintf(stderr, "ISO9660: invalid PVD signature\n"); close(); return false; }

        // Root directory record at offset 156
        rootLBA = read32le(&pvd[156 + 2]);
        rootSize = read32le(&pvd[156 + 10]);
        fprintf(stderr, "ISO9660: opened, root LBA=%u size=%u\n", rootLBA, rootSize);
        return true;
    }

    void close() { if (fp) { fclose(fp); fp = nullptr; } }

    // Find a file by path (case-insensitive), return true and fill lba/size
    bool findFile(const char* path, uint32_t& outLBA, uint32_t& outSize) {
        if (!fp) return false;

        uint32_t curLBA = rootLBA, curSize = rootSize;
        std::string spath(path);

        // Split path by /
        size_t start = 0;
        while (start < spath.size()) {
            if (spath[start] == '/') { start++; continue; }
            size_t end = spath.find('/', start);
            if (end == std::string::npos) end = spath.size();
            std::string component = spath.substr(start, end - start);

            auto entries = readDir(curLBA, curSize);
            fprintf(stderr, "ISO9660: findFile looking for '%s' in dir (LBA=%u, %zu entries)\n",
                component.c_str(), curLBA, entries.size());
            bool found = false;
            for (auto& e : entries) {
                int cmpResult = strcasecmp(e.name.c_str(), component.c_str());
                fprintf(stderr, "ISO9660:   cmp '%s' vs '%s' = %d\n", e.name.c_str(), component.c_str(), cmpResult);
                if (cmpResult == 0) {
                    if (end >= spath.size()) {
                        // Last component — this is the file
                        outLBA = e.lba;
                        outSize = e.size;
                        return true;
                    }
                    if (!e.isDir) return false;
                    curLBA = e.lba;
                    curSize = e.size;
                    found = true;
                    break;
                }
            }
            if (!found) return false;
            start = end;
        }
        return false;
    }

    // Extract a file to disk
    bool extractFile(const char* isoPath, const char* destPath) {
        fprintf(stderr, "ISO9660: extractFile '%s' -> '%s'\n", isoPath, destPath); fflush(stderr);
        uint32_t lba, size;
        if (!findFile(isoPath, lba, size)) { fprintf(stderr, "ISO9660: file not found\n"); return false; }
        fprintf(stderr, "ISO9660: found at LBA=%u size=%u, writing...\n", lba, size); fflush(stderr);

        FILE* out = fopen(destPath, "wb");
        if (!out) { fprintf(stderr, "ISO9660: cannot create %s\n", destPath); return false; }

        ISO_FSEEK(fp, (long long)lba * 2048, SEEK_SET);
        uint8_t buf[65536];
        uint32_t remaining = size;
        while (remaining > 0) {
            uint32_t chunk = remaining < sizeof(buf) ? remaining : sizeof(buf);
            size_t rd = fread(buf, 1, chunk, fp);
            if (rd == 0) break;
            fwrite(buf, 1, rd, out);
            remaining -= rd;
        }
        fclose(out);
        return remaining == 0;
    }

    // List root or given directory
    std::vector<ISO9660Entry> listDir(const char* dirPath = nullptr) {
        if (!fp) return {};
        if (!dirPath || dirPath[0] == '\0' || strcmp(dirPath, "/") == 0)
            return readDir(rootLBA, rootSize);

        uint32_t lba, size;
        // Navigate to directory
        uint32_t curLBA = rootLBA, curSize = rootSize;
        std::string spath(dirPath);
        size_t start = 0;
        while (start < spath.size()) {
            if (spath[start] == '/') { start++; continue; }
            size_t end = spath.find('/', start);
            if (end == std::string::npos) end = spath.size();
            std::string comp = spath.substr(start, end - start);
            auto entries = readDir(curLBA, curSize);
            bool found = false;
            for (auto& e : entries) {
                if (strcasecmp(e.name.c_str(), comp.c_str()) == 0 && e.isDir) {
                    curLBA = e.lba; curSize = e.size; found = true; break;
                }
            }
            if (!found) return {};
            start = end;
        }
        return readDir(curLBA, curSize);
    }
};

#endif
