/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

/*
    Todo: timers, verify spell targets (in the execevent switch), horde coordinates

*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "BattlegroundAV.h"
#include "Player.h" 
#include "ScriptedGossip.h"

// Alliance stuff
Position ally = {-200, -345, 6.75, 0}; 
uint8 NPC_RENFERAL = 13442; // she's at 729.20 -78.81 51.63

// Horde stuff
Position summon_object_horde = { -359, -134.24, 26.46, 0 };
uint8 NPC_renferal = 13442; // she's at -1319.56 -342.67 60.34

enum Quests {
    // Horde
    QUEST_A_GALLON_OF_BLOOD         = 7385, // 5x blood
    QUEST_LOKHOLAR_THE_ICE_LORD     = 6801, // 1x blood

    // Alliance
    QUEST_CRYSTAL_CLUSTER           = 7386, // 5x crystals
    QUEST_IVUS_THE_FOREST_LORD      = 6881, // 1x crystal
};

enum Yells {

    // HORDE
    YELL_LOKHOLAR_SUMMON             = 0, // WHO DARES SUMMON LOKHOLAR ? The blood of a thousand Stormpike soldiers shall I spill...none may stand against the Ice Lord!
    YELL_LOKHOLAR_ON_KILL            = 1, // I drink in your suffering, mortal. Let your essence congeal with Lokholar!
    YELL_LOKHOLAR_ON_LAST_WP         = 2, // Your base is forfeit, puny mortals!

    // ALLIANCE
    YELL_IVUS_SUMMON                 = 0, // Wicked, wicked, mortals!The forest weeps. The elements recoil at the destruction.Ivus must purge you from this world!
    YELL_IVUS_ON_KILL                = 1, // The forest weeps.The elements recoil at the destruction. Ivus must purge you from this world!
    YELL_IVUS_ON_LAST_WP             = 2, // I come to raze your bases, Frostwolf Clan. Ivus punishes you for your treachery![1]
};

enum Spells {

    // HORDE
    SPELL_TICL_BLIZZARD             = 21367, // Blizzard - Calls down a blizzard that lasts 10 sec., inflicting 2899 Frost damage every 2 sec. to all enemies in a selected area.
    SPELL_TICL_FROSTBOLT            = 21369, // Frost Bolt — Deals 23851-37544 Frost Damage and reduces movement speed by 50% for 4 seconds.
    SPELL_TICL_FROSTNOVA            = 14907, // Frost Nova — Inflicts 3911-4859 Frost damage to nearby enemies, immobilizing them for up to 8 sec.
    SPELL_TICL_FROSTSHOCK           = 19133, // Frost Shock - Inflicts 28914 Frost damage to an enemy and reduces its movement speed for 8 sec.
    SPELL_TICL_ICEBLAST             = 15878, // not confirmed
    SPELL_TICL_ICETOMB              = 16869, // Ice Tomb - Stuns an enemy, rendering it unable to move or attack for 6 sec.
    SPELL_TICL_SWELLOFSOULS         = 21307, // Swell of Souls - Increases melee attack power by 750, size by 20% and speed by 10%. (Stacks up to 10 times, lasts 2 hours)

    // ALLIANCE
    SPELL_IVUS_ENTANGLING_ROOTS     = 20654, // Entangling Roots - Entangles nearby enemies in roots, inflicting 14933 Nature damage every 3 sec. and immobilizing them for up to 15 sec.
    SPELL_IVUS_FARIEFIRE            = 21670, // Faerie Fire - Reduces an enemy's armor by 2000 for 1 min. While affected, the target cannot use stealth or invisibility.
    SPELL_IVUS_MOONFIRE             = 21669, // Moonfire - Inflicts 34762 Arcane damage to an enemy, then additional 9776 damage every 3 sec. for 12 sec.
    SPELL_IVUS_STARFIRE             = 21668, // Starfire - Causes 56722 Arcane damage to the target.
    SPELL_IVUS_WRATH                = 21667, // Wrath - Hurls a bolt of lightning at an enemy, inflicting 37941 to 42846 Nature damage.

    SPELL_RENFERAL_ENTANGLING_ROOTS = 22127,
    SPELL_RENFERAL_MOONFIRE         = 22206,
    SPELL_RENFERAL_REJUVENATION     = 15981,
};

enum Events {

    // HORDE
    EVENT_LOKHOLAR_BLIZZARD,
    EVENT_LOKHOLAR_FROSTBOLT, 
    EVENT_LOKHOLAR_FROSTNOVA, 
    EVENT_LOKHOLAR_FROSTSHOCK, 
    EVENT_LOKHOLAR_ICEBLAST, 
    EVENT_LOKHOLAR_ICETOMB,  

    // ALLIANCE
    EVENT_IVUS_ENTANGLING_ROOTS,
    EVENT_IVUS_FARIEFIRE,
    EVENT_IVUS_MOONFIRE,
    EVENT_IVUS_STARFIRE,
    EVENT_IVUS_WRATH,

    EVENT_RENFERAL_HEALTHCHECK,
    EVENT_RENFERAL_MOONFIRE,
    EVENT_RENFERAL_ENTANGLING_ROOTS,
};

class boss_lokholar_the_ice_lord : public CreatureScript
{
public:
    boss_lokholar_the_ice_lord() : CreatureScript("boss_lokholar_the_ice_lord") { }

    struct boss_lokholar_the_ice_lordAI : public npc_escortAI
    {
        boss_lokholar_the_ice_lordAI(Creature* creature) : npc_escortAI(creature) {
            me->MonsterYell(YELL_LOKHOLAR_SUMMON, 0, 0);
            // @todo: coords here
        }

        void EnterCombat(Unit* /*who*/) {
            events.RescheduleEvent(EVENT_LOKHOLAR_BLIZZARD, 10000);
            events.RescheduleEvent(EVENT_LOKHOLAR_FROSTBOLT, 10000);
            events.RescheduleEvent(EVENT_LOKHOLAR_FROSTNOVA, 10000);
            events.RescheduleEvent(EVENT_LOKHOLAR_FROSTSHOCK, 10000);
            events.RescheduleEvent(EVENT_LOKHOLAR_ICEBLAST, 10000);
            events.RescheduleEvent(EVENT_LOKHOLAR_ICETOMB, 10000);
        }

        void Reset() {
            events.Reset();
        }

        void EnterEvadeMode() { /* do we need it at all? */ }

        void WaypointReached(uint32 wp)
        {
            if(wp == 31) {
                me->MonsterYell(YELL_LOKHOLAR_ON_LAST_WP, 0, 0);
                defendPosition();
            }
        }

        void defendPosition()
        {
            SetEscortPaused(true);
            Position current_pos = me->GetPosition();
            me->SetHomePosition(current_pos); // Summon pos?
            me->GetMotionMaster()->MoveRandom();
        }

        void JustDied(Unit* /*killer*/) { /* do we need it at all? */ }

        void KilledUnit(Unit* victim) {
            if (victim->GetTypeId() == TYPEID_PLAYER)
            {
                /** Each time a player is killed, cast Swell of Souls spell. Maximum of 10 stacks allowed. */
                me->MonsterYell(YELL_LOKHOLAR_ON_KILL, 0, 0);
                DoCast(me, SPELL_TICL_SWELLOFSOULS, true);
            }
        }

        void UpdateEscortAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_LOKHOLAR_BLIZZARD: 
                    me->CastSpell(me->GetVictim(), SPELL_TICL_BLIZZARD, false);
                    events.RepeatEvent(15000);
                    break;

                case EVENT_LOKHOLAR_FROSTBOLT:
                    me->CastSpell(me->GetVictim(), SPELL_TICL_FROSTBOLT, false);
                    events.RepeatEvent(15000);
                    break;

                case EVENT_LOKHOLAR_FROSTNOVA:
                    me->CastSpell(me, SPELL_TICL_FROSTNOVA, false);
                    events.RepeatEvent(15000);
                    break;

                case EVENT_LOKHOLAR_FROSTSHOCK:
                    me->CastSpell(me->GetVictim(), SPELL_TICL_FROSTSHOCK, false);
                    events.RepeatEvent(15000);
                    break;

                case EVENT_LOKHOLAR_ICEBLAST: 
                    me->CastSpell(me->GetVictim(), SPELL_TICL_ICEBLAST, false);
                    events.RepeatEvent(15000);
                    break;
                case EVENT_LOKHOLAR_ICETOMB:
                    me->CastSpell(me->GetVictim(), SPELL_TICL_ICETOMB, false);
                    events.RepeatEvent(15000);
                    break;
            }

            DoMeleeAttackIfReady();
        }

        EventMap events; 
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_lokholar_the_ice_lordAI(creature);
    }
};

