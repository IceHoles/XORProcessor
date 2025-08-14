#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FileProcessor;
class QThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void browseInput();
    void browseOutput();
    void startProcessing();
    void stopProcessing();
    void updateProgress(int value);
    void processingFinished();
    void processFiles();

private:
    Ui::MainWindow *ui;
    FileProcessor *processor;
    QThread *processorThread;
    QTimer *timer;
    bool processingActive = false;
    void setupConnections();
    void updateUiState(bool running);
};
#endif // MAINWINDOW_H
