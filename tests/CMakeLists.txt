cmake_minimum_required(VERSION 3.10)
project(SpellChecker)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Общие исходники проекта
set(PROJECT_SOURCES
  ../src/back/vocab.cpp
  ../src/back/unistring.cpp
)

set(PROJECT_HEADERS
  ../src/back/vocab.hpp
  ../src/back/unistring.hpp
)

# Функция для создания теста
function(add_spellchecker_test TEST_NAME TEST_FILE)
  add_executable(${TEST_NAME} 
    ${TEST_FILE}
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
  )
  
  target_link_libraries(${TEST_NAME}
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
  )
  
  target_include_directories(${TEST_NAME} PRIVATE ${GTEST_INCLUDE_DIRS})
  
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Создаем отдельные тесты
add_spellchecker_test(vocabulary_test vocabulary.cpp)
add_spellchecker_test(unistring_test unistring.cpp)

enable_testing()