class boss_ivus_the_forest_lord : public CreatureScript
{
public:
    boss_ivus_the_forest_lord() : CreatureScript("boss_ivus_the_forest_lord") { }

    struct boss_ivus_the_forest_lordAI : public npc_escortAI
    {
        boss_ivus_the_forest_lordAI(Creature* creature) : npc_escortAI(creature) {
            me->MonsterYell(YELL_IVUS_SUMMON, 0, 0);

            AddWaypoint(0, -273, -294, 6, 600 * IN_MILLISECONDS);
            AddWaypoint(1, -402, -283, 13, 0);
            AddWaypoint(2, -441, -277, 20, 0);
            AddWaypoint(3, -487, -284, 28, 0);
            AddWaypoint(4, -523, -341, 34, 0);
            AddWaypoint(5, -544, -339, 37, 0);
            AddWaypoint(6, -578, -315, 45, 0);
            AddWaypoint(7, -620, -352, 55, 0);
            AddWaypoint(8, -624, -395, 58, 0);
            AddWaypoint(9, -671, -379, 65, 0);
            AddWaypoint(10, -712, -364, 66, 0);
            AddWaypoint(11, -718, -408, 67, 0);
            AddWaypoint(12, -760, -429, 64, 0);
            AddWaypoint(13, -812, -447, 54, 0);
            AddWaypoint(14, -844, -393, 50, 0);
            AddWaypoint(15, -898, -382, 48, 0);
            AddWaypoint(16, -1003, -399, 50, 0);
            AddWaypoint(17, -1043, -385, 50, 0);
            AddWaypoint(18, -1062, -363, 51, 0);
            AddWaypoint(19, -1092, -368, 51, 0);
            AddWaypoint(20, -1139, -349, 51, 0);
            AddWaypoint(21, -1198, -366, 53, 0);
            AddWaypoint(22, -1240, -368, 59, 0);
            AddWaypoint(23, -1248, -342, 59, 0);
            AddWaypoint(24, -1231, -316, 61, 0);
            AddWaypoint(25, -1212, -294, 70, 0);
            AddWaypoint(26, -1194, -273, 72, 0);
            AddWaypoint(27, -1206, -253, 72, 0);
            AddWaypoint(28, -1241, -250, 73, 0);
            AddWaypoint(29, -1262, -279, 74, 0);
            AddWaypoint(30, -1279, -289, 87, 0);
            AddWaypoint(31, -1293, -289, 90, 0);
            AddWaypoint(32, -1343, -294, 91, 0);

            Start(true, false, 0, NULL, false, false, false);
        }

