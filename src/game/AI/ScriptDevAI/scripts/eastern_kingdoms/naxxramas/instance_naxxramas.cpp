/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Instance_Naxxramas
SD%Complete: 90%
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

static const DialogueEntry aNaxxDialogue[] =
{
    {NPC_KELTHUZAD,         0,                  10000},
    {SAY_SAPP_DIALOG1,      NPC_KELTHUZAD,      5000},
    {SAY_SAPP_DIALOG2_LICH, NPC_THE_LICHKING,   17000},
    {SAY_SAPP_DIALOG3,      NPC_KELTHUZAD,      6000},
    {SAY_SAPP_DIALOG4_LICH, NPC_THE_LICHKING,   8000},
    {SAY_SAPP_DIALOG5,      NPC_KELTHUZAD,      0},
    {NPC_THANE,             0,                  10000},
    {SAY_KORT_TAUNT1,       NPC_THANE,          5000},
    {SAY_ZELI_TAUNT1,       NPC_ZELIEK,         6000},
    {SAY_BLAU_TAUNT1,       NPC_BLAUMEUX,       6000},
    {SAY_MORG_TAUNT1,       NPC_MOGRAINE,       7000},
    {SAY_BLAU_TAUNT2,       NPC_BLAUMEUX,       6000},
    {SAY_ZELI_TAUNT2,       NPC_ZELIEK,         5000},
    {SAY_KORT_TAUNT2,       NPC_THANE,          7000},
    {SAY_MORG_TAUNT2,       NPC_MOGRAINE,       0},
    {SAY_FAERLINA_INTRO,    NPC_FAERLINA,       10000},
    {FOLLOWERS_STAND,       0,                  3000},
    {FOLLOWERS_AURA,        0,                  30000},
    {FOLLOWERS_KNEEL,       0,                  0},
    {0, 0, 0}
};

instance_naxxramas::instance_naxxramas(Map* pMap) : ScriptedInstance(pMap),
    m_fChamberCenterX(0.0f),
    m_fChamberCenterY(0.0f),
    m_fChamberCenterZ(0.0f),
    m_uiSapphSpawnTimer(0),
    m_uiTauntTimer(0),
    m_uiHorseMenKilled(0),
    m_uiLivingPoisonTimer(0),
    m_uiScreamsTimer(2 * MINUTE * IN_MILLISECONDS),
    isFaerlinaIntroDone(false),
    DialogueHelper(aNaxxDialogue)
{
    Initialize();
}

void instance_naxxramas::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    InitializeDialogueHelper(this);
}

