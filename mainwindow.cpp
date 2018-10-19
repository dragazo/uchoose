#include <QCursor>
#include <QPainter>
#include <QRegion>
#include <QStatusBar>

#include <cmath>
#include <algorithm>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "nodeeditor.h"

// -------------- //

// -- settings -- //

// -------------- //

const QBrush NodeBrush(Qt::NoBrush);
const QPen   NodePen(QBrush(Qt::black), 3);

const QBrush ArcBrush(Qt::black);
const QPen   ArcPen(ArcBrush, 3, Qt::SolidLine, Qt::FlatCap, Qt::PenJoinStyle::MiterJoin);

const QBrush SelectedNodeBrush(Qt::NoBrush);
const QPen   SelectedNodePen(QBrush(0xefb12b), 3, Qt::DashDotLine);

const QBrush SelectionRectBrush(Qt::NoBrush);
const QPen   SelectionRectPen(QBrush(0xefb12b), 3, Qt::DashDotLine);

// ----------------- //

// -- ctor / dtor -- //

// ----------------- //

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

// ------------- //

// -- utility -- //

// ------------- //

QRectF MainWindow::boundingRect(QPointF a, QPointF b)
{
    return QRectF(std::min(a.x(), b.x()), std::min(a.y(), b.y()),
                  std::abs(a.x() - b.x()), std::abs(a.y() - b.y()));
}
QRectF MainWindow::inflate(QRectF rect, qreal padding)
{
    return QRectF(rect.x() - padding, rect.y() - padding,
                  rect.width() + 2 * padding, rect.height() + 2 * padding);
}

bool MainWindow::intersects(QRectF rect, QPointF point)
{
    return point.x() >= rect.left() && point.x() <= rect.right()
            && point.y() >= rect.top() && point.y() <= rect.bottom();
}

MainWindow::Map_t::iterator MainWindow::overNode(QPointF point)
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

void MainWindow::performSelect(QRectF rect, bool add)
{
    // if we're not adding, start by clearing the selection
    if (!add) selection.clear();

    // for each node
    for (auto i = map.begin(), _end = map.end(); i != _end; ++i)
    {
        // if this is in the rectangle
        if (intersects(rect, i->data.point))
        {
            // add it to the selection if it's not already in there
            if (std::none_of(selection.begin(), selection.end(), [i](Map_t::iterator other){return other == i;})) selection.push_back(i);
        }
    }

    this->setWindowTitle(QString('a'+selection.size()));
}

// --------------- //

// -- rendering -- //

// --------------- //

void MainWindow::paintNode(const Node_t &node, QPainter &painter)
{
    painter.drawEllipse(QRectF(node.data.point.x() - NodeRadius, node.data.point.y() - NodeRadius, 2*NodeRadius, 2*NodeRadius));

    painter.drawText(node.data.point, node.data.text);
}
void MainWindow::paintArc(const Node_t &from_node, const Arc_t &arc, QPainter &painter)
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
    painter.setBrush(NodeBrush);
    painter.setPen(NodePen);
    for (const auto &i : map) paintNode(i, painter);

    // paint each arc
    painter.setBrush(ArcBrush);
    painter.setPen(ArcPen);
    for (const auto &i : map) for (const auto &j : i.arcs) paintArc(i, j, painter);

    // for each selected node
    painter.setBrush(SelectedNodeBrush);
    painter.setPen(SelectedNodePen);
    for (auto i : selection)
    {
        // draw a halo around it
        painter.drawEllipse(QRectF(i->data.point.x() - SelectHaloOuterRadius, i->data.point.y() - SelectHaloOuterRadius,
                                   2 * SelectHaloOuterRadius, 2 * SelectHaloOuterRadius));
    }

    // if we're in a selection
    painter.setBrush(SelectionRectBrush);
    painter.setPen(SelectionRectPen);
    if (select_timer_id != 0)
    {
        // paint the selection rect
        painter.drawRect(boundingRect(select_start, select_stop));
    }
}

// -------------- //

// -- controls -- //

// -------------- //

