#pragma once

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
#include "ThemeManager.hpp"

/**
 * @brief Главная страница приложения для ввода текста.
 *
 * Класс предоставляет интерфейс для ввода текста двумя способами:
 * - непосредственный ввод с клавиатуры
 * - загрузка из текстового файла
 *
 * Содержит панель инструментов для проверки орфографии, исправления ошибок,
 * сохранения и копирования текста.
 */
class InputPage : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief Конструктор страницы ввода.
   * @param colors Цветовая схема темы оформления.
   * @param parent Родительский виджет (по умолчанию nullptr).
   */
  explicit InputPage(const ThemeColors &colors, QWidget *parent = nullptr);

  /**
   * @brief Получить указатель на текстовый редактор с проверкой орфографии.
   * @return Указатель на TextEditWithSpellCheck.
   */
  TextEditWithSpellCheck *getTextEdit() const;

  /**
   * @brief Определить, активен ли режим ввода с клавиатуры.
   * @return true если выбран ввод с клавиатуры, false если выбран файл.
   */
  bool isKeyboardMode() const;

  /**
   * @brief Получить путь к выбранному файлу.
   * @return Строка с путем к файлу.
   */
  QString getFilePath() const;

  /**
   * @brief Получить указатель на кнопку инструкции.
   * @return Указатель на QPushButton.
   */
  QPushButton *getInstructionButton() const { return btnInstruction; }

  /**
   * @brief Загрузить текст из файла и переключиться в режим клавиатуры.
   * @return true если загрузка успешна, false в противном случае.
   */
  bool loadAndSwitchToKeyboard();

  /**
   * @brief Обновить цветовую схему страницы.
   * @param colors Новая цветовая схема.
   */
  void updateThemeColors(const ThemeColors &colors);

  /**
   * @brief Получить верхнюю панель инструментов.
   * @return Указатель на виджет верхней панели.
   */
  QWidget *getTopBarWidget() const { return topBarWidget; }

public slots:
  /**
   * @brief Обработчик выбора файла через диалог.
   */
  void onFileSelected();

  /**
   * @brief Обработчик завершения проверки орфографии.
   * @param errorCount Количество найденных ошибок.
   */
  void onSpellCheckCompleted(int errorCount);

  /**
   * @brief Обработчик изменения возможности отмены изменений.
   * @param canRevert true если доступна отмена изменений.
   */
  void onCanRevertChanged(bool canRevert);

signals:
  /** @brief Сигнал запроса проверки орфографии. */
  void checkRequested();

  /** @brief Сигнал запроса автоматического исправления ошибок. */
  void fixRequested();

  /** @brief Сигнал запроса отмены последних изменений. */
  void revertRequested();

  /** @brief Сигнал запроса очистки текста. */
  void clearRequested();

  /** @brief Сигнал запроса копирования текста в буфер обмена. */
  void copyRequested();

  /** @brief Сигнал запроса сохранения текста в файл. */
  void saveRequested();

  /** @brief Сигнал запроса загрузки текста из файла. */
  void fileLoadRequested();

private slots:
  /**
   * @brief Обработчик переключения источника ввода.
   */
  void onSourceToggled();

  /**
   * @brief Обработчик загрузки текста из файла.
   */
  void onLoadFile();

  /**
   * @brief Обработчик изменения текста в редакторе.
   */
  void onTextChanged();

  /**
   * @brief Обновить состояние строки статуса.
   * @param errorCount Количество ошибок (-1 для состояния редактирования).
   */
  void updateStatusBar(int errorCount = -1);

private:
  /**
   * @brief Настроить главный компоновщик страницы.
   */
  void setupMainLayout();

  /**
   * @brief Настроить строку с контентом.
   */
  void setupContentRow();

  /**
   * @brief Настроить колонку с контентом.
   */
  void setupContentColumn();

  /**
   * @brief Настроить вступительный текст.
   */
  void setupIntroText();

  /**
   * @brief Настроить переключатели выбора источника ввода.
   */
  void setupInputChoice();

  /**
   * @brief Настроить страницу ввода с клавиатуры.
   */
  void setupKeyboardPage();

  /**
   * @brief Настроить страницу выбора файла.
   */
  void setupFilePage();

  /**
   * @brief Настроить строку состояния.
   */
  void setupStatusBar();

  /**
   * @brief Настроить кнопки строки состояния.
   */
  void setupStatusBarButtons();

  /**
   * @brief Настроить индикаторы строки состояния.
   */
  void setupStatusBarIndicators();

  /**
   * @brief Настроить соединения сигналов и слотов.
   */
  void setupConnections();

  /**
   * @brief Настроить кнопку вызова инструкции.
   */
  void setupInstructionButton();

  /**
   * @brief Показать диалоговое окно с инструкцией.
   */
  void showInstruction();

  /**
   * @brief Обновить состояние кнопок в зависимости от наличия текста.
   */
  void updateButtonsState();

  ThemeColors currentColors_; ///< Текущая цветовая схема темы.

  QVBoxLayout *mainLayout = nullptr;    ///< Главный вертикальный компоновщик.
  QHBoxLayout *contentRow = nullptr;    ///< Горизонтальная строка контента.
  QWidget *contentColumn = nullptr;     ///< Колонка контента.
  QVBoxLayout *contentLayout = nullptr; ///< Вертикальный компоновщик колонки.

  QLabel *introText = nullptr;              ///< Вступительный текст.
  QButtonGroup *inputChoiceGroup = nullptr; ///< Группа переключателей ввода.
  QRadioButton *radioKeyboard = nullptr;    ///< Переключатель "С клавиатуры".
  QRadioButton *radioFile = nullptr;        ///< Переключатель "Из файла".

  QStackedWidget *stack = nullptr; ///< Стек для переключения страниц.
  QWidget *pageKeyboard = nullptr; ///< Страница ввода с клавиатуры.
  QWidget *pageFile = nullptr;     ///< Страница выбора файла.

  TextEditWithSpellCheck *textInput =
      nullptr; ///< Поле ввода текста с проверкой.

  QStatusBar *statusBar = nullptr;  ///< Строка состояния.
  QPushButton *btnCheck = nullptr;  ///< Кнопка "Проверить".
  QPushButton *btnFix = nullptr;    ///< Кнопка "Исправить".
  QPushButton *btnRevert = nullptr; ///< Кнопка "Отменить изменения".
  QPushButton *btnClear = nullptr;  ///< Кнопка "Очистить".
  QPushButton *btnCopy = nullptr;   ///< Кнопка "Копировать".
  QPushButton *btnSave = nullptr;   ///< Кнопка "Сохранить".
  QLabel *statusInfo = nullptr;     ///< Информационная метка статуса.
  QLabel *statsLabel = nullptr;     ///< Метка со статистикой текста.

  QLineEdit *filePathEdit = nullptr;    ///< Поле пути к файлу.
  QPushButton *btnSelectFile = nullptr; ///< Кнопка выбора файла.
  QPushButton *btnLoadFile = nullptr;   ///< Кнопка загрузки файла.

  QPushButton *btnInstruction = nullptr; ///< Кнопка вызова инструкции.
  QWidget *topBarWidget = nullptr;       ///< Виджет верхней панели.

  int lastErrorCount_ = -1; ///< Последнее известное количество ошибок.
};