void instance_naxxramas::JustDidDialogueStep(int32 entry)
{
    switch (entry)
    {
        case FOLLOWERS_STAND:
        {
            isFaerlinaIntroDone = true;
            for (auto& followerGuid : m_lFaerlinaFollowersList)
            {
                if (Creature* follower = instance->GetCreature(followerGuid))
                {
                    if (follower->isAlive() && !follower->isInCombat())
                        follower->SetStandState(UNIT_STAND_STATE_STAND);
                }
            }
            break;
        }
        case FOLLOWERS_AURA:
        {
            for (auto& followerGuid : m_lFaerlinaFollowersList)
            {
                if (Creature* follower = instance->GetCreature(followerGuid))
                {
                    if (follower->isAlive() && !follower->isInCombat())
                        follower->CastSpell(follower, SPELL_DARK_CHANNELING, TRIGGERED_OLD_TRIGGERED);
                }
            }
            break;
        }
        case FOLLOWERS_KNEEL:
        {
            for (auto& followerGuid : m_lFaerlinaFollowersList)
            {
                if (Creature* follower = instance->GetCreature(followerGuid))
                {
                    if (follower->isAlive() && !follower->isInCombat())
                    {
                        follower->RemoveAurasDueToSpell(SPELL_DARK_CHANNELING);
                        follower->SetStandState(UNIT_STAND_STATE_KNEEL);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void instance_naxxramas::OnPlayerEnter(Player* pPlayer)
{
    // Function only used to summon Sapphiron in case of server reload
    if (GetData(TYPE_SAPPHIRON) != SPECIAL)
        return;

    // Check if already summoned
    if (GetSingleCreatureFromStorage(NPC_SAPPHIRON, true))
        return;

    pPlayer->SummonCreature(NPC_SAPPHIRON, aSapphPositions[0], aSapphPositions[1], aSapphPositions[2], aSapphPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);
}

void instance_naxxramas::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_ANUB_REKHAN:
        case NPC_FAERLINA:
        case NPC_GLUTH:
        case NPC_THADDIUS:
        case NPC_STALAGG:
        case NPC_FEUGEN:
        case NPC_ZELIEK:
        case NPC_THANE:
        case NPC_BLAUMEUX:
        case NPC_MOGRAINE:
        case NPC_SPIRIT_OF_BLAUMEUX:
        case NPC_SPIRIT_OF_MOGRAINE:
        case NPC_SPIRIT_OF_KORTHAZZ:
        case NPC_SPIRIT_OF_ZELIREK:
        case NPC_GOTHIK:
        case NPC_SAPPHIRON:
        case NPC_KELTHUZAD:
        case NPC_THE_LICHKING:
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
        case NPC_NAXXRAMAS_TRIGGER:
        {
            m_npcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            m_uiLivingPoisonTimer = 5 * IN_MILLISECONDS;
            break;
        }
        case NPC_ZOMBIE_CHOW:
        {
            m_lZombieChowList.push_back(pCreature->GetObjectGuid());
            pCreature->SetInCombatWithZone();
            break;
        }
        case NPC_CORPSE_SCARAB:
        {
            pCreature->SetInCombatWithZone();
            break;
        }
        case NPC_NAXXRAMAS_CULTIST:
        case NPC_NAXXRAMAS_ACOLYTE:
        {
            m_lFaerlinaFollowersList.push_back(pCreature->GetObjectGuid());
            break;
        }
        case NPC_SUB_BOSS_TRIGGER:  m_lGothTriggerList.push_back(pCreature->GetObjectGuid()); break;
        case NPC_TESLA_COIL:        m_lThadTeslaCoilList.push_back(pCreature->GetObjectGuid()); break;
        case NPC_UNREL_TRAINEE:
        case NPC_UNREL_DEATH_KNIGHT:
        case NPC_UNREL_RIDER:
            m_lUnrelentingSideList.push_back(pCreature->GetObjectGuid());
            break;
        case NPC_SPECT_TRAINEE:
        case NPC_SPECT_DEATH_KNIGHT:
        case NPC_SPECT_RIDER:
        case NPC_SPECT_HORSE:
            m_lSpectralSideList.push_back(pCreature->GetObjectGuid());
            break;
    }
}

void instance_naxxramas::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        // Arachnid Quarter
        case GO_ARAC_ANUB_DOOR:
            break;
        case GO_ARAC_ANUB_GATE:
            if (m_auiEncounter[TYPE_ANUB_REKHAN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARAC_FAER_WEB:
            break;
        case GO_ARAC_FAER_DOOR:
            if (m_auiEncounter[TYPE_FAERLINA] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARAC_MAEX_INNER_DOOR:
            break;
        case GO_ARAC_MAEX_OUTER_DOOR:
            if (m_auiEncounter[TYPE_FAERLINA] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;

        // Plague Quarter
        case GO_PLAG_NOTH_ENTRY_DOOR:
            break;
        case GO_PLAG_NOTH_EXIT_DOOR:
            if (m_auiEncounter[TYPE_NOTH] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_HEIG_ENTRY_DOOR:
            if (m_auiEncounter[TYPE_NOTH] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_HEIG_EXIT_HALLWAY:
            if (m_auiEncounter[TYPE_HEIGAN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_LOAT_DOOR:
            break;

        // Military Quarter
        case GO_MILI_GOTH_ENTRY_GATE:
            break;
        case GO_MILI_GOTH_EXIT_GATE:
            if (m_auiEncounter[TYPE_GOTHIK] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_MILI_GOTH_COMBAT_GATE:
            break;
        case GO_MILI_HORSEMEN_DOOR:
            if (m_auiEncounter[TYPE_GOTHIK] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CHEST_HORSEMEN_NORM:
            break;

        // Construct Quarter
        case GO_CONS_PATH_EXIT_DOOR:
            if (m_auiEncounter[TYPE_PATCHWERK] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_GLUT_EXIT_DOOR:
            if (m_auiEncounter[TYPE_GLUTH] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_THAD_DOOR:
            if (m_auiEncounter[TYPE_GLUTH] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_NOX_TESLA_FEUGEN:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                pGo->SetGoState(GO_STATE_READY);
            break;
        case GO_CONS_NOX_TESLA_STALAGG:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                pGo->SetGoState(GO_STATE_READY);
            break;

        // Frostwyrm Lair
        case GO_KELTHUZAD_WATERFALL_DOOR:
            if (m_auiEncounter[TYPE_SAPPHIRON] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_KELTHUZAD_EXIT_DOOR:
        case GO_KELTHUZAD_WINDOW_1:
        case GO_KELTHUZAD_WINDOW_2:
        case GO_KELTHUZAD_WINDOW_3:
        case GO_KELTHUZAD_WINDOW_4:
            break;

        // Eyes
        case GO_ARAC_EYE_RAMP:
        case GO_ARAC_EYE_BOSS:
            if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_EYE_RAMP:
        case GO_PLAG_EYE_BOSS:
            if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_MILI_EYE_RAMP:
        case GO_MILI_EYE_BOSS:
            if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_EYE_RAMP:
        case GO_CONS_EYE_BOSS:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;

        // Portals
        case GO_ARAC_PORTAL:
            if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        case GO_PLAG_PORTAL:
            if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        case GO_MILI_PORTAL:
            if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        case GO_CONS_PORTAL:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;

        default:
            return;
    }
    m_goEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_naxxramas::OnCreatureDeath(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_MR_BIGGLESWORTH:
            if ( m_auiEncounter[TYPE_KELTHUZAD] != DONE)
                DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_CAT_DIED, NPC_KELTHUZAD);
            break;
        case NPC_ZOMBIE_CHOW:
            pCreature->ForcedDespawn(2000);
            m_lZombieChowList.remove(pCreature->GetObjectGuid());
            break;
        case NPC_UNREL_TRAINEE:
            if (Creature* anchor = GetClosestAnchorForGothik(pCreature, true))
                pCreature->CastSpell(anchor, SPELL_A_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, pCreature->GetObjectGuid());
            m_lUnrelentingSideList.remove(pCreature->GetObjectGuid());
            pCreature->ForcedDespawn(4000);
            break;
        case NPC_UNREL_DEATH_KNIGHT:
            if (Creature* anchor = GetClosestAnchorForGothik(pCreature, true))
                pCreature->CastSpell(anchor, SPELL_B_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, pCreature->GetObjectGuid());
            m_lUnrelentingSideList.remove(pCreature->GetObjectGuid());
            pCreature->ForcedDespawn(4000);
            break;
        case NPC_UNREL_RIDER:
            if (Creature* anchor = GetClosestAnchorForGothik(pCreature, true))
                pCreature->CastSpell(anchor, SPELL_C_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, pCreature->GetObjectGuid());
            m_lUnrelentingSideList.remove(pCreature->GetObjectGuid());
            pCreature->ForcedDespawn(4000);
            break;
        case NPC_SPECT_TRAINEE:
        case NPC_SPECT_DEATH_KNIGHT:
        case NPC_SPECT_RIDER:
        case NPC_SPECT_HORSE:
            m_lSpectralSideList.remove(pCreature->GetObjectGuid());
            pCreature->ForcedDespawn(4000);
            break;
        default:
            break;
    }
}

bool instance_naxxramas::IsEncounterInProgress() const
{
    for (uint8 i = 0; i <= TYPE_KELTHUZAD; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;
    }

    // Some Encounters use SPECIAL while in progress
    return m_auiEncounter[TYPE_GOTHIK] == SPECIAL;
}

void instance_naxxramas::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_ANUB_REKHAN:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_ARAC_ANUB_DOOR);
            if (uiData == DONE)
                DoUseDoorOrButton(GO_ARAC_ANUB_GATE);
            break;
        case TYPE_FAERLINA:
            DoUseDoorOrButton(GO_ARAC_FAER_WEB);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_ARAC_FAER_DOOR);
                DoUseDoorOrButton(GO_ARAC_MAEX_OUTER_DOOR);
            }
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_MAEXXNA:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_ARAC_MAEX_INNER_DOOR, uiData);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_ARAC_EYE_RAMP);
                DoUseDoorOrButton(GO_ARAC_EYE_BOSS);
                DoRespawnGameObject(GO_ARAC_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_ARAC_PORTAL, GO_FLAG_NO_INTERACT, false);
                m_uiTauntTimer = 5000;
            }
            break;
        case TYPE_NOTH:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_PLAG_NOTH_ENTRY_DOOR);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_PLAG_NOTH_EXIT_DOOR);
                DoUseDoorOrButton(GO_PLAG_HEIG_ENTRY_DOOR);
            }
            break;
        case TYPE_HEIGAN:
            m_auiEncounter[uiType] = uiData;
            // Open the entrance door on encounter win or failure (we specifically set the GOState to avoid issue in case encounter is reset before gate is closed in Heigan script)
            if (uiData == DONE || uiData == FAIL)
            {
                if (GameObject* door = GetSingleGameObjectFromStorage(GO_PLAG_HEIG_ENTRY_DOOR))
                    door->SetGoState(GO_STATE_ACTIVE);
            }
            if (uiData == DONE)
                DoUseDoorOrButton(GO_PLAG_HEIG_EXIT_HALLWAY);
            break;
        case TYPE_LOATHEB:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_PLAG_LOAT_DOOR);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_PLAG_EYE_RAMP);
                DoUseDoorOrButton(GO_PLAG_EYE_BOSS);
                DoRespawnGameObject(GO_PLAG_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_PLAG_PORTAL, GO_FLAG_NO_INTERACT, false);
                m_uiTauntTimer = 5000;
            }
            break;
        case TYPE_RAZUVIOUS:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_GOTHIK:
            switch (uiData)
            {
                m_auiEncounter[uiType] = uiData;
                case IN_PROGRESS:
                    // Encounter begins: close the gate and start timer to summon unrelenting trainees
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    InitializeGothikTriggers();
                    break;
                case SPECIAL:
                    DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    for (auto& spectralGuid : m_lSpectralSideList)
                    {
                        if (Creature* spectral = instance->GetCreature(spectralGuid))
                            spectral->CastSpell(spectral, SPELL_SPECTRAL_ASSAULT, TRIGGERED_OLD_TRIGGERED);
                    }
                    for (auto& unrelentingGuid : m_lUnrelentingSideList)
                    {
                        if (Creature* unrelenting = instance->GetCreature(unrelentingGuid))
                            unrelenting->CastSpell(unrelenting, SPELL_UNRELENTING_ASSAULT, TRIGGERED_OLD_TRIGGERED);
                    }
                    break;
                case FAIL:
                    if (m_auiEncounter[uiType] == IN_PROGRESS)
                        DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    break;
                case DONE:
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_EXIT_GATE);
                    DoUseDoorOrButton(GO_MILI_HORSEMEN_DOOR);
                    // Open the central gate if Gothik is defeated before doing so
                    if (m_auiEncounter[uiType] == IN_PROGRESS)
                        DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    StartNextDialogueText(NPC_THANE);
                    break;
            }
            break;
        case TYPE_FOUR_HORSEMEN:
            // Skip if already set
            if (m_auiEncounter[uiType] == uiData)
                return;
            if (uiData == SPECIAL)
            {
                ++m_uiHorseMenKilled;

                if (m_uiHorseMenKilled == 4)
                    SetData(TYPE_FOUR_HORSEMEN, DONE);

                // Don't store special data
                break;
            }
            if (uiData == FAIL)
                m_uiHorseMenKilled = 0;
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_MILI_HORSEMEN_DOOR);
            if (uiData == DONE)
            {
                // Despawn spirits
                if (Creature* pSpirit = GetSingleCreatureFromStorage(NPC_SPIRIT_OF_BLAUMEUX))
                    pSpirit->ForcedDespawn();
                if (Creature* pSpirit = GetSingleCreatureFromStorage(NPC_SPIRIT_OF_MOGRAINE))
                    pSpirit->ForcedDespawn();
                if (Creature* pSpirit = GetSingleCreatureFromStorage(NPC_SPIRIT_OF_KORTHAZZ))
                    pSpirit->ForcedDespawn();
                if (Creature* pSpirit = GetSingleCreatureFromStorage(NPC_SPIRIT_OF_ZELIREK))
                    pSpirit->ForcedDespawn();

                DoUseDoorOrButton(GO_MILI_EYE_RAMP);
                DoUseDoorOrButton(GO_MILI_EYE_BOSS);
                DoRespawnGameObject(GO_MILI_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_MILI_PORTAL, GO_FLAG_NO_INTERACT, false);
                DoRespawnGameObject(GO_CHEST_HORSEMEN_NORM, 30 * MINUTE);
                m_uiTauntTimer = 5000;
            }
            break;
        case TYPE_PATCHWERK:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_CONS_PATH_EXIT_DOOR);
            break;
        case TYPE_GROBBULUS:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_GLUTH:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_CONS_GLUT_EXIT_DOOR);
                DoUseDoorOrButton(GO_CONS_THAD_DOOR);
            }
            break;
        case TYPE_THADDIUS:
            // Only process real changes here
            if (m_auiEncounter[uiType] == uiData)
                return;

            m_auiEncounter[uiType] = uiData;
            if (uiData == FAIL)
            {
                // Reset stage for phase 1
                // Respawn: Stalagg, Feugen, their respective Tesla Coil NPCs and Tesla GOs
                if (Creature* stalagg = GetSingleCreatureFromStorage(NPC_STALAGG))
                {
                    stalagg->ForcedDespawn();
                    stalagg->Respawn();
                }

                if (Creature* feugen = GetSingleCreatureFromStorage(NPC_FEUGEN))
                {
                    feugen->ForcedDespawn();
                    feugen->Respawn();
                }

                for (auto& teslaGuid : m_lThadTeslaCoilList)
                {
                    if (Creature* teslaCoil = instance->GetCreature(teslaGuid))
                    {
                        teslaCoil->ForcedDespawn();
                        teslaCoil->Respawn();
                    }
                }
                if (GameObject* stalaggTesla = GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_STALAGG))
                    stalaggTesla->SetGoState(GO_STATE_ACTIVE);
                if (GameObject* feugenTesla = GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_FEUGEN))
                    feugenTesla->SetGoState(GO_STATE_ACTIVE);
            }
            if (uiData != SPECIAL)
                DoUseDoorOrButton(GO_CONS_THAD_DOOR, uiData);
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_CONS_EYE_RAMP);
                DoUseDoorOrButton(GO_CONS_EYE_BOSS);
                DoRespawnGameObject(GO_CONS_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_CONS_PORTAL, GO_FLAG_NO_INTERACT, false);
                m_uiTauntTimer = 5000;
            }
            break;
        case TYPE_SAPPHIRON:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
            {
                DoUseDoorOrButton(GO_KELTHUZAD_WATERFALL_DOOR);
                StartNextDialogueText(NPC_KELTHUZAD);
            }
            // Start Sapph summoning process
            if (uiData == SPECIAL)
                m_uiSapphSpawnTimer = 22000;
            break;
        case TYPE_KELTHUZAD:
            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_KELTHUZAD_EXIT_DOOR);
            if (uiData == NOT_STARTED)
            {
                if (GameObject* pWindow = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_1))
                    pWindow->ResetDoorOrButton();
                if (GameObject* pWindow = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_2))
                    pWindow->ResetDoorOrButton();
                if (GameObject* pWindow = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_3))
                    pWindow->ResetDoorOrButton();
                if (GameObject* pWindow = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_4))
                    pWindow->ResetDoorOrButton();
            }
            break;
    }

    if (uiData == DONE || (uiData == SPECIAL && uiType == TYPE_SAPPHIRON))
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                   << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                   << m_auiEncounter[12] << " " << m_auiEncounter[13] << " " << m_auiEncounter[14] << " " << m_auiEncounter[15];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_naxxramas::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
               >> m_auiEncounter[12] >> m_auiEncounter[13] >> m_auiEncounter[14] >> m_auiEncounter[15];

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_naxxramas::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_naxxramas::Update(uint32 uiDiff)
{
    // Handle the continuous spawning of Living Poison blobs in Patchwerk corridor
    if (m_uiLivingPoisonTimer)
    {
        if (m_uiLivingPoisonTimer <= uiDiff)
        {
            if (Creature* trigger = GetSingleCreatureFromStorage(NPC_NAXXRAMAS_TRIGGER))
            {
                // Spawn 3 living poisons every 5 secs and make them cross the corridor and then despawn, for ever and ever
                for (uint8 i = 0; i < 3; i++)
                    if (Creature* poison = trigger->SummonCreature(NPC_LIVING_POISON, aLivingPoisonPositions[i].m_fX, aLivingPoisonPositions[i].m_fY, aLivingPoisonPositions[i].m_fZ, aLivingPoisonPositions[i].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                    {
                        poison->GetMotionMaster()->MovePoint(0, aLivingPoisonPositions[i + 3].m_fX, aLivingPoisonPositions[i + 3].m_fY, aLivingPoisonPositions[i + 3].m_fZ);
                        poison->ForcedDespawn(15000);
                    }
            }
            m_uiLivingPoisonTimer = 5000;
        }
        else
            m_uiLivingPoisonTimer -= uiDiff;
    }

    if (m_uiScreamsTimer && m_auiEncounter[TYPE_THADDIUS] != DONE)
    {
        if (m_uiScreamsTimer <= uiDiff)
        {
            if (Player* pPlayer = GetPlayerInMap())
                pPlayer->GetMap()->PlayDirectSoundToMap(SOUND_SCREAM1 + urand(0, 3));
            m_uiScreamsTimer = (2 * MINUTE + urand(0, 30)) * IN_MILLISECONDS;
        }
        else
            m_uiScreamsTimer -= uiDiff;
    }

    if (m_uiTauntTimer)
    {
        if (m_uiTauntTimer <= uiDiff)
        {
            DoTaunt();
            m_uiTauntTimer = 0;
        }
        else
            m_uiTauntTimer -= uiDiff;
    }

    if (m_uiSapphSpawnTimer)
    {
        if (m_uiSapphSpawnTimer <= uiDiff)
        {
            if (Player* pPlayer = GetPlayerInMap())
                pPlayer->SummonCreature(NPC_SAPPHIRON, aSapphPositions[0], aSapphPositions[1], aSapphPositions[2], aSapphPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);

            m_uiSapphSpawnTimer = 0;
        }
        else
            m_uiSapphSpawnTimer -= uiDiff;
    }

    DialogueUpdate(uiDiff);
}

// Initialize all triggers used in Gothik the Harvester encounter by flagging them with their position in the room and what kind of NPC they will summon
void instance_naxxramas::InitializeGothikTriggers()
{
    Creature* gothik = GetSingleCreatureFromStorage(NPC_GOTHIK);

    if (!gothik)
        return;

    CreatureList summonList;

    for (auto triggerGuid : m_lGothTriggerList)
    {
        if (Creature* trigger = instance->GetCreature(triggerGuid))
        {
            GothTrigger gt;
            gt.bIsAnchorHigh = (trigger->GetPositionZ() >= (gothik->GetPositionZ() - 5.0f));
            gt.bIsRightSide = IsInRightSideGothikArea(trigger);
            gt.summonTypeFlag = 0x00;
            m_mGothTriggerMap[trigger->GetObjectGuid()] = gt;

            // Keep track of triggers that will be used as summon point
            if (!gt.bIsAnchorHigh && gt.bIsRightSide)
                summonList.push_back(trigger);
        }
    }

    if (!summonList.empty())
    {
        // Sort summoning trigger NPCS by distance from Gothik
        // and flag them regarding of what they will summon
        summonList.sort(ObjectDistanceOrder(gothik));
        uint8 index = 0;
        for (auto trigger : summonList)
        {
            switch (index)
            {
                // Closest and furthest: Unrelenting Knights and Trainees
                case 0:
                case 3:
                    m_mGothTriggerMap[trigger->GetObjectGuid()].summonTypeFlag = SUMMON_FLAG_TRAINEE | SUMMON_FLAG_KNIGHT;
                    break;
                // Middle: only Unrelenting Trainee
                case 1:
                    m_mGothTriggerMap[trigger->GetObjectGuid()].summonTypeFlag = SUMMON_FLAG_TRAINEE;
                    break;
                // Other middle: Unrelenting Rider
                case 2:
                    m_mGothTriggerMap[trigger->GetObjectGuid()].summonTypeFlag = SUMMON_FLAG_RIDER;
                    break;
                default:
                    break;
            }
            ++index;
        }
    }
    else
        script_error_log("No suitable summon trigger found for Gothik combat area. Set up failed.");
}

Creature* instance_naxxramas::GetClosestAnchorForGothik(Creature* pSource, bool bRightSide)
{
    std::list<Creature* > lList;

    for (auto& itr : m_mGothTriggerMap)
    {
        if (!itr.second.bIsAnchorHigh)
            continue;

        if (itr.second.bIsRightSide != bRightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr.first))
            lList.push_back(pCreature);
    }

    if (!lList.empty())
    {
        lList.sort(ObjectDistanceOrder(pSource));
        return lList.front();
    }

    return nullptr;
}

void instance_naxxramas::GetGothikSummonPoints(CreatureList& lList, bool bRightSide)
{
    for (auto& itr : m_mGothTriggerMap)
    {
        if (itr.second.bIsAnchorHigh)
            continue;

        if (itr.second.bIsRightSide != bRightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr.first))
            lList.push_back(pCreature);
    }
}

