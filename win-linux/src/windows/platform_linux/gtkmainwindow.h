#ifndef GTKMAINWINDOW_H
#define GTKMAINWINDOW_H

#include <QWidget>
#include <QIcon>
#include <QCloseEvent>
#include <functional>

typedef std::function<bool(QEvent *ev)> FnEvent;
typedef std::function<void(QCloseEvent*)> FnCloseEvent;


class GtkMainWindowPrivate;
class GtkMainWindow
{
public:
    GtkMainWindow(QWidget *underlay, bool isCustomStyle, const FnEvent &qev, const FnCloseEvent &qcev);
    ~GtkMainWindow();

    void move(const QPoint &pos);
    void setGeometry(const QRect &rc);
    void setWindowIcon(const QIcon &icon);
    void setWindowTitle(const QString &title);
    void setBackgroundColor(const QString &color);
    void setFocus();
    void setWindowState(Qt::WindowStates ws);
    void show();
    void showMinimized();
    void showMaximized();
    void showNormal();
    void activateWindow();
    void setMinimumSize(int w, int h);
    void updateGeometry();
    void update();
    void hide() const;
    bool isMaximized();
    bool isMinimized();
    bool isActiveWindow();
    bool isFocused();
    bool isVisible() const;
    bool isHidden() const;
    QString windowTitle() const;
    QPoint mapToGlobal(const QPoint &pt) const;
    QPoint mapFromGlobal(const QPoint &pt) const;
    QSize size() const;
    QPoint pos() const;
    QRect geometry() const;
    QRect normalGeometry() const;
    Qt::WindowStates windowState() const;

private:
    GtkMainWindowPrivate *pimpl;
};

#endif // GTKMAINWINDOW_H
