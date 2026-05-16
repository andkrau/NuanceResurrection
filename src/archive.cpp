#include "archive.h"
#include "iso9660.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <string>

#ifdef _WIN32
  #include <windows.h>
  #include <shellapi.h>
  #include "miniz.h"
  #define PATH_SEP '\\'
  #define strcasecmp _stricmp
#else
  #include <sys/stat.h>
  #include <unistd.h>
  #include <strings.h>
  #define PATH_SEP '/'
#endif

// Defined in NuanceMain(_linux).cpp
// MediaRead() reads runtime data files directly from the ISO via this
extern std::string g_ISOPath;
extern std::string g_ISOPrefix;

namespace {

// Boot file names we look for (case-insensitive)
// The in-archive directory prefix (NUON/, nuon/, Nuon/) is handled by callers
static const char* const kBootFileNames[] = { "nuon.run", "NUON.CD", "cd_app.cof", nullptr };

// Inside the ISO, additionally try these full path candidates
static const char* const kIsoBootPaths[] = {
  "NUON/nuon.run", "NUON/NUON.CD", "NUON/cd_app.cof",
  "nuon/nuon.run", "nuon/NUON.CD", "nuon/cd_app.cof",
  "Nuon/nuon.run", "Nuon/NUON.CD", "Nuon/cd_app.cof",
  nullptr
};

static std::vector<std::string> g_tempPaths; // Temp paths created by us (files and dirs); cleaned up by CleanupArchives()

#ifndef _WIN32
std::vector<std::string> g_tempMounts; // Paths we mounted via FUSE (need fusermount -uz, not just rmdir)
#endif

bool HasExtensionICase(const std::string& s, const char* ext)
{
  const size_t slen = s.size();
  const size_t elen = strlen(ext);
  if (slen < elen) return false;
  return strcasecmp(s.c_str() + slen - elen, ext) == 0;
}

bool IsIsoPath(const std::string& s)
{
  return HasExtensionICase(s, ".iso") || HasExtensionICase(s, ".img");
}

bool IsZipPath(const std::string& s)
{
  return HasExtensionICase(s, ".zip");
}

// CHD detection probes the first 8 bytes for the "MComprHD" magic rather than
// trusting the .chd extension, so renamed files (or extensions like .gz/.bin
// that some tools default to) still resolve.
bool IsChdPath(const std::string& s)
{
  if (HasExtensionICase(s, ".chd")) return true;
  FILE* fp = fopen(s.c_str(), "rb");
  if (!fp) return false;
  char magic[8] = {};
  const size_t n = fread(magic, 1, 8, fp);
  fclose(fp);
  return n == 8 && memcmp(magic, "MComprHD", 8) == 0;
}

#ifndef _WIN32
bool IsOtherArchivePath(const std::string& s)
{
  static const char* exts[] = { ".7z", ".gz", ".rar", ".bz2", ".xz", nullptr };
  for (const char** e = exts; *e; e++)
    if (HasExtensionICase(s, *e)) return true;
  return false;
}
#endif

// Common helper: open an ISO, find a NUON boot file inside it, extract that
// boot file to <tempDir>/<subdir>/<bootname>, set g_ISOPath/g_ISOPrefix so
// MediaRead can pull data files from the ISO at runtime. Returns the path to
// the extracted boot file, or "" on failure.
std::string ExtractIsoBootAndArmDataReads(const char* isoPath, const std::string& tempDir)
{
  ISO9660Reader reader;
  if (!reader.open(isoPath))
    return "";

  uint32_t lba, fsize;
  const char* foundPath = nullptr;
  for (const char* const* f = kIsoBootPaths; *f; f++) {
    if (reader.findFile(*f, lba, fsize)) { foundPath = *f; break; }
  }

  if (!foundPath) {
    reader.close();
    return "";
  }

  std::string ff(foundPath);
  const std::string subdir   = ff.substr(0, ff.find('/'));
  const std::string bootName = ff.substr(ff.find('/') + 1);

  const std::string nuonDir = tempDir + PATH_SEP + subdir;
#ifdef _WIN32
  CreateDirectoryA(nuonDir.c_str(), NULL);
#else
  mkdir(nuonDir.c_str(), 0755);
#endif

  const std::string dstPath = nuonDir + PATH_SEP + bootName;
  const bool ok = reader.extractFile(foundPath, dstPath.c_str());
  reader.close();
  if (!ok) return "";

  g_ISOPath   = isoPath;
  g_ISOPrefix = subdir;
  return dstPath;
}

std::string MakeTempDir()
{
#ifdef _WIN32
  char tempPath[MAX_PATH];
  if (!GetTempPathA(MAX_PATH, tempPath)) return "";

  for (int attempt = 0; attempt < 100; attempt++) {
    char dirName[MAX_PATH];
    snprintf(dirName, sizeof(dirName), "%snuance_%08x_%d",
             tempPath, (unsigned)GetTickCount(), attempt);
    if (CreateDirectoryA(dirName, NULL))
      return dirName;
  }
  return "";
#else
  char tmpl[] = "/tmp/nuance_XXXXXX";
  char* dir = mkdtemp(tmpl);
  return dir ? std::string(dir) : std::string();
#endif
}

// Shell out to MAME's chdman to extract a CHD-DVD image to a plain ISO in
// tempDir. NUON discs are single-track DVD-Video so `chdman extractdvd`
// produces a flat 2048-byte-sector ISO that ISO9660Reader can open. CD-format
// CHDs (BIN/CUE) are not supported here — they would need `extractcd` plus
// a CUE-to-ISO conversion which isn't worth carrying when no NUON dumps use
// that format. Returns the path to the produced ISO, or "" on failure. The
// caller is responsible for adding the path to g_tempPaths so it gets cleaned
// up on shutdown.
std::string ExtractChdToIso(const char* chdPath, const std::string& tempDir)
{
  const std::string isoPath = tempDir + PATH_SEP + "extracted.iso";
  // Quote both paths so spaces in the source path survive the shell. -f
  // (force) lets the call succeed even if a stale extracted.iso exists.
#ifdef _WIN32
  const std::string redirect = " > NUL 2>&1";
#else
  const std::string redirect = " >/dev/null 2>&1";
#endif
  const std::string cmd =
      std::string("chdman extractdvd -i \"") + chdPath + "\" -o \"" + isoPath +
      "\" -f" + redirect;
  const int ret = system(cmd.c_str());
  if (ret != 0) {
    fprintf(stderr, "chdman extractdvd failed (exit %d). Is `chdman` (from MAME tools) on PATH?\n", ret);
    return "";
  }
  fprintf(stderr, "Extracted CHD: %s -> %s\n", chdPath, isoPath.c_str());
  return isoPath;
}

#ifdef _WIN32 // ZIP handling via miniz
// Returns the basename portion of a path (after the last '/' or '\')
const char* Basename(const char* path)
{
  const char* a = strrchr(path, '/');
  const char* b = strrchr(path, '\\');
  const char* sep = (a > b) ? a : b;
  return sep ? sep + 1 : path;
}

// Create every parent directory of a file path. Best-effort: ignores failures
// (which include "already exists"). Walks each separator in the path and tries
// CreateDirectoryA on each prefix.
void MakeDirsRecursive(const std::string& filePath)
{
  for (size_t i = 1; i < filePath.size(); i++) {
    if (filePath[i] == '\\' || filePath[i] == '/') {
      const std::string prefix = filePath.substr(0, i);
      CreateDirectoryA(prefix.c_str(), NULL);
    }
  }
}

// Walk a zip looking for an inner ISO and a direct boot file.
//
// If an inner ISO is found, only that ISO is extracted (caller will then open
// it with ISO9660Reader and read runtime data files from the ISO at original
// LBAs - much faster than extracting every entry).
//
// If no inner ISO but a boot file is found, every entry is extracted with its
// directory structure preserved, so that `MediaRead` can later locate any data
// files the game opens at runtime.
//
// Returns true if at least one of outBoot or outIso is filled.
bool ExtractZipBootOrIso(const char* zipPath, const std::string& tempDir, std::string& outBoot, std::string& outIso)
{
  mz_zip_archive zip;
  memset(&zip, 0, sizeof(zip));
  if (!mz_zip_reader_init_file(&zip, zipPath, 0))
    return false;

  const mz_uint numFiles = mz_zip_reader_get_num_files(&zip);

  // Pass 1: scan for indices of interest. Don't extract anything yet
  int innerIsoIdx = -1;
  int bootIdx     = -1;
  std::string innerIsoBasename;

  for (mz_uint i = 0; i < numFiles; i++) {
    if (mz_zip_reader_is_file_a_directory(&zip, i)) continue;
    mz_zip_archive_file_stat st;
    if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;

    const char* base = Basename(st.m_filename);

    if (innerIsoIdx < 0) {
      const size_t blen = strlen(base);
      if (blen >= 4 && (_stricmp(base + blen - 4, ".iso") == 0 ||
                        _stricmp(base + blen - 4, ".img") == 0)) {
        innerIsoIdx = (int)i;
        innerIsoBasename = base;
      }
    }
    if (bootIdx < 0) {
      for (const char* const* bp = kBootFileNames; *bp; bp++) {
        if (_stricmp(base, *bp) == 0) { bootIdx = (int)i; break; }
      }
    }
  }

  // Inner ISO takes priority - extract just it
  if (innerIsoIdx >= 0) {
    const std::string out = tempDir + PATH_SEP + innerIsoBasename;
    if (mz_zip_reader_extract_to_file(&zip, innerIsoIdx, out.c_str(), 0))
      outIso = out;
    mz_zip_reader_end(&zip);
    return !outIso.empty();
  }

  // No inner ISO: if a boot file is present, extract every entry preserving
  // directory structure so runtime data reads can find their files on disk
  if (bootIdx >= 0) {
    for (mz_uint i = 0; i < numFiles; i++) {
      if (mz_zip_reader_is_file_a_directory(&zip, i)) continue;
      mz_zip_archive_file_stat st;
      if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;

      // Build destination path with native separators.
      std::string dst = tempDir;
      dst += PATH_SEP;
      for (const char* p = st.m_filename; *p; p++)
        dst += (*p == '/' || *p == '\\') ? PATH_SEP : *p;

      MakeDirsRecursive(dst);

      if (mz_zip_reader_extract_to_file(&zip, i, dst.c_str(), 0)) {
        if ((int)i == bootIdx) outBoot = dst;
      }
    }
  }

  mz_zip_reader_end(&zip);
  return !outBoot.empty() || !outIso.empty();
}

#else // _WIN32 // Linux-only: FUSE-based mount / extract logic

std::string PopenLine(const std::string& cmd)
{
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) return "";
  char buf[1024] = {};
  if (fgets(buf, sizeof(buf), fp)) {
    const size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
  }
  pclose(fp);
  return buf;
}

