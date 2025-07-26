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
    SettingWidget::checkbox("Show placeholder in text input box",
                            s.showTextInputPlaceholder)
        ->addTo(layout);
    layout.addDescription("Gray-out recent messages was upstreamed as \"Reduce "
                          "opacity of message history\"");
    layout.addTitle("Behavior");
    SettingWidget::checkbox("Lowercase tab-completed usernames",
                            s.lowercaseUsernames)
        ->addTo(layout);
    {
        auto *groupLayout = new QFormLayout();
        groupLayout->addRow(
            this->createCheckBox("Allow \"bridge\" users to impersonate others",
                                 s.allowBridgeImpersonation));
        SettingWidget::lineEdit("Bridge user:", s.bridgeUser, "supabridge")
            ->addTo(layout);
        layout.addLayout(groupLayout);
    }
    //layout.addTitle("Emotes");
    //layout.addCheckbox("Enable loading 7TV emotes", s.enableLoadingSevenTV);
    layout.addTitle("Miscellaneous");
    SettingWidget::intInput(
        "High rate limit spam delay in milliseconds (mod/vip)",
        s.twitchHighRateLimitDelay,
        {
            .min = 100,
            .max = 2000,
            .singleStep = 100,
        })
        ->addTo(layout);
    SettingWidget::intInput(
        "Low rate limit spam delay in milliseconds (non mod/vip)",
        s.twitchLowRateLimitDelay,
        {
            .min = 500,
            .max = 3000,
            .singleStep = 1100,
        })
        ->addTo(layout);

    if (s.dankerinoThreeLetterApiEasterEgg)
    {
        SettingWidget::checkbox("Click to disable GraphQL easter egg and "
                                "advanced settings "
                                "(requires restart)",
                                s.dankerinoThreeLetterApiEasterEgg)
            ->addTo(layout);
        layout.addTitle("Random 'hacks'");
        SettingWidget::checkbox("Enable. Required for settings below to work!",
                                s.nonceFuckeryEnabled)
            ->addTo(layout);
        SettingWidget::checkbox("Abnormal nonce detection",
                                s.abnormalNonceDetection)
            ->addTo(layout);

        layout.addTitle("Client detection");
        SettingWidget::checkbox("Client detection highlights. ",
                                s.normalNonceDetection)
            ->setTooltip("Highlights messages sent from specified clients "
                         "using the specified color below.")
            ->addTo(layout);

        SettingWidget::colorButton("Webchat color", getSettings()->webchatColor)
            ->addTo(layout);
        SettingWidget::colorButton("Android color", getSettings()->androidColor)
            ->addTo(layout);
        SettingWidget::colorButton("iOS color", getSettings()->iosColor)
            ->addTo(layout);
    }
    layout.addStretch();
    // invisible element for width
    auto inv = new BaseWidget(this);
    //    inv->setScaleIndependantWidth(600);
    layout.addWidget(inv);
}

}  // namespace chatterino
