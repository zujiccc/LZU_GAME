# test02.pro - Qt项目文件
QT += core gui widgets xml
CONFIG += c++11

TARGET = test02
TEMPLATE = app

# 源文件
SOURCES += \
    main.cpp \
    widget.cpp \
    tmxmap.cpp

# 头文件
HEADERS += \
    PlayerItem.h \
    widget.h \
    tmxmap.h

# 翻译文件（如果需要）
TRANSLATIONS += test02_zh_CN.ts

# 如果使用MOC（元对象编译器）
FORMS +=

# 语言标准
QMAKE_CXXFLAGS += -std=c++11
