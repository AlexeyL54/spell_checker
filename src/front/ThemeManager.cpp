#include "ThemeManager.hpp"

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {
  setTheme(Dark);
};

void ThemeManager::setTheme(Theme theme) {
  if (currentTheme_ == theme)
    return;

  currentTheme_ = theme;
  currentColors_ = loadTheme(theme);
  emit themeChanged();
}

Theme ThemeManager::getTheme() { return currentTheme_; }

ThemeColors ThemeManager::getThemeColors() { return currentColors_; }

ThemeColors ThemeManager::loadTheme(Theme theme) const {
  ThemeColors colors;

  if (theme == Dark) {
    // Тёмная тема
    colors.background = QColor(30, 30, 30);     // #1e1e1e
    colors.surface = QColor(37, 37, 38);        // #252526
    colors.primary = QColor(0, 122, 204);       // #007acc
    colors.primaryDark = QColor(0, 102, 179);   // #0066b3
    colors.primaryLight = QColor(51, 153, 255); // #3399ff

    colors.textPrimary = QColor(204, 204, 204);   // #cccccc
    colors.textSecondary = QColor(153, 153, 153); // #999999
    colors.textDisabled = QColor(85, 85, 85);     // #555555

    colors.border = QColor(62, 62, 66);  // #3e3e42
    colors.divider = QColor(45, 45, 48); // #2d2d30

    colors.hover = QColor(42, 45, 46);     // #2a2d2e
    colors.pressed = QColor(0, 92, 153);   // #005c99
    colors.selected = QColor(0, 102, 179); // #0066b3

  } else {
    // Светлая тема
    colors.background = QColor(243, 243, 243);  // #f3f3f3
    colors.surface = QColor(255, 255, 255);     // #ffffff
    colors.primary = QColor(0, 102, 179);       // #0066b3
    colors.primaryDark = QColor(0, 82, 143);    // #00528f
    colors.primaryLight = QColor(51, 153, 255); // #3399ff

    colors.textPrimary = QColor(51, 51, 51);      // #333333
    colors.textSecondary = QColor(102, 102, 102); // #666666
    colors.textDisabled = QColor(170, 170, 170);  // #aaaaaa

    colors.border = QColor(224, 224, 224);  // #e0e0e0
    colors.divider = QColor(238, 238, 238); // #eeeeee

    colors.hover = QColor(240, 240, 240);  // #f0f0f0
    colors.pressed = QColor(0, 82, 143);   // #00528f
    colors.selected = QColor(0, 102, 179); // #0066b3
  }

  return colors;
}
