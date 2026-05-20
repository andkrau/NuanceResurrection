#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <string>

// Resolve a user-supplied input path to a loadable game file
//
// If the input is a plain .run / .cd / .cof / .nuon, returns the input unchanged.
// If the input is an ISO/IMG, ZIP (on Linux also 7z,RAR,etc.) or CHD, the
// archive is opened/mounted/extracted and the NUON boot file
// (nuon.run / NUON.CD / cd_app.cof) is extracted to a temp location whose
// path is returned. CHD support requires MAME's `chdman` on PATH.
//
// For ISO inputs (or ZIPs containing an ISO), this function also sets the
// g_ISOPath / g_ISOPrefix globals so that media.cpp's MediaRead() can read
// runtime data files directly from the ISO at their original LBAs without
// having to extract the entire disc
//
// Returns an empty string if extraction failed or no boot file could be found
std::string ResolveGameFile(const char* inputPath);

// Tear down any temp dirs / mounts created by ResolveGameFile()
// Call once at shutdown, but safe to call multiple times
void CleanupArchives();

#endif
