/*
 * SPDX-FileCopyrightText: 2026 Euro-Office contributors
 * SPDX-License-Identifier: AGPL-3.0-only
 */

#include <gtest/gtest.h>
#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>
#include "components/cfiledialog.h"

// The image filters of the insert image dialogs, and the MIME type check that
// runs on the chosen files before they are inserted.

namespace {

QString writeFile(const QTemporaryDir& dir, const QString& name, const QByteArray& data)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size())
        return QString();
    return path;
}

// The first bytes of the formats, enough for the MIME database
QByteArray isoMediaHeader(const char* brand)
{
    QByteArray data("\0\0\0\x18" "ftyp", 8);
    data.append(brand, 4);
    data.append("\0\0\0\0", 4);
    data.append("mif1", 4);
    data.append(brand, 4);
    return data;
}

const QByteArray c_webp("RIFF\x24\0\0\0WEBPVP8 \x18\0\0\0", 20);

} // namespace

TEST(FileDialogImagePatterns, OffersWebPAndAvif)
{
    for (bool multiple : {false, true}) {
        const QStringList patterns = CFileDialogWrapper::imagePatterns(multiple).split(' ');
        EXPECT_TRUE(patterns.contains("*.webp")) << multiple;
        EXPECT_TRUE(patterns.contains("*.avif")) << multiple;
        EXPECT_TRUE(patterns.contains("*.heic")) << multiple;
    }
}

TEST(FileDialogMimeCheck, AcceptsWebPAvifAndHeic)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QString webp = writeFile(dir, "photo.webp", c_webp);
    const QString avif = writeFile(dir, "photo.avif", isoMediaHeader("avif"));
    const QString heic = writeFile(dir, "photo.heic", isoMediaHeader("heic"));
    for (const QString& path : {webp, avif, heic}) {
        ASSERT_FALSE(path.isEmpty());
        EXPECT_TRUE(CFileDialogWrapper::hasMimeType(path, "image")) << path.toStdString();
    }
}

TEST(FileDialogMimeCheck, RejectsOtherFiles)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    const QString text = writeFile(dir, "notes.txt", "just text\n");
    ASSERT_FALSE(text.isEmpty());
    EXPECT_FALSE(CFileDialogWrapper::hasMimeType(text, "image"));
}
