#include <QCursor>
#include <QPainter>
#include <cmath>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "nodeeditor.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // !! TEMP STUFF !! //

    decltype(map)::Node node;
    decltype(map)::Arc arc;

    node.data.title = "first";
    node.data.text = "hello this is bob";
    node.data.point = QPoint(50, 50);

    arc.dest = 1; // arc from 0 -> 1
    arc.data.text = "choice 1";
    node.arcs.push_back(arc);
    map.push_back(node);

    node.data.title = "second";
    node.data.text = "hello this is fred";
    node.data.point = QPoint(200, 80);
    map.push_back(node);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// -- rendering -- //

void MainWindow::_paintNode(const Node_t &node, QPainter &painter)
{
    painter.drawEllipse(QRectF(node.data.point.x() - NodeRadius, node.data.point.y() - NodeRadius, 2*NodeRadius, 2*NodeRadius));

    painter.drawText(node.data.point, node.data.text);
}
void MainWindow::_paintArc(const Node_t &from_node, const Arc_t &arc, QPainter &painter)
{
    // if this arc is valid
    if (arc.dest < map.size())
    {
        // get the start and stop points
        QPointF start(from_node.data.point), stop(map[arc.dest].data.point);

        QPointF dir = stop - start;
        qreal mag = std::sqrt(dir.x() * dir.x() + dir.y() * dir.y());

        // if mag is such that the resulting line will be visible
        if (mag >= 2 * NodeRadius)
        {
            dir /= mag; // normalize dir
            QPointF right(-dir.y(), dir.x()); // create a vector pointing to the right of dir

            // correct the start/stop points
            start += dir * NodeRadius;
            stop -= dir * NodeRadius;

            // draw the corrected points
            painter.drawLine(start, stop - dir * ArrowHeight);

            // draw the arrow head
            QPointF arrow_points[] =
            {
                stop - dir * ArrowRecess,
                stop - dir * ArrowHeight + right * ArrowWidth,
                stop - dir * ArrowHeight - right * ArrowWidth,
            };
            painter.drawConvexPolygon(arrow_points, sizeof(arrow_points) / sizeof(arrow_points[0]));
        }
    }
    // otherwise is terminal
    else
    {
        // draw a special terminal symbol
        QPointF start(from_node.data.point);
        painter.drawLine(start, start + QPointF(0, 20));
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    // create a painter object
    QPainter painter(this);

    // paint each node
    for (const auto &i : map) _paintNode(i, painter);

    QBrush brush(QColor(197, 70, 250));
    QPen arc_pen(brush, 4, Qt::PenStyle::SolidLine, Qt::PenCapStyle::FlatCap, Qt::PenJoinStyle::MiterJoin);
    painter.setPen(arc_pen);

    // paint each arc
    for (const auto &i : map) for (const auto &j : i.arcs) _paintArc(i, j, painter);
}

// -- controls -- //

MainWindow::Map_t::iterator MainWindow::_overNode(QPointF point)
{
    // we'll do the distance comparison in terms of squares for speed
    const auto sqr_radius = NodeRadius * NodeRadius;

    // for each node
    QPointF d;
    auto i = map.begin(), _end = map.end();
    for (; i != _end; ++i)
    {
        // compute position difference
        d = point - i->data.point;

        // if we're within this node, stop searching
        if (d.x() * d.x() + d.y() * d.y() <= sqr_radius) break;
    }

    // return the found position
    return i;
}

void MainWindow::_begin_drag(Map_t::iterator node, QPointF mouse_start)
{
    // for safety, only do this if we're not in a drag action
    if (drag_timer_id == 0)
    {
        // record initial data
        drag_node = node;
        drag_node_start = node->data.point;
        drag_mouse_start = mouse_start;

        // start the timer
        drag_timer_id = startTimer(DragSleepTime);
    }
}
void MainWindow::_mid_drag(QPointF mouse_stop)
{
    // perform the node repositioning
    drag_node->data.point = drag_node_start + (mouse_stop - drag_mouse_start);
    update();
}
void MainWindow::_end_drag(QPointF mouse_stop)
{
    // for safety, only do this if we're in a drag action
    if (drag_timer_id != 0)
    {
        // stop the timer
        killTimer(drag_timer_id);
        drag_timer_id = 0;

        // perform the final node repositioning
        _mid_drag(mouse_stop);
    }
}
void MainWindow::_cancel_drag()
{
    // move the node back to the starting position (as if it never happened)
    _end_drag(drag_mouse_start);
}

void MainWindow::_prompt_editor(Map_t::iterator node)
{
    // create the editor
    NodeEditor editor;

    editor.title(node->data.title);
    editor.text(node->data.text);

    // if the user says ok, store the changes
    if (editor.exec() == QDialog::Accepted)
    {
        node->data.title = editor.title();
        node->data.text = editor.text();

        update();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    // default to ignoring the event
    e->ignore();

    // if this was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // get the node we're over
        auto node = _overNode(e->pos());
        // if we were indeed over a node
        if (node != map.end())
        {
            // begin a drag
            _begin_drag(node, e->pos());

            e->accept();
        }
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    // default to ignoring the event
    e->ignore();

    // if the released button was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // if we were in a drag action
        if (drag_timer_id != 0)
        {
            // end the drag
            _end_drag(e->pos());

            e->accept();
        }
    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    // default to ignoring the event
    e->ignore();

    // if this was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // cancel any drag action we might be in
        _cancel_drag();

        // get the node we're over
        auto node = _overNode(e->pos());
        // if we were indeed over a node
        if (node != map.end())
        {
            // open the editor for this node
            _prompt_editor(node);

            e->accept();
        }
    }
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    // if the timer was for a drag action
    if (e->timerId() == drag_timer_id)
    {
        // update the animation
        _mid_drag(mapFromGlobal(QCursor::pos()));
    }
}




















