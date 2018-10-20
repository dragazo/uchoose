#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QPointF>
#include <QTimerEvent>
#include <QMenu>

#include "adventure_map.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

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

    // the block of info used for drag events
    struct DragInfo
    {
        Map_t::iterator node;   // the node being dragged
        QPointF         origin; // the original position before the drag began
    };

private: // -- data -- //

    Ui::MainWindow *ui; // ui component (generated)

    Map_t map; // the adventure map to use for execution/rendering

    int drag_timer_id = 0; // the timer for the drag updater (zero if we're not in a drag event)
    QPointF drag_start;    // the starting position of the drag
    QPointF drag_stop;     // the ending position of the drag
    bool    drag_moved;    // marks if the mouse actually moved during the drag event (at all, not just net)
    std::vector<DragInfo> drag_info; // the info for each drag entity

    int select_timer_id = 0;  // the timer for the select updater (zero if we're not in a select event)
    QPointF select_start; // the mouse start point for a select event
    QPointF select_stop;  // the current stop position of the select action.

    std::vector<Map_t::iterator> selection; // all the nodes that are currently selected

    QPoint context_point;      // the position of the currently-opened context menu
    QMenu *background_context; // the context menu to use for right clicking in the background

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
    // if <mod> is false, clears the current selection and selects the items.
    // if <mod> is true, toggles items into / out of the selection.
    void performSelect(QRectF rect, bool mod);
    // as the performSelect() taking rect, but only affects a single node
    void performSelect(Map_t::iterator node, bool mod);

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

    // opens the main context menu at the specified point
    void openMainContext(QPoint point);

private slots: // -- private slot helpers -- //

    void background_context_add_node();

protected: // -- event overrides -- //

    virtual void paintEvent(QPaintEvent *e) override;

    virtual void mousePressEvent(QMouseEvent *e) override;
    virtual void mouseReleaseEvent(QMouseEvent *e) override;

    virtual void mouseDoubleClickEvent(QMouseEvent *e) override;

    virtual void timerEvent(QTimerEvent *e) override;
};

#endif // MAINWINDOW_H
