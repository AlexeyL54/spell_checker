#include "TextEditWithSpellCheck.hpp"
#include "ThemeManager.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextCursor>

/**
 * @brief Конструктор виджета текстового редактора с проверкой орфографии.
 * @param colors Цветовая схема темы для подсветки ошибок.
 * @param parent Родительский виджет (по умолчанию nullptr).
 */
TextEditWithSpellCheck::TextEditWithSpellCheck(const ThemeColors &colors,
                                               QWidget *parent)
    : QPlainTextEdit(parent), currentColors_(colors), selfUpdating_(false) {
  connect(this, &QPlainTextEdit::textChanged, this,
          &TextEditWithSpellCheck::onTextChanged);
  setUndoRedoEnabled(true);
}

/**
 * @brief Деструктор виджета.
 */
TextEditWithSpellCheck::~TextEditWithSpellCheck() {}

/**
 * @brief Устанавливает указатель на словарь.
 * @param vocab Словарь (должен существовать дольше, чем виджет)
 */
void TextEditWithSpellCheck::setVocabulary(Vocabulary *vocab) {
  vocab_ = vocab;
}

/**
 * @brief Устанавливает цвета из текущей темы.
 * @param colors Цветовая схема из ThemeManager
 */
void TextEditWithSpellCheck::setThemeColors(const ThemeColors &colors) {
  currentColors_ = colors;
  updateColors();
}

/**
 * @brief Запускает проверку орфографии текущего текста.
 *
 * Сохраняет исходный текст для отмены, находит ошибки и выделяет их.
 */
void TextEditWithSpellCheck::performSpellCheck() {
  if (!vocab_) {
    qDebug() << "SpellCheck: vocabulary is null!";
    return;
  }

  // Сохраняем исходный текст для отмены, если ещё не сохранён
  if (!hasOriginal_) {
    originalText_ = toPlainText();
    hasOriginal_ = true;
    emit canRevertChanged(true);
  }

  // Очищаем предыдущее форматирование и списки
  clearSpellCheck();
  fixedPositions_.clear();

  QString text = toPlainText();
  errors_ = findErrors(text);
  highlightErrors();

  emit spellCheckCompleted(errors_.size());
}

/**
 * @brief Автоматически исправляет все найденные ошибки, используя первый
 * вариант из списка.
 *
 * После замены выделяет исправленные слова мягким цветом.
 */
void TextEditWithSpellCheck::applyFirstCorrections() {
  if (!vocab_) {
    qDebug() << "applyFirstCorrections: vocabulary is null!";
    return;
  }

  if (!hasOriginal_) {
    originalText_ = toPlainText();
    hasOriginal_ = true;
    emit canRevertChanged(true);
  }

  // Находим актуальные ошибки в текущем тексте
  QString text = toPlainText();
  QVector<SpellError> currentErrors = findErrors(text);
  if (currentErrors.isEmpty())
    return;

  selfUpdating_ = true;

  clearFormats();
  fixedPositions_.clear();

  // Сортируем ошибки по убыванию start (замена справа налево)
  std::sort(currentErrors.begin(), currentErrors.end(),
            [](const SpellError &a, const SpellError &b) {
              return a.start > b.start;
            });

  // Применяем замены и сразу применяем форматирование
  for (const SpellError &err : currentErrors) {
    if (err.suggestions.isEmpty())
      continue;

    QString newWord = err.suggestions.first();
    QString correctedWord = preserveCase(err.word, newWord);

    // Заменяем слово
    replaceWordAt(err.start, err.length, correctedWord);

    // Выделяем
    QTextCharFormat fixedFormat;
    fixedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    fixedFormat.setUnderlineColor(currentColors_.spellFixed);
    applyFormatToRange(err.start, correctedWord.length(), fixedFormat);
  }

  errors_.clear();
  selfUpdating_ = false;
  emit spellCheckCompleted(0);
}

/**
 * @brief Восстанавливает исходный текст.
 */
