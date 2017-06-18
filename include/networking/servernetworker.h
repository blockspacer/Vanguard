#pragma once

#include "networking/networker.h"
#include "networking/uuid.h"
#include <vector>


class ServerNetworker : public Networker
{
    public:
        ServerNetworker(WriteBuffer &sendbuffer_);
        virtual ~ServerNetworker() override;
        void receive(Gamestate &state) override;
        void sendeventdata(Gamestate &state) override;
        void sendframedata(Gamestate &state);
        void registerlobby(Gamestate &state);
    protected:
    private:
        Timer lobbyreminder;
        xg::Guid serverid;
        ENetPeer *lobby;
};

