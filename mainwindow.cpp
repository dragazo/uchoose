#include <QCursor>
#include <QPainter>
#include <QRegion>
#include <QStatusBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>

#include <cmath>
#include <algorithm>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "nodeeditor.h"

// -------------- //

// -- settings -- //

// -------------- //

constexpr qreal NodeRadius = 25; // the radius of a node in pixels

constexpr qreal ArrowWidth = 4;   // the radius of an arc arrow
constexpr qreal ArrowHeight = 16; // the height of an arc arrow
constexpr qreal ArrowRecess = 5;  // the recess amount for the very tip of the arrow

constexpr qreal SelectHaloRadius = 30; // the radius for a selection halo

constexpr int DragSleepTime = 16;   // the frequency of drag   action frame updates (milliseconds)
constexpr int SelectSleepTime = 16; // the frequency of select action frame updates (milliseconds)

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
    // -- set up auto-generated ui -- //

    ui->setupUi(this);

    // -- build the background context menu -- //

    background_context = new QMenu(this);

    background_context->addAction("Add Node", this, SLOT(background_context_add_node()));

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

void MainWindow::performSelect(QRectF rect, bool mod)
{
    // if we're modifying the current selection
    if (mod)
    {
        // for each node
        for (auto i = map.begin(), _end = map.end(); i != _end; ++i)
        {
            // if this is in the rectangle
            if (intersects(rect, i->data.point))
            {
                // find an equivalent item already in the selection
                auto eq = std::find(selection.begin(), selection.end(), i);

                // if it's not in the selection, add it
                if (eq == selection.end()) selection.push_back(i);
                // otherwise it's already selected - remove it
                else selection.erase(eq);
            }
        }
    }
    // otherwise we're starting a new selection
    else
    {
        // clear the old selection
        selection.clear();

        // for each node
        for (auto i = map.begin(), _end = map.end(); i != _end; ++i)
        {
            // if this is in the rectangle
            if (intersects(rect, i->data.point))
            {
                // add it to the selection
                selection.push_back(i);
            }
        }
    }

    update();
}
void MainWindow::performSelect(Map_t::iterator node, bool mod)
{
    // if we're modifying the current selection
    if (mod)
    {
        // find an equivalent item already in the selection
        auto eq = std::find(selection.begin(), selection.end(), node);

        // if it's not in the selection, add it
        if (eq == selection.end()) selection.push_back(node);
        // otherwise it's already selected - remove it
        else selection.erase(eq);
    }
    // otherwise we're starting a new selection
    else
    {
        // clear the old selection
        selection.clear();

        // add it to the selection
        selection.push_back(node);
    }

    update();
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
        painter.drawEllipse(QRectF(i->data.point.x() - SelectHaloRadius, i->data.point.y() - SelectHaloRadius,
                                   2 * SelectHaloRadius, 2 * SelectHaloRadius));
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
        drag_moved = false;

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
        drag_moved = true;

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

void MainWindow::openMainContext(QPoint point)
{
    // store the context point
    context_point = point;

    // open the context menu (convert to screen coords)
    background_context->popup(point + this->pos());
}
void MainWindow::background_context_add_node()
{
    // create a default node
    Node_t node;
    node.data.point = context_point;

    // add it to the map
    map.emplace_back(std::move(node));

    // THIS INVALIDATES ITERATORS - clear the selection
    selection.clear();

    // update the display
    update();
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    // if this was a left click
    if (e->button() == Qt::LeftButton)
    {
        // get the node we're over (may not be)
        auto node = overNode(e->pos());

        // if we were over a node, begin a drag
        if (node != map.end()) _begin_drag(node, e->pos());
        // otherwise begin a selection
        else _begin_select(e->pos());
    }
    // if this was a right click
    else if(e->button() == Qt::RightButton)
    {
        // get the node we're over (may not be)
        auto node = overNode(e->pos());

        // if we weren't over a node, open the main context menu
        if (node == map.end()) openMainContext(e->pos());
    }

    e->accept();
}
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    // if the released button was a left click
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        // if we were in a drag action
        if (drag_timer_id != 0)
        {
            // record the drag_moved flag and end the drag action
            bool moved = drag_moved;
            _end_drag(e->pos());

            // if we didn't move
            if (!moved)
            {
                // get the node we're over
                auto node = overNode(e->pos());

                // perform a selection on it (sanity check for null)
                if (node != map.end()) performSelect(node, QApplication::keyboardModifiers() & Qt::ControlModifier);
            }
        }

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




















