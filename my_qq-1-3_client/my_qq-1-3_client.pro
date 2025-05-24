QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    create_group_pg.cpp \
    join_group_pg.cpp \
    login_page.cpp \
    main.cpp \
    register_page.cpp \
    widget.cpp

HEADERS += \
    create_group_pg.h \
    join_group_pg.h \
    login_page.h \
    message.h \
    recvthread.h \
    register_page.h \
    widget.h

FORMS += \
    create_group_pg.ui \
    join_group_pg.ui \
    login_page.ui \
    register_page.ui \
    widget.ui

TRANSLATIONS += \
    my_qq-1-3_client_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
