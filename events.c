#include <stdio.h>
#include <stdlib.h>
#include "types.h"


static NationalEventCard nationalDeck[20] = {
    { 0, "Tourism Hype", 0, 200 },     // Hotels earn double rent for 5 rounds
    { 1, "Fuel Shortage", 0, 200 },     // Railway rent doubles for 5 rounds
    { 2, "Heavy Floods", 0, 0 },     // Random coastal property damaged
    { 3, "Political Rally", 0, 0 },     // One random property closed for 2 rounds
    { 4, "Stock Market Rise", 0, 0 },     // All property values increase by 10%
    { 5, "Economic Downturn", 0, 0 },     // Property values decrease by 15%
    { 6, "Housing Subsidy", 0, 0 },     //House construction cost reduced by 30%
    { 7, "Interest Rate Cut", 0, 0 },     // Loan interest reduced by 2%
    { 8, "Interest Rate Increase", 0, 0 },     // Loan interest increased by 2%
    { 9, "Tax Amnesty", 2000, 0 },    // Each player receives LKR 2000
    { 10, "Power Failure", 0, 50 },    // Utility income halved for 3 rounds
    { 11, "Foreign Funding", 0, 0 },      // Commercial property values increase by 15%
    { 12, "Port Expansion", 0, 0 },     // railway station values increase by 20%
    { 13, "Festival Season", 0, 150 },    // Hotels receive 50% additional rent
    { 14, "Labour Strike", 0, 0 },      // Construction suspended for 2 rounds
    { 15, "Insurance Discount", 0, 0 },    // Premiums reduced by 20%
    { 16, "Property Revaluation", 0, 0 },    // Random property group appreciates by 15%
    { 17, "Currency Depreciation", 0, 0 },   // Construction costs increase by 10%
    { 18, "Government Grant", 5000, 0 },     // Random player receives LKR 5000
    { 19, "National Disaster", 0, 0 },      // Random developed property gets damaged
};

