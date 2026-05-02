#ifndef LOADINGPAGE_HPP
#define LOADINGPAGE_HPP

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QWidget>

class LoadingPage : public QWidget {
  Q_OBJECT

public:
  explicit LoadingPage(QWidget *parent = nullptr);

  /** @brief Показать индикатор загрузки (спиннер/бесконечный прогресс) */
  void showLoading();

  /** @brief Показать сообщение об ошибке */
  void showError(const QString &message);

  /** @brief Показать сообщение об успешной загрузке */
  void showSuccess(const QString &message);

  /** @brief Обновить статус загрузки */
  void setStatus(const QString &status);

private:
  void setupUI();

  QVBoxLayout *mainLayout = nullptr;
  QLabel *titleLabel = nullptr;
  QLabel *statusLabel = nullptr;
  QProgressBar *progressBar = nullptr;
};

#endif // LOADINGPAGE_HPP
