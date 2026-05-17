#include "InputPage.hpp"
#include "InstructionText.hpp"
#include "ThemeManager.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

/**
 * @brief Конструктор страницы ввода.
 * @param colors Цветовая схема темы оформления.
 * @param parent Родительский виджет (по умолчанию nullptr).
 */
InputPage::InputPage(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), currentColors_(colors), lastErrorCount_(-1) {
  setupMainLayout();
  setupContentRow();
  setupContentColumn();
  setupIntroText();
  setupInputChoice();
  setupStatusBar();
  setupKeyboardPage();
  setupFilePage();
  setupInstructionButton();
  setupConnections();

  contentLayout->addWidget(stack, 1);
  contentRow->addWidget(contentColumn, 1);
  mainLayout->addLayout(contentRow, 1);
}

/**
 * @brief Получить указатель на текстовый редактор с проверкой орфографии.
 * @return Указатель на TextEditWithSpellCheck.
 */
TextEditWithSpellCheck *InputPage::getTextEdit() const { return textInput; }

/**
 * @brief Определить, активен ли режим ввода с клавиатуры.
 * @return true если выбран ввод с клавиатуры, false если выбран файл.
 */
bool InputPage::isKeyboardMode() const { return radioKeyboard->isChecked(); }

/**
 * @brief Получить путь к выбранному файлу.
 * @return Строка с путем к файлу.
 */
QString InputPage::getFilePath() const { return filePathEdit->text(); }

/**
 * @brief Загрузить текст из файла и переключиться в режим клавиатуры.
 * @return true если загрузка успешна, false в противном случае.
 */
bool InputPage::loadAndSwitchToKeyboard() {
  QString path = filePathEdit->text();
  if (path.isEmpty()) {
    QMessageBox::warning(this, "Ошибка", "Файл не выбран.");
    return false;
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
    return false;
  }

  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  QString content = stream.readAll();
  file.close();

  textInput->setText(content);
  radioKeyboard->setChecked(true);
  return true;
}

/**
 * @brief Настроить главный компоновщик страницы.
 */
void InputPage::setupMainLayout() {
  mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
}

/**
 * @brief Настроить строку с контентом.
 */
void InputPage::setupContentRow() {
  contentRow = new QHBoxLayout();
  contentRow->setSpacing(0);
}

/**
 * @brief Настроить колонку с контентом.
 */
void InputPage::setupContentColumn() {
  contentColumn = new QWidget(this);
  contentLayout = new QVBoxLayout(contentColumn);
  contentLayout->setContentsMargins(24, 24, 24, 24);
  contentLayout->setSpacing(16);
}

/**
 * @brief Настроить вступительный текст.
 */
void InputPage::setupIntroText() {
  introText = new QLabel("Вы можете ввести текст с клавиатуры или "
                         "выбрать текстовый файл с диска.",
                         contentColumn);
  introText->setWordWrap(true);
  contentLayout->addWidget(introText);
}

/**
 * @brief Настроить переключатели выбора источника ввода.
 */
void InputPage::setupInputChoice() {
  inputChoiceGroup = new QButtonGroup(contentColumn);
  radioKeyboard = new QRadioButton("С клавиатуры", contentColumn);
  radioFile = new QRadioButton("Из файла", contentColumn);
  radioKeyboard->setObjectName("radioKeyboard");
  inputChoiceGroup->addButton(radioKeyboard);
  inputChoiceGroup->addButton(radioFile);

  // Создаём контейнер для радио-кнопок с объектным именем
  QWidget *choiceContainer = new QWidget(contentColumn);
  choiceContainer->setObjectName("inputChoiceWidget");
  QVBoxLayout *choiceRow = new QVBoxLayout(choiceContainer);
  choiceRow->setSpacing(8);
  choiceRow->addWidget(radioKeyboard);
  choiceRow->addWidget(radioFile);
  contentLayout->addWidget(choiceContainer);

  stack = new QStackedWidget(contentColumn);
}

/**
 * @brief Настроить строку состояния.
 */
void InputPage::setupStatusBar() {
  statusBar = new QStatusBar(this);
  statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  statusBar->setFixedHeight(35);
  statusBar->setObjectName("bottomStatusBar");

  // Кнопки
  btnCheck = new QPushButton("Проверить", this);
  btnCheck->setCursor(Qt::PointingHandCursor);

  btnFix = new QPushButton("Исправить", this);
  btnFix->setCursor(Qt::PointingHandCursor);
  btnFix->setEnabled(false);

  btnRevert = new QPushButton("Отменить изменения", this);
  btnRevert->setCursor(Qt::PointingHandCursor);
  btnRevert->setEnabled(false);

  btnClear = new QPushButton("Очистить", this);
  btnClear->setCursor(Qt::PointingHandCursor);

  btnCopy = new QPushButton("Копировать", this);
  btnCopy->setCursor(Qt::PointingHandCursor);

  btnSave = new QPushButton("Сохранить", this);
  btnSave->setCursor(Qt::PointingHandCursor);

  // Добавляем кнопки с разделителями
  statusBar->addWidget(btnCheck);
  statusBar->addWidget(btnFix);
  statusBar->addWidget(btnRevert);

  // Растягивающийся промежуток между группами кнопок
  QWidget *spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  statusBar->addWidget(spacer, 1);

  statusBar->addWidget(btnClear);
  statusBar->addWidget(btnCopy);
  statusBar->addWidget(btnSave);

  // Информационные индикаторы
  statusInfo = new QLabel("Готов к работе", this);
  statusInfo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusBar->addPermanentWidget(statusInfo);

  QLabel *encodingLabel = new QLabel("UTF-8", this);
  encodingLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusBar->addPermanentWidget(encodingLabel);

  statsLabel = new QLabel("0 строк, 0 символов", this);
  statsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  statusBar->addPermanentWidget(statsLabel);
}

