#include "MainWindow.hpp"
#include <QFile>
#include <QRegularExpression>
#include <QStringConverter>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Анализатор текста");
  resize(1200, 700);

  setupUI();
  setupConnections();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
  stackedWidget = new QStackedWidget(this);
  inputPage = new InputPage(this);
  stackedWidget->addWidget(inputPage); // индекс 0

  // Главный layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(stackedWidget);

  stackedWidget->setCurrentWidget(inputPage);
}

void MainWindow::setupConnections() {
  // Связываем сигналы InputPage
  // connect(inputPage, &InputPage::analyzeRequested, this,
  //        &MainWindow::onAnalyzeRequested);
}
