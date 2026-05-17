#pragma once

#include "../back/vocab.hpp"
#include "InputPage.hpp"
#include "LoadingPage.hpp"
#include "ThemeManager.hpp"
#include <QComboBox>
#include <QWidget>

/**
 * @brief Главное окно приложения.
 *
 * Класс управляет основным интерфейсом приложения, включая:
 * - отображение страницы загрузки словаря
 * - отображение основного контента (страница ввода)
 * - обработку действий пользователя (проверка, исправление, сохранение и т.д.)
 * - управление темой оформления (светлая/тёмная)
 */
class MainWindow : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief Конструктор главного окна.
   * @param parent Родительский виджет (по умолчанию nullptr).
   */
  explicit MainWindow(QWidget *parent = nullptr);

  /**
   * @brief Деструктор главного окна.
   */
  ~MainWindow();

private slots:
  /**
   * @brief Обработчик запроса проверки орфографии.
   */
  void onCheckRequested();

  /**
   * @brief Обработчик запроса исправления ошибок.
   */
  void onFixRequested();

  /**
   * @brief Обработчик запроса отмены изменений.
   */
  void onRevertRequested();

  /**
   * @brief Обработчик запроса очистки текста.
   */
  void onClearRequested();

  /**
   * @brief Обработчик запроса копирования текста в буфер обмена.
   */
  void onCopyRequested();

  /**
   * @brief Обработчик запроса сохранения текста в файл.
   */
  void onSaveRequested();

  /**
   * @brief Обработчик изменения темы оформления.
   * @param index Индекс выбранной темы в комбобоксе.
   */
  void onThemeChanged(int index);

  /**
   * @brief Обработчик начала загрузки словаря.
   */
  void onLoadStarted();

  /**
   * @brief Обработчик завершения загрузки словаря.
   */
  void onLoadFinished();

  /**
   * @brief Обработчик ошибки загрузки словаря.
   * @param error Текст сообщения об ошибке.
   */
  void onLoadError(const QString &error);

private:
  /**
   * @brief Настроить пользовательский интерфейс.
   */
  void setupUI();

  /**
   * @brief Настроить стековый виджет для переключения страниц.
   */
  void setupStackedWidget();

  /**
   * @brief Настроить виджет основного контента.
   */
  void setupMainContentLayout();

  /**
   * @brief Настроить верхнюю панель инструментов.
   */
  void setupTopBar();

  /**
   * @brief Настроить страницу загрузки.
   */
  void setupLoadingPage();

  /**
   * @brief Настроить селектор выбора темы.
   */
  void setupThemeSelector();

  /**
   * @brief Применить цветовую схему темы.
   * @param colors Цветовая схема для применения.
   */
  void applyTheme(const ThemeColors &colors);

  /**
   * @brief Загрузить текст из файла.
   * @param path Путь к файлу для загрузки.
   */
  void loadTextFromFile(const QString &path);

  /**
   * @brief Показать основной контент приложения.
   */
  void showMainContent();

  /**
   * @brief Включить/выключить элементы пользовательского интерфейса.
   * @param enabled true - включить, false - выключить.
   */
  void setUIElementsEnabled(bool enabled);

  InputPage *inputPage = nullptr;       ///< Страница ввода текста.
  ThemeManager *themeManager = nullptr; ///< Менеджер тем оформления.
  QComboBox *themeCombo = nullptr;      ///< Выпадающий список выбора темы.
  Vocabulary *vocabulary = nullptr;     ///< Словарь для проверки орфографии.

  QStackedWidget *stackedWidget = nullptr; ///< Стек для переключения страниц.
  LoadingPage *loadingPage = nullptr;      ///< Страница загрузки.
  QWidget *mainContent = nullptr;          ///< Виджет основного контента.

  QHBoxLayout *topBar = nullptr; ///< Верхняя панель инструментов.
};
