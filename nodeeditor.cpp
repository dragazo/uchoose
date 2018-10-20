#include <utility>
#include <limits>

#include "nodeeditor.h"
#include "ui_nodeeditor.h"

NodeEditor::NodeEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NodeEditor)
{
    ui->setupUi(this);

    // -- set up the arc layout -- //

    arcLayoutWidget = new QWidget;
    arcLayout = new QVBoxLayout;

    ui->ChoicesScrollArea->setWidget(arcLayoutWidget);
    arcLayoutWidget->setLayout(arcLayout);
}

NodeEditor::~NodeEditor()
{
    delete ui;
}

// -- accessors -- //

QString NodeEditor::title() const
{
    return ui->TitleText->text();
}
void NodeEditor::title(const QString &str)
{
    ui->TitleText->setText(str);
}

QString NodeEditor::text() const
{
    return ui->TextText->toPlainText();
}
void NodeEditor::text(const QString &str)
{
    ui->TextText->setPlainText(str);
}

void NodeEditor::addArc(std::size_t dest, const QString &text)
{
    // create the arc info container
    ArcInfoWidgets w;

    // create the layout object
    w.layout = new QHBoxLayout;

    // add them to the layout object
    w.layout->addWidget(w.check = new QCheckBox);
    w.layout->addWidget(w.dest = new QSpinBox);
    w.layout->addWidget(w.text = new QLineEdit);

    // add the layout object to display
    arcLayout->addLayout(w.layout);

    // set initial data
    w.check->setCheckState(Qt::Checked);
    w.dest->setMinimum(0); // this should be the default, but is put in to be explicit anbout it
    w.dest->setMaximum(std::numeric_limits<int>::max());
    w.dest->setValue(int(dest));
    w.text->setText(text);

    // add the widgets manager entry to the array
    arcInfo.push_back(w);
}
std::vector<NodeEditor::ArcInfo> NodeEditor::getArcs() const
{
    std::vector<ArcInfo> info; // the info array
    ArcInfo arc; // a single arc info entry to populate

    // for each arc we have
    for (auto &i : arcInfo)
    {
        // if this arc is flagged as enabled
        if (i.check->checkState() == Qt::Checked)
        {
            // populate its data
            arc.dest = decltype(arc.dest)(i.dest->value());
            arc.text = i.text->text();

            // add it to the list
            info.emplace_back(std::move(arc));
        }
    }

    // return it
    return info;
}

// -- button impl -- //

void NodeEditor::on_CancelButton_clicked()
{
    reject();
}

void NodeEditor::on_OKButton_clicked()
{
    accept();
}

void NodeEditor::on_AddArcButton_clicked()
{
    addArc(0, "N/A");
}