// Called whenever a player lands on an Event square (Table 1). Draws the top card
// (slot 0), applies its actual Appendix A effect, then sends that same card to the
// bottom of the deck, per Appendix A: "the card shall be returned to the bottom of
// the deck." Rent-multiplier cards stay active on the drawing player for the number
// of rounds stated on the card (or NATIONAL_CARD_DURATION_ROUNDS if the card doesn't
// state one); board-wide value/cost changes are applied immediately - reverting them
// still needs a round-tick handler, same as the Rule-LK 35 gap on the Regional
// Development Cards.
void drawNationalEventCard(GameState *gamestate, int playerIndex) {

    // 1) The top card is always sitting in slot 0.
    NationalEventCard drawnCard = nationalDeck[0];
    Player *player = &gamestate->players[playerIndex];

    printf("Economic Event \n\n%s\n\n ", drawnCard.name);

    // 2) Apply this card's real effect (Appendix A). Matched by cardId, since the
    //    deck reorders after every draw so the card is no longer always at the same
    //    array index.
    if (drawnCard.cardId == 0) {  // Tourism Hype
        // Hotels earn double rent for 5 rounds - only this player's own hotels.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == playerIndex && gamestate -> board[i].hasHotel == 1) {
                gamestate -> board[i].currentRent = gamestate -> board[i].currentRent * 2;
            }
            i = i + 1;
        }
        player -> nationalCard.isActive = 1;
        player -> nationalCard.effectId = drawnCard.rentMultiplierPercentage;
        player -> nationalCard.targetGroup = GROUP_NONE;
        player -> nationalCard.roundsRemaining = 5;

    } else if (drawnCard.cardId == 1) {  // Fuel Shortage
        // Railway rent doubles for 5 rounds - only this player's own stations.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == playerIndex && gamestate -> board[i].type == SQUARE_RAILWAY) {
                gamestate -> board[i].currentRent = gamestate -> board[i].currentRent * 2;
            }
            i = i + 1;
        }
        player -> nationalCard.isActive = 1;
        player -> nationalCard.effectId = drawnCard.rentMultiplierPercentage;
        player -> nationalCard.targetGroup = GROUP_NONE;
        player -> nationalCard.roundsRemaining = 5;

    } else if (drawnCard.cardId == 2) {  // Heavy Floods
        // Random coastal property damaged.
        // ASSUMPTION - Appendix A doesn't name exact squares, so "coastal" reuses the
        // same coastal groups used for the Regional Development Cards (Yellow and
        // Light Blue).
        int coastalSquares[6] = { 26, 27, 29, 6, 8, 9 };
        int pick = rand() % 6;
        gamestate -> board[coastalSquares[pick]].isDamaged = 1;

    } else if (drawnCard.cardId == 3) {  // Political Rally
        // One random property closed for 2 rounds.
        int propertySquares[22] = { 1, 3, 6, 8, 9, 11, 13, 14, 16, 18, 19, 21, 23, 24, 26, 27, 29, 31, 32, 34, 37, 39 };
        int pick = rand() % 22;
        gamestate -> board[propertySquares[pick]].isDamaged = 1;
        // ...still to do: reopening this property after 2 rounds needs a per-square
        // round counter and a round-tick handler, neither of which exist yet.

    } else if (drawnCard.cardId == 4) {  // Stock Market Rise
        // All property values increase by 10% - board-wide, not just this player.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 110) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 5) {  // Economic Downturn
        // Property values decrease by 15% - board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 85) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 6) {  // Housing Subsidy
        // House construction cost reduced by 30% - board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 70) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 7) {  // Interest Rate Cut
        // Loan interest reduced by 2% - only future loans (Rule-LK 13: existing loan
        // rates remain unchanged).
        gamestate -> economy.baseInterestRate = gamestate -> economy.baseInterestRate - 2;
        if (gamestate -> economy.baseInterestRate < 0) {
            gamestate -> economy.baseInterestRate = 0;
        }

    } else if (drawnCard.cardId == 8) {  // Interest Rate Increase
        // Loan interest increased by 2% - only future loans.
        gamestate -> economy.baseInterestRate = gamestate -> economy.baseInterestRate + 2;

    } else if (drawnCard.cardId == 9) {  // Tax Amnesty
        // Each player receives LKR 2,000 - everyone, not just the drawer.
        int i = 0;
        while (i < NUM_PLAYERS) {
            gamestate -> players[i].cash = gamestate -> players[i].cash + drawnCard.cashBonus;
            i = i + 1;
        }

    } else if (drawnCard.cardId == 10) {  // Power Failure
        // Utility income halved for 3 rounds - only this player's own utilities.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == playerIndex && gamestate -> board[i].type == SQUARE_UTILITY) {
                gamestate -> board[i].currentRent = gamestate -> board[i].currentRent / 2;
            }
            i = i + 1;
        }
        player -> nationalCard.isActive = 1;
        player -> nationalCard.effectId = drawnCard.rentMultiplierPercentage;
        player -> nationalCard.targetGroup = GROUP_NONE;
        player -> nationalCard.roundsRemaining = 3;

    } else if (drawnCard.cardId == 11) {  // Foreign Funding
        // Commercial property values increase by 15%.
        // ASSUMPTION - Appendix A doesn't define "commercial property" separately from
        // an ordinary property, so this applies to every property square board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 115) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 12) {  // Port Expansion
        // Railway station values increase by 20% - all 4 stations, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_RAILWAY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 120) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 13) {  // Festival Season
        // Hotels receive 50% additional rent - only this player's own hotels.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].ownerId == playerIndex && gamestate -> board[i].hasHotel == 1) {
                gamestate -> board[i].currentRent = (gamestate -> board[i].currentRent * 150) / 100;
            }
            i = i + 1;
        }
        player -> nationalCard.isActive = 1;
        player -> nationalCard.effectId = drawnCard.rentMultiplierPercentage;
        player -> nationalCard.targetGroup = GROUP_NONE;
        player -> nationalCard.roundsRemaining = NATIONAL_CARD_DURATION_ROUNDS;

    } else if (drawnCard.cardId == 14) {  // Labour Strike
        // Construction suspended for 2 rounds.
        // ...still to do: enforcing this needs a global "construction suspended" flag
        // that the building-construction logic in players.c/game.c can check, which
        // doesn't exist yet.

    } else if (drawnCard.cardId == 15) {  // Insurance Discount
        // Premiums reduced by 20%.
        // ...still to do: getInsurance() in finance.c computes its premium percentage
        // from a fixed 5/10/15% table and has no discount input yet.

    } else if (drawnCard.cardId == 16) {  // Property Revaluation
        // Random property group appreciates by 15%.
        PropertyGroup randomGroup = 1 + rand() % 8;
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].group == randomGroup) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 115) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 17) {  // Currency Depreciation
        // Construction costs increase by 10% - both houses and hotels, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 110) / 100;
                gamestate -> board[i].hotelCost = (gamestate -> board[i].hotelCost * 110) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 18) {  // Government Grant
        // Random player receives LKR 5,000 - not necessarily the drawer.
        int luckyPlayer = rand() % NUM_PLAYERS;
        gamestate -> players[luckyPlayer].cash = gamestate -> players[luckyPlayer].cash + drawnCard.cashBonus;

    } else {  // cardId == 19: National Disaster - random developed property damaged.
        int developedSquares[BOARD_SIZE];
        int developedCount = 0;
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].numHouses > 0 || gamestate -> board[i].hasHotel == 1) {
                developedSquares[developedCount] = i;
                developedCount = developedCount + 1;
            }
            i = i + 1;
        }
        if (developedCount > 0) {
            int pick = rand() % developedCount;
            gamestate -> board[developedSquares[pick]].isDamaged = 1;
        }
    }

    // 3) Send the drawn card to the bottom of the deck: shift every other card up one
    //    slot, then place the drawn card in the now-empty last slot.
    int j = 0;
    while (j < 19) {
        nationalDeck[j] = nationalDeck[j + 1];
        j = j + 1;
    }
    nationalDeck[19] = drawnCard;
}

