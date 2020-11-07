#include "Quest.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "ClientSession.h"
#include "LocalesMgr.h"
#include "Formulas.h"

void Quest::WarmingCache()
{ 
}

QuestTemplate const* Quest::GetQuestTemplate(uint32 entry)
{
    QuestTemplateContainer::const_iterator itr = _questTemplateStore.find(entry);
    if (itr != _questTemplateStore.end())
        return (itr->second);

    return NULL;
}

void Quest::CacheQuestTemplate(QuestTemplate* quest)
{
    //To prevent a crash we should set an Mutex here
    ACORE_WRITE_GUARD(ACE_RW_Thread_Mutex, rwMutex_);

    QuestTemplate* _quest = _questTemplateStore[quest->Id];
    _quest = quest;
    return;
}

void ClientSession::HandleQuestQueryOpcode(WorldPacket & recv_data)
{
    if (!_player)
        return;

    uint32 questId;
    recv_data >> questId;

    QuestTemplate const* quest = sQuest->GetQuestTemplate(questId);
    if (quest)
    {

        std::string questTitle = quest->GetTitle();
        std::string questDetails = quest->GetDetails();
        std::string questObjectives = quest->GetObjectives();
        std::string questEndText = quest->GetEndText();
        std::string questCompletedText = quest->GetCompletedText();

        std::string questObjectiveText[QUEST_OBJECTIVES_COUNT];
        for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            questObjectiveText[i] = quest->ObjectiveText[i];

        int32 locale = GetSessionDbLocaleIndex();
        if (locale >= 0)
        {
            if (QuestLocale const* localeData = sLocalesMgr->GetQuestLocale(quest->GetQuestId()))
            {
                LocalesMgr::GetLocaleString(localeData->Title, locale, questTitle);
                LocalesMgr::GetLocaleString(localeData->Details, locale, questDetails);
                LocalesMgr::GetLocaleString(localeData->Objectives, locale, questObjectives);
                LocalesMgr::GetLocaleString(localeData->EndText, locale, questEndText);
                LocalesMgr::GetLocaleString(localeData->CompletedText, locale, questCompletedText);

                for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                    LocalesMgr::GetLocaleString(localeData->ObjectiveText[i], locale, questObjectiveText[i]);
            }
        }

        WorldPacket data(SMSG_QUEST_QUERY_RESPONSE, 100);       // guess size

        data << uint32(quest->GetQuestId());                    // quest id
        data << uint32(quest->GetQuestMethod());                // Accepted values: 0, 1 or 2. 0 == IsAutoComplete() (skip objectives/details)
        data << uint32(quest->GetQuestLevel());                 // may be -1, static data, in other cases must be used dynamic level: Player::GetQuestLevel (0 is not known, but assuming this is no longer valid for quest intended for client)
        data << uint32(quest->GetMinLevel());                   // min level
        data << uint32(quest->GetZoneOrSort());                 // zone or sort to display in quest log

        data << uint32(quest->GetType());                       // quest type
        data << uint32(quest->GetSuggestedPlayers());           // suggested players count

        data << uint32(quest->GetRepObjectiveFaction());        // shown in quest log as part of quest objective
        data << uint32(quest->GetRepObjectiveValue());          // shown in quest log as part of quest objective

        data << uint32(quest->GetRepObjectiveFaction2());       // shown in quest log as part of quest objective OPPOSITE faction
        data << uint32(quest->GetRepObjectiveValue2());         // shown in quest log as part of quest objective OPPOSITE faction

        data << uint32(quest->GetNextQuestInChain());           // client will request this quest from NPC, if not 0
        data << uint32(quest->GetXPId());                       // used for calculating rewarded experience

        if (quest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
            data << uint32(0);                                  // Hide money rewarded
        else
            data << uint32(quest->GetRewOrReqMoney());          // reward money (below max lvl)

        data << uint32(quest->GetRewMoneyMaxLevel());           // used in XP calculation at client
        data << uint32(quest->GetRewSpell());                   // reward spell, this spell will display (icon) (casted if RewSpellCast == 0)
        data << int32(quest->GetRewSpellCast());                // casted spell

        // rewarded honor points
        data << Trinity::Honor::hk_honor_at_level(GetPlayer()->getLevel(), quest->GetRewHonorMultiplier());
        data << float(0);                                       // new reward honor (multipled by ~62 at client side)
        data << uint32(quest->GetSrcItemId());                  // source item id
        data << uint32(quest->GetFlags() & 0xFFFF);             // quest flags
        data << uint32(quest->GetCharTitleId());                // CharTitleId, new 2.4.0, player gets this title (id from CharTitles)
        data << uint32(quest->GetPlayersSlain());               // players slain
        data << uint32(quest->GetBonusTalents());               // bonus talents
        data << uint32(quest->GetRewArenaPoints());             // bonus arena points
        data << uint32(0);                                      // review rep show mask

        if (quest->HasFlag(QUEST_FLAGS_HIDDEN_REWARDS))
        {
            for (uint32 i = 0; i < QUEST_REWARDS_COUNT; ++i)
                data << uint32(0) << uint32(0);
            for (uint32 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
                data << uint32(0) << uint32(0);
        }
        else
        {
            for (uint32 i = 0; i < QUEST_REWARDS_COUNT; ++i)
            {
                data << uint32(quest->RewardItemId[i]);
                data << uint32(quest->RewardItemIdCount[i]);
            }
            for (uint32 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
            {
                data << uint32(quest->RewardChoiceItemId[i]);
                data << uint32(quest->RewardChoiceItemCount[i]);
            }
        }

        for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)        // reward factions ids
            data << uint32(quest->RewardFactionId[i]);

        for (uint32 i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)        // columnid+1 QuestFactionReward.dbc?
            data << int32(quest->RewardFactionValueId[i]);

        for (int i = 0; i < QUEST_REPUTATIONS_COUNT; ++i)           // unk (0)
            data << int32(quest->RewardFactionValueIdOverride[i]);

        data << quest->GetPointMapId();
        data << quest->GetPointX();
        data << quest->GetPointY();
        data << quest->GetPointOpt();

        data << questTitle;
        data << questObjectives;
        data << questDetails;
        data << questEndText;
        data << questCompletedText;                                  // display in quest objectives window once all objectives are completed

        for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            if (quest->RequiredNpcOrGo[i] < 0)
                data << uint32((quest->RequiredNpcOrGo[i] * (-1)) | 0x80000000);    // client expects gameobject template id in form (id|0x80000000)
            else
                data << uint32(quest->RequiredNpcOrGo[i]);

            data << uint32(quest->RequiredNpcOrGoCount[i]);
            data << uint32(quest->RequiredSourceItemId[i]);
            data << uint32(0);                                  // req source count?
        }

        for (uint32 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
        {
            data << uint32(quest->RequiredItemId[i]);
            data << uint32(quest->RequiredItemCount[i]);
        }

        for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            data << questObjectiveText[i];

        SendPacket(&data);
        sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Sent SMSG_QUEST_QUERY_RESPONSE questid=%u", quest->GetQuestId());
    }
    else
        SendPacketToNode(&recv_data);
}
