#include "HotkeyController.hpp"
#include "common/QLogging.hpp"
#include "controllers/hotkeys/HotkeyModel.hpp"
#include "singletons/Settings.hpp"

#include <QShortcut>

namespace chatterino {

boost::optional<HotkeyScope> HotkeyController::hotkeyScopeFromName(
    QString scopeName)
{
    for (auto ref : this->hotkeyScopeNames)
    {
        if (ref.second == scopeName)
        {
            return ref.first;
        }
    }
    qCDebug(chatterinoHotkeys) << "Unknown scope: " << scopeName;
    return {};
}

QString HotkeyController::hotkeyScopeToName(HotkeyScope scope)
{
    unsigned long scopeId = (unsigned long)(scope);
    return 0 <= scopeId && scopeId <= this->hotkeyScopeNames.size()
               ? this->hotkeyScopeNames.find(scope)->second
               : "";
}
bool hotkeySortCompare_(const std::shared_ptr<Hotkey> &a,
                        const std::shared_ptr<Hotkey> &b)
{
    if (a->scope() == b->scope())
    {
        return a->name() < b->name();
    }

    return a->scope() < b->scope();
}

HotkeyController::HotkeyController()
    : hotkeys_(hotkeySortCompare_)
{
    this->loadHotkeys();
    this->signalHolder_.managedConnect(
        this->hotkeys_.delayedItemsChanged, [this]() {
            qCDebug(chatterinoHotkeys) << "Reloading hotkeys!";
            this->onItemsUpdated.invoke();
        });
}

void HotkeyController::initialize(Settings &settings, Paths &paths)
{
}

void HotkeyController::loadHotkeys()
{
    auto defaultHotkeysAdded =
        pajlada::Settings::Setting<std::vector<QString>>::get(
            "/hotkeys/addedDefaults");
    auto set = std::set<QString>(defaultHotkeysAdded.begin(),
                                 defaultHotkeysAdded.end());

    auto keys = pajlada::Settings::SettingManager::getObjectKeys("/hotkeys");
    this->addDefaults(set);
    pajlada::Settings::Setting<std::vector<QString>>::set(
        "/hotkeys/addedDefaults", std::vector<QString>(set.begin(), set.end()));

    qCDebug(chatterinoHotkeys) << "Loading hotkeys...";
    for (const auto &key : keys)
    {
        if (key == "addedDefaults")
        {
            continue;
        }

        auto section = "/hotkeys/" + key;
        auto scopeName =
            pajlada::Settings::Setting<QString>::get(section + "/scope");
        auto keySequence =
            pajlada::Settings::Setting<QString>::get(section + "/keySequence");
        auto action =
            pajlada::Settings::Setting<QString>::get(section + "/action");
        auto arguments = pajlada::Settings::Setting<std::vector<QString>>::get(
            section + "/arguments");
        qCDebug(chatterinoHotkeys)
            << "Hotkey " << scopeName << keySequence << action << arguments;

        if (scopeName.isEmpty() || keySequence.isEmpty() || action.isEmpty())
        {
            continue;
        }
        auto scope = HotkeyController::hotkeyScopeFromName(scopeName);
        if (!scope)
        {
            continue;
        }
        this->hotkeys_.append(
            std::make_shared<Hotkey>(*scope, QKeySequence(keySequence), action,
                                     arguments, QString::fromStdString(key)));
    }
}

void HotkeyController::resetToDefaults()
{
    std::set<QString> addedSet;
    pajlada::Settings::Setting<std::vector<QString>>::set(
        "/hotkeys/addedDefaults",
        std::vector<QString>(addedSet.begin(), addedSet.end()));
    auto size = this->hotkeys_.raw().size();
    for (unsigned long i = 0; i < size; i++)
    {
        this->hotkeys_.removeAt(0);
    }

    // add defaults back
    this->saveHotkeys();
    this->loadHotkeys();
}
void HotkeyController::tryAddDefault(std::set<QString> &addedHotkeys,
                                     HotkeyScope scope,
                                     QKeySequence keySequence, QString action,
                                     std::vector<QString> args, QString name)
{
    qCDebug(chatterinoHotkeys) << "Try add default" << name;
    if (addedHotkeys.count(name) != 0)
    {
        qCDebug(chatterinoHotkeys) << "Already exists";
        return;  // hotkey was added before
    }
    qCDebug(chatterinoHotkeys) << "Inserted";
    this->hotkeys_.append(
        std::make_shared<Hotkey>(scope, keySequence, action, args, name));
    addedHotkeys.insert(name);
}

void HotkeyController::addDefaults(std::set<QString> &addedHotkeys)
{
    // split
    {
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Ctrl+W"), "delete",
                            std::vector<QString>(), "default delete");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Ctrl+R"), "changeChannel",
                            std::vector<QString>(), "default change channel");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Ctrl+F"), "showSearch",
                            std::vector<QString>(), "default show search");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Ctrl+F5"), "reconnect",
                            std::vector<QString>(), "default reconnect");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("F10"), "debug",
                            std::vector<QString>(), "default debug");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Alt+x"), "createClip",
                            std::vector<QString>(), "default create clip");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Alt+left"), "focus", {"left"},
                            "default focus left");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Alt+down"), "focus", {"down"},
                            "default focus down");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Alt+up"), "focus", {"up"},
                            "default focus up");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Alt+right"), "focus", {"right"},
                            "default focus right");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("PgUp"), "scrollPage", {"up"},
                            "default scroll page up");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("PgDown"), "scrollPage", {"down"},
                            "default scroll page down");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Split,
                            QKeySequence("Ctrl+End"), "scrollToBottom",
                            std::vector<QString>(), "default scroll to bottom");
    }

    // split input
    {
        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("Ctrl+E"), "openEmotesPopup",
                            std::vector<QString>(), "default emote picker");

        // all variations of send message :)
        {
            this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                                QKeySequence("Return"), "sendMessage",
                                std::vector<QString>(), "default send message");
            this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                                QKeySequence("Ctrl+Return"), "sendMessage",
                                {"keepInput"},
                                "default send message and keep text");

            this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                                QKeySequence("Shift+Return"), "sendMessage",
                                std::vector<QString>(), "default send message");
            this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                                QKeySequence("Ctrl+Shift+Return"),
                                "sendMessage", {"keepInput"},
                                "default send message and keep text");
        }

        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("Home"), "jumpCursor",
                            {"start", "withoutSelection"},
                            "default go to start of input");
        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("End"), "jumpCursor",
                            {"end", "withoutSelection"},
                            "default go to end of input");

        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("Shift+Home"), "jumpCursor",
                            {"start", "withSelection"},
                            "default go to start of input with selection");
        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("Shift+End"), "jumpCursor",
                            {"end", "withSelection"},
                            "default go to end of input with selection");

        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("Up"), "previousMessage",
                            std::vector<QString>(), "default previous message");
        this->tryAddDefault(addedHotkeys, HotkeyScope::SplitInput,
                            QKeySequence("Down"), "nextMessage",
                            std::vector<QString>(), "default next message");
    }

    // popup window
    {
        this->tryAddDefault(
            addedHotkeys, HotkeyScope::PopupWindow, QKeySequence("Escape"),
            "delete", std::vector<QString>(), "default close popup window");
        for (int i = 0; i < 8; i++)
        {
            this->tryAddDefault(
                addedHotkeys, HotkeyScope::PopupWindow,
                QKeySequence(QString("Ctrl+%1").arg(i + 1)), "openTab",
                {QString::number(i)},
                QString("default popup select tab #%1").arg(i + 1));
        }
        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("Ctrl+9"), "openTab", {"last"},
                            "default popup select last tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("Ctrl+Tab"), "openTab", {"next"},
                            "default popup select next tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("Ctrl+Shift+Tab"), "openTab",
                            {"previous"}, "default popup select previous tab");
        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("PgUp"), "scrollPage", {"up"},
                            "default popup scroll up");
        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("PgDown"), "scrollPage", {"down"},
                            "default popup scroll down");
        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("Return"), "accept",
                            std::vector<QString>(), "default popup accept");
        this->tryAddDefault(addedHotkeys, HotkeyScope::PopupWindow,
                            QKeySequence("Escape"), "reject",
                            std::vector<QString>(), "default popup reject");
        this->tryAddDefault(
            addedHotkeys, HotkeyScope::PopupWindow, QKeySequence("Ctrl+F"),
            "search", std::vector<QString>(), "default popup focus search box");
    }

    // window
    {
        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+P"), "openSettings",
                            std::vector<QString>(), "default open settings");
        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+T"), "newSplit",
                            std::vector<QString>(), "default new split");
        for (int i = 0; i < 8; i++)
        {
            this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                                QKeySequence(QString("Ctrl+%1").arg(i + 1)),
                                "openTab", {QString::number(i)},
                                QString("default select tab #%1").arg(i + 1));
        }
        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+9"), "openTab", {"last"},
                            "default select last tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+Tab"), "openTab", {"next"},
                            "default select next tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+Shift+Tab"), "openTab",
                            {"previous"}, "default select previous tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+N"), "popup",
                            std::vector<QString>(), "default new popup window");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence::ZoomIn, "zoom", {"in"},
                            "default zoom in");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence::ZoomOut, "zoom", {"out"},
                            "default zoom out");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+Shift+T"), "newTab",
                            std::vector<QString>(), "default new tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+Shift+W"), "removeTab",
                            std::vector<QString>(), "default remove tab");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+G"), "reopenSplit",
                            std::vector<QString>(), "default reopen split");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+H"), "toggleLocalR9K",
                            std::vector<QString>(), "default toggle local r9k");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+K"), "openQuickSwitcher",
                            std::vector<QString>(),
                            "default open quick switcher");

        this->tryAddDefault(addedHotkeys, HotkeyScope::Window,
                            QKeySequence("Ctrl+U"), "setTabVisibility",
                            {"toggle"}, "default toggle tab visibility");
    }
}

