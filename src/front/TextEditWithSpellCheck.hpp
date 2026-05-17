#pragma once

#include <QPlainTextEdit>
#include <QSet>
#include <QString>
#include <QVector>

#include "../back/vocab.hpp"
#include "ThemeManager.hpp"

/**
 * @brief Структура, описывающая орфографическую ошибку.
 */
struct SpellError {
  int start;                    ///< Начальная позиция слова в символах
  int length;                   ///< Длина слова в символах
  QString word;                 ///< Исходное слово
  QVector<QString> suggestions; ///< Варианты исправлений
};

/**
 * @brief Виджет QPlainTextEdit с проверкой орфографии.
 *
 * Позволяет выделять ошибочные слова, предлагать исправления,
 * автоматически исправлять все ошибки, отменять изменения и
 * копировать/сохранять текст.
 */
class TextEditWithSpellCheck : public QPlainTextEdit {
  Q_OBJECT

public:
  /**
   * @brief Конструктор виджета текстового редактора с проверкой орфографии.
   * @param colors Цветовая схема темы для подсветки ошибок.
   * @param parent Родительский виджет (по умолчанию nullptr).
   */
  explicit TextEditWithSpellCheck(const ThemeColors &colors,
                                  QWidget *parent = nullptr);

  /**
   * @brief Деструктор виджета.
   */
  ~TextEditWithSpellCheck();

  /**
   * @brief Устанавливает указатель на словарь.
   * @param vocab Словарь (должен существовать дольше, чем виджет)
   */
  void setVocabulary(Vocabulary *vocab);

  /**
   * @brief Устанавливает цвета из текущей темы.
   * @param colors Цветовая схема из ThemeManager
   */
  void setThemeColors(const ThemeColors &colors);

  /**
   * @brief Запускает проверку орфографии текущего текста.
   *
   * Сохраняет исходный текст для отмены, находит ошибки и выделяет их.
   */
  void performSpellCheck();

  /**
   * @brief Автоматически исправляет все найденные ошибки, используя первый
   * вариант из списка.
   *
   * После замены выделяет исправленные слова мягким цветом.
   */
  void applyFirstCorrections();

  /**
   * @brief Восстанавливает исходный текст.
   */
  void revertToOriginal();

  /**
   * @brief Очищает текст, сбрасывает все выделения и состояния.
   */
  void clearAll();

  /**
   * @brief Возвращает текущий текст.
   * @return Текст из редактора в виде строки.
   */
  QString getText() const;

  /**
   * @brief Устанавливает текст программно.
   * @param text Новый текст для установки.
   */
  void setText(const QString &text);

  /**
   * @brief Обновляет цвета подсветки ошибок и исправлений.
   */
  void updateColors();

protected:
  /**
   * @brief Обработчик нажатия кнопки мыши.
   * @param event Событие мыши.
   *
   * Переопределён для отображения контекстного меню с вариантами исправления
   * при клике на слово с орфографической ошибкой.
   */
  void mousePressEvent(QMouseEvent *event) override;

private slots:
  /**
   * @brief Сбрасывает выделение при ручном изменении текста.
   *
   * Вызывается при любом изменении текста пользователем.
   * Очищает подсветку ошибок и список исправленных позиций.
   */
  void onTextChanged();

signals:
  /**
   * @brief Сигнал о завершении проверки орфографии.
   * @param errorCount Количество найденных ошибок.
   */
  void spellCheckCompleted(int errorCount);

  /**
   * @brief Сигнал об изменении возможности отмены изменений.
   * @param canRevert true если доступна отмена, false в противном случае.
   */
  void canRevertChanged(bool canRevert);

private:
  /**
   * @brief Находит ошибку по позиции курсора.
   * @param position Позиция в документе.
   * @return Указатель на найденную ошибку или nullptr, если ошибка не найдена.
   */
  SpellError *findErrorAtPosition(int position);

  /**
   * @brief Показывает контекстное меню с вариантами исправления ошибки.
   * @param globalPos Глобальная позиция для отображения меню.
   * @param error Структура ошибки для исправления.
   */
  void showCorrectionMenu(const QPoint &globalPos, const SpellError &error);

  /**
   * @brief Применяет выбранное исправление к ошибке.
   * @param error Структура ошибки.
   * @param suggestion Выбранный вариант исправления.
   */
  void applyCorrection(const SpellError &error, const QString &suggestion);

  /**
   * @brief Добавляет слово в игнорируемые и убирает подсветку ошибки.
   * @param error Структура ошибки.
   */
  void ignoreWord(const SpellError &error);

  /**
   * @brief Удаляет ошибку из списка по позиции и длине.
   * @param start Начальная позиция.
   * @param length Длина слова.
   */
  void removeErrorFromList(int start, int length);

  /**
   * @brief Применяет регистр оригинального слова к исправленному слову.
   * @param originalWord Оригинальное слово (с ошибкой)
   * @param correctedWord Исправленное слово (обычно в нижнем регистре)
   * @return Слово с сохранённым регистром
   */
  QString preserveCase(const QString &originalWord,
                       const QString &correctedWord);

  /**
   * @brief Удаляет всё форматирование и очищает список ошибок.
   */
  void clearSpellCheck();

  /**
   * @brief Применяет красное волнистое подчёркивание к словам с ошибками.
   */
  void highlightErrors();

  /**
   * @brief Применяет мягкое выделение к исправленным позициям.
   */
  void highlightFixedPositions();

  /**
   * @brief Добавляет слово в список игнорируемых.
   * @param word Слово для добавления (регистр не важен).
   */
  void addIgnoredWord(const QString &word);

  /**
   * @brief Проверяет, игнорируется ли слово.
   * @param word Слово для проверки (регистр не важен).
   * @return true если слово в списке игнорируемых.
   */
  bool isWordIgnored(const QString &word) const;

  /**
   * @brief Находит все орфографические ошибки в тексте.
   * @param text Текст для анализа.
   * @return Вектор структур SpellError с информацией об ошибках.
   */
  QVector<SpellError> findErrors(const QString &text);

  /**
   * @brief Заменяет слово в документе по указанной позиции.
   * @param start Начальная позиция заменяемого слова.
   * @param length Длина заменяемого слова.
   * @param newWord Новое слово для вставки.
   */
  void replaceWordAt(int start, int length, const QString &newWord);

  /**
   * @brief Применяет форматирование к указанному диапазону текста.
   * @param start Начальная позиция.
   * @param length Длина диапазона.
   * @param format Формат для применения.
   */
  void applyFormatToRange(int start, int length, const QTextCharFormat &format);

  /**
   * @brief Очищает всё форматирование документа.
   */
  void clearFormats();

  Vocabulary *vocab_ = nullptr; ///< Указатель на словарь
  QVector<SpellError> errors_;  ///< Список текущих ошибок
  QSet<QString> ignoredWords_;  ///< Слова, отмеченные пользователем как
                                ///< правильные (в нижнем регистре)
  QString originalText_;        ///< Текст, сохранённый для отмены
  bool hasOriginal_ = false;    ///< Флаг наличия сохранённого текста
  bool selfUpdating_ = false;   ///< Предотвращает рекурсивные вызовы при
                                ///< программном изменении текста
  QVector<QPair<int, int>> fixedPositions_; ///< Позиции (начало, длина) слов,
                                            ///< исправленных автоматически
  ThemeColors currentColors_;               ///< Текущие цвета темы
};
