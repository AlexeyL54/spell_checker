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

  /**
   * @brief Возвращает указатель на кнопку инструкции
   */
  QPushButton *getInstructionButton() const { return btnInstruction; }

  /**
   * @brief Загружает текст из выбранного файла и переключается в режим
   * клавиатуры
   * @return true если загрузка успешна, false если ошибка
   */
  bool loadAndSwitchToKeyboard();

  void updateThemeColors(const ThemeColors &colors);

  /**
   * @brief Возвращает указатель на верхнюю панель для интеграции в MainWindow
   */
  QWidget *getTopBarWidget() const { return topBarWidget; }

public slots:
  void onFileSelected(); // Выбор файла

signals:
  // Сигналы, испускаемые при нажатии соответствующих кнопок
  void checkRequested();
  void fixRequested();
  void revertRequested();
  void clearRequested();
  void copyRequested();
  void saveRequested();
  void fileLoadRequested(); // Новый сигнал для загрузки файла

private slots:
  void onSourceToggled(); // Переключение между клавиатурой и файлом
  void onLoadFile();      // Загрузка файла

private:
  void setupMainLayout();
  void setupContentRow();
  void setupContentColumn();
  void setupIntroText();
  void setupInputChoice();
  void setupKeyboardPage();
  void setupFilePage();
  void setupButtons();
  void setupConnections();
  void setupInstructionButton();
  void showInstruction();
  void updateInstructionButtonStyle();

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
  QPushButton *btnLoadFile = nullptr;

  QPushButton *btnInstruction = nullptr;
  QWidget *topBarWidget =
      nullptr; // Сохраняем указатель на виджет верхней панели
};

#endif // INPUTPAGE_HPP