void TextEditWithSpellCheck::revertToOriginal() {
  if (!hasOriginal_)
    return;

  selfUpdating_ = true;
  setPlainText(originalText_);
  clearSpellCheck();
  fixedPositions_.clear();
  hasOriginal_ = false;
  emit canRevertChanged(false);
  selfUpdating_ = false;
}

/**
 * @brief Очищает текст, сбрасывает все выделения и состояния.
 */
void TextEditWithSpellCheck::clearAll() {
  selfUpdating_ = true;
  clear();
  clearSpellCheck();
  fixedPositions_.clear();
  hasOriginal_ = false;
  originalText_.clear();
  ignoredWords_.clear();
  emit canRevertChanged(false);
  emit spellCheckCompleted(0);
  selfUpdating_ = false;
}

/**
 * @brief Возвращает текущий текст.
 * @return Текст из редактора в виде строки.
 */
QString TextEditWithSpellCheck::getText() const { return toPlainText(); }

/**
 * @brief Устанавливает текст программно.
 * @param text Новый текст для установки.
 */
void TextEditWithSpellCheck::setText(const QString &text) {
  selfUpdating_ = true;
  setPlainText(text);
  clearSpellCheck();
  fixedPositions_.clear();
  hasOriginal_ = false;
  originalText_.clear();
  emit canRevertChanged(false);
  selfUpdating_ = false;
}

/**
 * @brief Обработчик нажатия кнопки мыши.
 * @param event Событие мыши.
 *
 * Переопределён для отображения контекстного меню с вариантами исправления
 * при клике на слово с орфографической ошибкой.
 */
void TextEditWithSpellCheck::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton) {
    QPlainTextEdit::mousePressEvent(event);
    return;
  }

  QTextCursor cursor = cursorForPosition(event->pos());
  int pos = cursor.position();

  SpellError *error = findErrorAtPosition(pos);
  if (error) {
    showCorrectionMenu(event->globalPosition().toPoint(), *error);
    event->accept();
    return;
  }

  QPlainTextEdit::mousePressEvent(event);
}

/**
 * @brief Находит ошибку по позиции курсора.
 * @param position Позиция в документе.
 * @return Указатель на найденную ошибку или nullptr, если ошибка не найдена.
 */
SpellError *TextEditWithSpellCheck::findErrorAtPosition(int position) {
  for (SpellError &err : errors_) {
    if (position >= err.start && position <= err.start + err.length) {
      return &err;
    }
  }
  return nullptr;
}

/**
 * @brief Показывает контекстное меню с вариантами исправления ошибки.
 * @param globalPos Глобальная позиция для отображения меню.
 * @param error Структура ошибки для исправления.
 */
void TextEditWithSpellCheck::showCorrectionMenu(const QPoint &globalPos,
                                                const SpellError &error) {
  QMenu menu;

  // Добавляем варианты исправлений
  for (const QString &suggestion : error.suggestions) {
    menu.addAction(suggestion, [this, error, suggestion]() {
      applyCorrection(error, suggestion);
    });
  }

  menu.addSeparator();
  menu.addAction("Отметить как правильное",
                 [this, error]() { ignoreWord(error); });

  menu.exec(globalPos);
}

/**
 * @brief Применяет выбранное исправление к ошибке.
 * @param error Структура ошибки.
 * @param suggestion Выбранный вариант исправления.
 */
void TextEditWithSpellCheck::applyCorrection(const SpellError &error,
                                             const QString &suggestion) {
  QString correctedWord = preserveCase(error.word, suggestion);

  selfUpdating_ = true;
  replaceWordAt(error.start, error.length, correctedWord);

  // Применяем форматирование для исправленного слова
  QTextCharFormat fixedFormat;
  fixedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  fixedFormat.setUnderlineColor(currentColors_.spellFixed);
  applyFormatToRange(error.start, correctedWord.length(), fixedFormat);

  selfUpdating_ = false;
}

/**
 * @brief Добавляет слово в игнорируемые и убирает подсветку ошибки.
 * @param error Структура ошибки.
 */
