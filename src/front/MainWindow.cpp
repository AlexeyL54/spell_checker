#include "MainWindow.hpp"
#include "qlogging.h"
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Анализатор текста");
  resize(1200, 700);

  vocabulary = new Vocabulary("../vocab/russian-words/russian.txt");
  vocabulary->loadVocab();

  qDebug() << "Словарь загружен";

  // Создаём менеджер тем
  themeManager = new ThemeManager(this);

  setupUI();
  setupThemeSelector();

  // Передаём словарь в TextEditWithSpellCheck
  inputPage->getTextEdit()->setVocabulary(vocabulary);

  // Применяем начальную тему (Тёмная по умолчанию)
  applyTheme(themeManager->getThemeColors());
}

MainWindow::~MainWindow() { delete vocabulary; }

void MainWindow::setupUI() {
  // Основной вертикальный layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Верхняя панель для выбора темы
  QHBoxLayout *topBar = new QHBoxLayout();
  topBar->setContentsMargins(12, 8, 12, 8);
  topBar->addStretch();
  QLabel *themeLabel = new QLabel("Тема:", this);
  themeCombo = new QComboBox(this);
  themeCombo->addItem("Тёмная");
  themeCombo->addItem("Светлая");
  topBar->addWidget(themeLabel);
  topBar->addWidget(themeCombo);
  mainLayout->addLayout(topBar);

  // Страница ввода
  inputPage = new InputPage(this);
  mainLayout->addWidget(inputPage, 1);

  // Подключаем сигналы InputPage к слотам MainWindow
  connect(inputPage, &InputPage::checkRequested, this,
          &MainWindow::onCheckRequested);
  connect(inputPage, &InputPage::fixRequested, this,
          &MainWindow::onFixRequested);
  connect(inputPage, &InputPage::revertRequested, this,
          &MainWindow::onRevertRequested);
  connect(inputPage, &InputPage::clearRequested, this,
          &MainWindow::onClearRequested);
  connect(inputPage, &InputPage::copyRequested, this,
          &MainWindow::onCopyRequested);
  connect(inputPage, &InputPage::saveRequested, this,
          &MainWindow::onSaveRequested);
}

void MainWindow::setupThemeSelector() {
  connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::onThemeChanged);
}

void MainWindow::applyTheme(const ThemeColors &colors) {
  // Применяем палитру ко всему приложению
  QPalette pal = qApp->palette();

  pal.setColor(QPalette::Window, colors.background);
  pal.setColor(QPalette::Base, colors.surface);
  pal.setColor(QPalette::AlternateBase, colors.surface);
  pal.setColor(QPalette::Text, colors.textPrimary);
  pal.setColor(QPalette::WindowText, colors.textPrimary);
  pal.setColor(QPalette::Button, colors.primary);
  pal.setColor(QPalette::ButtonText, colors.textPrimary);
  pal.setColor(QPalette::Highlight, colors.selected);
  pal.setColor(QPalette::HighlightedText, colors.textPrimary);

  qApp->setPalette(pal);

  // Дополнительные стили для кнопок и полей ввода
  QString styleSheet =
      QString("QPushButton {"
              "  background-color: %1;"
              "  color: %2;"
              "  border: 1px solid %3;"
              "  border-radius: 4px;"
              "  padding: 6px 12px;"
              "}"
              "QPushButton:hover { background-color: %4; }"
              "QPushButton:pressed { background-color: %5; }"
              "QPlainTextEdit, QLineEdit {"
              "  background-color: %6;"
              "  color: %7;"
              "  border: 1px solid %3;"
              "  border-radius: 4px;"
              "}")
          .arg(colors.primary.name(), colors.textPrimary.name(),
               colors.border.name(), colors.hover.name(), colors.pressed.name(),
               colors.surface.name(), colors.textPrimary.name());

  qApp->setStyleSheet(styleSheet);
}

void MainWindow::loadTextFromFile(const QString &path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл.");
    return;
  }
  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  QString content = stream.readAll();
  file.close();

  inputPage->getTextEdit()->setText(content);
  // Переключаем на режим клавиатуры (автоматически)
  // Для этого нужно, чтобы радио-кнопка "С клавиатуры" была выбрана
  // Мы можем вызвать setChecked, но это вызовет сигнал toggled, который
  // переключит стек. Для простоты - через указатель на radioKeyboard в
  // InputPage нет публичного доступа. Добавим метод в InputPage для
  // принудительного переключения на клавиатурный режим: Но мы не добавили такой
  // метод. Вместо этого можно эмулировать нажатие. Однако логика описана: при
  // нажатии "Проверить" или "Исправить" если выбран файл, загружаем файл и
  // переключаем режим. Мы делаем это в onCheckRequested/onFixRequested, вызывая
  // loadTextFromFile и затем устанавливая radioKeyboard->setChecked(true). Так
  // как radioKeyboard не доступен, добавим публичный метод в InputPage: void
  // setKeyboardMode(bool enabled) { if(enabled)
  // radioKeyboard->setChecked(true); } Для простоты добавим этот метод сейчас.
  // Но мы не хотим переписывать InputPage.hpp снова. Добавим в существующий:
  // Допишем в конец public секции InputPage: void setKeyboardMode(bool
  // enabled);
}

void MainWindow::onCheckRequested() {
  // Если выбран режим файла, загружаем текст из файла и переключаем на
  // клавиатуру
  if (!inputPage->isKeyboardMode()) {
    QString path = inputPage->getFilePath();
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
    inputPage->getTextEdit()->setText(content);
    // Переключаем на клавиатурный режим – получаем radio-кнопку через findChild
    // Более надёжно – добавить метод в InputPage. Сделаем временное решение:
    QRadioButton *rb = inputPage->findChild<QRadioButton *>("radioKeyboard");
    if (rb)
      rb->setChecked(true);
  }
  inputPage->getTextEdit()->performSpellCheck();
}

void MainWindow::onFixRequested() {
  if (!inputPage->isKeyboardMode()) {
    QString path = inputPage->getFilePath();
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
    inputPage->getTextEdit()->setText(content);
    QRadioButton *rb = inputPage->findChild<QRadioButton *>("radioKeyboard");
    if (rb)
      rb->setChecked(true);
  }
  inputPage->getTextEdit()->applyFirstCorrections();
}

void MainWindow::onRevertRequested() {
  inputPage->getTextEdit()->revertToOriginal();
}

void MainWindow::onClearRequested() { inputPage->getTextEdit()->clearAll(); }

void MainWindow::onCopyRequested() {
  QString text = inputPage->getTextEdit()->getText();
  QApplication::clipboard()->setText(text);
}

void MainWindow::onSaveRequested() {
  QString path = QFileDialog::getSaveFileName(
      this, "Сохранить текст", QDir::homePath(), "Текстовые файлы (*.txt)");
  if (path.isEmpty())
    return;

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл.");
    return;
  }
  QTextStream stream(&file);
  stream.setEncoding(QStringConverter::Utf8);
  stream << inputPage->getTextEdit()->getText();
  file.close();
}

void MainWindow::onThemeChanged(int index) {
  Theme theme = (index == 0) ? Dark : Light;
  themeManager->setTheme(theme);
  applyTheme(themeManager->getThemeColors());
}
