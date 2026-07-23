#pragma once

#include "Cubed/ui/chat_box.hpp"
#include "Cubed/ui/image.hpp"
#include "Cubed/ui/item_slot.hpp"
#include "Cubed/ui/row_layout.hpp"
#include "Cubed/ui/ui_manager.hpp"
#include "Cubed/ui/widget.hpp"

namespace Cubed {
class WorldScene;
class WorldUIManager : public UIManager {
public:
    WorldUIManager(const WorldUIManager&) = delete;
    WorldUIManager(WorldUIManager&&) = delete;
    WorldUIManager& operator=(const WorldUIManager&) = delete;
    WorldUIManager& operator=(WorldUIManager&&) = delete;

    WorldUIManager(WorldScene& scene);
    ~WorldUIManager();

    void init() override;
    void render(Renderer& renderer) override;
    void update(float dt) override;
    void set_chatting(bool chantting, bool sned);

    void add_chat_message(ChatMessage& message);

private:
    bool handle_key_event(const KeyEvent& e) override;
    void update_hotbar();
    WorldScene& m_scene;

    ChatBox* m_chat_box = nullptr;
    Image* m_mircophone = nullptr;
    Label* m_disbable_voice = nullptr;
    RowLayout* m_hotbar = nullptr;

    std::vector<ItemSlot*> m_hotbar_slot;
};
} // namespace Cubed