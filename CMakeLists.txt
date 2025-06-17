cmake_minimum_required(VERSION 3.10)
project(PollingApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Путь к Qt (если нужно, укажи свой путь)
set(CMAKE_PREFIX_PATH "/path/to/Qt/lib/cmake") # Замени на путь к Qt, если не в стандартном месте

# Найти зависимости
find_package(Qt6 COMPONENTS Core Gui Qml Quick REQUIRED)
find_package(libwebsockets REQUIRED)
find_package(libpqxx REQUIRED)
find_package(cpp-jwt REQUIRED)

# Включить автоматическую обработку QML
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Добавить папки с исходниками
add_subdirectory(src/backend)
add_subdirectory(src/frontend)
add_subdirectory(src/common)
add_subdirectory(tests)

# Создать исполняемый файл
add_executable(polling_app src/main.cpp)

# Линковка библиотек
target_link_libraries(polling_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    websockets
    pqxx
    cpp-jwt
)
