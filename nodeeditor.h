#ifndef NODEEDITOR_H
#define NODEEDITOR_H

#include <QDialog>
#include <QString>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

#include <vector>

namespace Ui {
class NodeEditor;
}

class NodeEditor : public QDialog
{
    Q_OBJECT

private: // -- implementation types -- //

    struct ArcInfoWidgets
    {
        QLayout *layout; // the layout object that is the parent of the following:

        QCheckBox *check; // the widget representing if this arc should remain in the arc collection
        QSpinBox  *dest;  // the widget representing the dest state of an arc
        QLineEdit *text;  // the widget representing the text of an arc
    };

public: // -- types -- //

    struct ArcInfo
    {
    public:
        std::size_t dest; // the destination state for picking this arc
        QString     text; // the descriptive text for this arc
    };

private: // -- data -- //

    Ui::NodeEditor *ui; // auto-generated ui object

    QWidget    *arcLayoutWidget; // the layout object container
    QBoxLayout *arcLayout;       // the layout manager for displaying arcs

    std::vector<ArcInfoWidgets> arcInfo; // info for all the arcs in this editor

public: // -- ctor / dtor -- //

    explicit NodeEditor(QWidget *parent = nullptr);
    virtual ~NodeEditor() override;

public: // -- accessors -- //

    // gets/sets the title field
    QString title() const;
    void title(const QString &str);

    // gets/sets the text field
    QString text() const;
    void text(const QString &str);

    // adds an arc info entry
    void addArc(std::size_t dest, const QString &text);
    // gets all the arc info entries.
    std::vector<ArcInfo> getArcs() const;

private slots: // -- auto-linked slots -- //

    void on_CancelButton_clicked();

    void on_OKButton_clicked();

    void on_AddArcButton_clicked();
};

#endif // NODEEDITOR_H
