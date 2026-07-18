#pragma once

#include "Cubed/ui/chat_box.hpp"
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

    void set_chatting(bool chantting, bool sned);

    void add_chat_message(ChatMessage& message);

private:
    bool handle_key_event(const KeyEvent& e) override;
    WorldScene& m_scene;
    ChatBox* m_chat_box;
};
} // namespace Cubed