std::string MountPath(const char* archivePath)
{
  const std::string mountPoint = MakeTempDir();
  if (mountPoint.empty()) return "";

  const std::string path(archivePath);
  std::string cmd;
  int ret = -1;

  const bool isIso = IsIsoPath(path);
  const bool isZip = IsZipPath(path);

  if (isIso) {
    cmd = "fuseiso \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
    if (ret != 0) {
      // Last resort: extract NUON dir via 7z (not a mount - don't track for fusermount)
      cmd = "7z x -y -o\"" + mountPoint + "\" \"" + path + "\" NUON/ nuon/ Nuon/ > /dev/null 2>&1";
      ret = system(cmd.c_str());
      if (ret == 0) {
        fprintf(stderr, "Extracted NUON dir from ISO: %s\n", path.c_str());
        g_tempPaths.push_back(mountPoint);
        return mountPoint;
      }
    }
  }
  if (ret != 0 && isZip) {
    cmd = "mount-zip \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
  }
  if (ret != 0 && isZip) {
    cmd = "fuse-zip -r \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
  }
  if (ret != 0) {
    cmd = "archivemount -o readonly \"" + path + "\" \"" + mountPoint + "\" 2>/dev/null";
    ret = system(cmd.c_str());
  }

  if (ret != 0) {
    fprintf(stderr, "Failed to mount: %s\n", archivePath);
    rmdir(mountPoint.c_str());
    return "";
  }

  fprintf(stderr, "Mounted: %s -> %s\n", archivePath, mountPoint.c_str());
  g_tempMounts.push_back(mountPoint);
  return mountPoint;
}