// Table 4 — all 12 Regional Development Cards.
// NOTE: the struct only models a single flat percentage, so "Water Shortage" (which has
// two different percentages for two different things) is recorded with its primary
// number here — the comment on each line still states the real Table 4 effect in full.
static RegionalDevelopmentCard regionalDeck[12] = {
    { 0, "Southern Tourism Boom", 40 },     // Galle Fort, Unawatuna, Hikkaduwa rental income +40%
    { 1, "Port City Expansion", 25 },     // Pettah, Maradana, Colombo Fort Station values +25%
    { 2, "IT Industry Growth", 20 },     // Maharagama, Nugegoda, Kottawa values +20%
    { 3, "Northern Development Programme", 30 },    // Jaffna Town, Nallur, Trincomalee values +30%
    { 4, "Tea Export Boom", 35 },     // Nuwara Eliya value +35%
    { 5, "Airport Expansion", 30 },     // Negombo, Katunayake, Ja-Ela rents +30%
    { 6, "University City Growth", 20 },     // Peradeniya, Kandy City values +20%
    { 7, "Beach Pollution", -30 },     // Southern coastal rents -30%
    { 8, "Flood Damage", -20 },     // Low-lying coastal properties lose 20% value
    { 9, "Transport Strike", -40 },     // Railway revenue reduced by 40%
    { 10, "Electricity Tariff Increase", 25 },     // Utility rent +25%
    { 11, "Water Shortage", 20 },     // Water utility revenue +20%; surrounding properties -10%
};

