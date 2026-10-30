/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include "casctabdata.h"

// CAscTabData (src/casctabdata.cpp) holds a tab's state: title with the
// unsaved-changes marker and read-only suffix, the editor features JSON, and
// the view type derived from the document type.

TEST(TabDataViewType, EditorDocumentTypesOpenInAnEditorView)
{
    for (AscEditorType type : {AscEditorType::etDocument, AscEditorType::etSpreadsheet,
                               AscEditorType::etPresentation, AscEditorType::etPdf,
                               AscEditorType::etDraw}) {
        CAscTabData tab("doc", type);
        EXPECT_TRUE(tab.isViewType(cvwtEditor)) << static_cast<int>(type);
        EXPECT_EQ(tab.contentType(), type);
    }
}

TEST(TabDataViewType, OtherTypesOpenInASimpleView)
{
    CAscTabData tab("page", AscEditorType::etUndefined);
    EXPECT_TRUE(tab.isViewType(cvwtSimple));
}

TEST(TabDataTitle, UnsavedChangesAddAStar)
{
    CAscTabData tab("report.docx");
    EXPECT_EQ(tab.title(), "report.docx");

    tab.setChanged(true);
    EXPECT_EQ(tab.title(), "*report.docx");
    EXPECT_EQ(tab.title(true), "report.docx");

    tab.setChanged(false);
    EXPECT_EQ(tab.title(), "report.docx");
}

TEST(TabDataTitle, ModifiedStaysSetAfterSaving)
{
    CAscTabData tab("report.docx");
    EXPECT_FALSE(tab.modified());

    tab.setChanged(true);
    tab.setChanged(false);
    EXPECT_FALSE(tab.hasChanges());
    EXPECT_TRUE(tab.modified());
}

TEST(TabDataTitle, ReadOnlyFeatureAddsASuffixUntilCleared)
{
    CAscTabData tab("report.docx");
    tab.setFeatures(L"{\"readonly\":true}");
    EXPECT_TRUE(tab.title().startsWith("report.docx ("));
    EXPECT_EQ(tab.title(true), "report.docx");

    tab.setFeatures(L"{\"readonly\":false}");
    EXPECT_EQ(tab.title(), "report.docx");
}

TEST(TabDataFeatures, LaterFeaturesAreMergedIn)
{
    CAscTabData tab("report.docx");
    tab.setFeatures(L"{\"a\":1,\"b\":\"x\"}");
    tab.setFeatures(L"{\"b\":\"y\",\"c\":true}");

    EXPECT_TRUE(tab.hasFeature(L"\"a\":1"));
    EXPECT_TRUE(tab.hasFeature(L"\"b\":\"y\""));
    EXPECT_FALSE(tab.hasFeature(L"\"b\":\"x\""));
    EXPECT_TRUE(tab.hasFeature(L"\"c\":true"));
}

TEST(TabDataFeatures, InvalidJsonIsIgnored)
{
    CAscTabData tab("report.docx");
    tab.setFeatures(L"{\"a\":1}");
    tab.setFeatures(L"not json");

    EXPECT_TRUE(tab.hasFeature(L"\"a\":1"));
    EXPECT_FALSE(tab.hasFeature(L"not json"));
}

TEST(TabDataState, CloseAndReuse)
{
    CAscTabData tab("report.docx");
    EXPECT_FALSE(tab.closed());

    tab.close();
    EXPECT_TRUE(tab.closed());

    tab.reuse();
    EXPECT_FALSE(tab.closed());
}

TEST(TabDataState, UrlCanBeSetFromQStringOrWString)
{
    CAscTabData tab("report.docx");
    tab.setUrl(QString("https://example.com/a"));
    EXPECT_EQ(tab.url(), L"https://example.com/a");

    tab.setUrl(std::wstring(L"https://example.com/b"));
    EXPECT_EQ(tab.url(), L"https://example.com/b");
}
