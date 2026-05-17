#pragma once

#include <QColor>
#include <QObject>

/**
 * @brief Перечисление доступных тем оформления.
 */
enum Theme {
  Light, ///< Светлая тема
  Dark   ///< Тёмная тема
};

/**
 * @brief Структура, содержащая цвета темы оформления.
 *
 * Хранит все цвета, используемые в приложении для визуального оформления:
 * фоновые цвета, цвета текста, акцентные цвета, цвета для проверки орфографии и
 * т.д.
 */
struct ThemeColors {
  // Основные цвета
  QColor background;   ///< Фон приложения
  QColor surface;      ///< Поверхности (панели, карточки)
  QColor primary;      ///< Акцентный цвет
  QColor primaryDark;  ///< Тёмный вариант акцента
  QColor primaryLight; ///< Светлый вариант акцента

  // Цвета текста
  QColor textPrimary;   ///< Основной текст
  QColor textSecondary; ///< Второстепенный текст
  QColor textDisabled;  ///< Неактивный текст

  // Цвета границ и разделителей
  QColor border;  ///< Границы элементов
  QColor divider; ///< Разделители

  // Состояния
  QColor hover;    ///< Цвет при наведении курсора
  QColor pressed;  ///< Цвет при нажатии
  QColor selected; ///< Цвет выбранного элемента

  // Цвета для проверки орфографии
  QColor spellError; ///< Цвет волнистого подчёркивания орфографической ошибки
  QColor spellFixed; ///< Цвет волнистого подчёркивания исправленного слова
};

/**
 * @brief Менеджер тем оформления приложения.
 *
 * Класс отвечает за управление текущей темой оформления,
 * предоставляет доступ к цветам темы и уведомляет об изменении темы.
 */
class ThemeManager : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Конструктор менеджера тем.
   * @param parent Родительский объект (по умолчанию nullptr).
   */
  explicit ThemeManager(QObject *parent = nullptr);

  /**
   * @brief Устанавливает текущую тему оформления.
   * @param theme Новая тема (Light или Dark).
   *
   * Если тема не изменилась, ничего не делает.
   * При изменении темы обновляет цвета и испускает сигнал themeChanged().
   */
  void setTheme(Theme theme);

  /**
   * @brief Возвращает текущую тему оформления.
   * @return Текущая тема.
   */
  Theme getTheme();

  /**
   * @brief Возвращает цвета текущей темы.
   * @return Структура ThemeColors с цветами текущей темы.
   */
  ThemeColors getThemeColors();

signals:
  /**
   * @brief Сигнал об изменении темы.
   *
   * Испускается после успешного изменения темы.
   * Подписчики могут обновить свои цвета в ответ на этот сигнал.
   */
  void themeChanged();

private:
  /**
   * @brief Загружает цветовую схему для указанной темы.
   * @param theme Тема, для которой нужно загрузить цвета.
   * @return Структура ThemeColors с цветами указанной темы.
   */
  ThemeColors loadTheme(Theme theme) const;

  Theme currentTheme_;        ///< Текущая активная тема
  ThemeColors currentColors_; ///< Цвета текущей темы
};