// Called every 15 rounds (Section 2.10). Drawn the same way as drawNationalEventCard:
// the top card (slot 0) is used, then sent to the bottom of the deck. The effect stays
// active for REGIONAL_CARD_DURATION_ROUNDS rounds, for the whole economy (unlike a
// national card, which only affects the one player who drew it).
void drawRegionalDevelopmentCard(GameState *gamestate) {

    // 1) The top card is always sitting in slot 0.
    RegionalDevelopmentCard drawnCard = regionalDeck[0];

    printf("Regional Development Card \n\n%s\n\n ", drawnCard.name);

    // 2) Mark that card as the active regional effect.
    gamestate -> economy.regionalCard.isActive = 1;
    gamestate -> economy.regionalCard.effectId = drawnCard.valuePercentageChange;
    gamestate -> economy.regionalCard.targetGroup = GROUP_NONE;
    gamestate -> economy.regionalCard.roundsRemaining = REGIONAL_CARD_DURATION_ROUNDS;

    // 3) Work out which squares this card affects (Table 4) and whether its percentage
    //    changes rental income or property/utility value. Up to 4 squares per card;
    //    unused slots stay -1. Matched by cardId, since the deck reorders after every
    //    draw so the card is no longer always at the same array index.
    int targetSquares[4] = { -1, -1, -1, -1 };
    int isRentEffect = 0;

    if (drawnCard.cardId == 0) {  // Southern Tourism Boom
        targetSquares[0] = 26;
        targetSquares[1] = 27;
        targetSquares[2] = 29;
        isRentEffect = 1;
    } else if (drawnCard.cardId == 1) {  // Port City Expansion
        targetSquares[0] = 1;
        targetSquares[1] = 3;
        targetSquares[2] = 5;
        isRentEffect = 0;
    } else if (drawnCard.cardId == 2) {  // IT Industry Growth
        targetSquares[0] = 13;
        targetSquares[1] = 11;
        targetSquares[2] = 14;
        isRentEffect = 0;
    } else if (drawnCard.cardId == 3) {  // Northern Development Programme
        targetSquares[0] = 31;
        targetSquares[1] = 32;
        targetSquares[2] = 34;
        isRentEffect = 0;
    } else if (drawnCard.cardId == 4) {  // Tea Export Boom
        targetSquares[0] = 37;
        isRentEffect = 0;
    } else if (drawnCard.cardId == 5) {  // Airport Expansion
        targetSquares[0] = 16;
        targetSquares[1] = 18;
        targetSquares[2] = 19;
        isRentEffect = 1;
    } else if (drawnCard.cardId == 6) {  // University City Growth
        targetSquares[0] = 23;
        targetSquares[1] = 21;
        isRentEffect = 0;
    } else if (drawnCard.cardId == 7) {  // Beach Pollution
        // ASSUMPTION - Table 4 doesn't name exact squares, so this targets the same
        // southern coastal group as Southern Tourism Boom (Galle Fort, Unawatuna,
        // Hikkaduwa).
        targetSquares[0] = 26;
        targetSquares[1] = 27;
        targetSquares[2] = 29;
        isRentEffect = 1;
    } else if (drawnCard.cardId == 8) {  // Flood Damage
        // ASSUMPTION - Table 4 doesn't name exact squares, so this targets the Light
        // Blue group (Bambalapitiya, Wellawatte, Mount Lavinia), the low-lying Colombo
        // coast.
        targetSquares[0] = 6;
        targetSquares[1] = 8;
        targetSquares[2] = 9;
        isRentEffect = 0;
    } else if (drawnCard.cardId == 9) {  // Transport Strike
        targetSquares[0] = 5;
        targetSquares[1] = 15;
        targetSquares[2] = 25;
        targetSquares[3] = 35;
        isRentEffect = 1;
    } else if (drawnCard.cardId == 10) {  // Electricity Tariff Increase
        targetSquares[0] = 12;
        isRentEffect = 1;
    } else {  // cardId == 11: Water Shortage - National Water Supply and Drainage
        // Board revenue +20%. The surrounding-properties -10% is a second number
        // that the deck can't store, so it's applied separately in step 5 below.
        targetSquares[0] = 28;
        isRentEffect = 1;
    }

    // 4) Apply the card's percentage change to every square it targets.
    int i = 0;
    while (i < 4) {
        if (targetSquares[i] != -1) {
            Square *square = &gamestate -> board[targetSquares[i]];

            if (isRentEffect == 1) {
                square -> currentRent = (square -> currentRent * (100 + drawnCard.valuePercentageChange)) / 100;
            } else {
                square -> currentValue = (square -> currentValue * (100 + drawnCard.valuePercentageChange)) / 100;
            }
        }
        i = i + 1;
    }

    // 5) Water Shortage's second effect: the properties surrounding the water utility
    // (Unawatuna, Hikkaduwa) lose 10% value. Table 4 gives this card two percentages,
    // so the second one is hardcoded here rather than stored in regionalDeck.
    if (drawnCard.cardId == 11) {  // Water Shortage
        gamestate -> board[27].currentValue = (gamestate -> board[27].currentValue * 90) / 100;
        gamestate -> board[29].currentValue = (gamestate -> board[29].currentValue * 90) / 100;
    }

    // ...still to do (Rule-LK 35): once this card's 15 rounds expire, these squares
    // should return to their normal market-adjusted values unless another active event
    // is still influencing them. That needs a round-tick handler in game.c/finance.c
    // that remembers each square's pre-event baseline - not part of drawing the card.

    // 6) Send the drawn card to the bottom of the deck: shift every other card up one
    //    slot, then place the drawn card in the now-empty last slot.
    int j = 0;
    while (j < 11) {
        regionalDeck[j] = regionalDeck[j + 1];
        j = j + 1;
    }
    regionalDeck[11] = drawnCard;
}

