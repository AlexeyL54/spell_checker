#ifndef INPUTPAGE_HPP
#define INPUTPAGE_HPP

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "TextEditWithSpellCheck.hpp"

class InputPage : public QWidget {
  Q_OBJECT

public:
  explicit InputPage(QWidget *parent = nullptr);

  // Геттеры для доступа к элементам
  TextEditWithSpellCheck *getTextEdit() const;
  bool isKeyboardMode() const;
  QString getFilePath() const;

signals:
  // Сигналы, испускаемые при нажатии соответствующих кнопок
  void checkRequested();
  void fixRequested();
  void revertRequested();
  void clearRequested();
  void copyRequested();
  void saveRequested();

private slots:
  void onFileSelected();  // Выбор файла
  void onSourceToggled(); // Переключение между клавиатурой и файлом

private:
  void setupMainLayout();
  void setupContentRow();
  void setupContentColumn();
  void setupIntroText();
  void setupInputChoice();
  void setupKeyboardPage();
  void setupFilePage();
  void setupButtons(); // Новые общие кнопки
  void setupConnections();

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

  // Страница "С клавиатуры"
  TextEditWithSpellCheck *textInput = nullptr;
  // Кнопки действий
  QPushButton *btnCheck = nullptr;
  QPushButton *btnFix = nullptr;
  QPushButton *btnRevert = nullptr;
  QPushButton *btnClear = nullptr;
  QPushButton *btnCopy = nullptr;
  QPushButton *btnSave = nullptr;

  // Страница "С файла"
  QLineEdit *filePathEdit = nullptr;
  QPushButton *btnSelectFile = nullptr;
};

#endif // INPUTPAGE_HPP
