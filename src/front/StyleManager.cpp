#include "StyleManager.hpp"

QString StyleManager::getGlobalStyleSheet(const ThemeColors &colors) {
  return buttonStyle(colors) + statusBarStyle(colors) + textEditStyle(colors) +
         comboBoxStyle(colors) + radioButtonStyle(colors) +
         lineEditStyle(colors) + instructionButtonStyle(colors) +
         fileButtonsStyle(colors) +

         // Базовые стили для всех виджетов
         QString("QWidget {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "}")
             .arg(colors.background.name(), colors.textPrimary.name()) +

         // QLabel
         QString("QLabel {"
                 "  color: %1;"
                 "  background-color: transparent;"
                 "}")
             .arg(colors.textPrimary.name()) +

         // QMenu
         QString("QMenu {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  border: 1px solid %3;"
                 "}"
                 "QMenu::item {"
                 "  padding: 4px 20px;"
                 "}"
                 "QMenu::item:selected {"
                 "  background-color: %4;"
                 "}"
                 "QMenu::separator {"
                 "  height: 1px;"
                 "  background-color: %3;"
                 "  margin: 4px 8px;"
                 "}")
             .arg(colors.surface.name(), colors.textPrimary.name(),
                  colors.border.name(), colors.selected.name());
}

QString StyleManager::buttonStyle(const ThemeColors &colors) {
  // Единый стиль для ВСЕХ кнопок (кроме специальных)
  return QString("QPushButton {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  border: 1px solid %3;"
                 "  border-radius: 4px;"
                 "  padding: 6px 12px;"
                 "  min-width: 100px;"
                 "}"
                 "QPushButton:hover {"
                 "  background-color: %4;"
                 "  border-color: %1;"
                 "}"
                 "QPushButton:pressed {"
                 "  background-color: %5;"
                 "}"
                 "QPushButton:disabled {"
                 "  background-color: %6;"
                 "  color: %7;"
                 "  border-color: %8;"
                 "}")
      .arg(colors.primary.name(),      // %1 - фон кнопки
           colors.textPrimary.name(),  // %2 - цвет текста
           colors.border.name(),       // %3 - цвет границы
           colors.hover.name(),        // %4 - цвет при наведении
           colors.pressed.name(),      // %5 - цвет при нажатии
           colors.surface.name(),      // %6 - фон disabled
           colors.textDisabled.name(), // %7 - текст disabled
           colors.divider.name());     // %8 - граница disabled
}

QString StyleManager::statusBarStyle(const ThemeColors &colors) {
  return QString("QStatusBar {"
                 "  background-color: %1;"
                 "  border-top: 1px solid %2;"
                 "}"
                 "QStatusBar::item {"
                 "  border: none;"
                 "}"
                 // Кнопки в статус-баре используют те же стили, что и обычные
                 // кнопки
                 "QStatusBar QPushButton {"
                 "  min-width: 80px;"
                 "  margin: 2px;"
                 "}")
      .arg(colors.surface.name(), colors.border.name());
}

QString StyleManager::textEditStyle(const ThemeColors &colors) {
  return QString("QPlainTextEdit, QTextEdit {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  border: 1px solid %3;"
                 "  border-radius: 4px;"
                 "  padding: 8px;"
                 "  selection-background-color: %4;"
                 "  selection-color: %2;"
                 "}"
                 "QPlainTextEdit:focus, QTextEdit:focus {"
                 "  border-color: %5;"
                 "}"
                 "QPlainTextEdit:disabled, QTextEdit:disabled {"
                 "  background-color: %6;"
                 "  color: %7;"
                 "  border-color: %3;"
                 "}")
      .arg(colors.surface.name(),       // %1
           colors.textPrimary.name(),   // %2
           colors.border.name(),        // %3
           colors.selected.name(),      // %4
           colors.primary.name(),       // %5
           colors.background.name(),    // %6
           colors.textDisabled.name()); // %7
}

QString StyleManager::comboBoxStyle(const ThemeColors &colors) {
  return QString("QComboBox {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  border: 1px solid %3;"
                 "  border-radius: 4px;"
                 "  padding: 5px 12px;"
                 "  min-width: 80px;"
                 "}"
                 "QComboBox:hover {"
                 "  border-color: %4;"
                 "}"
                 "QComboBox::drop-down {"
                 "  border: none;"
                 "}"
                 "QComboBox::down-arrow {"
                 "  image: none;"
                 "  width: 0px;"
                 "}"
                 "QComboBox QAbstractItemView {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  border: 1px solid %3;"
                 "  selection-background-color: %5;"
                 "}")
      .arg(colors.surface.name(),     // %1
           colors.textPrimary.name(), // %2
           colors.border.name(),      // %3
           colors.primary.name(),     // %4
           colors.selected.name());   // %5
}

QString StyleManager::radioButtonStyle(const ThemeColors &colors) {
  return QString("QRadioButton {"
                 "  color: %1;"
                 "  background-color: transparent;"
                 "  spacing: 8px;"
                 "  padding: 4px 0px;"
                 "}"
                 "QRadioButton::indicator {"
                 "  width: 16px;"
                 "  height: 16px;"
                 "  border-radius: 8px;"
                 "  border: 2px solid %2;"
                 "  background-color: %3;"
                 "}"
                 "QRadioButton::indicator:hover {"
                 "  border-color: %4;"
                 "}"
                 "QRadioButton::indicator:checked {"
                 "  background-color: %4;"
                 "  border-color: %4;"
                 "}"
                 "QRadioButton:disabled {"
                 "  color: %5;"
                 "}")
      .arg(colors.textPrimary.name(),   // %1
           colors.border.name(),        // %2
           colors.surface.name(),       // %3
           colors.primary.name(),       // %4
           colors.textDisabled.name()); // %5
}

QString StyleManager::lineEditStyle(const ThemeColors &colors) {
  return QString("QLineEdit {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  border: 1px solid %3;"
                 "  border-radius: 4px;"
                 "  padding: 6px 8px;"
                 "}"
                 "QLineEdit:focus {"
                 "  border-color: %4;"
                 "}"
                 "QLineEdit:disabled {"
                 "  background-color: %5;"
                 "  color: %6;"
                 "  border-color: %3;"
                 "}")
      .arg(colors.surface.name(),       // %1
           colors.textPrimary.name(),   // %2
           colors.border.name(),        // %3
           colors.primary.name(),       // %4
           colors.background.name(),    // %5
           colors.textDisabled.name()); // %6
}

QString StyleManager::instructionButtonStyle(const ThemeColors &colors) {
  return QString("QPushButton#instructionButton {"
                 "  background-color: %1;"
                 "  color: %2;"
                 "  font-size: 16px;"
                 "  font-weight: bold;"
                 "  border: 1px solid %3;"
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
                 "  background-color: %5;"
                 "}")
      .arg(colors.primary.name(),     // %1
           colors.textPrimary.name(), // %2
           colors.border.name(),      // %3
           colors.hover.name(),       // %4
           colors.pressed.name());    // %5
}

QString StyleManager::fileButtonsStyle(const ThemeColors &colors) {
  // Специальные кнопки для файловой страницы (могут иметь специальные
  // objectName) По умолчанию они используют стандартный стиль кнопок через
  // селектор по id
  return QString("QPushButton#btnSelectFile, QPushButton#btnLoadFile {"
                 "  /* Наследуют стандартный стиль кнопки */"
                 "  min-width: 100px;"
                 "}");
}
