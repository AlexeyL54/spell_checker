cmake_minimum_required(VERSION 3.10)
project(SpellChecker)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(SOURCES
    main.cpp
    src/front/InputPage.cpp
    src/front/MainWindow.cpp
    src/front/TextEditWithSpellCheck.cpp
    src/front/ThemeManager.cpp
    src/front/StyleManager.cpp
    src/front/LoadingPage.cpp
    src/back/vocab.cpp
    src/back/unistring.cpp
)

set(HEADERS
    src/front/InputPage.hpp
    src/front/MainWindow.hpp
    src/front/TextEditWithSpellCheck.hpp
    src/front/ThemeManager.hpp
    src/front/StyleManager.hpp
    src/front/LoadingPage.hpp
    src/back/vocab.hpp
    src/back/unistring.hpp
)

add_executable(SpellChecker ${SOURCES} ${HEADERS})

target_link_libraries(SpellChecker Qt6::Core Qt6::Widgets)