// Called every 10 rounds (Section 2.3). Rolls a fresh inflation rate and applies it
// to every property's price, rent and construction costs (Rule-LK 13/14). Existing
// loan interest rates are untouched (Rule-LK 13) - only the base rate used for loans
// taken out from now on moves.
void inflationChange(GameState *gamestate) {

    // 1) Pick a random inflation rate from the allowed list.
    int possibleRates[6] = { -3, 0, 2, 5, 8, 12 };
    gamestate -> economy.inflationRate = possibleRates[rand() % 6];

    printf("Inflation\n\nNew inflation rate : %d%%\n\n", gamestate -> economy.inflationRate);

    // 2) Apply that rate to every property's price, rent and construction costs
    //    (Rule-LK 14 formula: New Value = Previous Value x (1 + Inflation Rate)).
    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        int isProperty = 0;
        if (square -> type == SQUARE_PROPERTY) {
            isProperty = 1;
        }

        if (isProperty == 1) {
            square -> currentValue = (square -> currentValue * (100 + gamestate -> economy.inflationRate)) / 100;
            square -> purchasePrice = (square -> purchasePrice * (100 + gamestate -> economy.inflationRate)) / 100;
            square -> currentRent = (square -> currentRent * (100 + gamestate -> economy.inflationRate)) / 100;
            square -> houseCost = (square -> houseCost * (100 + gamestate -> economy.inflationRate)) / 100;
            square -> hotelCost = (square -> hotelCost * (100 + gamestate -> economy.inflationRate)) / 100;
        }

        i = i + 1;
    }

    // 3) Loan interest rates move with inflation too, but only for loans taken out
    //    from now on - takeLoan() (finance.c) only reads baseInterestRate at the
    //    moment a new loan is created, so existing loans keep their locked-in rate.
    gamestate -> economy.baseInterestRate = (gamestate -> economy.baseInterestRate * (100 + gamestate -> economy.inflationRate)) / 100;
    if (gamestate -> economy.baseInterestRate < 0) {
        gamestate -> economy.baseInterestRate = 0;
    }

    // ...still to do: Rule-LK 13 also lists "Insurance premiums" and "Repair costs."
    // Insurance premiums don't need separate handling here - getInsurance()
    // (finance.c) already computes them as a percentage of currentValue, so they
    // inflate automatically once currentValue does. Repair costs have no stored
    // field or system yet (see yetToMake.txt), so there's nothing to apply
    // inflation to there.
}

