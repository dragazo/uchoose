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

    static constexpr int DragSleepTime = 16;   // the frequency of drag   action frame updates (milliseconds)
    static constexpr int SelectSleepTime = 16; // the frequency of select action frame updates (milliseconds)

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

    int drag_timer_id = 0;     // the timer for the drag updater (zero if we're not in a drag event)
    Map_t::iterator drag_node; // iterator to the node being dragged (undefined if not in drag action)
    QPointF drag_node_start;   // the position of the node when the drag started (und if not in drag)
    QPointF drag_mouse_start;  // the position of the mouse when the drag started (und. etc.)
    QPointF drag_mouse_stop;   // the current stop position of the drag action.

    int select_timer_id = 0;    // the timer for the select updater (zero if we're not in a select event)
    QPointF select_mouse_start; // the mouse start point for a select event
    QPointF select_mouse_stop;  // the current stop position of the select action.

    std::vector<Map_t::iterator> selection; // all the nodes that are currently selected

public: // -- ctor / dtor / asgn -- //

    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() override;

private: // -- helpers -- //

    // returns the smallest rectangle containing the specified points
    static QRectF boundingRect(QPointF a, QPointF b);
    // inflates the rectangle by <padding> on all sides
    static QRectF inflate(QRectF rect, qreal padding);

    // returns true iff the rect intersects the given point
    static bool intersects(QRectF rect, QPointF point);

    // finds the (first) node that the given point is within. returns map.end() if there is no such node.
    Map_t::iterator overNode(QPointF point);

    // performs a selection action for every node in the given rectangle.
    // if add is true, adds items to the current selection, otherwise clears current selection before adding the new selection.
    void performSelect(QRectF rect, bool add);

    // helpers for painting nodes and arcs
    void paintNode(const Node_t &node, QPainter &paint);
    void paintArc(const Node_t &from_node, const Arc_t &arc, QPainter &paint);

    // these process node drag subactions
    void _begin_drag(Map_t::iterator node, QPointF mouse_start);
    void _mid_drag(QPointF mouse_stop);
    void _end_drag(QPointF mouse_stop);
    void _cancel_drag();

    // these process select subactions
    void _begin_select(QPointF mouse_start);
    void _mid_select(QPointF mouse_stop);
    void _end_select(QPointF mouse_stop);
    void _cancel_select();

    // opens an editor interface for the given node
    void prompt_editor(Map_t::iterator node);

protected: // -- event overrides -- //

    virtual void paintEvent(QPaintEvent *e) override;

    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *e) override;

    virtual void timerEvent(QTimerEvent *e) override;
};

#endif // MAINWINDOW_H
