/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <memory>
#include "components/ctabpanel.h"
#include "OfficeFileFormats.h"
#include "fakesdk.h"

// CTabPanel (src/components/ctabpanel.cpp), the widget around an editor's
// view, against the fake SDK: which views it creates and what it asks them
// to open. Unit test build only (SCOPE unit), as it uses fakesdk.h.

namespace {

class TabPanelTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        FakeSdk::reset();
        m_panel = std::make_unique<CTabPanel>(nullptr, QSize(800, 600));
    }

    std::unique_ptr<CTabPanel> m_panel;
};

} // namespace

TEST_F(TabPanelTest, InitAsEditorCreatesAnEditorView)
{
    m_panel->initAsEditor();

    ASSERT_NE(m_panel->cef(), nullptr);
    EXPECT_EQ(m_panel->cef()->GetType(), cvwtEditor);
    EXPECT_EQ(FakeSdk::view(m_panel->cef()->GetId()), m_panel->cef());
}

TEST_F(TabPanelTest, InitAsSimpleCreatesAPlainView)
{
    m_panel->initAsSimple();

    ASSERT_NE(m_panel->cef(), nullptr);
    EXPECT_EQ(m_panel->cef()->GetType(), cvwtSimple);
}

TEST_F(TabPanelTest, OpensALocalFileOfAKnownFormat)
{
    FakeSdk::setFileFormat(L"/home/me/report.docx", AVS_OFFICESTUDIO_FILE_DOCUMENT_DOCX);
    m_panel->initAsEditor();

    EXPECT_TRUE(m_panel->openLocalFile(L"/home/me/report.docx", L""));
    EXPECT_EQ(FakeSdk::calls("CCefViewEditor::OpenLocalFile"),
              std::vector<std::wstring>{L"/home/me/report.docx"});
}

TEST_F(TabPanelTest, RefusesALocalFileOfUnknownFormat)
{
    m_panel->initAsEditor();

    EXPECT_FALSE(m_panel->openLocalFile(L"/home/me/notes.xyz", L""));
    EXPECT_TRUE(FakeSdk::calls("CCefViewEditor::OpenLocalFile").empty());
}

TEST_F(TabPanelTest, OpensRecentFilesById)
{
    m_panel->initAsEditor();

    EXPECT_TRUE(m_panel->openRecentFile(7));
    EXPECT_EQ(FakeSdk::calls("CCefViewEditor::OpenRecentFile"), std::vector<std::wstring>{L"7"});
}