// Called every 10 rounds (Section 2.9). Picks a random property group for a Market
// Boom and a different one for a Market Decline, then applies their Rule-LK 31/32
// percentage changes to every square in those groups.
void propertyMarketChange(GameState *gamestate) {

    PropertyGroup allGroups[8] = {
        GROUP_BROWN, GROUP_LIGHT_BLUE, GROUP_PINK, GROUP_ORANGE,
        GROUP_RED, GROUP_YELLOW, GROUP_GREEN, GROUP_DARK_BLUE
    };

    // 1) Pick a boom group and a decline group - they must not be the same group,
    //    and neither may repeat the group that had that same event last review
    //    (Rule-LK 30).
    PropertyGroup boomGroup = allGroups[rand() % 8];
    while (boomGroup == gamestate -> economy.lastBoomGroup) {
        boomGroup = allGroups[rand() % 8];
    }

    PropertyGroup declineGroup = allGroups[rand() % 8];
    while (declineGroup == boomGroup || declineGroup == gamestate -> economy.lastDeclineGroup) {
        declineGroup = allGroups[rand() % 8];
    }

    // 2) Record the boom/decline groups and how long they last (Rule-LK 31/32: both
    //    remain active for 10 rounds).
    gamestate -> economy.marketBoom.isActive = 1;
    gamestate -> economy.marketBoom.targetGroup = boomGroup;
    gamestate -> economy.marketBoom.roundsRemaining = 10;

    gamestate -> economy.marketDecline.isActive = 1;
    gamestate -> economy.marketDecline.targetGroup = declineGroup;
    gamestate -> economy.marketDecline.roundsRemaining = 10;

    gamestate -> economy.lastBoomGroup = boomGroup;
    gamestate -> economy.lastDeclineGroup = declineGroup;

    // 3) Apply the Market Boom effect to every square in the boom group (Rule-LK 31)
    //    and the Market Decline effect to every square in the decline group
    //    (Rule-LK 32).
    int i = 0;
    while (i < BOARD_SIZE) {
        Square *square = &gamestate -> board[i];

        if (square -> group == boomGroup) {
            square -> purchasePrice = (square -> purchasePrice * 115) / 100;   // +15%
            square -> mortgageValue = (square -> mortgageValue * 115) / 100;   // +15%
            square -> currentRent = (square -> currentRent * 125) / 100;       // +25%
            square -> houseCost = (square -> houseCost * 110) / 100;           // +10%
            square -> hotelCost = (square -> hotelCost * 110) / 100;           // +10%
            square -> currentValue = (square -> currentValue * 120) / 100;     // +20%
        }

        if (square -> group == declineGroup) {
            square -> currentValue = (square -> currentValue * 85) / 100;      // -15%
            square -> currentRent = (square -> currentRent * 80) / 100;        // -20%
            square -> mortgageValue = (square -> mortgageValue * 90) / 100;    // -10%
        }

        i = i + 1;
    }

    // ...still to do: Rule-LK 32 also says auction starting prices drop 25% during a
    // decline, but there's no stored "auction starting price" field, and Rule-LK 19's
    // auction system (starting bid = 50% of market value) isn't built yet either -
    // see yetToMake.txt.

    // ...still to do (Rule-LK 33): a group shouldn't be reselected for 30 rounds, not
    // just "not the same as last review." lastBoomGroup/lastDeclineGroup only
    // remember the single most recent group, which stops immediate repeats but can't
    // enforce a 30-round cooldown across all 8 groups - that needs a per-group
    // rounds-since-used tracker that doesn't exist yet.
}

