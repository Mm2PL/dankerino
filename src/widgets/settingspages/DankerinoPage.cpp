#include "DankerinoPage.hpp"

#include "Application.hpp"
#include "common/Version.hpp"
#include "singletons/Settings.hpp"
#include "widgets/BaseWindow.hpp"
#include "widgets/helper/Line.hpp"
#include "widgets/settingspages/GeneralPageView.hpp"
#include "widgets/settingspages/SettingWidget.hpp"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFontDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>

namespace chatterino {

DankerinoPage::DankerinoPage()
{
    auto *y = new QVBoxLayout;
    auto *x = new QHBoxLayout;
    auto *view = GeneralPageView::withNavigation(this);
    this->view_ = view;
    x->addWidget(view);
    auto *z = new QFrame;
    z->setLayout(x);
    y->addWidget(z);
    this->setLayout(y);

    this->initLayout(*view);
}

bool DankerinoPage::filterElements(const QString &query)
{
    if (this->view_)
        return this->view_->filterElements(query) || query.isEmpty();
    else
        return false;
}

void DankerinoPage::initLayout(GeneralPageView &layout)
{
    auto &s = *getSettings();

    layout.addTitle("Appearance");
    layout.addCheckbox("Show placeholder in text input box",
                       s.showTextInputPlaceholder);
    layout.addDescription("Gray-out recent messages was upstreamed as \"Reduce "
                          "opacity of message history\"");
    layout.addTitle("Behavior");
    layout.addCheckbox("Lowercase tab-completed usernames",
                       s.lowercaseUsernames);
    {
        auto *groupLayout = new QFormLayout();
        auto *lineEdit = this->createLineEdit(s.bridgeUser);
        groupLayout->addRow(
            this->createCheckBox("Allow \"bridge\" users to impersonate others",
                                 s.allowBridgeImpersonation));
        lineEdit->setPlaceholderText("supabridge");
        groupLayout->addRow("Bridge user:", lineEdit);
        layout.addLayout(groupLayout);
    }
    //layout.addTitle("Emotes");
    //layout.addCheckbox("Enable loading 7TV emotes", s.enableLoadingSevenTV);
    layout.addTitle("Miscellaneous");
    layout.addIntInput("High rate limit spam delay in milliseconds (mod/vip)",
                       s.twitchHighRateLimitDelay, 100, 2000, 100);
    layout.addIntInput(
        "Low rate limit spam delay in milliseconds (non mod/vip)",
        s.twitchLowRateLimitDelay, 500, 3000, 1100);

    if (s.dankerinoThreeLetterApiEasterEgg)
    {
        layout.addCheckbox("Click to disable GraphQL easter egg and "
                           "advanced settings "
                           "(requires restart)",
                           s.dankerinoThreeLetterApiEasterEgg);
        layout.addTitle("Random 'hacks'");
        layout.addCheckbox("Enable. Required for settings below to work!",
                           s.nonceFuckeryEnabled);
        layout.addCheckbox("Abnormal nonce detection",
                           s.abnormalNonceDetection);

        layout.addCheckbox("Webchat detection highlights. ",
                           s.normalNonceDetection, false,
                           "Highlights messages sent from webchat in orange or "
                           "the specified color below.");

        SettingWidget::colorButton("Webchat detected color",
                                   getSettings()->webchatColor)
            ->addTo(layout);
        //layout.addColorButton(",
        //
        //                      getSettings()->webchatColor);
    }
    layout.addStretch();
    // invisible element for width
    auto inv = new BaseWidget(this);
    //    inv->setScaleIndependantWidth(600);
    layout.addWidget(inv);
}

}  // namespace chatterino