void HotkeyController::save()
{
    this->saveHotkeys();
}

void HotkeyController::saveHotkeys()
{
    auto defaultHotkeysAdded =
        pajlada::Settings::Setting<std::vector<QString>>::get(
            "/hotkeys/addedDefaults");

    // make sure that hotkeys are deleted
    pajlada::Settings::SettingManager::getInstance()->set(
        "/hotkeys", rapidjson::Value(rapidjson::kObjectType));

    // re-add /hotkeys/addedDefaults as previous set call deleted that key
    pajlada::Settings::Setting<std::vector<QString>>::set(
        "/hotkeys/addedDefaults",
        std::vector<QString>(defaultHotkeysAdded.begin(),
                             defaultHotkeysAdded.end()));

    for (auto &hotkey : this->hotkeys_)
    {
        auto section = "/hotkeys/" + hotkey->name().toStdString();
        pajlada::Settings::Setting<QString>::set(section + "/action",
                                                 hotkey->action());
        pajlada::Settings::Setting<QString>::set(
            section + "/keySequence", hotkey->keySequence().toString());

        auto scopeName = HotkeyController::hotkeyScopeToName(hotkey->scope());
        pajlada::Settings::Setting<QString>::set(section + "/scope", scopeName);
        pajlada::Settings::Setting<std::vector<QString>>::set(
            section + "/arguments", hotkey->arguments());
    }
}

