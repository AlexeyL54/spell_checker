#include "MainWindow.hpp"
#include "StyleManager.hpp"
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
  // Применяем палитру
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

  // Единая таблица стилей
  QString styleSheet = StyleManager::getGlobalStyleSheet(colors);
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

  QLabel *themeLabel = new QLabel("Тема: ", this);
  themeCombo = new QComboBox(this);
  themeCombo->addItem("☁ Тёмная");
  themeCombo->addItem("☀ Светлая");
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
    connect(edit, &TextEditWithSpellCheck::canRevertChanged, inputPage,
            &InputPage::onCanRevertChanged);
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
