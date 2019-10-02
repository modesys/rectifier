#ifndef QFluidGridLayout_h_
#define QFluidGridLayout_h_

#include <QLayout>
#include <QGridLayout>
#include <QRect>
#include <QStyle>
#include <QWidgetItem>

class QFluidGridLayout : public QLayout
{
public:

    enum Direction { downToUp, UpToDown };
    QFluidGridLayout(QWidget *parent = nullptr)
        : QLayout(parent)
    {
        setContentsMargins(8,8,8,8);
        setSizeConstraint(QLayout::SetMinAndMaxSize);
    }

    ~QFluidGridLayout() {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    void addItem(QLayoutItem *item) {
        itemList.append(item);
    }

    Qt::Orientations expandingDirections() const {
        return nullptr;
    }

    bool hasHeightForWidth() const {
        return false;
    }

    int heightForWidth(int width) const {
        int height = doLayout(QRect(0, 0, width, 0), true, true);
        return height;
    }

    bool hasWidthForHeight() const {
        return true;
    }

    int widthForHeight(int height) const {
        int width = doLayout(QRect(0, 0, 0, height), true, false);
        return width;
    }

    int count() const {
        return itemList.size();
    }

    QLayoutItem *itemAt(int index) const {
        return itemList.value(index);
    }

    QSize minimumSize() const {
        QSize size;
        QLayoutItem *item;
        foreach (item, itemList)
            size = size.expandedTo(item->minimumSize());
        size += QSize(2*margin(), 2*margin());
        return size;
    }

    void setGeometry(const QRect &rect) {
        QLayout::setGeometry(rect);
        doLayout(rect); 
    }

    QSize sizeHint() const {
        return minimumSize();
    }

    QLayoutItem *takeAt(int index) {
        if (index >= 0 && index < itemList.size())
            return itemList.takeAt(index);
        else
            return nullptr; }
private:
    int doLayout(const QRect &rect, bool testOnly = false, bool width = false) const
    {
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;
        int lineWidth = 0;
        QLayoutItem* item;
        foreach(item,itemList)
        {
            QWidget* widget = item->widget();   
            if (y + item->sizeHint().height() > effectiveRect.bottom() && lineWidth > 0) {
                y = effectiveRect.y();
                x += lineWidth + right;
                lineWidth = 0;
            }
            if (!testOnly) {
                item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
            }
            y += item->sizeHint().height() + top;
            lineHeight = qMax(lineHeight, item->sizeHint().height());
            lineWidth = qMax(lineWidth, item->sizeHint().width());
        }
        if (width) {
            return y + lineHeight - rect.y() + bottom;
        }
        else {
            return x + lineWidth - rect.x() + right;
        }
    }
    QList<QLayoutItem *> itemList;
    Direction dir;
};

#endif // QFluidGridLayout_h_