HotkeyModel *HotkeyController::createModel(QObject *parent)
{
    HotkeyModel *model = new HotkeyModel(parent);
    model->initialize(&this->hotkeys_);
    return model;
}

std::vector<QShortcut *> HotkeyController::shortcutsForScope(
    HotkeyScope scope,
    std::map<QString, std::function<QString(std::vector<QString>)>> actionMap,
    QWidget *parent)
{
    std::vector<QShortcut *> output;
    for (const auto &hotkey : this->hotkeys_)
    {
        if (hotkey->scope() != scope)
        {
            continue;
        }
        auto target = actionMap.find(hotkey->action());
        if (target == actionMap.end())
        {
            qCDebug(chatterinoHotkeys)
                << "Unimplemeneted hotkey action:" << hotkey->action() << "in "
                << hotkey->getCategory();
            continue;
        }
        auto createShortcutFromKeySeq = [&](QKeySequence qs) {
            auto s = new QShortcut(qs, parent);
            s->setContext(hotkey->getContext());
            auto functionPointer = target->second;
            QObject::connect(
                s, &QShortcut::activated, parent,
                [functionPointer, hotkey, this]() {
                    qCDebug(chatterinoHotkeys)
                        << "Shortcut pressed: " << hotkey->action();
                    QString output = functionPointer(hotkey->arguments());
                    if (!output.isEmpty())
                    {
                        this->showHotkeyError(hotkey, output);
                    }
                });
            output.push_back(s);
        };
        auto qs = QKeySequence(hotkey->keySequence());

        if ((qs[0] & Qt::Key_Return) || (qs[1] & Qt::Key_Return) ||
            (qs[2] & Qt::Key_Return) || (qs[3] & Qt::Key_Return))
        {
            std::array<int, 4> keys{qs[0], qs[1], qs[2], qs[3]};
            for (auto &key : keys)
            {
                auto newKey = (key & ~Qt::Key_Return);
                if (newKey != key)
                {
                    newKey |= Qt::Key_Enter;
                    key = newKey;
                }
            }
            auto copy = QKeySequence(keys[0], keys[1], keys[2], keys[3]);
            createShortcutFromKeySeq(copy);
        }
        createShortcutFromKeySeq(qs);
    }
    return output;
}
void HotkeyController::showHotkeyError(const std::shared_ptr<Hotkey> hotkey,
                                       QString warning)
{
    auto msgBox = new QMessageBox(
        QMessageBox::Icon::Warning, "Hotkey error",
        QString(
            "There was an error while executing your hotkey named \"%1\": \n%2")
            .arg(hotkey->name(), warning),
        QMessageBox::Ok);
    msgBox->exec();
}

std::shared_ptr<Hotkey> HotkeyController::getHotkeyByName(QString name)
{
    for (auto &hotkey : this->hotkeys_)
    {
        if (hotkey->name() == name)
        {
            return hotkey;
        }
    }
    return nullptr;
}

void HotkeyController::replaceHotkey(QString oldName,
                                     std::shared_ptr<Hotkey> newHotkey)
{
    int i = 0;
    for (auto &hotkey : this->hotkeys_)
    {
        if (hotkey->name() == oldName)
        {
            this->hotkeys_.removeAt(i);
            break;
        }
        i++;
    }
    this->hotkeys_.append(newHotkey);
}

bool HotkeyController::isDuplicate(std::shared_ptr<Hotkey> hotkey,
                                   QString ignoreNamed)
{
    for (const auto shared : this->hotkeys_)
    {
        if (shared->scope() == hotkey->scope() &&
            shared->keySequence() == hotkey->keySequence() &&
            shared->name() != hotkey->name() && shared->name() != ignoreNamed)
        {
            return true;
        }
    }
    return false;
}
}  // namespace chatterino