std::string MountAndFind(const char* archivePath)
{
  fprintf(stderr, "Mounting: %s\n", archivePath);

  const std::string mp = MountPath(archivePath);
  if (mp.empty()) return "";

  std::string result = PopenLine(
      "find \"" + mp + "\" -maxdepth 5 \\( -iname 'nuon.run' -o -iname 'NUON.CD' \\) -print -quit 2>/dev/null");

  if (result.empty()) {
    const std::string iso = PopenLine(
        "find \"" + mp + "\" -maxdepth 2 \\( -iname '*.iso' -o -iname '*.img' \\) -print -quit 2>/dev/null");
    if (!iso.empty()) {
      fprintf(stderr, "Found ISO inside: %s\n", iso.c_str());
      const std::string isoTempDir = MakeTempDir();
      if (!isoTempDir.empty()) {
        g_tempPaths.push_back(isoTempDir);
        const std::string boot = ExtractIsoBootAndArmDataReads(iso.c_str(), isoTempDir);
        if (!boot.empty()) {
          fprintf(stderr, "  ISO data path: %s/%s/\n", iso.c_str(), g_ISOPrefix.c_str());
          result = boot;
        }
      }
    }
  }

  if (result.empty())
    result = PopenLine(
        "find \"" + mp + "\" -maxdepth 5 -iname 'cd_app.cof' -print -quit 2>/dev/null");

  if (result.empty()) {
    fprintf(stderr, "No NUON game found in: %s\n", archivePath);
    return "";
  }

  fprintf(stderr, "Found: %s\n", result.c_str());
  return result;
}

