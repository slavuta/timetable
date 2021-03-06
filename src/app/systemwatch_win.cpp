#include "systemwatch_win.h"

#include <QWidget>
#include <windows.h>
#include <QtCore>
#include <QtGui>
#include <QLibrary>

#ifndef PBT_APMSUSPEND
#include <pbt.h>
#endif

class WinSystemWatch::MessageWindow : public QWidget
{
public:
    MessageWindow(WinSystemWatch *parent)
        : syswatch(parent)
    {
        create();

        QLibrary wtsapi32Lib("wtsapi32");

        wtsapi32Lib.load();
        wtsapi32Lib.loadHints();

        if(wtsapi32Lib.isLoaded())
        {
            QLibrary user32Lib("user32");

            user32Lib.load();
            user32Lib.loadHints();

            if(user32Lib.isLoaded())
            {
                WTSRegisterSessionNotification = (RegisterSessionNotification)wtsapi32Lib.resolve("WTSRegisterSessionNotification");
                if(WTSRegisterSessionNotification)
                {
                    WTSRegisterSessionNotification(MessageWindow::winId(),0);
                }
            }
        }
    }

    bool winEvent(MSG *m, long* result)
    {
        if (syswatch->processWinEvent(m, result))
        {
            return true;
        }
        else
        {
            return QWidget::winEvent(m, result);
        }
    }

    WinSystemWatch *syswatch;

    typedef bool (*RegisterSessionNotification)(HWND,DWORD);
    RegisterSessionNotification WTSRegisterSessionNotification;
};

WinSystemWatch::WinSystemWatch() 
{
    _msgWnd = new MessageWindow(this);
}

WinSystemWatch::~WinSystemWatch()
{
    delete _msgWnd;
    _msgWnd = NULL;
}

bool WinSystemWatch::processWinEvent(MSG *m, long* result)
{
    qDebug() << QDateTime::currentDateTime().toString() << "m->message=" << m->message;
    qDebug() << QDateTime::currentDateTime().toString() << "m->wParam=" << m->wParam;

    if(m->message == 689) // WM_WTSSESSION_CHANGE
    {
        if(m->wParam == 0x5)
        {
            qDebug()<<"WTS_SESSION_LOGON"<<"\n";
            emit wakeup();
        }
        else if(m->wParam == 0x6)
        {
            qDebug()<<"WTS_SESSION_LOGOFF"<<"\n";
            emit sleep();
        }
        else if(m->wParam == 0x7)
        {
            qDebug()<<"WTS_SESSION_LOCK"<<"\n";
            emit sleep();
        }
        else if(m->wParam == 0x8)
        {
            qDebug()<<"WTS_SESSION_UNLOCK"<<"\n";
            emit wakeup();
        }
    }

    return false; // Let Qt handle the right return value.
}
