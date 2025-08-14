#ifndef FILEPROCESSOR_H
#define FILEPROCESSOR_H

#include <QObject>
#include <QByteArray>
#include <QDir>
#include <QFuture>
#include <QtConcurrent>


class FileProcessor : public QObject
{
    Q_OBJECT
public:
    explicit FileProcessor(QObject *parent = nullptr);
    void processFiles(const QString &inputDir, const QString &mask,
                      const QString &outputDir, bool deleteOriginals,
                      int overwritePolicy, const QByteArray &xorKey);
    void cancel();

signals:
    void progressChanged(int value);
    void finished();

private:
    QFuture<void> future;
    QAtomicInt cancelFlag;
    QByteArray processFile(const QString &inputPath, const QString &outputPath,
                           const QByteArray &xorKey) const;
    QString getUniqueFilename(const QString &path) const;
};
#endif // FILEPROCESSOR_H