/**
 * @brief Настроить страницу ввода с клавиатуры.
 */
void InputPage::setupKeyboardPage() {
  pageKeyboard = new QWidget(contentColumn);
  QVBoxLayout *layoutKeyboard = new QVBoxLayout(pageKeyboard);
  layoutKeyboard->setContentsMargins(0, 0, 0, 0);
  layoutKeyboard->setSpacing(0);

  textInput = new TextEditWithSpellCheck(currentColors_, contentColumn);
  textInput->setPlaceholderText("Введите текст для анализа...");
  textInput->setMinimumHeight(200);
  layoutKeyboard->addWidget(textInput);

  // Добавляем statusBar вниз страницы клавиатуры
  if (statusBar) {
    layoutKeyboard->addWidget(statusBar);
  }

  stack->addWidget(pageKeyboard);
}

/**
 * @brief Настроить страницу выбора файла.
 */
void InputPage::setupFilePage() {
  pageFile = new QWidget(contentColumn);
  QVBoxLayout *layoutFile = new QVBoxLayout(pageFile);

  filePathEdit = new QLineEdit(contentColumn);
  filePathEdit->setReadOnly(true);
  filePathEdit->setPlaceholderText("Файл не выбран");
  layoutFile->addWidget(filePathEdit);

  QHBoxLayout *fileButtonsLayout = new QHBoxLayout();
  fileButtonsLayout->setSpacing(12);

  btnSelectFile = new QPushButton("Выбрать файл", contentColumn);
  btnSelectFile->setObjectName("btnSelectFile"); // Добавляем объектное имя
  btnSelectFile->setCursor(Qt::PointingHandCursor);

  btnLoadFile = new QPushButton("Загрузить", contentColumn);
  btnLoadFile->setObjectName("btnLoadFile"); // Добавляем объектное имя
  btnLoadFile->setCursor(Qt::PointingHandCursor);
  btnLoadFile->setEnabled(false);

  fileButtonsLayout->addWidget(btnSelectFile);
  fileButtonsLayout->addWidget(btnLoadFile);
  fileButtonsLayout->addStretch();
  layoutFile->addLayout(fileButtonsLayout);

  stack->addWidget(pageFile);
}

/**
 * @brief Настроить соединения сигналов и слотов.
 */
void InputPage::setupConnections() {
  connect(btnSelectFile, &QPushButton::clicked, this,
          &InputPage::onFileSelected);
  connect(btnLoadFile, &QPushButton::clicked, this, &InputPage::onLoadFile);
  connect(radioKeyboard, &QRadioButton::toggled, this,
          &InputPage::onSourceToggled);
  connect(radioFile, &QRadioButton::toggled, this, &InputPage::onSourceToggled);
  connect(btnCheck, &QPushButton::clicked, this, &InputPage::checkRequested);
  connect(btnFix, &QPushButton::clicked, this, &InputPage::fixRequested);
  connect(btnRevert, &QPushButton::clicked, this, &InputPage::revertRequested);
  connect(btnClear, &QPushButton::clicked, this, &InputPage::clearRequested);
  connect(btnCopy, &QPushButton::clicked, this, &InputPage::copyRequested);
  connect(btnSave, &QPushButton::clicked, this, &InputPage::saveRequested);
  connect(textInput, &QPlainTextEdit::textChanged, this,
          &InputPage::onTextChanged);
  connect(textInput, &TextEditWithSpellCheck::canRevertChanged, this,
          &InputPage::onCanRevertChanged);

  radioKeyboard->setChecked(true);
  stack->setCurrentIndex(0);

  // Инициализируем состояние кнопок
  updateButtonsState();
}

/**
 * @brief Обновить состояние строки статуса.
 * @param errorCount Количество ошибок (-1 для состояния редактирования).
 */
void InputPage::updateStatusBar(int errorCount) {
  if (!statsLabel || !statusInfo)
    return;

  // Обновляем статистику текста
  QString text = textInput->toPlainText();
  int lines = text.split('\n').size();
  int chars = text.length();
  statsLabel->setText(QString("%1 строк, %2 символов").arg(lines).arg(chars));

  // Обновляем состояние кнопок
  updateButtonsState();

  // Обновляем статус проверки (только для информационных сообщений)
  if (errorCount < 0) {
    statusInfo->setText("Редактирование...");
    lastErrorCount_ = -1;
  } else if (errorCount == 0) {
    statusInfo->setText("Ошибок не найдено");
    lastErrorCount_ = 0;
  } else {
    statusInfo->setText(QString("Найдено ошибок: %1").arg(errorCount));
    lastErrorCount_ = errorCount;
  }
}

