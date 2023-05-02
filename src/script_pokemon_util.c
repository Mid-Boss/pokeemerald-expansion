#include "global.h"
#include "battle.h"
#include "battle_gfx_sfx_util.h"
#include "battle_util.h"
#include "berry.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "event_data.h"
#include "international_string_util.h"
#include "link.h"
#include "link_rfu.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeball.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sprite.h"
#include "string_util.h"
#include "tv.h"
#include "constants/items.h"
#include "constants/battle_frontier.h"

static void CB2_ReturnFromChooseHalfParty(void);
static void CB2_ReturnFromChooseBattleFrontierParty(void);

extern struct Evolution gEvolutionTable[][EVOS_PER_MON];

void HealPlayerParty(void)
{
    u8 i, j;
    u8 ppBonuses;
    u8 arg[4];

    // restore HP.
    for(i = 0; i < gPlayerPartyCount; i++)
    {
        u16 maxHP = GetMonData(&gPlayerParty[i], MON_DATA_MAX_HP);
        arg[0] = maxHP;
        arg[1] = maxHP >> 8;
        SetMonData(&gPlayerParty[i], MON_DATA_HP, arg);
        ppBonuses = GetMonData(&gPlayerParty[i], MON_DATA_PP_BONUSES);

        // restore PP.
        for(j = 0; j < MAX_MON_MOVES; j++)
        {
            arg[0] = CalculatePPWithBonus(GetMonData(&gPlayerParty[i], MON_DATA_MOVE1 + j), ppBonuses, j);
            SetMonData(&gPlayerParty[i], MON_DATA_PP1 + j, arg);
        }

        // since status is u32, the four 0 assignments here are probably for safety to prevent undefined data from reaching SetMonData.
        arg[0] = 0;
        arg[1] = 0;
        arg[2] = 0;
        arg[3] = 0;
        SetMonData(&gPlayerParty[i], MON_DATA_STATUS, arg);
    }
}

u8 ScriptGiveMon(u16 species, u8 level, u16 item, u32 unused1, u32 unused2, u8 unused3)
{
    u16 nationalDexNum;
    int sentToPc;
    u8 heldItem[2];
    struct Pokemon mon;
    u16 targetSpecies;

    CreateMon(&mon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    heldItem[0] = item;
    heldItem[1] = item >> 8;
    SetMonData(&mon, MON_DATA_HELD_ITEM, heldItem);

    // In case a mon with a form changing item is given. Eg: SPECIES_ARCEUS with ITEM_SPLASH_PLATE will transform into SPECIES_ARCEUS_WATER upon gifted.
    targetSpecies = GetFormChangeTargetSpecies(&mon, FORM_ITEM_HOLD, 0);
    if (targetSpecies != SPECIES_NONE)
    {
        SetMonData(&mon, MON_DATA_SPECIES, &targetSpecies);
        CalculateMonStats(&mon);
    }

    sentToPc = GiveMonToPlayer(&mon);
    nationalDexNum = SpeciesToNationalPokedexNum(species);

    // Don't set Pokédex flag for MON_CANT_GIVE
    switch(sentToPc)
    {
    case MON_GIVEN_TO_PARTY:
    case MON_GIVEN_TO_PC:
        GetSetPokedexFlag(nationalDexNum, FLAG_SET_SEEN);
        GetSetPokedexFlag(nationalDexNum, FLAG_SET_CAUGHT);
        break;
    }
    return sentToPc;
}

u8 ScriptGiveEgg(u16 species)
{
    struct Pokemon mon;
    u8 isEgg;

    CreateEgg(&mon, species, TRUE);
    isEgg = TRUE;
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);

    return GiveMonToPlayer(&mon);
}

void HasEnoughMonsForDoubleBattle(void)
{
    switch (GetMonsStateToDoubles())
    {
    case PLAYER_HAS_TWO_USABLE_MONS:
        gSpecialVar_Result = PLAYER_HAS_TWO_USABLE_MONS;
        break;
    case PLAYER_HAS_ONE_MON:
        gSpecialVar_Result = PLAYER_HAS_ONE_MON;
        break;
    case PLAYER_HAS_ONE_USABLE_MON:
        gSpecialVar_Result = PLAYER_HAS_ONE_USABLE_MON;
        break;
    }
}

