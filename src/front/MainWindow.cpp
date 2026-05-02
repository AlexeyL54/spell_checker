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

  // Устанавливаем цвета для TextEdit
  if (inputPage && inputPage->getTextEdit()) {
    inputPage->getTextEdit()->setThemeColors(themeManager->getThemeColors());
  }
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
  pal.setColor(QPalette::ToolTipBase, colors.surface);
  pal.setColor(QPalette::ToolTipText, colors.textPrimary);
  pal.setColor(QPalette::PlaceholderText, colors.textDisabled);

  // Цвета для полей ввода
  pal.setColor(QPalette::Midlight, colors.border);
  pal.setColor(QPalette::Mid, colors.divider);
  pal.setColor(QPalette::Dark, colors.border);
  pal.setColor(QPalette::Shadow, colors.border);

  qApp->setPalette(pal);

  // Полная стилизация всех элементов
  QString styleSheet = QString(
                           // Основные стили для всех виджетов
                           "QWidget {"
                           "  background-color: %1;"
                           "  color: %2;"
                           "}"

                           // Кнопки
                           "QPushButton {"
                           "  background-color: %3;"
                           "  color: %2;"
                           "  border: 1px solid %4;"
                           "  border-radius: 4px;"
                           "  padding: 6px 12px;"
                           "  min-width: 100px;"
                           "}"
                           "QPushButton:hover {"
                           "  background-color: %5;"
                           "  border-color: %4;"
                           "}"
                           "QPushButton:pressed {"
                           "  background-color: %6;"
                           "}"
                           "QPushButton:disabled {"
                           "  background-color: %7;"
                           "  color: %8;"
                           "  border-color: %9;"
                           "}"

                           // Поля ввода
                           "QPlainTextEdit, QLineEdit {"
                           "  background-color: %10;"
                           "  color: %2;"
                           "  border: 1px solid %4;"
                           "  border-radius: 4px;"
                           "  padding: 4px;"
                           "  selection-background-color: %11;"
                           "  selection-color: %2;"
                           "}"
                           "QPlainTextEdit:focus, QLineEdit:focus {"
                           "  border-color: %3;"
                           "}"
                           "QPlainTextEdit:disabled, QLineEdit:disabled {"
                           "  background-color: %7;"
                           "  color: %8;"
                           "  border-color: %9;"
                           "}"

                           // QComboBox
                           "QComboBox {"
                           "  background-color: %10;"
                           "  color: %2;"
                           "  border: 1px solid %4;"
                           "  border-radius: 4px;"
                           "  padding: 5px 12px;"
                           "  min-width: 80px;"
                           "}"
                           "QComboBox:hover {"
                           "  border-color: %3;"
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
                           "  background-color: %10;"
                           "  color: %2;"
                           "  border: 1px solid %4;"
                           "  selection-background-color: %11;"
                           "}"

                           // QLabel
                           "QLabel {"
                           "  color: %2;"
                           "  background-color: transparent;"
                           "}"

                           // QRadioButton
                           "QRadioButton {"
                           "  color: %2;"
                           "  background-color: transparent;"
                           "  spacing: 8px;"
                           "}"
                           "QRadioButton::indicator {"
                           "  width: 16px;"
                           "  height: 16px;"
                           "  border-radius: 8px;"
                           "  border: 1px solid %4;"
                           "  background-color: %10;"
                           "}"
                           "QRadioButton::indicator:checked {"
                           "  background-color: %3;"
                           "  border-color: %3;"
                           "}"
                           "QRadioButton::indicator:hover {"
                           "  border-color: %3;"
                           "}"
                           "QRadioButton:disabled {"
                           "  color: %8;"
                           "}"

                           // QMenu
                           "QMenu {"
                           "  background-color: %10;"
                           "  color: %2;"
                           "  border: 1px solid %4;"
                           "}"
                           "QMenu::item {"
                           "  padding: 4px 20px;"
                           "}"
                           "QMenu::item:selected {"
                           "  background-color: %11;"
                           "}"
                           "QMenu::separator {"
                           "  height: 1px;"
                           "  background-color: %9;"
                           "  margin: 4px 8px;"
                           "}")
                           .arg(colors.background.name(),   // %1
                                colors.textPrimary.name(),  // %2
                                colors.primary.name(),      // %3
                                colors.border.name(),       // %4
                                colors.hover.name(),        // %5
                                colors.pressed.name(),      // %6
                                colors.surface.name(),      // %7
                                colors.textDisabled.name(), // %8
                                colors.divider.name(),      // %9
                                colors.surface.name(),      // %10
                                colors.selected.name());    // %11

  qApp->setStyleSheet(styleSheet);
}

void MainWindow::onCheckRequested() {
  // Проверка орфографии текущего текста в поле ввода
  inputPage->getTextEdit()->performSpellCheck();
}

void MainWindow::onFixRequested() {
  // Исправление ошибок в текущем тексте
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

  // Обновляем цвета в TextEditWithSpellCheck
  if (inputPage && inputPage->getTextEdit()) {
    inputPage->getTextEdit()->setThemeColors(themeManager->getThemeColors());
  }
}