// Right is right side from gothik (eastern), i.e. right is living and left is spectral
bool instance_naxxramas::IsInRightSideGothikArea(Unit* unit)
{
    if (GameObject* combatGate = GetSingleGameObjectFromStorage(GO_MILI_GOTH_COMBAT_GATE))
        return (combatGate->GetPositionY() >= unit->GetPositionY());

    script_error_log("left/right side check, Gothik combat area failed.");
    return true;
}

bool instance_naxxramas::IsSuitableTriggerForSummon(Unit* trigger, uint8 flag)
{
    return m_mGothTriggerMap[trigger->GetObjectGuid()].summonTypeFlag & flag;
}

void instance_naxxramas::SetChamberCenterCoords(float fX, float fY, float fZ)
{
    m_fChamberCenterX = fX;
    m_fChamberCenterY = fY;
    m_fChamberCenterZ = fZ;
}

void instance_naxxramas::DoTaunt()
{
    if (m_auiEncounter[TYPE_KELTHUZAD] != DONE)
    {
        uint8 uiWingsCleared = 0;

        if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
            ++uiWingsCleared;

        if (m_auiEncounter[TYPE_LOATHEB] == DONE)
            ++uiWingsCleared;

        if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
            ++uiWingsCleared;

        if (m_auiEncounter[TYPE_THADDIUS] == DONE)
            ++uiWingsCleared;

        switch (uiWingsCleared)
        {
            case 1: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT1, NPC_KELTHUZAD); break;
            case 2: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT2, NPC_KELTHUZAD); break;
            case 3: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT3, NPC_KELTHUZAD); break;
            case 4: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT4, NPC_KELTHUZAD); break;
        }
    }
}

