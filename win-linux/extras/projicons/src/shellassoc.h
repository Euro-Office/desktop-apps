#ifndef SHELLASSOC_H
#define SHELLASSOC_H

#include <vector>

struct AssocPair {
    const wchar_t* extension;
    const wchar_t* progId;
};

// Protected types (UCPD) are left to the user: with openDefaultApps,
// Settings > Default apps is opened for them.
bool SetUserFileAssoc(const std::vector<AssocPair> &assocList, bool openDefaultApps = true);

#endif // SHELLASSOC_H
