#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fileprocessor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QSettings>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    processor = new FileProcessor();
    processorThread = new QThread(this);
    processor->moveToThread(processorThread);
    processorThread->start();

    timer = new QTimer(this);

    setupConnections();
    updateUiState(false);

    QSettings settings;
    ui->inputPathEdit->setText(settings.value("inputPath", QDir::currentPath()).toString());
    ui->inputMaskEdit->setText(settings.value("inputMask", "*.txt").toString());
    ui->deleteOriginalsCheck->setChecked(settings.value("deleteOriginals", false).toBool());
    ui->outputPathEdit->setText(settings.value("outputPath", QDir::currentPath()).toString());
    ui->overwriteCombo->setCurrentIndex(settings.value("overwritePolicy", 0).toInt());
    ui->xorKeyEdit->setText(settings.value("xorKey", "AABBCCDDEEFF0011").toString());
    ui->pollingIntervalSpin->setValue(settings.value("pollingInterval", 1000).toInt());
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("inputMask", ui->inputMaskEdit->text());
    settings.setValue("deleteOriginals", ui->deleteOriginalsCheck->isChecked());
    settings.setValue("outputPath", ui->outputPathEdit->text());
    settings.setValue("overwritePolicy", ui->overwriteCombo->currentIndex());
    settings.setValue("xorKey", ui->xorKeyEdit->text());
    settings.setValue("pollingInterval", ui->pollingIntervalSpin->value());

    processorThread->quit();
    processorThread->wait();
    delete processor;
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->inputBrowseButton, &QPushButton::clicked, this, &MainWindow::browseInput);
    connect(ui->outputBrowseButton, &QPushButton::clicked, this, &MainWindow::browseOutput);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startProcessing);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopProcessing);
    connect(processor, &FileProcessor::progressChanged, this, &MainWindow::updateProgress);
    connect(processor, &FileProcessor::finished, this, &MainWindow::processingFinished);
    connect(timer, &QTimer::timeout, this, &MainWindow::processFiles);
}

void MainWindow::browseInput()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите входной каталог");
    if (!dir.isEmpty()) {
        ui->inputPathEdit->setText(dir);
    }
}

void MainWindow::browseOutput()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите выходной каталог");
    if (!dir.isEmpty()) {
        ui->outputPathEdit->setText(dir);
    }
}

void MainWindow::startProcessing()
{
    if (ui->inputPathEdit->text().isEmpty() ||
        ui->outputPathEdit->text().isEmpty() ||
        ui->xorKeyEdit->text().length() != 16) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста заполните этот ключ корректно (XOR ключ должен состоять из 16 16-значных символов 0-9 A-F == 8 байт)");
        return;
    }

    processingActive = true;
    updateUiState(true);

    if (ui->runModeCombo->currentIndex() == 0) {
        processFiles();
    } else {
        timer->start(ui->pollingIntervalSpin->value());
    }
}

void MainWindow::stopProcessing()
{
    processingActive = false;
    timer->stop();
    processor->cancel();
    updateUiState(false);
}

void MainWindow::updateProgress(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::processingFinished()
{
    if (ui->runModeCombo->currentIndex() == 0) {
        processingActive = false;
        updateUiState(false);
    }
}

void MainWindow::processFiles()
{
    if (!processingActive) return;

    QByteArray xorKey = QByteArray::fromHex(ui->xorKeyEdit->text().toLatin1());
    if (xorKey.size() != 8) {
        QMessageBox::critical(this, "Ошибка", "Неверный размер XOR ключа");
        return;
    }

    processor->processFiles(
        ui->inputPathEdit->text(),
        ui->inputMaskEdit->text(),
        ui->outputPathEdit->text(),
        ui->deleteOriginalsCheck->isChecked(),
        ui->overwriteCombo->currentIndex(),
        xorKey
        );
}

void MainWindow::updateUiState(bool running)
{
    ui->startButton->setEnabled(!running);
    ui->stopButton->setEnabled(running);
    ui->inputPathEdit->setEnabled(!running);
    ui->inputMaskEdit->setEnabled(!running);
    ui->outputPathEdit->setEnabled(!running);
    ui->runModeCombo->setEnabled(!running);
    ui->pollingIntervalSpin->setEnabled(!running && ui->runModeCombo->currentIndex() == 1);

    if (!running) {
        ui->progressBar->setValue(0);
    }
}
