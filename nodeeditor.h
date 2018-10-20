#ifndef NODEEDITOR_H
#define NODEEDITOR_H

#include <QDialog>
#include <QString>

namespace Ui {
class NodeEditor;
}

class NodeEditor : public QDialog
{
    Q_OBJECT

public:
    explicit NodeEditor(QWidget *parent = nullptr);
    virtual ~NodeEditor() override;

public: // -- accessors -- //

    // gets/sets the title field
    QString title() const;
    void title(const QString &str);

    // gets/sets the text field
    QString text() const;
    void text(const QString &str);

private slots: // -- auto-linked slots -- //

    void on_CancelButton_clicked();

    void on_OKButton_clicked();

private:
    Ui::NodeEditor *ui;
};

#endif // NODEEDITOR_H
