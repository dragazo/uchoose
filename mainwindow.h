#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qpainter.h>
#include <QMouseEvent>
#include <QPointF>
#include <QTimerEvent>

#include "adventure_map.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private: // -- settings -- //

    static constexpr qreal NodeRadius = 25; // the radius of a node in pixels

    static constexpr qreal ArrowWidth = 4;   // the radius of an arc arrow
    static constexpr qreal ArrowHeight = 16; // the height of an arc arrow
    static constexpr qreal ArrowRecess = 5;  // the recess amount for the very tip of the arrow

    static constexpr int DragSleepTime = 16; // the frequency of drag action frame updates (milliseconds)

private: // -- types -- //

    struct NodePayload
    {
        QPointF point;

        QString title;
        QString text;
    };
    struct ArcPayload
    {
        QString text;
    };

    typedef AdventureMap<NodePayload, ArcPayload> Map_t;
    typedef Map_t::Node Node_t;
    typedef Map_t::Arc  Arc_t;

private: // -- data -- //

    Ui::MainWindow *ui; // ui component (generated)

    Map_t map; // the adventure map to use for execution/rendering

    int drag_timer_id = 0;     // the timer if for the drag updater (zero if we're not in a drag event)
    Map_t::iterator drag_node; // iterator to the node being dragged (undefined if not in drag action)
    QPointF drag_node_start;   // the position of the node when the drag started (und if not in drag)
    QPointF drag_mouse_start;  // the position of the mouse when the drag started (und. etc.)

public: // -- ctor / dtor / asgn -- //

    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() override;

private: // -- helpers -- //

    // helpers for painting nodes and arcs
    void _paintNode(const Node_t &node, QPainter &paint);
    void _paintArc(const Node_t &from_node, const Arc_t &arc, QPainter &paint);

    // finds the (first) node that the given point is within. returns map.end() if there is no such node.
    Map_t::iterator _overNode(QPointF point);

    // these process node drag begin/end
    void _begin_drag(Map_t::iterator node, QPointF mouse_start);
    void _mid_drag(QPointF mouse_stop);
    void _end_drag(QPointF mouse_stop);
    void _cancel_drag();

    // opens an editor interface for the given node
    void _prompt_editor(Map_t::iterator node);

protected: // -- event overrides -- //

    virtual void paintEvent(QPaintEvent *e) override;

    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *e) override;

    virtual void timerEvent(QTimerEvent *e) override;
};

#endif // MAINWINDOW_H
