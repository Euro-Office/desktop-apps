/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef DOCAUTOMATION_H
#define DOCAUTOMATION_H

// Handles the --convert-to and --print-to-file CLI flags by shelling out to
// the packaged x2t converter, without starting CEF/Qt UI. --print-to-file is
// an alias for --convert-to that forces PDF output, since x2t's PDF export is
// already what "Print" ultimately produces.
namespace NSDocAutomation {
    bool ShouldRun();
    int Run();
}

#endif // DOCAUTOMATION_H
