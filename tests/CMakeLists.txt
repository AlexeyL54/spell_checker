cmake_minimum_required(VERSION 3.10)
project(SpellChecker)

set(CMAKE_CXX_STANDARD 11)

# Ищем Google Test в системе
find_package(GTest REQUIRED)

set(SOURCES
  vocabulary.cpp
  ../src/back/vocab.cpp
)

set(HEADERS
  ../src/back/vocab.hpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_executable(test ${SOURCES} ${HEADERS})

# Линкуем с Google Test
target_link_libraries(test GTest::gtest GTest::gtest_main)

# Включаем поддержку потоков
find_package(Threads REQUIRED)
target_link_libraries(test Threads::Threads)

# Добавляем директории с заголовочными файлами GTest
target_include_directories(test PRIVATE ${GTEST_INCLUDE_DIRS})