        void EnterCombat(Unit* /*who*/) {
            events.RescheduleEvent(EVENT_IVUS_ENTANGLING_ROOTS, 10000);
            events.RescheduleEvent(EVENT_IVUS_FARIEFIRE, 10000);
            events.RescheduleEvent(EVENT_IVUS_MOONFIRE, 10000);
            events.RescheduleEvent(EVENT_IVUS_STARFIRE, 10000);
            events.RescheduleEvent(EVENT_IVUS_WRATH, 10000);
        }

        void Reset() {
            events.Reset();
        }

        void EnterEvadeMode() { /* do we need it at all? */ }

        void WaypointReached(uint32 wp)
        {
            if (wp == 33) {
                me->MonsterYell(YELL_IVUS_ON_LAST_WP, 0, 0);
                defendPosition();
            }
        }

        void defendPosition()
        {
            SetEscortPaused(true);
            Position current_pos = me->GetPosition();
            me->SetHomePosition(current_pos); // Summon pos?
            me->GetMotionMaster()->MoveRandom();
        }

        void JustDied(Unit* /*killer*/) { /* do we need it at all? */ }

        void KilledUnit(Unit* victim) {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                me->MonsterYell(YELL_IVUS_ON_KILL, 0, 0);
        }

        void UpdateEscortAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_IVUS_ENTANGLING_ROOTS:
                    me->CastSpell(me->GetVictim(), SPELL_IVUS_ENTANGLING_ROOTS, false);
                    events.RepeatEvent(15000);
                    break;
                case EVENT_IVUS_FARIEFIRE:
                    me->CastSpell(me->GetVictim(), SPELL_IVUS_FARIEFIRE, false);
                    events.RepeatEvent(15000);
                    break;
                case EVENT_IVUS_MOONFIRE:
                    me->CastSpell(me->GetVictim(), SPELL_IVUS_MOONFIRE, false);
                    events.RepeatEvent(15000);
                    break;
                case EVENT_IVUS_STARFIRE:
                    me->CastSpell(me->GetVictim(), SPELL_IVUS_STARFIRE, false);
                    events.RepeatEvent(15000);
                    break;
                case EVENT_IVUS_WRATH:
                    me->CastSpell(me->GetVictim(), SPELL_IVUS_WRATH, false);
                    events.RepeatEvent(15000);
                    break;
            }

