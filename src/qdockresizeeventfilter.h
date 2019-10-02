#ifndef QDockResizeEventFilter_h_
#define QDockResizeEventFilter_h_

#include <QObject>
#include <QLayout>
#include <QEvent>
#include <QDockWidget>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QMouseEvent>

#include "../Rectifier/src/qfluidgridlayout.h"
#include "../Rectifier/ui/mainwindow.h"


class QDockResizeEventFilter : public QObject
{

public:
    friend QMainWindow;
    friend QLayoutPrivate;
    QDockResizeEventFilter(QWidget* dockChild, QFluidGridLayout* layout, QObject* parent = nullptr)
        : QObject(parent), m_dockChild(dockChild), m_layout(layout)
    {

    }

protected:

    bool eventFilter(QObject *p_obj, QEvent *p_event)
    {  
        if (p_event->type() == QEvent::Resize)
        {
            QResizeEvent* resizeEvent   = static_cast<QResizeEvent*>(p_event);
            QMainWindow* mainWindow     = dynamic_cast<QMainWindow*>(p_obj->parent());              
            QDockWidget* dock           = static_cast<QDockWidget*>(p_obj);

            // determine resize direction
            if (resizeEvent->oldSize().height() != resizeEvent->size().height())
            {
                // vertical expansion
                QSize fixedSize(m_layout->widthForHeight(m_dockChild->size().height()), m_dockChild->size().height());
                if (dock->size().width() != fixedSize.width())
                {
                    m_dockChild->setFixedWidth(fixedSize.width());
                    dock->setFixedWidth(fixedSize.width());

                    // cause mainWindow dock layout recalculation
                    QDockWidget* dummy = new QDockWidget;
                    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, dummy);
                    mainWindow->removeDockWidget(dummy);

                    // adding dock widgets causes the separator move event to end
                    // restart it by synthesizing a mouse press event
                    QPoint mousePos = mainWindow->mapFromGlobal(QCursor::pos());
                    mousePos.setY(dock->rect().bottom());
                    QCursor::setPos(mainWindow->mapToGlobal(mousePos));
                    QMouseEvent* grabSeparatorEvent = new QMouseEvent(QMouseEvent::MouseButtonPress,mousePos,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                    qApp->postEvent(mainWindow, grabSeparatorEvent);
                }
            }
            if (resizeEvent->oldSize().width() != resizeEvent->size().width())
            {
                // Do nothing
            }           
        }   
        return false;
    }

private:

    QWidget* m_dockChild;
    QFluidGridLayout* m_layout;
};

#endif // QDockResizeEventFilter_h__
