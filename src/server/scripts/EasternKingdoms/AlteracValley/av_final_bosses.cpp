/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-GPL2
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Spells
{
    // Horde
    SPELL_WHIRLWIND                               = 15589,
    SPELL_WHIRLWIND2                              = 13736,
    SPELL_KNOCKDOWN                               = 19128,
    SPELL_FRENZY                                  = 8269,
    SPELL_SWEEPING_STRIKES                        = 18765, // not sure
    SPELL_CLEAVE                                  = 20677, // not sure
    SPELL_WINDFURY                                = 35886, // not sure
    SPELL_STORMPIKE                               = 51876,  // not sure

    // Alliance
    SPELL_AVATAR                                  = 19135,
    SPELL_THUNDERCLAP                             = 15588,
    SPELL_STORMBOLT                               = 20685 // not sure
};

enum Yells
{
    // Horde
    H_YELL_AGGRO                                    = 0,
    H_YELL_EVADE                                    = 1,
    H_YELL_RESPAWN                                  = 2,
    H_YELL_RANDOM                                   = 3,

    // Alliance
    A_YELL_AGGRO                                    = 0,
    A_YELL_EVADE                                    = 1, 
    A_YELL_RANDOM                                   = 2,
    A_YELL_SPELL                                    = 3,
};

class boss_drekthar : public CreatureScript
{
public:
    boss_drekthar() : CreatureScript("boss_drekthar") { }

    struct boss_drektharAI : public ScriptedAI
    {
        boss_drektharAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 WhirlwindTimer;
        uint32 Whirlwind2Timer;
        uint32 KnockdownTimer;
        uint32 FrenzyTimer;
        uint32 YellTimer;
        uint32 ResetTimer;

        void Reset()
        {
            WhirlwindTimer    = urand(1 * IN_MILLISECONDS, 20 * IN_MILLISECONDS);
            Whirlwind2Timer   = urand(1 * IN_MILLISECONDS, 20 * IN_MILLISECONDS);
            KnockdownTimer    = 12 * IN_MILLISECONDS;
            FrenzyTimer       = 6 * IN_MILLISECONDS;
            ResetTimer        = 5 * IN_MILLISECONDS;
            YellTimer         = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS); //20 to 30 seconds
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(H_YELL_AGGRO);
        }

        void JustRespawned()
        {
            Reset();
            Talk(H_YELL_RESPAWN);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (WhirlwindTimer <= diff)
            {
                DoCastVictim(SPELL_WHIRLWIND);
                WhirlwindTimer =  urand(8 * IN_MILLISECONDS, 18 * IN_MILLISECONDS);
            }
            else WhirlwindTimer -= diff;

            if (Whirlwind2Timer <= diff)
            {
                DoCastVictim(SPELL_WHIRLWIND2);
                Whirlwind2Timer = urand(7 * IN_MILLISECONDS, 25 * IN_MILLISECONDS);
            }
            else Whirlwind2Timer -= diff;

            if (KnockdownTimer <= diff)
            {
                DoCastVictim(SPELL_KNOCKDOWN);
                KnockdownTimer = urand(10 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
            }
            else KnockdownTimer -= diff;

            if (FrenzyTimer <= diff)
            {
                DoCastVictim(SPELL_FRENZY);
                FrenzyTimer = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS);
            }
            else FrenzyTimer -= diff;

            if (YellTimer <= diff)
            {
                Talk(H_YELL_RANDOM);
                YellTimer = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS); //20 to 30 seconds
            }
            else YellTimer -= diff;

            // check if creature is not outside of building
            if (ResetTimer <= diff)
            {
                if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
                {
                    EnterEvadeMode();
                    Talk(H_YELL_EVADE);
                }
                ResetTimer = 5 * IN_MILLISECONDS;
            }
            else ResetTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_drektharAI(creature);
    }
};

class boss_vanndar : public CreatureScript
{
public:
    boss_vanndar() : CreatureScript("boss_vanndar") { }

    struct boss_vanndarAI : public ScriptedAI
    {
        boss_vanndarAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 AvatarTimer;
        uint32 ThunderclapTimer;
        uint32 StormboltTimer;
        uint32 ResetTimer;
        uint32 YellTimer;

        void Reset()
        {
            AvatarTimer        = 3 * IN_MILLISECONDS;
            ThunderclapTimer   = 4 * IN_MILLISECONDS;
            StormboltTimer     = 6 * IN_MILLISECONDS;
            ResetTimer         = 5 * IN_MILLISECONDS;
            YellTimer = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(A_YELL_AGGRO);
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (AvatarTimer <= diff)
            {
                DoCastVictim(SPELL_AVATAR);
                AvatarTimer =  urand(15 * IN_MILLISECONDS, 20 * IN_MILLISECONDS);
            }
            else AvatarTimer -= diff;

            if (ThunderclapTimer <= diff)
            {
                DoCastVictim(SPELL_THUNDERCLAP);
                ThunderclapTimer = urand(5 * IN_MILLISECONDS, 15 * IN_MILLISECONDS);
            }
            else ThunderclapTimer -= diff;

            if (StormboltTimer <= diff)
            {
                DoCastVictim(SPELL_STORMBOLT);
                StormboltTimer = urand(10 * IN_MILLISECONDS, 25 * IN_MILLISECONDS);
            }
            else StormboltTimer -= diff;

            if (YellTimer <= diff)
            {
                Talk(A_YELL_RANDOM);
                YellTimer = urand(20 * IN_MILLISECONDS, 30 * IN_MILLISECONDS); //20 to 30 seconds
            }
            else YellTimer -= diff;

            // check if creature is not outside of building
            if (ResetTimer <= diff)
            {
                if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 50)
                {
                    EnterEvadeMode();
                    Talk(A_YELL_EVADE);
                }
                ResetTimer = 5 * IN_MILLISECONDS;
            }
            else ResetTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_vanndarAI(creature);
    }
};

void AddSC_av_final_bosses()
{
	new boss_drekthar;
	new boss_vanndar;
}
