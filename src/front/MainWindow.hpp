#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QStackedWidget>
#include <QWidget>

#include "InputPage.hpp"

class MainWindow : public QWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  // Слоты для навигации между страницами
  // void onAnalyzeRequested();

private:
  void setupUI();
  void setupConnections();
  void createSearchItems(); // Создаёт элементы для поиска из готовых файлов

  QStackedWidget *stackedWidget = nullptr;
  InputPage *inputPage = nullptr;
};

#endif // MAINWINDOW_H