static bool8 CheckPartyMonHasHeldItem(u16 item)
{
    int i;

    for(i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (species != SPECIES_NONE && species != SPECIES_EGG && GetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM) == item)
            return TRUE;
    }
    return FALSE;
}

bool8 DoesPartyHaveEnigmaBerry(void)
{
    bool8 hasItem = CheckPartyMonHasHeldItem(ITEM_ENIGMA_BERRY_E_READER);
    if (hasItem == TRUE)
        GetBerryNameByBerryType(ItemIdToBerryType(ITEM_ENIGMA_BERRY_E_READER), gStringVar1);

    return hasItem;
}

void CreateScriptedWildMon(u16 species, u8 level, u16 item)
{
    u8 heldItem[2];

    ZeroEnemyPartyMons();
    CreateMon(&gEnemyParty[0], species, level, USE_RANDOM_IVS, 0, 0, OT_ID_PLAYER_ID, 0);
    if (item)
    {
        heldItem[0] = item;
        heldItem[1] = item >> 8;
        SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, heldItem);
    }
}
void CreateScriptedDoubleWildMon(u16 species1, u8 level1, u16 item1, u16 species2, u8 level2, u16 item2)
{
    u8 heldItem1[2];
    u8 heldItem2[2];

    ZeroEnemyPartyMons();

    CreateMon(&gEnemyParty[0], species1, level1, 32, 0, 0, OT_ID_PLAYER_ID, 0);
    if (item1)
    {
        heldItem1[0] = item1;
        heldItem1[1] = item1 >> 8;
        SetMonData(&gEnemyParty[0], MON_DATA_HELD_ITEM, heldItem1);
    }

    CreateMon(&gEnemyParty[3], species2, level2, 32, 0, 0, OT_ID_PLAYER_ID, 0);
    if (item2)
    {
        heldItem2[0] = item2;
        heldItem2[1] = item2 >> 8;
        SetMonData(&gEnemyParty[3], MON_DATA_HELD_ITEM, heldItem2);
    }
}

void ScriptSetMonMoveSlot(u8 monIndex, u16 move, u8 slot)
{
// Allows monIndex to go out of bounds of gPlayerParty. Doesn't occur in vanilla
#ifdef BUGFIX
    if (monIndex >= PARTY_SIZE)
#else
    if (monIndex > PARTY_SIZE)
#endif
        monIndex = gPlayerPartyCount - 1;

    SetMonMoveSlot(&gPlayerParty[monIndex], move, slot);
}

// Note: When control returns to the event script, gSpecialVar_Result will be
// TRUE if the party selection was successful.
void ChooseHalfPartyForBattle(void)
{
    gMain.savedCallback = CB2_ReturnFromChooseHalfParty;
    VarSet(VAR_FRONTIER_FACILITY, FACILITY_MULTI_OR_EREADER);
    InitChooseHalfPartyForBattle(0);
}

