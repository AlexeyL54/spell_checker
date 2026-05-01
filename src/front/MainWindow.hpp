#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../back/vocab.hpp"
#include "InputPage.hpp"
#include "ThemeManager.hpp"
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

private:
  void setupUI();
  void setupThemeSelector();
  void applyTheme(const ThemeColors &colors);
  void loadTextFromFile(const QString &path);

  InputPage *inputPage = nullptr;
  ThemeManager *themeManager = nullptr;
  QComboBox *themeCombo = nullptr;
  Vocabulary *vocabulary = nullptr;
};

#endif // MAINWINDOW_H