            DoMeleeAttackIfReady();
        }

        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ivus_the_forest_lordAI(creature);
    }
};

// update creature_template set ScriptName='npc_av_renferal' where entry =13442;
class npc_av_renferal : public CreatureScript
{
public:
    npc_av_renferal() : CreatureScript("npc_av_renferal") { }

    struct npc_av_renferalAI : public npc_escortAI
    {
        npc_av_renferalAI(Creature* creature) : npc_escortAI(creature) { 
            AddWaypoint(1, -229, -238, 60, 0);
            AddWaypoint(2, -174, -233, 100, 0);
            AddWaypoint(3, -110, -262, 60, 0);
            AddWaypoint(4, -53, -235, 100, 0);
            AddWaypoint(5, -28, -231, 90, 0);
            AddWaypoint(6, 4, -243, 110, 0);
            AddWaypoint(7, 72, -244, 160, 0);
            AddWaypoint(8, 111, -339, 400, 0);
            AddWaypoint(9, 125, -371, 420, 0);
            AddWaypoint(10, 143, -392, 420, 0);
            AddWaypoint(11, 200, -410, 420, 0);
            AddWaypoint(12, 230, -419, 380, 0);
            AddWaypoint(13, 251, -412, 320, 0);
            AddWaypoint(14, 274, -392, 110, 0);
            AddWaypoint(15, 297, -382, 20, 0);
            AddWaypoint(16, 383, -391, -10, 0);
            AddWaypoint(17, 462, -369, -10, 0);
            AddWaypoint(18, 498, -336, -10, 0);
            AddWaypoint(19, 520, -324, 00, 0);
            AddWaypoint(20, 546, -322, 80, 0);
            AddWaypoint(21, 581, -332, 290, 0);
            AddWaypoint(22, 601, -338, 300, 0);
            AddWaypoint(23, 623, -323, 300, 0);
            AddWaypoint(24, 635, -299, 300, 0);
            AddWaypoint(25, 635, -267, 300, 0);
            AddWaypoint(26, 630, -231, 370, 0);
            AddWaypoint(27, 624, -189, 380, 0);
            AddWaypoint(28, 620, -154, 330, 0);
            AddWaypoint(29, 618, -131, 330, 0);
            AddWaypoint(30, 629, -98, 400, 0);
            AddWaypoint(31, 634, -48, 420, 0);
        }