void TextEditWithSpellCheck::ignoreWord(const SpellError &error) {
  addIgnoredWord(error.word);

  selfUpdating_ = true;

  // Удаляем слово из списка ошибок
  removeErrorFromList(error.start, error.length);

  // Очищаем форматирование для этого слова
  QTextCharFormat defaultFormat;
  defaultFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);
  applyFormatToRange(error.start, error.length, defaultFormat);

  selfUpdating_ = false;
}

/**
 * @brief Удаляет ошибку из списка по позиции и длине.
 * @param start Начальная позиция.
 * @param length Длина слова.
 */
void TextEditWithSpellCheck::removeErrorFromList(int start, int length) {
  for (int i = 0; i < errors_.size(); ++i) {
    if (errors_[i].start == start && errors_[i].length == length) {
      errors_.removeAt(i);
      break;
    }
  }
}

/**
 * @brief Сбрасывает выделение при ручном изменении текста.
 *
 * Вызывается при любом изменении текста пользователем.
 * Очищает подсветку ошибок и список исправленных позиций.
 */
void TextEditWithSpellCheck::onTextChanged() {
  if (selfUpdating_)
    return;

  // При ручном изменении текста сбрасываем выделение и состояния
  // Но не сбрасываем originalText_, чтобы можно было отменить изменения
  clearSpellCheck();
  fixedPositions_.clear();
}

/**
 * @brief Применяет регистр оригинального слова к исправленному слову.
 * @param originalWord Оригинальное слово (с ошибкой)
 * @param correctedWord Исправленное слово (обычно в нижнем регистре)
 * @return Слово с сохранённым регистром
 */
QString TextEditWithSpellCheck::preserveCase(const QString &originalWord,
                                             const QString &correctedWord) {
  if (originalWord.isEmpty() || correctedWord.isEmpty()) {
    return correctedWord;
  }

  // Если оригинальное слово полностью в верхнем регистре
  if (originalWord == originalWord.toUpper()) {
    return correctedWord.toUpper();
  }

  // Если первая буква заглавная, а остальные строчные
  if (originalWord[0].isUpper() &&
      (originalWord.length() == 1 ||
       originalWord.mid(1) == originalWord.mid(1).toLower())) {
    if (!correctedWord.isEmpty()) {
      QString result = correctedWord;
      result[0] = result[0].toUpper();
      return result;
    }
  }

  // Если слово начинается с заглавной, но имеет смешанный регистр - сохраняем
  // только первую заглавную
  if (originalWord[0].isUpper()) {
    QString result = correctedWord;
    result[0] = result[0].toUpper();
    return result;
  }

  // В остальных случаях возвращаем исправленное слово как есть
  return correctedWord;
}

/**
 * @brief Удаляет всё форматирование и очищает список ошибок.
 */
void TextEditWithSpellCheck::clearSpellCheck() {
  clearFormats();
  errors_.clear();
}

/**
 * @brief Применяет красное волнистое подчёркивание к словам с ошибками.
 */
void TextEditWithSpellCheck::highlightErrors() {
  QTextCharFormat errorFormat;
  errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  errorFormat.setUnderlineColor(currentColors_.spellError);

  for (const SpellError &err : errors_) {
    applyFormatToRange(err.start, err.length, errorFormat);
  }
}

/**
 * @brief Применяет мягкое выделение к исправленным позициям.
 */
void TextEditWithSpellCheck::highlightFixedPositions() {
  QTextCharFormat fixedFormat;
  fixedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  fixedFormat.setUnderlineColor(currentColors_.spellFixed);

  for (const std::pair<int, int> &pos : fixedPositions_) {
    applyFormatToRange(pos.first, pos.second, fixedFormat);
  }
}

/**
 * @brief Добавляет слово в список игнорируемых.
 * @param word Слово для добавления (регистр не важен).
 */
void TextEditWithSpellCheck::addIgnoredWord(const QString &word) {
  ignoredWords_.insert(word.toLower());
}

/**
 * @brief Проверяет, игнорируется ли слово.
 * @param word Слово для проверки (регистр не важен).
 * @return true если слово в списке игнорируемых.
 */
