#include "Groups.h"
#include "Player.h"
#include "ClientSession.h"
#include "WorldPacket.h"
#include "Opcodes.h"

void ClientSession::SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res, uint32 val /* = 0 */)
{
    WorldPacket data(SMSG_PARTY_COMMAND_RESULT, 4 + member.size() + 1 + 4 + 4);
    data << uint32(operation);
    data << member;
    data << uint32(res);
    data << uint32(val);                                    // LFD cooldown related (used with ERR_PARTY_LFG_BOOT_COOLDOWN_S and ERR_PARTY_LFG_BOOT_NOT_ELIGIBLE_S)

    SendPacket(&data);
}
