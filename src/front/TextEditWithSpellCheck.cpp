#include "TextEditWithSpellCheck.hpp"
#include "../back/unistring.hpp"
#include "../back/vocab.hpp"
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
    replaceWordAt(err.start, err.length, newWord);
    // Запоминаем позицию замены
    fixedPositions_.append(qMakePair(err.start, newWord.length()));
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
          menu.addAction(sugg, [this, err, sugg]() {
            // Заменяем слово на выбранный вариант
            selfUpdating_ = true;
            replaceWordAt(err.start, err.length, sugg);
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

void TextEditWithSpellCheck::clearSpellCheck() {
  clearFormats();
  errors_.clear();
}

void TextEditWithSpellCheck::highlightErrors() {
  QTextCharFormat errorFormat;
  errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  errorFormat.setUnderlineColor(Qt::red);

  for (const SpellError &err : errors_) {
    applyFormatToRange(err.start, err.length, errorFormat);
  }
}

void TextEditWithSpellCheck::highlightFixedPositions() {
  QTextCharFormat fixedFormat;
  fixedFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
  fixedFormat.setUnderlineColor(QColor(100, 150, 255)); // мягкий синий

  for (const auto &pos : fixedPositions_) {
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

  // Регулярное выражение для слов: кириллица и латиница
  QRegularExpression wordRegex(R"(\b[а-яА-ЯёЁa-zA-Z]+\b)");
  QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    int start = match.capturedStart();
    int length = match.capturedLength();
    QString word = match.captured();

    // Пропускаем игнорируемые слова
    if (isWordIgnored(word))
      continue;

    // Преобразуем QString в UTF-8 std::string и затем в Unistring
    std::string utf8Word = word.toUtf8().toStdString();
    utf8::Unistring ustr(utf8Word);

    try {
      if (vocab_->isInVocab(ustr))
        continue; // слово есть в словаре

      // Получаем исправления
      std::vector<utf8::Unistring> suggestionsUtf8 =
          vocab_->checkWordSpelling(ustr);
      QVector<QString> suggestions;
      for (const auto &su : suggestionsUtf8) {
        QString sug = QString::fromUtf8(su.to_string().c_str());
        suggestions.append(sug);
      }

      if (!suggestions.isEmpty()) {
        result.append({start, length, word, suggestions});
      }
    } catch (const std::exception &e) {
      qDebug() << "Exception in findErrors:" << e.what();
      continue;
    } catch (...) {
      qDebug() << "Unknown exception in findErrors";
      continue;
    }
  }

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
