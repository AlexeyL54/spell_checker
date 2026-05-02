#include "LoadingPage.hpp"

LoadingPage::LoadingPage(QWidget *parent) : QWidget(parent) { setupUI(); }

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

void LoadingPage::showLoading() {
  titleLabel->setText("Загрузка словаря");
  progressBar->setRange(0, 0); // Бесконечный режим
  progressBar->setTextVisible(false);
  statusLabel->setStyleSheet("");
  statusLabel->setText("Идёт загрузка, пожалуйста, подождите...");
}

void LoadingPage::showError(const QString &message) {
  titleLabel->setText("Ошибка загрузки");
  progressBar->setRange(0, 100);
  progressBar->setValue(100);
  progressBar->setTextVisible(false);
  statusLabel->setStyleSheet("color: #ff6666;");
  statusLabel->setText(message);
}

void LoadingPage::showSuccess(const QString &message) {
  titleLabel->setText("Готово!");
  progressBar->setRange(0, 100);
  progressBar->setValue(100);
  progressBar->setTextVisible(false);
  statusLabel->setStyleSheet("color: #66ff66;");
  statusLabel->setText(message);
}

void LoadingPage::setStatus(const QString &status) {
  statusLabel->setText(status);
}
