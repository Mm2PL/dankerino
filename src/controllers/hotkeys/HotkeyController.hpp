#pragma once
#include "common/SignalVector.hpp"
#include "common/Singleton.hpp"
#include "controllers/hotkeys/HotkeyScope.hpp"

namespace chatterino {
class Hotkey;
class Settings;
class Paths;

class HotkeyModel;

class HotkeyController final : public Singleton
{
public:
    HotkeyController();
    HotkeyModel *createModel(QObject *parent);

    virtual void initialize(Settings &settings, Paths &paths) override;
    std::vector<QShortcut *> shortcutsForScope(
        HotkeyScope scope,
        std::map<QString, std::function<void(std::vector<QString>)>> actionMap,
        QWidget *parent);

    void save() override;

private:
    SignalVector<std::shared_ptr<Hotkey>> hotkeys_;
    void loadHotkeys();
    void saveHotkeys();
    void resetToDefaults();
};

}  // namespace chatterino