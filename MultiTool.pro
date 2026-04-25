QT       += core gui widgets network

CONFIG += c++11

TARGET = MultiTool
TEMPLATE = app

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/core/config.cpp \
    src/core/main_window.cpp \
    src/core/plugin_manager.cpp \
    src/ui/sidebar.cpp \
    src/ui/theme.cpp \
    src/plugins/color_picker/color_picker.cpp \
    src/plugins/csv_tool/csv_tool.cpp \
    src/plugins/hash_tool/hash_tool.cpp \
    src/plugins/image_tool/image_tool.cpp \
    src/plugins/json_formatter/json_formatter.cpp \
    src/plugins/regex_tool/regex_tool.cpp \
    src/plugins/sql_formatter/sql_formatter.cpp \
    src/plugins/text_tools/text_tools.cpp \
    src/plugins/timestamp_tool/timestamp_tool.cpp \
    src/plugins/upload_tool/upload_tool.cpp \
    src/plugins/upload_tool/upload_config.cpp \
    src/plugins/upload_tool/project_manage_dialog.cpp

HEADERS += \
    src/core/config.h \
    src/core/main_window.h \
    src/core/plugin_manager.h \
    src/ui/sidebar.h \
    src/ui/theme.h \
    src/plugins/base.h \
    src/plugins/color_picker/color_picker.h \
    src/plugins/csv_tool/csv_tool.h \
    src/plugins/hash_tool/hash_tool.h \
    src/plugins/image_tool/image_tool.h \
    src/plugins/json_formatter/json_formatter.h \
    src/plugins/regex_tool/regex_tool.h \
    src/plugins/sql_formatter/sql_formatter.h \
    src/plugins/text_tools/text_tools.h \
    src/plugins/timestamp_tool/timestamp_tool.h \
    src/plugins/upload_tool/upload_tool.h \
    src/plugins/upload_tool/upload_config.h \
    src/plugins/upload_tool/project_manage_dialog.h

RESOURCES += \
    resources.qrc
