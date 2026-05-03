#include "TextEditWithSpellCheck.hpp"
#include "../back/unistring.hpp"
#include "../back/vocab.hpp"
#include "ThemeManager.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QTextCursor>

TextEditWithSpellCheck::TextEditWithSpellCheck(QWidget *parent)
    : QPlainTextEdit(parent), selfUpdating_(false) {
  connect(this, &QPlainTextEdit::textChanged, this,
          &TextEditWithSpellCheck::onTextChanged);
  setUndoRedoEnabled(true);
}

TextEditWithSpellCheck::~TextEditWithSpellCheck() {}

void TextEditWithSpellCheck::setVocabulary(Vocabulary *vocab) {
  vocab_ = vocab;
}

void TextEditWithSpellCheck::setThemeColors(const ThemeColors &colors) {
  currentColors_ = colors;
  updateColors();
}

void TextEditWithSpellCheck::performSpellCheck() {
  if (!vocab_) {
    qDebug() << "SpellCheck: vocabulary is null!";
    return;
  }

  // Сохраняем исходный текст для отмены, если ещё не сохранён
  if (!hasOriginal_) {
    originalText_ = toPlainText();
    hasOriginal_ = true;
  }

  // Очищаем предыдущее форматирование и списки
  clearSpellCheck();
  fixedPositions_.clear();

  QString text = toPlainText();
  errors_ = findErrors(text);
  highlightErrors();

  qDebug() << "Spell check completed, found" << errors_.size() << "errors";
}

void TextEditWithSpellCheck::applyFirstCorrections() {
  if (!vocab_) {
    qDebug() << "applyFirstCorrections: vocabulary is null!";
    return;
  }

  // Сохраняем исходный текст для отмены, если ещё не сохранён
  if (!hasOriginal_) {
    originalText_ = toPlainText();
    hasOriginal_ = true;
  }

  // Находим актуальные ошибки в текущем тексте
  QString text = toPlainText();
  QVector<SpellError> currentErrors = findErrors(text);
  if (currentErrors.isEmpty())
    return;

  // Заменяем слова с конца, чтобы не смещать позиции
  selfUpdating_ = true;
  fixedPositions_.clear();

  for (int i = currentErrors.size() - 1; i >= 0; --i) {
    const SpellError &err = currentErrors[i];
    if (err.suggestions.isEmpty())
      continue;

    QString newWord = err.suggestions.first();
    // Сохраняем регистр оригинального слова
    QString correctedWord = preserveCase(err.word, newWord);
    replaceWordAt(err.start, err.length, correctedWord);
    // Запоминаем позицию замены
    fixedPositions_.append(qMakePair(err.start, correctedWord.length()));
  }

  // Очищаем все форматы и применяем мягкое выделение к исправленным словам
  clearFormats();
  highlightFixedPositions();

  // Очищаем список ошибок, так как после исправления они больше не актуальны
  errors_.clear();

  selfUpdating_ = false;
}

void TextEditWithSpellCheck::revertToOriginal() {
  if (!hasOriginal_)
    return;

  selfUpdating_ = true;
  setPlainText(originalText_);
  clearSpellCheck();
  fixedPositions_.clear();
  hasOriginal_ = false; // После отмены оригинал более не действителен
  selfUpdating_ = false;
}

void TextEditWithSpellCheck::clearAll() {
  selfUpdating_ = true;
  clear();
  clearSpellCheck();
  fixedPositions_.clear();
  hasOriginal_ = false;
  originalText_.clear();
  ignoredWords_.clear();
  selfUpdating_ = false;
}

QString TextEditWithSpellCheck::getText() const { return toPlainText(); }

void TextEditWithSpellCheck::setText(const QString &text) {
  selfUpdating_ = true;
  setPlainText(text);
  clearSpellCheck();
  fixedPositions_.clear();
  hasOriginal_ = false;
  selfUpdating_ = false;
}

void TextEditWithSpellCheck::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QTextCursor cursor = cursorForPosition(event->pos());
    int pos = cursor.position();

    // Ищем ошибку, содержащую эту позицию
    for (const SpellError &err : errors_) {
      if (pos >= err.start && pos <= err.start + err.length) {
        // Показываем меню с вариантами
        QMenu menu;
        for (const QString &sugg : err.suggestions) {
          // Захватываем err и sugg по значению для лямбды
          menu.addAction(sugg, [this, err, sugg]() {
            // Сохраняем регистр оригинального слова
            QString correctedWord = preserveCase(err.word, sugg);
            selfUpdating_ = true;
            replaceWordAt(err.start, err.length, correctedWord);
            // После замены перезапускаем проверку для обновления выделения
            performSpellCheck();
            selfUpdating_ = false;
          });
        }
        menu.addSeparator();
        menu.addAction("Отметить как правильное", [this, err]() {
          // Добавляем слово в игнорируемые
          addIgnoredWord(err.word);
          // Перезапускаем проверку, чтобы убрать выделение со всех вхождений
          // этого слова
          performSpellCheck();
        });
        menu.exec(event->globalPosition().toPoint());
        event->accept();
        return;
      }
    }
  }
  QPlainTextEdit::mousePressEvent(event);
}

