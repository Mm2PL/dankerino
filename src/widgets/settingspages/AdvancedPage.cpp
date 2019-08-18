#include "AdvancedPage.hpp"

#include "Application.hpp"
#include "controllers/taggedusers/TaggedUsersController.hpp"
#include "controllers/taggedusers/TaggedUsersModel.hpp"
#include "singletons/Logging.hpp"
#include "singletons/Paths.hpp"
#include "util/Helpers.hpp"
#include "util/LayoutCreator.hpp"

#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QTableView>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrent>

namespace chatterino {

AdvancedPage::AdvancedPage()
    : SettingsPage("Advanced", ":/settings/advanced.svg")
{
    LayoutCreator<AdvancedPage> layoutCreator(this);

    auto tabs = layoutCreator.emplace<QTabWidget>();

    {
        auto layout = tabs.appendTab(new QVBoxLayout, "Cache");
        auto folderLabel = layout.emplace<QLabel>();

        folderLabel->setTextFormat(Qt::RichText);
        folderLabel->setTextInteractionFlags(Qt::TextBrowserInteraction |
                                             Qt::LinksAccessibleByKeyboard |
                                             Qt::LinksAccessibleByKeyboard);
        folderLabel->setOpenExternalLinks(true);

        getSettings()->cachePath.connect([folderLabel](const auto &,
                                                       auto) mutable {
            QString newPath = getPaths()->cacheDirectory();

            QString pathShortened = "Cache saved at <a href=\"file:///" +
                                    newPath +
                                    "\"><span style=\"color: white;\">" +
                                    shortenString(newPath, 50) + "</span></a>";

            folderLabel->setText(pathShortened);
            folderLabel->setToolTip(newPath);
        });

        layout->addStretch(1);

        auto selectDir = layout.emplace<QPushButton>("Set custom cache folder");

        QObject::connect(
            selectDir.getElement(), &QPushButton::clicked, this, [this] {
                auto dirName = QFileDialog::getExistingDirectory(this);

                getSettings()->cachePath = dirName;
            });

        auto resetDir =
            layout.emplace<QPushButton>("Reset custom cache folder");
        QObject::connect(resetDir.getElement(), &QPushButton::clicked, this,
                         []() mutable {
                             getSettings()->cachePath = "";  //
                         });

        // Logs end

        // Timeoutbuttons
        QStringList units = QStringList();
        units.append("s");
        units.append("m");
        units.append("h");
        units.append("d");
        units.append("w");

        std::vector<QString> durationsPerUnit =
            getSettings()->timeoutDurationsPerUnit;
        std::vector<QString>::iterator itDurationPerUnit;
        itDurationPerUnit = durationsPerUnit.begin();

        std::vector<QString> durationUnits =
            getSettings()->timeoutDurationUnits;
        std::vector<QString>::iterator itUnit;
        itUnit = durationUnits.begin();

        for (int i = 0; i < 8; i++)
        {
            durationInputs.append(new QLineEdit());
            unitInputs.append(new QComboBox());
        }
        itDurationInput = durationInputs.begin();
        itUnitInput = unitInputs.begin();

        auto timeoutLayout = tabs.appendTab(new QVBoxLayout, "Timeouts");
        auto texts = timeoutLayout.emplace<QVBoxLayout>().withoutMargin();
        {
            auto infoLabel = texts.emplace<QLabel>();
            infoLabel->setText(
                "Customize your timeout buttons in seconds (s), "
                "minutes (m), hours (h), days (d) or weeks (w).");

            infoLabel->setAlignment(Qt::AlignCenter);

            auto maxLabel = texts.emplace<QLabel>();
            maxLabel->setText("(maximum timeout duration = 2 w)");
            maxLabel->setAlignment(Qt::AlignCenter);
        }
        texts->setContentsMargins(0, 0, 0, 15);
        texts->setSizeConstraint(QLayout::SetMaximumSize);

        // build one line for each customizable button
        for (int i = 0; i < 8; i++)
        {
            auto timeout = timeoutLayout.emplace<QHBoxLayout>().withoutMargin();
            {
                auto buttonLabel = timeout.emplace<QLabel>();
                buttonLabel->setText("Button " + QString::number(i + 1) + ": ");

                QLineEdit *lineEditDurationInput = *itDurationInput;
                lineEditDurationInput->setObjectName(QString::number(i));
                lineEditDurationInput->setValidator(
                    new QIntValidator(1, 99, this));
                lineEditDurationInput->setText(*itDurationPerUnit);
                lineEditDurationInput->setAlignment(Qt::AlignRight);
                lineEditDurationInput->setMaximumWidth(30);
                timeout.append(lineEditDurationInput);

                QComboBox *timeoutDurationUnit = *itUnitInput;
                timeoutDurationUnit->setObjectName(QString::number(i));
                timeoutDurationUnit->addItems(units);
                timeoutDurationUnit->setCurrentText(*itUnit);
                timeout.append(timeoutDurationUnit);

                QObject::connect(lineEditDurationInput, &QLineEdit::textChanged,
                                 this, &AdvancedPage::timeoutDurationChanged);

                QObject::connect(timeoutDurationUnit,
                                 &QComboBox::currentTextChanged, this,
                                 &AdvancedPage::timeoutUnitChanged);

                timeout->addStretch();
            }
            timeout->setContentsMargins(40, 0, 0, 0);
            timeout->setSizeConstraint(QLayout::SetMaximumSize);

            ++itDurationPerUnit;
            ++itUnit;
            ++itDurationInput;
            ++itUnitInput;
        }
        timeoutLayout->addStretch();
    }
    // Timeoutbuttons end
}

void AdvancedPage::timeoutDurationChanged(const QString &newDuration)
{
    QObject *sender = QObject::sender();
    int index = sender->objectName().toInt();

    itDurationInput = durationInputs.begin() + index;
    QLineEdit *durationPerUnit = *itDurationInput;

    itUnitInput = unitInputs.begin() + index;
    QComboBox *cbUnit = *itUnitInput;
    QString unit = cbUnit->currentText();

    int valueInUnit = newDuration.toInt();

    // safety mechanism for setting days and weeks
    if (unit == "d")
    {
        if (valueInUnit > 14)
        {
            durationPerUnit->setText("14");
            return;
        }
    }
    else if (unit == "w")
    {
        if (valueInUnit > 2)
        {
            durationPerUnit->setText("2");
            return;
        }
    }

    std::vector<QString> durationsPerUnit =
        getSettings()->timeoutDurationsPerUnit;
    durationsPerUnit[index] = newDuration;
    getSettings()->timeoutDurationsPerUnit = durationsPerUnit;
}

void AdvancedPage::timeoutUnitChanged(const QString &newUnit)
{
    QObject *sender = QObject::sender();
    int index = sender->objectName().toInt();

    itDurationInput = durationInputs.begin() + index;
    QLineEdit *durationPerUnit = *itDurationInput;

    // safety mechanism for changing units (i.e. to days or weeks)
    AdvancedPage::timeoutDurationChanged(durationPerUnit->text());

    std::vector<QString> durationUnits = getSettings()->timeoutDurationUnits;
    durationUnits[index] = newUnit;
    getSettings()->timeoutDurationUnits = durationUnits;
}

}  // namespace chatterino
