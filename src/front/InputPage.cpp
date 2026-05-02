#include "InputPage.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

InputPage::InputPage(QWidget *parent) : QWidget(parent) {
  setupMainLayout();
  setupContentRow();
  setupContentColumn();
  setupIntroText();
  setupInputChoice();
  setupKeyboardPage();
  setupFilePage();
  setupButtons();
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

  // Загружаем текст в поле ввода
  textInput->setText(content);

  // Переключаемся на режим клавиатуры
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

  QVBoxLayout *choiceRow = new QVBoxLayout();
  choiceRow->setSpacing(8);
  choiceRow->addWidget(radioKeyboard);
  choiceRow->addWidget(radioFile);
  contentLayout->addLayout(choiceRow);

  stack = new QStackedWidget(contentColumn);
}

void InputPage::setupKeyboardPage() {
  pageKeyboard = new QWidget(contentColumn);
  QVBoxLayout *layoutKeyboard = new QVBoxLayout(pageKeyboard);

  textInput = new TextEditWithSpellCheck(contentColumn);
  textInput->setPlaceholderText("Введите текст для анализа...");
  textInput->setMinimumHeight(200);
  layoutKeyboard->addWidget(textInput);

  stack->addWidget(pageKeyboard);
}

void InputPage::setupFilePage() {
  pageFile = new QWidget(contentColumn);
  QVBoxLayout *layoutFile = new QVBoxLayout(pageFile);

  filePathEdit = new QLineEdit(contentColumn);
  filePathEdit->setReadOnly(true);
  filePathEdit->setPlaceholderText("Файл не выбран");
  layoutFile->addWidget(filePathEdit);

  // Горизонтальный layout для кнопок выбора файла и загрузки
  QHBoxLayout *fileButtonsLayout = new QHBoxLayout();
  fileButtonsLayout->setSpacing(12);

  btnSelectFile = new QPushButton("Выбрать файл", contentColumn);
  btnSelectFile->setCursor(Qt::PointingHandCursor);

  btnLoadFile = new QPushButton("Загрузить", contentColumn);
  btnLoadFile->setCursor(Qt::PointingHandCursor);
  btnLoadFile->setEnabled(false); // Изначально отключена, пока не выбран файл

  fileButtonsLayout->addWidget(btnSelectFile);
  fileButtonsLayout->addWidget(btnLoadFile);
  fileButtonsLayout->addStretch();

  layoutFile->addLayout(fileButtonsLayout);

  stack->addWidget(pageFile);
}

void InputPage::setupButtons() {
  // Создаём горизонтальный layout для кнопок
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setSpacing(12);

  btnCheck = new QPushButton("Проверить", this);
  btnFix = new QPushButton("Исправить", this);
  btnRevert = new QPushButton("Отменить изменения", this);
  btnClear = new QPushButton("Очистить", this);
  btnCopy = new QPushButton("Копировать", this);
  btnSave = new QPushButton("Сохранить", this);

  for (auto btn : {btnCheck, btnFix, btnRevert, btnClear, btnCopy, btnSave}) {
    btn->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(btn);
  }
  buttonLayout->addStretch();

  // Добавляем layout с кнопками на страницу клавиатуры (под textInput)
  QVBoxLayout *keyboardLayout =
      qobject_cast<QVBoxLayout *>(pageKeyboard->layout());
  if (keyboardLayout) {
    keyboardLayout->addLayout(buttonLayout);
  }
}

void InputPage::setupConnections() {
  connect(btnSelectFile, &QPushButton::clicked, this,
          &InputPage::onFileSelected);
  connect(btnLoadFile, &QPushButton::clicked, this, &InputPage::onLoadFile);
  connect(radioKeyboard, &QRadioButton::toggled, this,
          &InputPage::onSourceToggled);
  connect(radioFile, &QRadioButton::toggled, this, &InputPage::onSourceToggled);

  // Подключаем кнопки к сигналам
  connect(btnCheck, &QPushButton::clicked, this, &InputPage::checkRequested);
  connect(btnFix, &QPushButton::clicked, this, &InputPage::fixRequested);
  connect(btnRevert, &QPushButton::clicked, this, &InputPage::revertRequested);
  connect(btnClear, &QPushButton::clicked, this, &InputPage::clearRequested);
  connect(btnCopy, &QPushButton::clicked, this, &InputPage::copyRequested);
  connect(btnSave, &QPushButton::clicked, this, &InputPage::saveRequested);

  // Начальное состояние: клавиатурный режим, стек показывает страницу
  // клавиатуры
  radioKeyboard->setChecked(true);
  stack->setCurrentIndex(0);
}

void InputPage::onFileSelected() {
  QString path =
      QFileDialog::getOpenFileName(this, "Выберите файл", QDir::homePath(),
                                   "Текстовые файлы (*.txt);;Все файлы (*.*)");
  if (!path.isEmpty()) {
    filePathEdit->setText(path);
    btnLoadFile->setEnabled(true); // Активируем кнопку загрузки
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

  // Загружаем текст в поле ввода
  textInput->setText(content);

  // Переключаемся на режим клавиатуры
  radioKeyboard->setChecked(true);
}

void InputPage::onSourceToggled() {
  if (radioKeyboard->isChecked()) {
    stack->setCurrentIndex(0);
  } else {
    stack->setCurrentIndex(1);
  }
}