bool TextEditWithSpellCheck::isWordIgnored(const QString &word) const {
  return ignoredWords_.contains(word.toLower());
}

/**
 * @brief Находит все орфографические ошибки в тексте.
 * @param text Текст для анализа.
 * @return Вектор структур SpellError с информацией об ошибках.
 */
QVector<SpellError> TextEditWithSpellCheck::findErrors(const QString &text) {
  QVector<SpellError> result;

  if (!vocab_)
    return result;

  QRegularExpression wordRegex(R"([А-Яа-яЁёA-Za-z]+)");
  QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    int start = match.capturedStart();
    int length = match.capturedLength();
    QString word = match.captured();

    // Пропускаем игнорируемые слова
    if (isWordIgnored(word))
      continue;

    QString lowerWord = word.toLower();

    // Проверяем наличие в словаре
    if (vocab_->isInVocab(lowerWord)) {
      continue;
    }

    try {
      QVector<QString> suggestions = vocab_->checkWordSpelling(lowerWord);

      if (!suggestions.isEmpty()) {
        // Сохраняем оригинальное слово (с сохранением регистра)
        result.append({start, length, word, suggestions});
      }
    } catch (const std::exception &e) {
      continue;
    } catch (...) {
      continue;
    }
  }

  return result;
}

/**
 * @brief Заменяет слово в документе по указанной позиции.
 * @param start Начальная позиция заменяемого слова.
 * @param length Длина заменяемого слова.
 * @param newWord Новое слово для вставки.
 */
void TextEditWithSpellCheck::replaceWordAt(int start, int length,
                                           const QString &newWord) {
  QTextCursor cursor(document());
  cursor.setPosition(start);
  cursor.setPosition(start + length, QTextCursor::KeepAnchor);

  // Проверяем, что позиции валидны
  if (cursor.position() == cursor.anchor())
    return;

  // Блокируем сигналы, чтобы не вызвать рекурсию
  bool oldState = document()->blockSignals(true);
  cursor.insertText(newWord);
  document()->blockSignals(oldState);
}

/**
 * @brief Применяет форматирование к указанному диапазону текста.
 * @param start Начальная позиция.
 * @param length Длина диапазона.
 * @param format Формат для применения.
 */
void TextEditWithSpellCheck::applyFormatToRange(int start, int length,
                                                const QTextCharFormat &format) {
  // Проверяем границы
  if (start < 0 || length <= 0)
    return;

  QTextDocument *doc = document();
  if (!doc)
    return;

  // -1 для символа конца документа
  int docLength = doc->characterCount() - 1;
  if (start >= docLength)
    return;

  // Корректируем длину, чтобы не выходить за пределы
  int actualLength = length;
  if (start + actualLength > docLength) {
    actualLength = docLength - start;
  }
  if (actualLength <= 0)
    return;

  // Применение форматирования
  QTextCursor cursor(doc);
  cursor.setPosition(start);
  cursor.setPosition(start + actualLength, QTextCursor::KeepAnchor);

  if (cursor.position() == cursor.anchor())
    return;

  doc->blockSignals(true);
  cursor.mergeCharFormat(format);
  doc->blockSignals(false);
}

/**
 * @brief Очищает всё форматирование документа.
 */
void TextEditWithSpellCheck::clearFormats() {
  QTextDocument *doc = document();
  if (!doc)
    return;

  // Очистка форматирования
  QTextCursor cursor(doc);
  cursor.select(QTextCursor::Document);

  // Создаём базовое форматирование без подчёркивания
  QTextCharFormat defaultFormat;
  defaultFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);

  doc->blockSignals(true);
  cursor.mergeCharFormat(defaultFormat);
  doc->blockSignals(false);
}

/**
 * @brief Обновляет цвета подсветки ошибок и исправлений.
 */
void TextEditWithSpellCheck::updateColors() {
  if (!errors_.isEmpty()) {
    highlightErrors();
  }
  if (!fixedPositions_.isEmpty()) {
    highlightFixedPositions();
  }
}