/**
 * @brief Обновить состояние кнопок в зависимости от наличия текста.
 */
void InputPage::updateButtonsState() {
  if (!btnCheck || !btnFix || !btnRevert)
    return;

  // Получаем текст и проверяем наличие букв
  QString text = textInput->toPlainText();
  bool hasLetters = false;

  for (QChar ch : text) {
    if (ch.isLetter()) {
      hasLetters = true;
      break;
    }
  }

  // Кнопки "Проверить" и "Исправить" активны, если есть буквы
  btnCheck->setEnabled(hasLetters);
  btnFix->setEnabled(hasLetters);
  btnCopy->setEnabled(hasLetters);
  btnSave->setEnabled(hasLetters);
  btnClear->setEnabled(hasLetters);

  // Кнопка "Отменить изменения" управляется своим сигналом
}

/**
 * @brief Обработчик изменения возможности отмены изменений.
 * @param canRevert true если доступна отмена изменений.
 */
void InputPage::onCanRevertChanged(bool canRevert) {
  if (btnRevert) {
    btnRevert->setEnabled(canRevert);
  }
}

/**
 * @brief Обработчик завершения проверки орфографии.
 * @param errorCount Количество найденных ошибок.
 */
void InputPage::onSpellCheckCompleted(int errorCount) {
  updateStatusBar(errorCount);
}

/**
 * @brief Обработчик изменения текста в редакторе.
 */
void InputPage::onTextChanged() { updateStatusBar(-1); }

/**
 * @brief Обработчик выбора файла через диалог.
 */
void InputPage::onFileSelected() {
  QString path =
      QFileDialog::getOpenFileName(this, "Выберите файл", QDir::homePath(),
                                   "Текстовые файлы (*.txt);;Все файлы (*.*)");
  if (!path.isEmpty()) {
    filePathEdit->setText(path);
    if (btnLoadFile)
      btnLoadFile->setEnabled(true);
  }
}

/**
 * @brief Обработчик загрузки текста из файла.
 */
void InputPage::onLoadFile() {
  QString path = filePathEdit->text();
  if (path.isEmpty()) {
    QMessageBox::warning(this, "Ошибка", "Файл не выбран.");
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
    return;
  }

  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  QString content = stream.readAll();
  file.close();

  textInput->setText(content);
  radioKeyboard->setChecked(true);
}

/**
 * @brief Обработчик переключения источника ввода.
 */
void InputPage::onSourceToggled() {
  if (radioKeyboard->isChecked()) {
    stack->setCurrentIndex(0);
  } else {
    stack->setCurrentIndex(1);
  }
}

/**
 * @brief Настроить кнопку вызова инструкции.
 */
void InputPage::setupInstructionButton() {
  btnInstruction = new QPushButton("?", this);
  btnInstruction->setObjectName("instructionButton");
  int buttonSize = 28;
  btnInstruction->setFixedSize(buttonSize, buttonSize);
  btnInstruction->setCursor(Qt::PointingHandCursor);
  connect(btnInstruction, &QPushButton::clicked, this,
          &InputPage::showInstruction);
  btnInstruction->setEnabled(false);
}

/**
 * @brief Показать диалоговое окно с инструкцией.
 */
void InputPage::showInstruction() {
  QDialog *dialog = new QDialog(this);
  dialog->setWindowTitle("Инструкция по использованию");
  dialog->setMinimumWidth(600);
  dialog->setMinimumHeight(500);

  QVBoxLayout *layout = new QVBoxLayout(dialog);

  QTextEdit *textEdit = new QTextEdit(dialog);
  textEdit->setPlainText(getInstructionText());
  textEdit->setReadOnly(true);
  textEdit->setFrameShape(QFrame::NoFrame);

  QFont font;
  font.setFamily("Consolas");
  font.setPointSize(10);
  textEdit->setFont(font);

  QPalette pal = textEdit->palette();
  pal.setColor(QPalette::Base, QColor(30, 30, 35));
  pal.setColor(QPalette::Text, QColor(220, 220, 220));
  textEdit->setPalette(pal);

  layout->addWidget(textEdit);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  QPushButton *btnClose = new QPushButton("Закрыть", dialog);
  btnClose->setCursor(Qt::PointingHandCursor);
  btnClose->setFixedWidth(100);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btnClose);
  buttonLayout->addStretch();
  layout->addLayout(buttonLayout);

  connect(btnClose, &QPushButton::clicked, dialog, &QDialog::accept);
  dialog->exec();
  delete dialog;
}

/**
 * @brief Обновить цветовую схему страницы.
 * @param colors Новая цветовая схема.
 */
void InputPage::updateThemeColors(const ThemeColors &colors) {
  currentColors_ = colors;
  // Цвета обновляются глобально через MainWindow::applyTheme
  if (textInput) {
    textInput->setThemeColors(colors);
  }
}
