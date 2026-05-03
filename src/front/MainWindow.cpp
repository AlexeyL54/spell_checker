#include "MainWindow.hpp"
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Анализатор текста");
  resize(1200, 700);

  setupUI();         // Теперь только создаёт виджеты, но не layout
  showLoadingPage(); // Создаёт layout и показывает загрузку

  // Создаём менеджер тем
  themeManager = new ThemeManager(this);
  setupThemeSelector();
  applyTheme(themeManager->getThemeColors());

  // Создаём словарь и загружаем асинхронно
  vocabulary = new Vocabulary("../vocab/russian-words/russian.txt", this);

  // Подключаем сигналы
  connect(vocabulary, &Vocabulary::loadStarted, this,
          &MainWindow::onLoadStarted);
  /*connect(vocabulary, &Vocabulary::loadProgress, this,
          &MainWindow::onLoadProgress);*/
  connect(vocabulary, &Vocabulary::loadFinished, this,
          &MainWindow::onLoadFinished);
  connect(vocabulary, &Vocabulary::loadError, this, &MainWindow::onLoadError);

  vocabulary->loadVocabAsync();
}

MainWindow::~MainWindow() { delete vocabulary; }

void MainWindow::setupUI() {
  // НЕ СОЗДАЁМ layout здесь, а только создаём виджеты
  // Layout будет создан в showLoadingPage()

  // Верхняя панель будет создана в showMainContent()
  topBar = nullptr;
  themeCombo = nullptr;

  // Страница ввода
  inputPage = new InputPage(this);

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

                           // Специальный стиль для кнопки инструкции
                           "QPushButton#instructionButton {"
                           "  background-color: %3;"
                           "  color: %2;"
                           "  font-size: 16px;"
                           "  font-weight: bold;"
                           "  border: 1px solid %4;"
                           "  border-radius: 14px;"
                           "  padding: 0px;"
                           "  min-width: 28px;"
                           "  max-width: 28px;"
                           "  min-height: 28px;"
                           "  max-height: 28px;"
                           "}"
                           "QPushButton#instructionButton:hover {"
                           "  background-color: %5;"
                           "}"
                           "QPushButton#instructionButton:pressed {"
                           "  background-color: %6;"
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

  // Обновляем цвета в InputPage (для кнопки инструкции)
  if (inputPage) {
    inputPage->updateThemeColors(themeManager->getThemeColors());
  }
}

void MainWindow::showLoadingPage() {
  // Создаём стек и страницы
  stackedWidget = new QStackedWidget(this);
  loadingPage = new LoadingPage(this);
  mainContent = new QWidget(this);

  // Создаём layout для mainContent
  QVBoxLayout *mainContentLayout = new QVBoxLayout(mainContent);
  mainContentLayout->setContentsMargins(0, 0, 0, 0);
  mainContentLayout->setSpacing(0);

  // Создаём верхнюю панель
  QHBoxLayout *topBarLayout = new QHBoxLayout();
  topBarLayout->setContentsMargins(12, 8, 12, 8);

  // Добавляем кнопку инструкции из InputPage
  QPushButton *instructionBtn = inputPage->getInstructionButton();
  if (instructionBtn) {
    topBarLayout->addWidget(instructionBtn);
  }

  topBarLayout->addStretch();

  QLabel *themeLabel = new QLabel("Тема:", this);
  themeCombo = new QComboBox(this);
  themeCombo->addItem("Тёмная");
  themeCombo->addItem("Светлая");
  topBarLayout->addWidget(themeLabel);
  topBarLayout->addWidget(themeCombo);

  mainContentLayout->addLayout(topBarLayout);

  // Сохраняем указатели для дальнейшего использования
  topBar = topBarLayout;

  // Добавляем страницу ввода
  mainContentLayout->addWidget(inputPage, 1);

  // Добавляем страницы в стек
  stackedWidget->addWidget(loadingPage);
  stackedWidget->addWidget(mainContent);

  // Создаём главный layout для MainWindow
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(stackedWidget);

  // Показываем страницу загрузки
  stackedWidget->setCurrentWidget(loadingPage);

  // Отключаем UI элементы
  if (themeCombo) {
    themeCombo->setEnabled(false);
  }

  // Отключаем кнопку инструкции во время загрузки
  if (instructionBtn) {
    instructionBtn->setEnabled(false);
  }
}

void MainWindow::showMainContent() {
  if (stackedWidget) {
    stackedWidget->setCurrentWidget(mainContent);
  }

  if (themeCombo) {
    themeCombo->setEnabled(true);
  }

  QPushButton *instructionBtn = inputPage->getInstructionButton();
  if (instructionBtn) {
    instructionBtn->setEnabled(true);
  }

  if (inputPage && inputPage->getTextEdit() && vocabulary) {
    TextEditWithSpellCheck *edit = inputPage->getTextEdit();
    edit->setVocabulary(vocabulary);

    // Подключаем сигнал проверки орфографии к обновлению статус-бара
    connect(edit, &TextEditWithSpellCheck::spellCheckCompleted, inputPage,
            &InputPage::onSpellCheckCompleted);
  }
}
void MainWindow::onLoadStarted() {
  if (loadingPage) {
    loadingPage->showLoading();
  }
}

void MainWindow::onLoadFinished() {
  if (loadingPage) {
    loadingPage->showSuccess("Словарь успешно загружен!");
  }

  // Небольшая задержка перед показом основного контента
  QTimer::singleShot(500, this, &MainWindow::showMainContent);
}

void MainWindow::onLoadError(const QString &error) {
  if (loadingPage) {
    loadingPage->showError(error);
  }
}