static void CB2_ReturnFromChooseHalfParty(void)
{
    switch (gSelectedOrderFromParty[0])
    {
    case 0:
        gSpecialVar_Result = FALSE;
        break;
    default:
        gSpecialVar_Result = TRUE;
        break;
    }

    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void ChoosePartyForBattleFrontier(void)
{
    gMain.savedCallback = CB2_ReturnFromChooseBattleFrontierParty;
    InitChooseHalfPartyForBattle(gSpecialVar_0x8004 + 1);
}

static void CB2_ReturnFromChooseBattleFrontierParty(void)
{
    switch (gSelectedOrderFromParty[0])
    {
    case 0:
        gSpecialVar_Result = FALSE;
        break;
    default:
        gSpecialVar_Result = TRUE;
        break;
    }

    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

void ReducePlayerPartyToSelectedMons(void)
{
    struct Pokemon party[MAX_FRONTIER_PARTY_SIZE];
    int i;

    CpuFill32(0, party, sizeof party);

    // copy the selected pokemon according to the order.
    for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; i++)
        if (gSelectedOrderFromParty[i]) // as long as the order keeps going (did the player select 1 mon? 2? 3?), do not stop
            party[i] = gPlayerParty[gSelectedOrderFromParty[i] - 1]; // index is 0 based, not literal

    CpuFill32(0, gPlayerParty, sizeof gPlayerParty);

    // overwrite the first 4 with the order copied to.
    for (i = 0; i < MAX_FRONTIER_PARTY_SIZE; i++)
        gPlayerParty[i] = party[i];

    CalculatePlayerPartyCount();
}

u8 ScriptGiveCustomMon(u16 species, u8 level, u16 item, u8 ball, u8 nature, u8 abilityNum, u8 *evs, u8 *ivs, u16 *moves, bool8 isShiny)
{
    u16 nationalDexNum;
    int sentToPc;
    u8 heldItem[2];
    struct Pokemon mon;
    u8 i;
    u8 evTotal = 0;
    
    if (nature == NUM_NATURES || nature == 0xFF)
        nature = Random() % NUM_NATURES;
    
    if (isShiny)
        CreateShinyMonWithNature(&mon, species, level, nature);
    else
        CreateMonWithNature(&mon, species, level, 32, nature);
    
    for (i = 0; i < NUM_STATS; i++)
    {
        // ev
        if (evs[i] != 0xFF && evTotal < 510)
        {
            // only up to 510 evs
            if ((evTotal + evs[i]) > 510)
                evs[i] = (510 - evTotal);
            
            evTotal += evs[i];
            SetMonData(&mon, MON_DATA_HP_EV + i, &evs[i]);
        }
        
        // iv
        if (ivs[i] != 32 && ivs[i] != 0xFF)
            SetMonData(&mon, MON_DATA_HP_IV + i, &ivs[i]);
    }
    CalculateMonStats(&mon);
    
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] == 0 || moves[i] == 0xFF || moves[i] > MOVES_COUNT)
            continue;
        
        SetMonMoveSlot(&mon, moves[i], i);
    }
    
    //ability
    if (abilityNum == 0xFF || GetAbilityBySpecies(species, abilityNum) == 0)
    {
        do {
            abilityNum = Random() % 3;  // includes hidden abilities
        } while (GetAbilityBySpecies(species, abilityNum) == 0);
    }
    
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);
    
    //ball
    if (ball <= POKEBALL_COUNT)
        SetMonData(&mon, MON_DATA_POKEBALL, &ball);
    
    //item
    heldItem[0] = item;
    heldItem[1] = item >> 8;
    SetMonData(&mon, MON_DATA_HELD_ITEM, heldItem);
    
    // give player the mon
    //sentToPc = GiveMonToPlayer(&mon);
    SetMonData(&mon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
    SetMonData(&mon, MON_DATA_OT_GENDER, &gSaveBlock2Ptr->playerGender);
    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            break;
    }

    if (i >= PARTY_SIZE)
    {
        sentToPc = SendMonToPC(&mon);
    }
    else
    {
        sentToPc = MON_GIVEN_TO_PARTY;
        CopyMon(&gPlayerParty[i], &mon, sizeof(mon));
        gPlayerPartyCount = i + 1;
    }
    
    nationalDexNum = SpeciesToNationalPokedexNum(species); 
    switch(sentToPc)
    {
    case MON_GIVEN_TO_PARTY:
    case MON_GIVEN_TO_PC:
        GetSetPokedexFlag(nationalDexNum, FLAG_SET_SEEN);
        GetSetPokedexFlag(nationalDexNum, FLAG_SET_CAUGHT);
        break;
    case MON_CANT_GIVE:
        break;
    }
    
    return sentToPc;
}