// Used in Gluth fight: move all spawned Zombie Chow towards Gluth to be devoured
void instance_naxxramas::HandleDecimateEvent()
{
    if (Creature* gluth = GetSingleCreatureFromStorage(NPC_GLUTH))
    {
        for (auto& zombieGuid : m_lZombieChowList)
        {
            if (Creature* zombie = instance->GetCreature(zombieGuid))
            {
                if (zombie->isAlive())
                {
                    zombie->AI()->SetReactState(REACT_PASSIVE);
                    zombie->AttackStop();
                    zombie->SetTarget(nullptr);
                    zombie->AI()->DoResetThreat();
                    zombie->GetMotionMaster()->Clear();
                    zombie->SetWalk(true);
                    zombie->GetMotionMaster()->MoveFollow(gluth, ATTACK_DISTANCE, 0);
                }
            }
        }
    }
}

InstanceData* GetInstanceData_instance_naxxramas(Map* pMap)
{
    return new instance_naxxramas(pMap);
}

bool instance_naxxramas::DoHandleAreaTrigger(AreaTriggerEntry const* areaTrigger)
{
    if (areaTrigger->id == AREATRIGGER_KELTHUZAD)
    {
        SetChamberCenterCoords(areaTrigger->x, areaTrigger->y, areaTrigger->z);

        if (GetData(TYPE_KELTHUZAD) == NOT_STARTED)
        {
            if (Creature* kelthuzad = GetSingleCreatureFromStorage(NPC_KELTHUZAD))
            {
                if (kelthuzad->isAlive())
                {
                    SetData(TYPE_KELTHUZAD, IN_PROGRESS);
                    kelthuzad->SetInCombatWithZone();
                }
            }
        }
    }

    if (areaTrigger->id == AREATRIGGER_FAERLINA_INTRO)
    {
        if (GetData(TYPE_FAERLINA) != NOT_STARTED)
            return false;
        if (!isFaerlinaIntroDone)
            StartNextDialogueText(SAY_FAERLINA_INTRO);
    }

    if (areaTrigger->id == AREATRIGGER_THADDIUS_DOOR)
    {
        if (GetData(TYPE_THADDIUS) == NOT_STARTED)
        {
            if (Creature* thaddius = GetSingleCreatureFromStorage(NPC_THADDIUS))
            {
                SetData(TYPE_THADDIUS, SPECIAL);
                DoScriptText(SAY_THADDIUS_GREET, thaddius);
            }
        }
    }

    if (areaTrigger->id == AREATRIGGER_FROSTWYRM_TELE)
    {
        // Area trigger handles teleport in DB. Here we only need to check if all the end wing encounters are done
        if (GetData(TYPE_THADDIUS) != DONE || GetData(TYPE_LOATHEB) != DONE || GetData(TYPE_MAEXXNA) != DONE ||
                GetData(TYPE_FOUR_HORSEMEN) != DONE)
            return true;
    }

    return false;
}

bool AreaTrigger_at_naxxramas(Player* player, AreaTriggerEntry const* areaTrigger)
{
    if (player->isGameMaster() || !player->isAlive())
        return false;

    if (instance_naxxramas* instance = (instance_naxxramas*)player->GetInstanceData())
        return instance->DoHandleAreaTrigger(areaTrigger);

    return false;
}

bool ProcessEventId_decimate(uint32 eventId, Object* source, Object* /*target*/, bool /*isStart*/)
{
    if (instance_naxxramas* instance = (instance_naxxramas*)((Creature*)source)->GetInstanceData())
    {
        if (eventId == EVENT_ID_DECIMATE)
            instance->HandleDecimateEvent();
    }
    return false;   // return false so DBScripts can be triggered
}

void AddSC_instance_naxxramas()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_naxxramas";
    pNewScript->GetInstanceData = &GetInstanceData_instance_naxxramas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_naxxramas";
    pNewScript->pAreaTrigger = &AreaTrigger_at_naxxramas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_decimate";
    pNewScript->pProcessEventId = &ProcessEventId_decimate;
    pNewScript->RegisterSelf();
}
