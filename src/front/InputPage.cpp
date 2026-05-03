#include "InputPage.hpp"
#include "InstructionText.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

InputPage::InputPage(QWidget *parent) : QWidget(parent), lastErrorCount_(-1) {
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

TextEditWithSpellCheck *InputPage::getTextEdit() const { return textInput; }

bool InputPage::isKeyboardMode() const { return radioKeyboard->isChecked(); }

QString InputPage::getFilePath() const { return filePathEdit->text(); }

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

void InputPage::setupMainLayout() {
  mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
}

void InputPage::setupContentRow() {
  contentRow = new QHBoxLayout();
  contentRow->setSpacing(0);
}

void InputPage::setupContentColumn() {
  contentColumn = new QWidget(this);
  contentLayout = new QVBoxLayout(contentColumn);
  contentLayout->setContentsMargins(24, 24, 24, 24);
  contentLayout->setSpacing(16);
}

void InputPage::setupIntroText() {
  introText = new QLabel("Вы можете ввести текст с клавиатуры или "
                         "выбрать текстовый файл с диска.",
                         contentColumn);
  introText->setWordWrap(true);
  contentLayout->addWidget(introText);
}

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

void InputPage::setupStatusBar() {
  statusBar = new QStatusBar(this);
  statusBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  statusBar->setFixedHeight(32);
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

void InputPage::setupKeyboardPage() {
  pageKeyboard = new QWidget(contentColumn);
  QVBoxLayout *layoutKeyboard = new QVBoxLayout(pageKeyboard);
  layoutKeyboard->setContentsMargins(0, 0, 0, 0);
  layoutKeyboard->setSpacing(0);

  textInput = new TextEditWithSpellCheck(contentColumn);
  textInput->setPlaceholderText("Введите текст для анализа...");
  textInput->setMinimumHeight(200);
  layoutKeyboard->addWidget(textInput);

  // Добавляем statusBar вниз страницы клавиатуры
  if (statusBar) {
    layoutKeyboard->addWidget(statusBar);
  }

  stack->addWidget(pageKeyboard);
}

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
  // (не меняем её состояние здесь)
}

void InputPage::onCanRevertChanged(bool canRevert) {
  if (btnRevert) {
    btnRevert->setEnabled(canRevert);
  }
}

void InputPage::onSpellCheckCompleted(int errorCount) {
  updateStatusBar(errorCount);
}

void InputPage::onTextChanged() { updateStatusBar(-1); }

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

void InputPage::onSourceToggled() {
  if (radioKeyboard->isChecked()) {
    stack->setCurrentIndex(0);
  } else {
    stack->setCurrentIndex(1);
  }
}

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

void InputPage::applyStatusBarStyle() {
  if (!statusBar)
    return;

  // Используем цвета из текущей темы, без привязки к светлой/тёмной
  QString styleSheet =
      QString(
          // Стиль для статус-бара
          "QStatusBar#bottomStatusBar {"
          "  background-color: %1;"
          "  border-top: 1px solid %2;"
          "  border-bottom: none;"
          "  padding: 0px;"
          "  margin: 0px;"
          "}"
          "QStatusBar::item {"
          "  border: none;"
          "}"

          // Кнопки в статус-баре
          "QStatusBar#bottomStatusBar QPushButton {"
          "  background-color: %1;"
          "  border: 1px solid %2;"
          "  border-radius: 4px;"
          "  padding: 5px 12px;"
          "  margin: 2px;"
          "  color: %3;"
          "  min-width: 80px;"
          "}"
          "QStatusBar#bottomStatusBar QPushButton:hover {"
          "  background-color: %4;"
          "  border-color: %5;"
          "}"
          "QStatusBar#bottomStatusBar QPushButton:pressed {"
          "  background-color: %6;"
          "  color: %3;"
          "}"
          "QStatusBar#bottomStatusBar QPushButton:disabled {"
          "  background-color: %1;"
          "  color: %7;"
          "  border-color: %2;"
          "}"

          // Метки в статус-баре
          "QStatusBar#bottomStatusBar QLabel {"
          "  background: transparent;"
          "  padding: 4px 6px;"
          "  margin: 2px;"
          "  color: %3;"
          "  font-size: 11px;"
          "}"

          // ========== СТИЛИ ДЛЯ ОСТАЛЬНЫХ ЭЛЕМЕНТОВ ==========

          // Кнопка "?" в верхней панели
          "QPushButton#instructionButton {"
          "  background-color: %5;"
          "  color: %3;"
          "  font-size: 16px;"
          "  font-weight: bold;"
          "  border: 1px solid %2;"
          "  border-radius: 14px;"
          "  padding: 0px;"
          "  min-width: 28px;"
          "  max-width: 28px;"
          "  min-height: 28px;"
          "  max-height: 28px;"
          "}"
          "QPushButton#instructionButton:hover {"
          "  background-color: %4;"
          "}"
          "QPushButton#instructionButton:pressed {"
          "  background-color: %6;"
          "}"

          // Стили для RadioButton (переключатели)
          "QRadioButton {"
          "  color: %3;"
          "  background-color: transparent;"
          "  spacing: 8px;"
          "  padding: 4px 0px;"
          "}"
          "QRadioButton::indicator {"
          "  width: 16px;"
          "  height: 16px;"
          "  border-radius: 8px;"
          "  border: 2px solid %2;"
          "  background-color: %1;"
          "}"
          "QRadioButton::indicator:hover {"
          "  border-color: %5;"
          "}"
          "QRadioButton::indicator:checked {"
          "  background-color: %5;"
          "  border-color: %5;"
          "}"
          "QRadioButton:disabled {"
          "  color: %7;"
          "}"
          "QRadioButton:disabled::indicator {"
          "  border-color: %2;"
          "  background-color: %8;"
          "}"

          // Стили для области выбора способа ввода (группа радио-кнопок)
          "QWidget#inputChoiceWidget {"
          "  background-color: %1;"
          "  border: 1px solid %2;"
          "  border-radius: 6px;"
          "  padding: 8px;"
          "}"

          // Стили для текстового поля ввода
          "QPlainTextEdit {"
          "  background-color: %1;"
          "  color: %3;"
          "  border: 1px solid %2;"
          "  border-radius: 4px;"
          "  padding: 8px;"
          "  selection-background-color: %5;"
          "  selection-color: %3;"
          "}"
          "QPlainTextEdit:focus {"
          "  border-color: %5;"
          "}"
          "QPlainTextEdit:disabled {"
          "  background-color: %8;"
          "  color: %7;"
          "  border-color: %2;"
          "}"

          // Стили для QLineEdit (поле пути к файлу)
          "QLineEdit {"
          "  background-color: %1;"
          "  color: %3;"
          "  border: 1px solid %2;"
          "  border-radius: 4px;"
          "  padding: 6px 8px;"
          "}"
          "QLineEdit:focus {"
          "  border-color: %5;"
          "}"
          "QLineEdit:disabled {"
          "  background-color: %8;"
          "  color: %7;"
          "  border-color: %2;"
          "}"

          // Стили для QComboBox (выбор темы)
          "QComboBox {"
          "  background-color: %1;"
          "  color: %3;"
          "  border: 1px solid %2;"
          "  border-radius: 4px;"
          "  padding: 5px 12px;"
          "  min-width: 80px;"
          "}"
          "QComboBox:hover {"
          "  border-color: %5;"
          "}"
          "QComboBox::drop-down {"
          "  border: none;"
          "}"
          "QComboBox::down-arrow {"
          "  image: none;"
          "  border: none;"
          "  width: 0px;"
          "}"
          "QComboBox QAbstractItemView {"
          "  background-color: %1;"
          "  color: %3;"
          "  border: 1px solid %2;"
          "  selection-background-color: %5;"
          "}"

          // Стили для QLabel (обычные метки)
          "QLabel {"
          "  color: %3;"
          "  background-color: transparent;"
          "}"

          // Стили для вкладок/окон файловой страницы
          "QPushButton#btnSelectFile, QPushButton#btnLoadFile {"
          "  background-color: %5;"
          "  color: %3;"
          "  border: 1px solid %2;"
          "  border-radius: 4px;"
          "  padding: 6px 16px;"
          "  min-width: 100px;"
          "}"
          "QPushButton#btnSelectFile:hover, QPushButton#btnLoadFile:hover {"
          "  background-color: %4;"
          "}"
          "QPushButton#btnSelectFile:pressed, QPushButton#btnLoadFile:pressed {"
          "  background-color: %6;"
          "}"
          "QPushButton#btnSelectFile:disabled, "
          "QPushButton#btnLoadFile:disabled {"
          "  background-color: %8;"
          "  color: %7;"
          "  border-color: %2;"
          "}")
          .arg(currentColors_.surface.name(),      // %1 - фон
               currentColors_.border.name(),       // %2 - цвет границы
               currentColors_.textPrimary.name(),  // %3 - цвет текста
               currentColors_.hover.name(),        // %4 - цвет при наведении
               currentColors_.primary.name(),      // %5 - акцентный цвет
               currentColors_.pressed.name(),      // %6 - цвет при нажатии
               currentColors_.textDisabled.name(), // %7 - цвет disabled текста
               currentColors_.background.name());  // %8 - фон для disabled

  statusBar->setStyleSheet(styleSheet);

  // Применяем стиль ко всему виджету InputPage
  setStyleSheet(styleSheet);
}

void InputPage::updateInstructionButtonStyle() {
  if (!btnInstruction)
    return;

  int buttonSize = btnInstruction->width();
  btnInstruction->setStyleSheet(
      QString("QPushButton {"
              "  background-color: %1;"
              "  color: %2;"
              "  font-size: 16px;"
              "  font-weight: bold;"
              "  border: 1px solid %3;"
              "  border-radius: %4px;"
              "  padding: 0px;"
              "}"
              "QPushButton:hover {"
              "  background-color: %5;"
              "}"
              "QPushButton:pressed {"
              "  background-color: %6;"
              "}")
          .arg(currentColors_.primary.name(), currentColors_.textPrimary.name(),
               currentColors_.border.name(), QString::number(buttonSize / 2),
               currentColors_.hover.name(), currentColors_.pressed.name()));
}

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

void InputPage::updateThemeColors(const ThemeColors &colors) {
  currentColors_ = colors;
  updateInstructionButtonStyle();
  applyStatusBarStyle();

  if (textInput) {
    textInput->setThemeColors(colors);
  }
}