u8 ScriptCheckMonStats(u16 dexNum, u8 rarity)
{
    u8 i;
    u8 maxEvolutionLine = 4;
    u16 targetSpecies;
    u16 evoSpecies;
    u16 statTotal;
    u16 bronzeThres = 450;
    u16 silverThres = 525;
    u16 goldThres = 1000;
    u16 threshold;

    targetSpecies = NationalPokedexNumToSpecies(dexNum);

    statTotal = gSpeciesInfo[targetSpecies].baseHP +
                gSpeciesInfo[targetSpecies].baseAttack +
                gSpeciesInfo[targetSpecies].baseSpeed +
                gSpeciesInfo[targetSpecies].baseSpAttack +
                gSpeciesInfo[targetSpecies].baseDefense +
                gSpeciesInfo[targetSpecies].baseSpDefense;

    switch (rarity)
    {
    case (0):
        threshold = bronzeThres;
        break;
    case (1):
        threshold = silverThres;
        break;
    case (2):
        threshold = goldThres;
        break;
    }

    if (statTotal > threshold)
        return TRUE;

    if (gEvolutionTable[targetSpecies - 1][0].targetSpecies == targetSpecies)
        return TRUE;

    for (i = 0; i < maxEvolutionLine; i++)
    {
        if (gEvolutionTable[targetSpecies][0].targetSpecies == SPECIES_NONE 
            || gEvolutionTable[targetSpecies][0].method == EVO_MEGA_EVOLUTION
            || gEvolutionTable[targetSpecies][0].method == EVO_MOVE_MEGA_EVOLUTION
            || gEvolutionTable[targetSpecies][0].method == EVO_PRIMAL_REVERSION)
        {
            statTotal = gSpeciesInfo[targetSpecies].baseHP +
                gSpeciesInfo[targetSpecies].baseAttack +
                gSpeciesInfo[targetSpecies].baseSpeed +
                gSpeciesInfo[targetSpecies].baseSpAttack +
                gSpeciesInfo[targetSpecies].baseDefense +
                gSpeciesInfo[targetSpecies].baseSpDefense;

            if (statTotal > threshold)
                return TRUE;
            else
                break;
        }
        else
        {
            targetSpecies = gEvolutionTable[targetSpecies][0].targetSpecies;
        }

        /*if (gEvolutionTable[targetSpecies][0].targetSpecies == SPECIES_NONE
            || gEvolutionTable[targetSpecies][0].method == EVO_MEGA_EVOLUTION
            || gEvolutionTable[targetSpecies][0].method == EVO_MOVE_MEGA_EVOLUTION
            || gEvolutionTable[targetSpecies][0].method == EVO_PRIMAL_REVERSION)
        {
            statTotal = gSpeciesInfo[gEvolutionTable[targetSpecies][0].targetSpecies].baseHP +
                gSpeciesInfo[gEvolutionTable[targetSpecies][0].targetSpecies].baseAttack +
                gSpeciesInfo[gEvolutionTable[targetSpecies][0].targetSpecies].baseSpeed +
                gSpeciesInfo[gEvolutionTable[targetSpecies][0].targetSpecies].baseSpAttack +
                gSpeciesInfo[gEvolutionTable[targetSpecies][0].targetSpecies].baseDefense +
                gSpeciesInfo[gEvolutionTable[targetSpecies][0].targetSpecies].baseSpDefense;

            if (statTotal > threshold)
                return TRUE;
            break;
        }*/
    }
    
    return FALSE;
}

