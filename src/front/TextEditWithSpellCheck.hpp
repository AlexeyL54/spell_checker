#ifndef TEXTEDITWITHSPELLCHECK_HPP
#define TEXTEDITWITHSPELLCHECK_HPP

#include <QPlainTextEdit>
#include <QSet>
#include <QString>
#include <QVector>

#include "ThemeManager.hpp"

class Vocabulary;
struct ThemeColors;

/**
 * @brief Структура, описывающая орфографическую ошибку.
 */
struct SpellError {
  int start;                    // Начальная позиция слова в символах (QString)
  int length;                   // Длина слова в символах
  QString word;                 // Исходное слово
  QVector<QString> suggestions; // Варианты исправлений
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
  explicit TextEditWithSpellCheck(const ThemeColors &colors,
                                  QWidget *parent = nullptr);
  ~TextEditWithSpellCheck();

  /**
   * @brief Устанавливает указатель на словарь.
   * @param vocab Словарь (должен существовать дольше, чем виджет)
   */
  void setVocabulary(Vocabulary *vocab);

  /**
   * @brief Устанавливает цвета из текущей темы
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
   * @brief Восстанавливает текст исходный текст.
   */
  void revertToOriginal();

  /**
   * @brief Очищает текст, сбрасывает все выделения и состояния.
   */
  void clearAll();

  /**
   * @brief Возвращает текущий текст.
   */
  QString getText() const;

  /**
   * @brief Устанавливает текст программы.
   */
  void setText(const QString &text);

  /**
   * Обновляет цвета подсветки
   */
  void updateColors();

protected:
  void mousePressEvent(QMouseEvent *event) override;

private slots:
  /**
   * Сбрасывает выделение при ручном изменении текста
   */
  void onTextChanged();

signals:
  void spellCheckCompleted(int errorCount);
  void canRevertChanged(bool canRevert);

private:
  /**
   * @brief Применяет регистр оригинального слова к исправленному слову
   * @param originalWord Оригинальное слово (с ошибкой)
   * @param correctedWord Исправленное слово (обычно в нижнем регистре)
   * @return Слово с сохранённым регистром
   */
  QString preserveCase(const QString &originalWord,
                       const QString &correctedWord);

  /**
   *Удаляет всё форматирование и очищает список ошибок
   */
  void clearSpellCheck();

  /**
   * Применяет красное подчёркивание к ошибкам
   */
  void highlightErrors();

  /**
   * Применяет мягкое выделение к указанным позициям
   */
  void highlightFixedPositions();

  /**
   * Добавляет слово в список игнорируемых
   */
  void addIgnoredWord(const QString &word);

  /**
   * Проверяет, игнорируется ли слово
   */
  bool isWordIgnored(const QString &word) const;

  /**
   * Находит все ошибки в тексте
   */
  QVector<SpellError> findErrors(const QString &text);

  /**
   * Заменяет слово в документе
   */
  void replaceWordAt(int start, int length, const QString &newWord);

  /**
   * Применяет форматирование к диапазону
   */
  void applyFormatToRange(int start, int length, const QTextCharFormat &format);

  /**
   * Очищает всё форматирование документа
   */
  void clearFormats();

  Vocabulary *vocab_ = nullptr;
  QVector<SpellError> errors_; // Текущие ошибки
  QSet<QString> ignoredWords_; // Слова, отмеченные пользователем как
                               // правильные (в нижнем регистре)
  QString originalText_;       // Текст, сохранённый для отмены
  bool hasOriginal_ = false;   // Флаг наличия сохранённого текста
  bool selfUpdating_ = false;  // Предотвращает рекурсивные вызовы при
                               // программном изменении текста

  QVector<QPair<int, int>> fixedPositions_; // Позиции (начало, длина) слов,
                                            // исправленных автоматически

  ThemeColors currentColors_; // Текущие цвета темы
};

#endif // TEXTEDITWITHSPELLCHECK_HPP
