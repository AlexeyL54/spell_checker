#pragma once

#include "ThemeManager.hpp"
#include <QString>

/**
 * @brief Централизованный менеджер стилей для всего приложения.
 *
 * Класс предоставляет статические методы для генерации единой таблицы стилей
 * на основе текущей цветовой схемы. Все виджеты приложения стилизуются через
 * этот класс, что обеспечивает единообразие внешнего вида.
 *
 * Использование:
 * @code
 * ThemeColors colors = themeManager->getThemeColors();
 * QString styleSheet = StyleManager::getGlobalStyleSheet(colors);
 * qApp->setStyleSheet(styleSheet);
 * @endcode
 */
class StyleManager {
public:
  /**
   * @brief Генерирует полную таблицу стилей для приложения.
   *
   * Объединяет все частные стили (кнопки, поля ввода, комбобоксы и т.д.)
   * в единую строку CSS, которая применяется ко всему приложению.
   *
   * @param colors Цветовая схема из ThemeManager
   * @return QString Полная таблица стилей для установки через
   * QApplication::setStyleSheet()
   */
  static QString getGlobalStyleSheet(const ThemeColors &colors);

private:
  /**
   * @brief Генерирует стили для всех типов кнопок.
   *
   * Определяет единый внешний вид для QPushButton:
   * - Обычное состояние
   * - Состояние наведения (hover)
   * - Состояние нажатия (pressed)
   * - Отключённое состояние (disabled)
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для кнопок
   */
  static QString buttonStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для строки состояния.
   *
   * Определяет внешний вид QStatusBar и кнопок внутри него.
   * Кнопки в статус-баре наследуют основной стиль, но имеют
   * уменьшенную минимальную ширину и отступы.
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для статус-бара
   */
  static QString statusBarStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для текстовых полей.
   *
   * Определяет внешний вид QPlainTextEdit и QTextEdit:
   * - Фон и цвет текста
   * - Границы и скругление углов
   * - Цвет выделения текста
   * - Стиль при фокусе и в отключённом состоянии
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для текстовых полей
   */
  static QString textEditStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для выпадающих списков.
   *
   * Определяет внешний вид QComboBox:
   * - Основной виджет
   * - Состояние при наведении
   * - Стиль выпадающей кнопки (скрыта)
   * - Стиль всплывающего списка с вариантами
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для комбобоксов
   */
  static QString comboBoxStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для радиокнопок.
   *
   * Определяет внешний вид QRadioButton:
   * - Стиль текста
   * - Стиль индикатора (круглая кнопка)
   * - Состояния: обычное, наведение, выбрано, отключено
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для радиокнопок
   */
  static QString radioButtonStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для однострочных полей ввода.
   *
   * Определяет внешний вид QLineEdit:
   * - Фон и цвет текста
   * - Границы и скругление
   * - Стиль при фокусе и в отключённом состоянии
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для полей ввода
   */
  static QString lineEditStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для кнопки инструкции ("?").
   *
   * Определяет внешний вид кнопки с objectName="instructionButton":
   * - Круглая форма (через border-radius)
   * - Увеличенный шрифт
   * - Фиксированные размеры
   * - Состояния при наведении и нажатии
   *
   * @param colors Цветовая схема
   * @return QString CSS-правила для кнопки инструкции
   */
  static QString instructionButtonStyle(const ThemeColors &colors);

  /**
   * @brief Генерирует стили для кнопок выбора и загрузки файла.
   *
   * Определяет внешний вид кнопок с objectName="btnSelectFile" и "btnLoadFile".
   * В текущей реализации наследует стандартный стиль кнопок.
   *
   * @param colors Цветовая схема (оставлен для будущих расширений)
   * @return QString CSS-правила для файловых кнопок
   */
  static QString fileButtonsStyle(const ThemeColors &colors);
};