u16 ScriptGetRandomMon(u8 gen, u8 rarity) // 0 = All Gens, 1-8 = Gens 1-8
{
    u16 national_dex_num;
    int gen1Dex = 151;
    int gen1DexLegArr[5] = { 144, 145, 146, 150, 151 };
    int gen2Dex = 251;
    int gen2DexLegArr[6] = { 243, 244, 245, 249, 250, 251 };
    int gen3Dex = 386;
    int gen3DexLegArr[10] = { 377, 378, 379, 380, 381, 382, 383, 384, 385, 386 };
    int gen4Dex = 493;
    int gen4DexLegArr[14] = { 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493 };
    int gen5Dex = 649;
    int gen5DexLegArr[12] = { 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649 };
    int gen6Dex = 721;
    int gen6DexLegArr[6] = { 716, 717, 718, 719, 720, 721 };
    int gen7Dex = 809;
    int gen7DexLegArr[25] = { 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809 };
    int gen8Dex = 905;
    int gen8DexLegArr[12] = { 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 905 };
    int allLeg[90] = { 144, 145, 146, 150, 151, 243, 244, 245, 249, 250, 251, 377, 378, 379, 380, 381, 382, 383, 384,
        385, 386, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 638, 639, 640, 641, 642,
        643, 644, 645, 646, 647, 648, 649, 716, 717, 718, 719, 720, 721, 785, 786, 787, 788, 789, 790, 791, 792,
        793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 888, 889, 890, 891,
        892, 893, 894, 895, 896, 897, 898, 905 };
    u16 targetSpecies;
    u8 isLeg;
    u8 rarityCheck;
    u8 sanityCheck;
    u16 i;
    
    if (!FlagGet(FLAG_SYS_POKEDEX_GET))
        FlagSet(FLAG_SYS_POKEDEX_GET);
    if (!FlagGet(FLAG_SYS_POKEMON_GET))
        FlagSet(FLAG_SYS_POKEMON_GET);
    if (!FlagGet(FLAG_SYS_NATIONAL_DEX))
        FlagSet(FLAG_SYS_NATIONAL_DEX);

    isLeg = TRUE;
    rarityCheck = TRUE;
    sanityCheck = TRUE;
    switch (gen)
    {
    case (0):
        while (isLeg || rarityCheck)
        {
            national_dex_num = Random() % gen8Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == allLeg[i])
                {
                    isLeg = TRUE;
                    break;
                }
                else
                    isLeg = FALSE;
            }
            rarityCheck = ScriptCheckMonStats(national_dex_num, rarity);
        }
        break;
    case (1):
        while (isLeg || rarityCheck || sanityCheck)
        {
            isLeg = TRUE;
            rarityCheck = TRUE;
            sanityCheck = TRUE;

            national_dex_num = Random() % gen1Dex;
            if (national_dex_num <= gen1Dex)
                sanityCheck = FALSE;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen1DexLegArr[i])
                {
                    isLeg = TRUE;
                    break;
                }
                else
                    isLeg = FALSE;
            }
            rarityCheck = ScriptCheckMonStats(national_dex_num, rarity);
        }
        break;
    case (2):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen2Dex - gen1Dex)) + gen1Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen2DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    case (3):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen3Dex - gen2Dex)) + gen2Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen3DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    case (4):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen4Dex - gen3Dex)) + gen3Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen4DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    case (5):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen5Dex - gen4Dex)) + gen4Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen5DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    case (6):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen6Dex - gen5Dex)) + gen5Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen6DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    case (7):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen7Dex - gen6Dex)) + gen6Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen7DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    case (8):
        while (isLeg)
        {
            national_dex_num = (Random() % (gen8Dex - gen7Dex)) + gen7Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == gen8DexLegArr[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    default:
        while (isLeg)
        {
            national_dex_num = Random() % gen8Dex;
            for (i = 0; i < 90; i++)
            {
                if (national_dex_num == allLeg[i])
                    break;
            }
            if (i >= 90)
                isLeg = FALSE;
        }
        break;
    }
    
        targetSpecies = NationalPokedexNumToSpecies(national_dex_num);

    return targetSpecies;
}

/*u16 ScriptGiveRandomMon(u8 gen, u8 rarity)
{
    u16 targetSpecies;

    if (!FlagGet(FLAG_SYS_POKEDEX_GET))
        FlagSet(FLAG_SYS_POKEDEX_GET);
    if (!FlagGet(FLAG_SYS_POKEMON_GET))
        FlagSet(FLAG_SYS_POKEMON_GET);
    if (!FlagGet(FLAG_SYS_NATIONAL_DEX))
        FlagSet(FLAG_SYS_NATIONAL_DEX);

    targetSpecies = ScriptGetRandomMon(gen, rarity);
    
    //ScriptGiveMon(targetSpecies, 50, ITEM_NONE, 0, 0, 0);

    return targetSpecies;
}*/