#endif // _WIN32

} // anonymous namespace

//

std::string ResolveGameFile(const char* inputPath)
{
  if (!inputPath || !*inputPath) return "";
  const std::string input(inputPath);

  // CHD: extract to a temp ISO via `chdman extractdvd` and then handle the
  // produced ISO exactly like a regular ISO input. Same code path on Windows
  // and Linux because chdman is cross-platform; on Linux we deliberately skip
  // the FUSE-mount path (no point mounting a flat ISO we just produced).
  if (IsChdPath(input)) {
    const std::string tempDir = MakeTempDir();
    if (tempDir.empty()) return "";
    g_tempPaths.push_back(tempDir);
    const std::string iso = ExtractChdToIso(inputPath, tempDir);
    if (iso.empty()) return "";
    g_tempPaths.push_back(iso);
    return ExtractIsoBootAndArmDataReads(iso.c_str(), tempDir);
  }

#ifdef _WIN32
  if (!IsIsoPath(input) && !IsZipPath(input)) return input; // pass through

  const std::string tempDir = MakeTempDir();
  if (tempDir.empty()) return "";
  g_tempPaths.push_back(tempDir);

  if (IsIsoPath(input))
    return ExtractIsoBootAndArmDataReads(inputPath, tempDir);

  // .zip
  std::string boot, innerIso;
  if (!ExtractZipBootOrIso(inputPath, tempDir, boot, innerIso))
    return "";
  if (!boot.empty()) return boot;
  if (!innerIso.empty()) {
    g_tempPaths.push_back(innerIso); // make sure the extracted inner ISO gets cleaned up
    return ExtractIsoBootAndArmDataReads(innerIso.c_str(), tempDir);
  }
  return "";
#else
  if (!IsIsoPath(input) && !IsZipPath(input) && !IsOtherArchivePath(input))
    return input;
  return MountAndFind(inputPath);
#endif
}

void CleanupArchives()
{
#ifdef _WIN32
  for (auto it = g_tempPaths.rbegin(); it != g_tempPaths.rend(); ++it) {
    // SHFileOperationA needs a double-NUL-terminated source string.
    char from[MAX_PATH + 2] = {};
    strncpy(from, it->c_str(), MAX_PATH);
    SHFILEOPSTRUCTA op = {};
    op.wFunc  = FO_DELETE;
    op.pFrom  = from;
    op.fFlags = FOF_NO_UI;
    SHFileOperationA(&op);
  }
  g_tempPaths.clear();
#else
  for (auto it = g_tempMounts.rbegin(); it != g_tempMounts.rend(); ++it) {
    const std::string cmd = "fusermount -uz \"" + *it + "\" 2>/dev/null";
    system(cmd.c_str());
    rmdir(it->c_str());
  }
  g_tempMounts.clear();
  for (auto& d : g_tempPaths) {
    const std::string cmd = "rm -rf \"" + d + "\" 2>/dev/null";
    system(cmd.c_str());
  }
  g_tempPaths.clear();
#endif
}