// Table 4-equivalent for Section 2.7 - just the 8 regulation names, since (unlike the
// card decks) each regulation's effect is a different shape and doesn't share one
// common percentage field worth storing in a struct.
static char *governmentRegulationNames[8] = {
    "Increase Property Tax",
    "Reduce Loan Interest",
    "Housing Subsidy",
    "Luxury Property Tax",
    "Railway Modernization",
    "Electricity Tariff Revision",
    "Insurance Regulation",
    "Anti-Speculation Act",
};

// Called every 20 rounds (Section 2.7). Picks one of the 8 regulations at random and
// applies its effect (Rule-LK 24).
void governmentRegulationChange(GameState *gamestate) {

    int regulationIndex = rand() % 8;

    printf("Government Regulation\n\n%s\n\n", governmentRegulationNames[regulationIndex]);

    // 1) Mark this regulation as the active one. Rule-LK 24 doesn't state how long a
    //    regulation stays in force, so this reuses its own 20-round draw interval,
    //    same as how the National/Regional cards reuse their own draw interval.
    gamestate -> economy.governmentRegulation.isActive = 1;
    gamestate -> economy.governmentRegulation.effectId = regulationIndex;
    gamestate -> economy.governmentRegulation.targetGroup = GROUP_NONE;
    gamestate -> economy.governmentRegulation.roundsRemaining = GOVERNMENT_REGULATION_DURATION_ROUNDS;

    // 2) Apply whichever regulation was picked.
    if (regulationIndex == 0) {
        // Increase Property Tax - Income Tax increases by 50%.
        // ...still to do: there's no stored Income Tax amount anywhere on the board,
        // and Rule 11's tax collection isn't implemented in finance.c yet either, so
        // there's nothing here to increase by 50% - see yetToMake.txt.

    } else if (regulationIndex == 1) {
        // Reduce Loan Interest - interest decreases by 2%, future loans only.
        gamestate -> economy.baseInterestRate = gamestate -> economy.baseInterestRate - 2;
        if (gamestate -> economy.baseInterestRate < 0) {
            gamestate -> economy.baseInterestRate = 0;
        }

    } else if (regulationIndex == 2) {
        // Housing Subsidy - house construction costs reduce 30%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 70) / 100;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 3) {
        // Luxury Property Tax - hotels pay a tax of 25% of property value.
        // ASSUMPTION - "annual maintenance tax" implies this should repeat every
        // round, but there's no round-tick handler yet to keep charging it (see
        // yetToMake.txt), so it's charged once, immediately, instead of never at all.
        int i = 0;
        while (i < BOARD_SIZE) {
            Square *square = &gamestate -> board[i];
            if (square -> hasHotel == 1 && square -> ownerId != -1) {
                int tax = (square -> currentValue * 25) / 100;
                gamestate -> players[square -> ownerId].cash = gamestate -> players[square -> ownerId].cash - tax;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 4) {
        // Railway Modernization - railway rents increase 25%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_RAILWAY) {
                gamestate -> board[i].currentRent = (gamestate -> board[i].currentRent * 125) / 100;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 5) {
        // Electricity Tariff Revision - utility rents increase 20%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_UTILITY) {
                gamestate -> board[i].currentRent = (gamestate -> board[i].currentRent * 120) / 100;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 6) {
        // Insurance Regulation - premiums decrease 15%, coverage unchanged.
        // ...still to do: getInsurance() (finance.c) computes its premium from a
        // fixed 5/10/15% table with no discount input yet - same gap as the
        // Insurance Discount National Event Card.

    } else {
        // regulationIndex == 7: Anti-Speculation Act - players may own at most three
        // undeveloped properties; additional purchases need development within 5
        // rounds.
        // ...still to do: enforcing this needs a purchase-time check in players.c/
        // game.c, neither of which exist yet.
    }
}

// Called every 10 rounds (Section 2.2 / Rule-LK 10). Picks one of the 5 disaster
// types and damages one randomly selected developed property. An insured property
// (if its policy covers that disaster type) gets compensation credited immediately;
// otherwise the owner pays the full repair cost once they can afford it (Rule-LK 11).
void triggerDisaster(GameState *gamestate) {

    // 1) Build a list of every developed property (has a house or a hotel) and pick
    //    one at random. Nothing to damage if nothing has been built yet.
    int developedSquares[BOARD_SIZE];
    int developedCount = 0;
    int i = 0;
    while (i < BOARD_SIZE) {
        if (gamestate -> board[i].numHouses > 0 || gamestate -> board[i].hasHotel == 1) {
            developedSquares[developedCount] = i;
            developedCount = developedCount + 1;
        }
        i = i + 1;
    }

    if (developedCount == 0) {
        return;
    }

    int squareIndex = developedSquares[rand() % developedCount];
    Square *square = &gamestate -> board[squareIndex];

    DisasterType disaster = rand() % 5;
    char *disasterNames[5] = { "Fire", "Flood", "Riot", "Building Collapse", "Electrical Failure" };

    printf("Disaster\n\n%s\n\nAffected Property :\n%s\n\n", disasterNames[disaster], square -> name);

    // 2) Mark the property damaged - Rule-LK 11 says a damaged building can't collect
    //    rent until it's repaired.
    square -> isDamaged = 1;

    // 3) Work out the repair cost.
    // ASSUMPTION - the brief never gives a repair cost formula, so this reuses the
    // same 10%-of-current-value figure Rule-LK 17 already uses for renovation.
    int repairCost = (square -> currentValue * 10) / 100;

    // 4) Check whether this property's insurance policy actually covers this
    //    disaster type (Table 10 / Section 1.2). Building Collapse and Electrical
    //    Failure aren't listed under either policy's "Protect Against" list, so
    //    they're never covered - the owner always pays for those two.
    int isCovered = 0;
    if (square -> insurancePolicy == INSURANCE_BASIC) {
        if (disaster == DISASTER_FIRE || disaster == DISASTER_FLOOD) {
            isCovered = 1;
        }
    } else if (square -> insurancePolicy == INSURANCE_COMPREHENSIVE) {
        if (disaster == DISASTER_FIRE || disaster == DISASTER_FLOOD || disaster == DISASTER_RIOT) {
            isCovered = 1;
        }
    } else if (square -> insurancePolicy == INSURANCE_BUSINESS_INTERRUPTION) {
        if (square -> hasHotel == 1) {
            isCovered = 1;
        }
    }

    if (isCovered == 1 && square -> ownerId != -1) {

        // 5) Pay out compensation instead of charging the owner (Rule-LK 10).
        int compensationPercentage = 80;
        if (square -> insurancePolicy == INSURANCE_COMPREHENSIVE) {
            compensationPercentage = 100;
        } else if (square -> insurancePolicy == INSURANCE_BUSINESS_INTERRUPTION) {
            compensationPercentage = 100;
        }

        int compensation = (repairCost * compensationPercentage) / 100;
        gamestate -> players[square -> ownerId].cash = gamestate -> players[square -> ownerId].cash + compensation;
        square -> isDamaged = 0; // repaired immediately using the claim

        printf("Insurance Claim Approved.\nCompensation Paid :\nLKR %d.\n\n", compensation);

    } else if (square -> ownerId != -1) {

        // 6) Uninsured (or this disaster type isn't covered) - the owner pays in
        //    full, once they can afford it (Rule-LK 11: repairs happen automatically
        //    once the owner has sufficient funds).
        if (gamestate -> players[square -> ownerId].cash >= repairCost) {
            gamestate -> players[square -> ownerId].cash = gamestate -> players[square -> ownerId].cash - repairCost;
            square -> isDamaged = 0;
            printf("Repair Cost Paid :\nLKR %d.\n\n", repairCost);
        } else {
            printf("Owner cannot yet afford the LKR %d repair - property remains damaged.\n\n", repairCost);
        }
    }

    // ...still to do: Business Interruption Insurance should also cover "lost rental
    // income for five rounds" on top of the repair cost - that needs a per-square
    // rounds-remaining tracker and a round-tick handler that don't exist yet.
}

