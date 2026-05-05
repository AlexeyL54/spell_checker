cmake_minimum_required(VERSION 3.10)
project(SpellChecker)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Test)

# Общие исходники проекта
set(PROJECT_SOURCES
  ../src/back/vocab.cpp
)

set(PROJECT_HEADERS
  ../src/back/vocab.hpp
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
    Threads::Threads
    Qt6::Core 
    Qt6::Widgets
    Qt6::Test
  )
  
  target_include_directories(${TEST_NAME} PRIVATE ${GTEST_INCLUDE_DIRS})
  
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

add_spellchecker_test(vocabulary_test vocabulary.cpp)

enable_testing()
