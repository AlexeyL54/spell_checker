#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../back/vocab.hpp"
#include "InputPage.hpp"
#include "LoadingPage.hpp"
#include "ThemeManager.hpp"
#include "qboxlayout.h"
#include <QComboBox>
#include <QWidget>

class MainWindow : public QWidget {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onCheckRequested();
  void onFixRequested();
  void onRevertRequested();
  void onClearRequested();
  void onCopyRequested();
  void onSaveRequested();
  void onThemeChanged(int index);

  void onLoadStarted();
  void onLoadProgress(int wordsLoaded);
  void onLoadFinished();
  void onLoadError(const QString &error);

private:
  void setupUI();
  void setupThemeSelector();
  void applyTheme(const ThemeColors &colors);
  void loadTextFromFile(const QString &path);
  void showLoadingPage();
  void showMainContent();

  InputPage *inputPage = nullptr;
  ThemeManager *themeManager = nullptr;
  QComboBox *themeCombo = nullptr;
  Vocabulary *vocabulary = nullptr;

  QStackedWidget *stackedWidget = nullptr;
  LoadingPage *loadingPage = nullptr;
  QWidget *mainContent = nullptr;

  QHBoxLayout *topBar = nullptr;
};

#endif // MAINWINDOW_H
