#include "LoadingPage.hpp"

/**
 * @brief Конструктор страницы загрузки.
 * @param parent Родительский виджет (по умолчанию nullptr).
 */
LoadingPage::LoadingPage(QWidget *parent) : QWidget(parent) { setupUI(); }

/**
 * @brief Настроить пользовательский интерфейс.
 *
 * Создаёт и настраивает все визуальные компоненты страницы:
 * - заголовок
 * - метку статуса
 * - прогресс-бар
 * - центрированное выравнивание
 */
void LoadingPage::setupUI() {
  mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(40, 40, 40, 40);
  mainLayout->setSpacing(20);

  // Заголовок
  titleLabel = new QLabel("Загрузка словаря", this);
  titleLabel->setAlignment(Qt::AlignCenter);
  QFont titleFont = titleLabel->font();
  titleFont.setPointSize(18);
  titleFont.setBold(true);
  titleLabel->setFont(titleFont);
  mainLayout->addWidget(titleLabel);

  // Статус загрузки
  statusLabel = new QLabel("Пожалуйста, подождите...", this);
  statusLabel->setAlignment(Qt::AlignCenter);
  statusLabel->setWordWrap(true);
  QFont statusFont = statusLabel->font();
  statusFont.setPointSize(10);
  statusLabel->setFont(statusFont);
  mainLayout->addWidget(statusLabel);

  // Прогресс-бар в бесконечном режиме (спиннер)
  progressBar = new QProgressBar(this);
  progressBar->setRange(0, 0); // Бесконечный режим
  progressBar->setTextVisible(false);
  mainLayout->addWidget(progressBar);

  // Центрируем содержимое
  mainLayout->setAlignment(Qt::AlignCenter);
}

/**
 * @brief Показать индикатор загрузки (спиннер/бесконечный прогресс).
 *
 * Переводит страницу в режим ожидания загрузки с отображением бегущего
 * прогресс-бара и стандартным текстом статуса.
 */
void LoadingPage::showLoading() {
  titleLabel->setText("Загрузка словаря");
  progressBar->setRange(0, 0); // Бесконечный режим
  progressBar->setTextVisible(false);
  statusLabel->setStyleSheet("");
  statusLabel->setText("Идёт загрузка, пожалуйста, подождите...");
}

/**
 * @brief Показать сообщение об ошибке.
 * @param message Текст сообщения об ошибке.
 *
 * Переводит страницу в режим ошибки с красным текстом сообщения
 * и остановленным прогресс-баром на 100%.
 */
void LoadingPage::showError(const QString &message) {
  titleLabel->setText("Ошибка загрузки");
  progressBar->setRange(0, 100);
  progressBar->setValue(100);
  progressBar->setTextVisible(false);
  statusLabel->setStyleSheet("color: #ff6666;");
  statusLabel->setText(message);
}

/**
 * @brief Показать сообщение об успешной загрузке.
 * @param message Текст сообщения об успешной загрузке.
 *
 * Переводит прогресс-бар в завершённое состояние (100%).
 * Не изменяет заголовок и текст статуса, только визуальное состояние прогресса.
 */
void LoadingPage::showSuccess(const QString &message) {
  progressBar->setRange(0, 100);
  progressBar->setValue(100);
  progressBar->setTextVisible(false);
}

/**
 * @brief Обновить статус загрузки.
 * @param status Новый текст статуса.
 *
 * Позволяет динамически обновлять информационное сообщение о процессе загрузки.
 */
void LoadingPage::setStatus(const QString &status) {
  statusLabel->setText(status);
}
