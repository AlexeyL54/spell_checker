#ifndef TEXTEDITWITHSPELLCHECK_HPP
#define TEXTEDITWITHSPELLCHECK_HPP

#include <QPlainTextEdit>
#include <QSet>
#include <QVector>
#include <QString>

class Vocabulary;

/**
 * @brief Структура, описывающая орфографическую ошибку.
 */
struct SpellError {
    int start;                  ///< Начальная позиция слова в символах (QString)
    int length;                 ///< Длина слова в символах
    QString word;               ///< Исходное слово
    QVector<QString> suggestions; ///< Варианты исправлений
};

/**
 * @brief Виджет QPlainTextEdit с проверкой орфографии.
 *
 * Позволяет выделять ошибочные слова, предлагать исправления,
 * автоматически исправлять все ошибки, отменять изменения и копировать/сохранять текст.
 */
class TextEditWithSpellCheck : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit TextEditWithSpellCheck(QWidget *parent = nullptr);
    ~TextEditWithSpellCheck();

    /**
     * @brief Устанавливает указатель на словарь.
     * @param vocab Словарь (должен существовать дольше, чем виджет)
     */
    void setVocabulary(Vocabulary* vocab);

    /**
     * @brief Запускает проверку орфографии текущего текста.
     *
     * Сохраняет исходный текст для отмены, находит ошибки и выделяет их.
     */
    void performSpellCheck();

    /**
     * @brief Автоматически исправляет все найденные ошибки, используя первый вариант из списка.
     *
     * После замены выделяет исправленные слова мягким цветом.
     */
    void applyFirstCorrections();

    /**
     * @brief Отменяет последнюю операцию проверки или исправления.
     *
     * Восстанавливает текст, который был до вызова performSpellCheck() или applyFirstCorrections().
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
    void setText(const QString& text);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onTextChanged();   ///< Сбрасывает выделение при ручном изменении текста

private:
    void clearSpellCheck();                     ///< Удаляет всё форматирование и очищает список ошибок
    void highlightErrors();                     ///< Применяет красное подчёркивание к ошибкам
    void highlightFixedPositions();             ///< Применяет мягкое выделение к указанным позициям
    void addIgnoredWord(const QString& word);   ///< Добавляет слово в список игнорируемых
    bool isWordIgnored(const QString& word) const; ///< Проверяет, игнорируется ли слово
    QVector<SpellError> findErrors(const QString& text); ///< Находит все ошибки в тексте
    void replaceWordAt(int start, int length, const QString& newWord); ///< Заменяет слово в документе
    void applyFormatToRange(int start, int length, const QTextCharFormat& format); ///< Применяет формат к диапазону
    void clearFormats();                        ///< Очищает всё форматирование документа

    Vocabulary* vocab_ = nullptr;
    QVector<SpellError> errors_;                ///< Текущие ошибки
    QSet<QString> ignoredWords_;                ///< Слова, отмеченные пользователем как правильные (в нижнем регистре)
    QString originalText_;                      ///< Текст, сохранённый для отмены
    bool hasOriginal_ = false;                  ///< Флаг наличия сохранённого текста
    bool selfUpdating_ = false;                 ///< Предотвращает рекурсивные вызовы при программном изменении текста

    QVector<QPair<int, int>> fixedPositions_;   ///< Позиции (начало, длина) слов, исправленных автоматически
};

#endif // TEXTEDITWITHSPELLCHECK_HPP
