#include "fileprocessor.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>


FileProcessor::FileProcessor(QObject *parent) : QObject(parent), cancelFlag(false), future() {}

void FileProcessor::cancel()
{
    cancelFlag.storeRelaxed(true);
}

void FileProcessor::processFiles(const QString &inputDir, const QString &mask,
                                 const QString &outputDir, bool deleteOriginals,
                                 int overwritePolicy, const QByteArray &xorKey)
{
    cancelFlag.storeRelaxed(false);

    (void)QtConcurrent::run([=]() {
        QDir dir(inputDir);
        QStringList files = dir.entryList(QStringList(mask), QDir::Files);
        int total = files.size();
        int processed = 0;

        foreach (const QString &file, files) {
            if (cancelFlag.loadRelaxed()) break;

            QString inputPath = dir.filePath(file);
            QString outputPath = QDir(outputDir).filePath(file);

            if (QFile::exists(outputPath)) {
                if (overwritePolicy == 0) {
                    QFile::remove(outputPath);
                } else {
                    outputPath = getUniqueFilename(outputPath);
                }
            }
            QByteArray result = processFile(inputPath, outputPath, xorKey);
            if (result.isEmpty()) {
                qWarning() << "Ошибка обработки файла:" << inputPath;
                continue;
            }
            if (deleteOriginals) {
                QFile::remove(inputPath);
            }
            processed++;
            emit progressChanged(static_cast<int>((processed * 100) / total));
        }

        emit finished();
    });
}

QByteArray FileProcessor::processFile(const QString &inputPath, const QString &outputPath,
                                      const QByteArray &xorKey) const
{
    QFile inFile(inputPath);
    if (!inFile.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }

    QFile outFile(outputPath);
    if (!outFile.open(QIODevice::WriteOnly)) {
        return QByteArray();
    }

    const qint64 bufferSize = 1024 * 1024; // 1 MB
    qint64 totalSize = inFile.size();
    qint64 processed = 0;

    while (!inFile.atEnd()) {
        QByteArray buffer = inFile.read(bufferSize);
        for (int i = 0; i < buffer.size(); ++i) {
            buffer[i] = buffer[i] ^ xorKey[i % xorKey.size()];
        }

        if (outFile.write(buffer) != buffer.size()) {
            return QByteArray();
        }

        processed += buffer.size();
        int progress = static_cast<int>((processed * 100) / totalSize);
        emit const_cast<FileProcessor*>(this)->progressChanged(progress);

        if (cancelFlag.loadRelaxed()) {
            break;
        }
    }

    inFile.close();
    outFile.close();
    return QByteArray("OK");
}

QString FileProcessor::getUniqueFilename(const QString &path) const
{
    QFileInfo fi(path);
    QString base = fi.path() + "/" + fi.completeBaseName();
    QString ext = fi.suffix();

    int counter = 1;
    QString newPath;
    do {
        newPath = QString("%1_(%2).%3").arg(base).arg(counter++).arg(ext);
    } while (QFile::exists(newPath));

    return newPath;
}