void MainWindow::_begin_drag(Map_t::iterator node, QPointF mouse_start)
{
    // for safety, only do this if we're not in a drag action
    if (drag_timer_id == 0)
    {
        // record initial data
        drag_start = mouse_start;
        drag_stop = mouse_start;

        // if the drag node is in the selection
        if (std::any_of(selection.begin(), selection.end(), [node](Map_t::iterator o){return o==node;}))
        {
            // populate drag_info
            drag_info.resize(selection.size());
            for (std::size_t i = 0; i < selection.size(); ++i)
                drag_info[i] = {selection[i], selection[i]->data.point};
        }
        // otherwise drag node is not selected
        else
        {
            // deselect everything
            selection.clear();

            // only the dragged node will be dragged
            drag_info.resize(1);
            drag_info[0] = {node, node->data.point};
        }

        // start the timer
        drag_timer_id = startTimer(DragSleepTime);
    }
}
void MainWindow::_mid_drag(QPointF mouse_stop)
{
    // for efficiency, only do this if the mouse moved
    if (drag_stop != mouse_stop)
    {
        drag_stop = mouse_stop;

        // compute the net position difference
        QPointF dr = mouse_stop - drag_start;

        // perform the node repositioning
        for (auto &i : drag_info)
            i.node->data.point = i.origin + dr;

        // update display
        update();
    }
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
    // drag back to the starting position (as if nothing happened)
    _end_drag(drag_start);
}

void MainWindow::_begin_select(QPointF mouse_start)
{
    // for safety only do this if we're not in a select event
    if (select_timer_id == 0)
    {
        // record the initial data
        select_start = mouse_start;
        select_stop = mouse_start;

        // start the timer
        select_timer_id = startTimer(SelectSleepTime);
    }
}
void MainWindow::_mid_select(QPointF mouse_stop)
{
    // for efficiency, only do this if the mouse moved
    if (select_stop != mouse_stop)
    {
        select_stop = mouse_stop;

        // redraw
        update();
    }
}
void MainWindow::_end_select(QPointF mouse_stop)
{
    // for safety, only do this if we're in a select action
    if (select_timer_id != 0)
    {
        // stop the timer
        killTimer(select_timer_id);
        select_timer_id = 0;

        // perform the selection
        performSelect(boundingRect(select_start, mouse_stop),
                      QApplication::keyboardModifiers() & Qt::ControlModifier);

        // update the frame
        update();
    }
}
void MainWindow::_cancel_select()
{
    // for safety, only do this if we're in a select action
    if (select_timer_id != 0)
    {
        // stop the timer
        killTimer(select_timer_id);
        select_timer_id = 0;

        // redraw
        update();
    }
}

void MainWindow::prompt_editor(Map_t::iterator node)
{
    // create the editor
    NodeEditor editor;

    // provide editor with the current data
    editor.title(node->data.title);
    editor.text(node->data.text);

    // if the user says ok, store the changes
    if (editor.exec() == QDialog::Accepted)
    {
        // save the new data
        node->data.title = editor.title();
        node->data.text = editor.text();

        // redraw with new data
        update();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    // if this was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // get the node we're over (may not be)
        auto node = overNode(e->pos());

        // if we were over a node, begin a drag
        if (node != map.end()) _begin_drag(node, e->pos());
        // otherwise begin a selection
        else _begin_select(e->pos());
    }

    e->accept();
}
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    // if the released button was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // if we were in a drag action, end it
        if (drag_timer_id != 0) _end_drag(e->pos());
        // if we were in a select action, end it
        if (select_timer_id != 0) _end_select(e->pos());
    }

    e->accept();
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    // if this was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // cancel any drag action we might be in
        _cancel_drag();

        // get the node we're over (may not be)
        auto node = overNode(e->pos());

        // if we were over a node, open an editor for it
        if (node != map.end()) prompt_editor(node);
    }

    e->accept();
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    // handle drag action
    if (e->timerId() == drag_timer_id) _mid_drag(mapFromGlobal(QCursor::pos()));
    // handle select action
    else if(e->timerId() == select_timer_id) _mid_select(mapFromGlobal(QCursor::pos()));

    e->accept();
}




















