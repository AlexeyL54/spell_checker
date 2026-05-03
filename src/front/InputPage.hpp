#ifndef INPUTPAGE_HPP
#define INPUTPAGE_HPP

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include "TextEditWithSpellCheck.hpp"

class InputPage : public QWidget {
  Q_OBJECT

public:
  explicit InputPage(QWidget *parent = nullptr);

  TextEditWithSpellCheck *getTextEdit() const;
  bool isKeyboardMode() const;
  QString getFilePath() const;
  QPushButton *getInstructionButton() const { return btnInstruction; }
  bool loadAndSwitchToKeyboard();
  void updateThemeColors(const ThemeColors &colors);
  QWidget *getTopBarWidget() const { return topBarWidget; }

public slots:
  void onFileSelected();
  void onSpellCheckCompleted(int errorCount);

signals:
  void checkRequested();
  void fixRequested();
  void revertRequested();
  void clearRequested();
  void copyRequested();
  void saveRequested();
  void fileLoadRequested();

private slots:
  void onSourceToggled();
  void onLoadFile();
  void onTextChanged();
  void updateStatusBar(int errorCount = -1);

private:
  void setupMainLayout();
  void setupContentRow();
  void setupContentColumn();
  void setupIntroText();
  void setupInputChoice();
  void setupKeyboardPage();
  void setupFilePage();
  void setupStatusBar();
  void setupStatusBarButtons();
  void setupStatusBarIndicators();
  void setupConnections();
  void setupInstructionButton();
  void showInstruction();
  void updateInstructionButtonStyle();
  void applyStatusBarStyle();

  ThemeColors currentColors_;

  QVBoxLayout *mainLayout = nullptr;
  QHBoxLayout *contentRow = nullptr;
  QWidget *contentColumn = nullptr;
  QVBoxLayout *contentLayout = nullptr;

  QLabel *introText = nullptr;
  QButtonGroup *inputChoiceGroup = nullptr;
  QRadioButton *radioKeyboard = nullptr;
  QRadioButton *radioFile = nullptr;

  QStackedWidget *stack = nullptr;
  QWidget *pageKeyboard = nullptr;
  QWidget *pageFile = nullptr;

  TextEditWithSpellCheck *textInput = nullptr;

  // Status bar
  QStatusBar *statusBar = nullptr;
  QPushButton *btnCheck = nullptr;
  QPushButton *btnFix = nullptr;
  QPushButton *btnRevert = nullptr;
  QPushButton *btnClear = nullptr;
  QPushButton *btnCopy = nullptr;
  QPushButton *btnSave = nullptr;
  QLabel *statusInfo = nullptr;
  QLabel *statsLabel = nullptr;

  // File page
  QLineEdit *filePathEdit = nullptr;
  QPushButton *btnSelectFile = nullptr;
  QPushButton *btnLoadFile = nullptr;

  QPushButton *btnInstruction = nullptr;
  QWidget *topBarWidget = nullptr;

  int lastErrorCount_ = -1;
};

#endif // INPUTPAGE_HPP
