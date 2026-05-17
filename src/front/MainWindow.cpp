#include "MainWindow.hpp"
#include "StyleManager.hpp"
#include "ThemeManager.hpp"
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QVBoxLayout>

/**
 * @brief Конструктор главного окна.
 * @param parent Родительский виджет (по умолчанию nullptr).
 */
MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Анализатор текста");
  resize(1200, 700);

  themeManager = new ThemeManager(this);

  // Порядок важен
  setupUI();
  setupStackedWidget();
  setupMainContentLayout();
  setupTopBar();
  setupLoadingPage();
  setupThemeSelector();

  applyTheme(themeManager->getThemeColors());

  // Создаём словарь и загружаем асинхронно
  vocabulary = new Vocabulary("../vocab/russian-words/russian.txt", this);

  connect(vocabulary, &Vocabulary::loadStarted, this,
          &MainWindow::onLoadStarted);
  connect(vocabulary, &Vocabulary::loadFinished, this,
          &MainWindow::onLoadFinished);
  connect(vocabulary, &Vocabulary::loadError, this, &MainWindow::onLoadError);

  vocabulary->loadVocabAsync();
}

/**
 * @brief Деструктор главного окна.
 */
MainWindow::~MainWindow() { delete vocabulary; }

/**
 * @brief Настроить пользовательский интерфейс.
 */
void MainWindow::setupUI() {
  ThemeColors currentColors = themeManager->getThemeColors();

  // Страница ввода
  inputPage = new InputPage(currentColors, this);

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

/**
 * @brief Настроить стековый виджет для переключения страниц.
 */
void MainWindow::setupStackedWidget() {
  stackedWidget = new QStackedWidget(this);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addWidget(stackedWidget);
}

/**
 * @brief Настроить виджет основного контента.
 */
void MainWindow::setupMainContentLayout() {
  mainContent = new QWidget(this);

  QVBoxLayout *mainContentLayout = new QVBoxLayout(mainContent);
  mainContentLayout->setContentsMargins(0, 0, 0, 0);
  mainContentLayout->setSpacing(0);

  mainContentLayout->addWidget(inputPage, 1);
}

/**
 * @brief Настроить верхнюю панель инструментов.
 */
void MainWindow::setupTopBar() {
  // Создаём горизонтальный layout для верхней панели
  QHBoxLayout *topBarLayout = new QHBoxLayout();
  topBarLayout->setContentsMargins(12, 8, 12, 8);

  // Добавляем кнопку инструкции из InputPage
  QPushButton *instructionBtn = inputPage->getInstructionButton();
  if (instructionBtn) {
    topBarLayout->addWidget(instructionBtn);
  }

  topBarLayout->addStretch();

  // Добавляем селектор темы
  QLabel *themeLabel = new QLabel("Тема: ", this);
  themeCombo = new QComboBox(this);
  themeCombo->addItem("☁ Тёмная");
  themeCombo->addItem("☀ Светлая");

  topBarLayout->addWidget(themeLabel);
  topBarLayout->addWidget(themeCombo);

  // Добавляем верхнюю панель в layout основного контента
  QVBoxLayout *mainContentLayout =
      qobject_cast<QVBoxLayout *>(mainContent->layout());
  if (mainContentLayout) {
    mainContentLayout->insertLayout(0, topBarLayout);
  }

  topBar = topBarLayout;
}

/**
 * @brief Настроить страницу загрузки.
 */
void MainWindow::setupLoadingPage() {
  loadingPage = new LoadingPage(this);
  stackedWidget->addWidget(loadingPage);
  stackedWidget->addWidget(mainContent);

  // Показываем страницу загрузки
  stackedWidget->setCurrentWidget(loadingPage);
}

/**
 * @brief Настроить селектор выбора темы.
 */
void MainWindow::setupThemeSelector() {
  connect(themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::onThemeChanged);
}

/**
 * @brief Применить цветовую схему темы.
 * @param colors Цветовая схема для применения.
 */
void MainWindow::applyTheme(const ThemeColors &colors) {
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

/**
 * @brief Обработчик запроса проверки орфографии.
 */
void MainWindow::onCheckRequested() {
  // Проверка орфографии текущего текста в поле ввода
  inputPage->getTextEdit()->performSpellCheck();
}

/**
 * @brief Обработчик запроса исправления ошибок.
 */
void MainWindow::onFixRequested() {
  // Исправление ошибок в текущем тексте
  inputPage->getTextEdit()->applyFirstCorrections();
}

/**
 * @brief Обработчик запроса отмены изменений.
 */
void MainWindow::onRevertRequested() {
  inputPage->getTextEdit()->revertToOriginal();
}

/**
 * @brief Обработчик запроса очистки текста.
 */
void MainWindow::onClearRequested() { inputPage->getTextEdit()->clearAll(); }

/**
 * @brief Обработчик запроса копирования текста в буфер обмена.
 */
void MainWindow::onCopyRequested() {
  QString text = inputPage->getTextEdit()->getText();
  QApplication::clipboard()->setText(text);
}

/**
 * @brief Обработчик запроса сохранения текста в файл.
 */
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

/**
 * @brief Обработчик изменения темы оформления.
 * @param index Индекс выбранной темы в комбобоксе.
 */
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

/**
 * @brief Показать основной контент приложения.
 */
void MainWindow::showMainContent() {
  if (stackedWidget) {
    stackedWidget->setCurrentWidget(mainContent);
  }

  // Включаем UI элементы
  setUIElementsEnabled(true);

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

/**
 * @brief Включить/выключить элементы пользовательского интерфейса.
 * @param enabled true - включить, false - выключить.
 */
void MainWindow::setUIElementsEnabled(bool enabled) {
  if (themeCombo) {
    themeCombo->setEnabled(enabled);
  }

  QPushButton *instructionBtn = inputPage->getInstructionButton();
  if (instructionBtn) {
    instructionBtn->setEnabled(enabled);
  }
}

/**
 * @brief Обработчик начала загрузки словаря.
 */
void MainWindow::onLoadStarted() {
  if (loadingPage) {
    loadingPage->showLoading();
  }
}

/**
 * @brief Обработчик завершения загрузки словаря.
 */
void MainWindow::onLoadFinished() {
  if (loadingPage) {
    loadingPage->showSuccess("Словарь успешно загружен!");
  }

  // Небольшая задержка перед показом основного контента
  QTimer::singleShot(500, this, &MainWindow::showMainContent);
}

/**
 * @brief Обработчик ошибки загрузки словаря.
 * @param error Текст сообщения об ошибке.
 */
void MainWindow::onLoadError(const QString &error) {
  if (loadingPage) {
    loadingPage->showError(error);
  }
}