        void EnterCombat(Unit* /*who*/) {
            events.RescheduleEvent(EVENT_RENFERAL_HEALTHCHECK, 5000);
            events.RescheduleEvent(EVENT_RENFERAL_MOONFIRE, 10000);
            events.RescheduleEvent(EVENT_RENFERAL_ENTANGLING_ROOTS, 10000); 
        }

        void Reset() {
            events.Reset();
        }

        void EnterEvadeMode() { /* do we need it at all? */ }

        void WaypointReached(uint32 wp)
        {
            if (wp == 123456) {
                // talk and spawn the summoning object
            }
        }

        void JustDied(Unit* /*killer*/) { /* do we need it at all? */ }

        void UpdateEscortAI(uint32 diff)
        {
            // temp shit, should be handled in OnQuestReward
            if (!eventStarted)
            {
                Map* map = me->GetMap();
                if (!map || !map->IsBattleground()) return;
                BattlegroundAV* av = (BattlegroundAV*)sBattlegroundMgr->GetBattleground(map->GetInstanceId());
                if (!av) return;

                if (av->GetAllianceCrystals() >= 50) { // @todo: 500, 50 is for debugging
                    Start(true, false, 0, NULL, false, false, false);
                    eventStarted = true;
                }
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_RENFERAL_HEALTHCHECK:
                    if(!me->HasAura(SPELL_RENFERAL_REJUVENATION) && me->GetHealthPct() <= 50) // @todo guess pct
                    me->CastSpell(me, SPELL_RENFERAL_REJUVENATION, false);
                    events.RepeatEvent(3000);
                    break;
                case EVENT_RENFERAL_MOONFIRE:
                    me->CastSpell(me->GetVictim(), SPELL_RENFERAL_MOONFIRE, false);
                    events.RepeatEvent(15000);
                    break;
                case EVENT_RENFERAL_ENTANGLING_ROOTS:
                    me->CastSpell(me->GetVictim(), SPELL_RENFERAL_ENTANGLING_ROOTS, false);
                    events.RepeatEvent(15000);
                    break;
            }

            DoMeleeAttackIfReady();
        }

        EventMap events;
        bool eventStarted = false;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_av_renferalAI(creature);
    }


    bool OnQuestReward(Player* /*player*/, Creature* creature, const Quest* quest, uint32 /*slot*/)
    {
        sLog->outString("Renferal::OnQuestComplete, Q:%u", quest->GetQuestId());
        Map* map = creature->GetMap();
        if (!map || !map->IsBattleground()) { sLog->outString("NO MAP!");  return false; }

        BattlegroundAV* av = (BattlegroundAV*)sBattlegroundMgr->GetBattleground(map->GetInstanceId());
        if (!av) return false;

        if (quest->GetQuestId() == QUEST_CRYSTAL_CLUSTER) {
            av->AddOrRemoveAllianceCrystals(5);
            sLog->outString("Deposited %u crystals, left: %u", 5, av->GetAllianceCrystals());
        } 

        if (quest->GetQuestId() == QUEST_IVUS_THE_FOREST_LORD) {
            av->AddOrRemoveAllianceCrystals(1);
            sLog->outString("Deposited %u crystals, left: %u", 1, av->GetAllianceCrystals());
        }

        return true;
    }

};


void AddSC_av_summonable_bosses()
{
    // Bosses
    new boss_ivus_the_forest_lord();
    new boss_lokholar_the_ice_lord();

    // Summoners 
    new npc_av_renferal;
    //new npc_thurloga;
}
