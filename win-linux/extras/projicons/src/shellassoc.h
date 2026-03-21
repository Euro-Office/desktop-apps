#ifndef SHELLASSOC_H
#define SHELLASSOC_H

#include <vector>

struct AssocPair {
    const wchar_t* extension;
    const wchar_t* progId;
};

bool SetUserFileAssoc(const std::vector<AssocPair> &assocList);
// bool ResetUserFileAssoc(const wchar_t* ext, bool notifySystem = true);
// bool ResetUserFileAssoc(const wchar_t** extArray, size_t count);

#endif // SHELLASSOC_H