void TextEditWithSpellCheck::onTextChanged() {
  if (selfUpdating_)
    return;

  // При ручном изменении текста сбрасываем выделение и состояния
  // Но не сбрасываем originalText_, чтобы можно было отменить изменения
  clearSpellCheck();
  fixedPositions_.clear();
}

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

  // Если слово начинается с заглавной, но имеет смешанный регистр
  // (например, "ПрИмЕр") - сохраняем только первую заглавную
  if (originalWord[0].isUpper()) {
    QString result = correctedWord;
    result[0] = result[0].toUpper();
    return result;
  }

  // В остальных случаях возвращаем исправленное слово как есть (обычно в нижнем
  // регистре)
  return correctedWord;
}

void TextEditWithSpellCheck::clearSpellCheck() {
  clearFormats();
  errors_.clear();
}

void TextEditWithSpellCheck::highlightErrors() {
  QTextCharFormat errorFormat;
  errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  errorFormat.setUnderlineColor(currentColors_.spellError);

  for (const SpellError &err : errors_) {
    applyFormatToRange(err.start, err.length, errorFormat);
  }
}

void TextEditWithSpellCheck::highlightFixedPositions() {
  QTextCharFormat fixedFormat;
  fixedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  fixedFormat.setUnderlineColor(currentColors_.spellFixed);

  for (const std::pair<int, int> &pos : fixedPositions_) {
    applyFormatToRange(pos.first, pos.second, fixedFormat);
  }
}

void TextEditWithSpellCheck::addIgnoredWord(const QString &word) {
  ignoredWords_.insert(word.toLower());
}

bool TextEditWithSpellCheck::isWordIgnored(const QString &word) const {
  return ignoredWords_.contains(word.toLower());
}

QVector<SpellError> TextEditWithSpellCheck::findErrors(const QString &text) {
  QVector<SpellError> result;

  if (!vocab_)
    return result;

  QRegularExpression wordRegex(
      R"((?<=^|\s|[^\p{L}])[а-яА-ЯёЁa-zA-Z]+(?=$|\s|[^\p{L}]))");

  QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    int start = match.capturedStart();
    int length = match.capturedLength();
    QString word = match.captured();

    qDebug() << "word: " << word;

    // Пропускаем игнорируемые слова (сравниваем в нижнем регистре)
    if (isWordIgnored(word))
      continue;

    QString lowerWord = word.toLower();
    std::string utf8Word = lowerWord.toUtf8().toStdString();
    utf8::Unistring ustr(utf8Word);

    // Проверяем наличие в словаре (сравниваем в нижнем регистре)
    if (vocab_->isInVocab(ustr)) {
      qDebug() << "слово есть в словаре, пропускаем";
      continue;
    }

    qDebug() << "слово не найдено, ищем исправления для: " << ustr.to_string();

    try {
      std::vector<utf8::Unistring> suggestionsUtf8 =
          vocab_->checkWordSpelling(ustr);

      QVector<QString> suggestions;
      for (const Unistring &su : suggestionsUtf8) {
        QString sug = QString::fromUtf8(su.to_string().c_str());
        suggestions.append(sug);
        qDebug() << "предложение: " << sug;
      }

      if (!suggestions.isEmpty()) {
        // Сохраняем оригинальное слово (с сохранением регистра)
        result.append({start, length, word, suggestions});
      } else {
        qDebug() << "нет предложений для: " << word;
      }
    } catch (const std::exception &e) {
      qDebug() << "Exception in findErrors:" << e.what();
      continue;
    } catch (...) {
      qDebug() << "Unknown exception in findErrors";
      continue;
    }
  }

  qDebug() << "Найдено ошибок:" << result.size();
  return result;
}

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

void TextEditWithSpellCheck::applyFormatToRange(int start, int length,
                                                const QTextCharFormat &format) {
  // Проверяем границы
  if (start < 0 || length <= 0)
    return;

  QTextDocument *doc = document();
  if (!doc)
    return;

  int docLength = doc->characterCount() - 1; // -1 для символа конца документа
  if (start >= docLength)
    return;

  // Корректируем длину, чтобы не выходить за пределы
  int actualLength = length;
  if (start + actualLength > docLength) {
    actualLength = docLength - start;
  }
  if (actualLength <= 0)
    return;

  // Безопасное применение формата
  QTextCursor cursor(doc);
  cursor.setPosition(start);
  cursor.setPosition(start + actualLength, QTextCursor::KeepAnchor);

  // Проверяем, что позиции валидны
  if (cursor.position() == cursor.anchor())
    return;

  // Блокируем сигналы, чтобы избежать рекурсии
  doc->blockSignals(true);
  cursor.mergeCharFormat(format);
  doc->blockSignals(false);
}

void TextEditWithSpellCheck::clearFormats() {
  QTextDocument *doc = document();
  if (!doc)
    return;

  // Безопасная очистка всех форматов
  QTextCursor cursor(doc);
  cursor.select(QTextCursor::Document);

  // Создаём базовый формат без подчёркивания
  QTextCharFormat defaultFormat;
  defaultFormat.setUnderlineStyle(QTextCharFormat::NoUnderline);

  // Применяем безопасно - оборачиваем в блокировку обновлений
  doc->blockSignals(true);
  cursor.mergeCharFormat(defaultFormat);
  doc->blockSignals(false);
}

void TextEditWithSpellCheck::updateColors() {
  // Перерисовываем ошибки с новыми цветами, если они есть
  if (!errors_.isEmpty()) {
    highlightErrors();
  }
  if (!fixedPositions_.isEmpty()) {
    highlightFixedPositions();
  }
}
