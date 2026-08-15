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

// Draws the top card (Appendix A), applies its effect, then sends it to the
// bottom of the deck. Card effects apply permanently, not reverted later.
void drawNationalEventCard(GameState *gamestate, int playerIndex) {

    // 1) The top card is always sitting in slot 0.
    NationalEventCard drawnCard = nationalDeck[0];
    Player *player = &gamestate->players[playerIndex];

    printf("Economic Event \n\n%s\n\n ", drawnCard.name);

    // 2) Apply this card's real effect (Appendix A), matched by cardId since the
    //    deck reorders after every draw and the card isn't always at the same index.
    if (drawnCard.cardId == 0) {  // Tourism Hype
        // Hotels earn double rent for 5 rounds, only this player's own hotels.
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
        // Railway rent doubles for 5 rounds, only this player's own stations.
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
        // ASSUMPTION: "coastal" reuses the Regional Cards' Yellow/Light
        // Blue groups, since Appendix A gives no squares.
        int coastalSquares[6] = { 26, 27, 29, 6, 8, 9 };
        int pick = rand() % 6;
        gamestate -> board[coastalSquares[pick]].isDamaged = 1;

    } else if (drawnCard.cardId == 3) {  // Political Rally
        // One random property closed for 2 rounds, reopened automatically
        // by board.c's reopenClosedSquares().
        int propertySquares[22] = { 1, 3, 6, 8, 9, 11, 13, 14, 16, 18, 19, 21, 23, 24, 26, 27, 29, 31, 32, 34, 37, 39 };
        int pick = rand() % 22;
        gamestate -> board[propertySquares[pick]].isDamaged = 1;
        gamestate -> board[propertySquares[pick]].closedRoundsLeft = 2;

    } else if (drawnCard.cardId == 4) {  // Stock Market Rise
        // All property values increase by 10%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 110) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 110) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 5) {  // Economic Downturn
        // Property values decrease by 15%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 85) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 85) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 6) {  // Housing Subsidy
        // House construction cost reduced by 30%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 70) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 7) {  // Interest Rate Cut
        // Loan interest reduced by 2%, future loans only (Rule-LK 13).
        gamestate -> economy.baseInterestRate = gamestate -> economy.baseInterestRate - 2;
        if (gamestate -> economy.baseInterestRate < 0) {
            gamestate -> economy.baseInterestRate = 0;
        }

    } else if (drawnCard.cardId == 8) {  // Interest Rate Increase
        // Loan interest increased by 2%, future loans only.
        gamestate -> economy.baseInterestRate = gamestate -> economy.baseInterestRate + 2;

    } else if (drawnCard.cardId == 9) {  // Tax Amnesty
        // Each player receives LKR 2,000, not just the drawer.
        int i = 0;
        while (i < NUM_PLAYERS) {
            gamestate -> players[i].cash = gamestate -> players[i].cash + drawnCard.cashBonus;
            i = i + 1;
        }

    } else if (drawnCard.cardId == 10) {  // Power Failure
        // Utility income halved for 3 rounds, only this player's own utilities.
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
        // ASSUMPTION: "commercial property" applies board-wide, since
        // Appendix A doesn't define it separately.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 115) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 115) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 12) {  // Port Expansion
        // Railway station values increase by 20%, all 4 stations.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_RAILWAY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 120) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 120) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 13) {  // Festival Season
        // Hotels receive 50% additional rent, only this player's own hotels.
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
        // Construction suspended for 2 rounds, checked in decideConstruction().
        gamestate -> economy.constructionSuspendedRoundsLeft = 2;

    } else if (drawnCard.cardId == 15) {  // Insurance Discount
        // ASSUMPTION: discount is stored on Economy, so it applies to
        // every player, not just the drawer.
        gamestate -> economy.insurancePremiumDiscountPercent = 20;

    } else if (drawnCard.cardId == 16) {  // Property Revaluation
        // Random property group appreciates by 15%.
        PropertyGroup randomGroup = 1 + rand() % 8;
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].group == randomGroup) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 115) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 115) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 17) {  // Currency Depreciation
        // Construction costs increase by 10%, houses and hotels both.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 110) / 100;
                gamestate -> board[i].hotelCost = (gamestate -> board[i].hotelCost * 110) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 18) {  // Government Grant
        // Random player receives LKR 5,000, not necessarily the drawer.
        int luckyPlayer = rand() % NUM_PLAYERS;
        gamestate -> players[luckyPlayer].cash = gamestate -> players[luckyPlayer].cash + drawnCard.cashBonus;

    } else {  // cardId == 19: National Disaster, random developed property damaged.
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

// Table 4: all 12 cards. Water Shortage's second number is applied
// separately below since the struct only stores one percentage.
static RegionalDevelopmentCard regionalDeck[12] = {
    { 0, "Southern Tourism Boom", 40 },     // Galle Fort, Unawatuna, Hikkaduwa rental income +40%
    { 1, "Port City Expansion", 25 },     // Pettah, Maradana, Colombo Fort Station values +25%
    { 2, "IT Industry Growth", 20 },     // Maharagama, Nugegoda, Kottawa values +20%
    { 3, "Northern Development Programme", 30 },    // Jaffna Town, Nallur, Trincomalee values +30%
    { 4, "Tea Export Boom", 35 },     // Nuwara Eliya value +35%
    { 5, "Airport Expansion", 30 },     // Negombo, Katunayake, Ja-Ela rents +30%
    { 6, "University City Growth", 20 },     //Peradeniya, Kandy City values +20%
    { 7, "Beach Pollution", -30 },     // Southern coastal rents -30%
    { 8, "Flood Damage", -20 },     //Low-lying coastal properties lose 20% value
    { 9, "Transport Strike", -40 },     // Railway revenue reduced by 40%
    { 10, "Electricity Tariff Increase", 25 },     //Utility rent +25%
    { 11, "Water Shortage", 20 },     // Water utility revenue +20%; surrounding properties -10%
};

// Table 4 target lookup used by drawRegionalDevelopmentCard() (up to 4
// squares per card, unused slots -1).
static void getRegionalCardTargets(int cardId, int targetSquares[4], int *isRentEffect) {
    targetSquares[0] = -1;
    targetSquares[1] = -1;
    targetSquares[2] = -1;
    targetSquares[3] = -1;

    if (cardId == 0) {  // Southern Tourism Boom
        targetSquares[0] = 26;
        targetSquares[1] = 27;
        targetSquares[2] = 29;
        *isRentEffect = 1;
    } else if (cardId == 1) {  // Port City Expansion
        targetSquares[0] = 1;
        targetSquares[1] = 3;
        targetSquares[2] = 5;
        *isRentEffect = 0;
    } else if (cardId == 2) {  // IT Industry Growth
        targetSquares[0] = 13;
        targetSquares[1] = 11;
        targetSquares[2] = 14;
        *isRentEffect = 0;
    } else if (cardId == 3) {  // Northern Development Programme
        targetSquares[0] = 31;
        targetSquares[1] = 32;
        targetSquares[2] = 34;
        *isRentEffect = 0;
    } else if (cardId == 4) {  // Tea Export Boom
        targetSquares[0] = 37;
        *isRentEffect = 0;
    } else if (cardId == 5) {  // Airport Expansion
        targetSquares[0] = 16;
        targetSquares[1] = 18;
        targetSquares[2] = 19;
        *isRentEffect = 1;
    } else if (cardId == 6) {  // University City Growth
        targetSquares[0] = 23;
        targetSquares[1] = 21;
        *isRentEffect = 0;
    } else if (cardId == 7) {  // Beach Pollution
        // ASSUMPTION: same group as Southern Tourism Boom.
        targetSquares[0] = 26;
        targetSquares[1] = 27;
        targetSquares[2] = 29;
        *isRentEffect = 1;
    } else if (cardId == 8) {  // Flood Damage
        // ASSUMPTION: Light Blue group, the low-lying Colombo coast.
        targetSquares[0] = 6;
        targetSquares[1] = 8;
        targetSquares[2] = 9;
        *isRentEffect = 0;
    } else if (cardId == 9) {  // Transport Strike
        targetSquares[0] = 5;
        targetSquares[1] = 15;
        targetSquares[2] = 25;
        targetSquares[3] = 35;
        *isRentEffect = 1;
    } else if (cardId == 10) {  // Electricity Tariff Increase
        targetSquares[0] = 12;
        *isRentEffect = 1;
    } else {  // cardId == 11: Water Shortage
        // The surrounding -10% is a second number, handled by the caller.
        targetSquares[0] = 28;
        *isRentEffect = 1;
    }
}

// Called every 15 rounds (Section 2.10). Affects the whole economy, not
// just the drawer like a National Event Card.
void drawRegionalDevelopmentCard(GameState *gamestate) {

    // 1) The top card is always sitting in slot 0.
    RegionalDevelopmentCard drawnCard = regionalDeck[0];

    printf("Regional Development Card \n\n%s\n\n ", drawnCard.name);

    // 2) Mark that card as the active regional effect. cardId is stored so
    //    displayMarketConditions() (Rule-LK 36) can look its name back up later.
    gamestate -> economy.regionalCard.isActive = 1;
    gamestate -> economy.regionalCard.effectId = drawnCard.valuePercentageChange;
    gamestate -> economy.regionalCard.cardId = drawnCard.cardId;
    gamestate -> economy.regionalCard.targetGroup = GROUP_NONE;
    gamestate -> economy.regionalCard.roundsRemaining = REGIONAL_CARD_DURATION_ROUNDS;

    // 3) Work out which squares this card affects and whether it changes rent or value.
    int targetSquares[4];
    int isRentEffect = 0;
    getRegionalCardTargets(drawnCard.cardId, targetSquares, &isRentEffect);

    // 4) Apply the card's percentage change to every square it targets.
    int i = 0;
    while (i < 4) {
        if (targetSquares[i] != -1) {
            Square *square = &gamestate -> board[targetSquares[i]];

            if (isRentEffect == 1) {
                square -> currentRent = (square -> currentRent * (100 + drawnCard.valuePercentageChange)) / 100;
            } else {
                square -> currentValue = (square -> currentValue * (100 + drawnCard.valuePercentageChange)) / 100;
                square -> purchasePrice = (square -> purchasePrice * (100 + drawnCard.valuePercentageChange)) / 100;
            }
        }
        i = i + 1;
    }

    // 5) Water Shortage's second effect: Unawatuna and Hikkaduwa lose 10% value.
    if (drawnCard.cardId == 11) {  // Water Shortage
        gamestate -> board[27].currentValue = (gamestate -> board[27].currentValue * 90) / 100;
        gamestate -> board[27].purchasePrice = (gamestate -> board[27].purchasePrice * 90) / 100;
        gamestate -> board[29].currentValue = (gamestate -> board[29].currentValue * 90) / 100;
        gamestate -> board[29].purchasePrice = (gamestate -> board[29].purchasePrice * 90) / 100;
    }

    // 6) Send the drawn card to the bottom of the deck: shift every other card up one
    //    slot, then place the drawn card in the now-empty last slot.
    int j = 0;
    while (j < 11) {
        regionalDeck[j] = regionalDeck[j + 1];
        j = j + 1;
    }
    regionalDeck[11] = drawnCard;
}

// Called every 10 rounds (Section 2.3). Applies a new inflation rate to
// property prices/rent/costs and future loan interest (Rule-LK 13/14).
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

    // 3) Loan interest moves with inflation too, future loans only.
    gamestate -> economy.baseInterestRate = (gamestate -> economy.baseInterestRate * (100 + gamestate -> economy.inflationRate)) / 100;
    if (gamestate -> economy.baseInterestRate < 0) {
        gamestate -> economy.baseInterestRate = 0;
    }

    // Repair costs (Rule-LK 13) aren't a stored field of their own, but
    // they're computed off currentValue/houseCost/hotelCost, which are
    // already inflated above, so they move with inflation indirectly.
}

// Called every 10 rounds (Section 2.9). Picks a boom group and a
// different decline group, applies Rule-LK 31/32 to each.
void propertyMarketChange(GameState *gamestate) {

    PropertyGroup allGroups[8] = {
        GROUP_BROWN, GROUP_LIGHT_BLUE, GROUP_PINK, GROUP_ORANGE,
        GROUP_RED, GROUP_YELLOW, GROUP_GREEN, GROUP_DARK_BLUE
    };

    // 1) Pick boom/decline groups, skipping the last-used ones (Rule-LK 30)
    //    and any group still on cooldown (Rule-LK 33).
    PropertyGroup boomGroup = allGroups[rand() % 8];
    while (boomGroup == gamestate -> economy.lastBoomGroup || gamestate -> economy.groupCooldownRounds[boomGroup] > 0) {
        boomGroup = allGroups[rand() % 8];
    }

    PropertyGroup declineGroup = allGroups[rand() % 8];
    while (declineGroup == boomGroup || declineGroup == gamestate -> economy.lastDeclineGroup || gamestate -> economy.groupCooldownRounds[declineGroup] > 0) {
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

    // Rule-LK 33: start each group's 30-round cooldown right away, the
    // moment it's picked, so it can't be picked again too soon.
    gamestate -> economy.groupCooldownRounds[boomGroup] = GROUP_COOLDOWN_ROUNDS;
    gamestate -> economy.groupCooldownRounds[declineGroup] = GROUP_COOLDOWN_ROUNDS;

    // 3) Apply the Market Boom effect (Rule-LK 31) to the boom group's squares and
    //    the Market Decline effect (Rule-LK 32) to the decline group's squares.
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
}

// Section 2.7's 8 regulation names, each with a differently-shaped effect.
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

// Called every 20 rounds. Picks one regulation at random (Rule-LK 24).
void governmentRegulationChange(GameState *gamestate) {

    int regulationIndex = rand() % 8;

    printf("Government Regulation\n\n%s\n\n", governmentRegulationNames[regulationIndex]);

    // 1) Mark it active for 20 rounds (its own draw interval, same as
    //    the other card systems).
    gamestate -> economy.governmentRegulation.isActive = 1;
    gamestate -> economy.governmentRegulation.effectId = regulationIndex;
    gamestate -> economy.governmentRegulation.cardId = regulationIndex;
    gamestate -> economy.governmentRegulation.targetGroup = GROUP_NONE;
    gamestate -> economy.governmentRegulation.roundsRemaining = GOVERNMENT_REGULATION_DURATION_ROUNDS;

    // 2) Apply whichever regulation was picked.
    if (regulationIndex == 0) {
        // Increase Property Tax: Income Tax +50%.
        gamestate -> economy.currentIncomeTaxPercent = (gamestate -> economy.currentIncomeTaxPercent * 150) / 100;

    } else if (regulationIndex == 1) {
        // Reduce Loan Interest: -2%, future loans only.
        gamestate -> economy.baseInterestRate = gamestate -> economy.baseInterestRate - 2;
        if (gamestate -> economy.baseInterestRate < 0) {
            gamestate -> economy.baseInterestRate = 0;
        }

    } else if (regulationIndex == 2) {
        // Housing Subsidy: house construction costs -30%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 70) / 100;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 3) {
        // Luxury Property Tax: charged every round by finance.c's
        // applyRecurringLuxuryTax(), not here.

    } else if (regulationIndex == 4) {
        // Railway Modernization: railway rents +25%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_RAILWAY) {
                gamestate -> board[i].currentRent = (gamestate -> board[i].currentRent * 125) / 100;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 5) {
        // Electricity Tariff Revision: utility rents +20%, board-wide.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_UTILITY) {
                gamestate -> board[i].currentRent = (gamestate -> board[i].currentRent * 120) / 100;
            }
            i = i + 1;
        }

    } else if (regulationIndex == 6) {
        // Insurance Regulation: premiums -15%, read by getInsurance().
        gamestate -> economy.insurancePremiumDiscountPercent = 15;

    } else {
        // regulationIndex == 7: Anti-Speculation Act, max 3 undeveloped
        // properties per player, enforced by decideToPurchase(). Every
        // already-owned undeveloped property gets a 5-round deadline too.
        gamestate -> economy.antiSpeculationActive = 1;
        int i = 0;
        while (i < BOARD_SIZE) {
            Square *square = &gamestate -> board[i];
            if (square -> type == SQUARE_PROPERTY && square -> ownerId != -1 && square -> numHouses == 0 && square -> hasHotel == 0) {
                square -> developmentDeadlineRounds = 5;
            }
            i = i + 1;
        }
    }
}

// Called every 10 rounds (Rule-LK 10). Damages a random developed property;
// insured owners get compensation, others pay full repair (Rule-LK 11).
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

    // 2) Mark damaged, no rent until repaired (Rule-LK 11).
    square -> isDamaged = 1;

    // 3) ASSUMPTION: no repair formula given. Cost is a percentage of the
    // property value PLUS its houses/hotel, scaled by disaster severity.
    int repairPercent = 10; // Riot
    if (disaster == DISASTER_FIRE) {
        repairPercent = 20;
    } else if (disaster == DISASTER_FLOOD) {
        repairPercent = 15;
    } else if (disaster == DISASTER_COLLAPSE) {
        repairPercent = 30; // matches DEPRECIATION_MAX_PERCENTAGE
    } else if (disaster == DISASTER_ELECTRICAL) {
        repairPercent = 5;
    }

    int buildingValue = square -> numHouses * square -> houseCost;
    if (square -> hasHotel == 1) {
        buildingValue = square -> hotelCost;
    }

    int repairCost = ((square -> currentValue + buildingValue) * repairPercent) / 100;

    // 4) Check whether the insurance policy covers this disaster type (Table 10).
    //    Building Collapse and Electrical Failure aren't covered by either policy.
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

        // 6) Uninsured (or not covered), the owner pays in full once they can afford
        //    it (Rule-LK 11: repairs happen automatically once funds are sufficient)
        if (gamestate -> players[square -> ownerId].cash >= repairCost) {
            gamestate -> players[square -> ownerId].cash = gamestate -> players[square -> ownerId].cash - repairCost;
            square -> isDamaged = 0;
            printf("Repair Cost Paid :\nLKR %d.\n\n", repairCost);
        } else {
            printf("Owner cannot yet afford the LKR %d repair - Property remains damaged!\n\n", repairCost);
        }
    }

    // TODO: Business Interruption's 5 rounds of lost rent isn't tracked yet.
}

// Rule-LK 18: a separate deck from Appendix A, triggered every 15 rounds
// and affecting every player at once.
static EconomicEventCard economicDeck[8] = {
    { 0, "Tourism Boom" },                 // Hotel rent doubles; Southern coastal values +15%
    { 1, "Fuel Crisis" },                  // Railway rent doubles; development costs +20%
    { 2, "Heavy Monsoon" },                // Coastal properties -10% value
    { 3, "Economic Recession" },           // Property values -15%, rent -10%, loan interest +15%
    { 4, "Stock Market Boom" },            // Property values +10%, loan interest -10%
    { 5, "Government Housing Programme" }, // House construction costs -25%
    { 6, "Foreign Investment" },           // Commercial (all) properties +20%
    { 7, "Political Unrest" },             // Hotel rent -50%
};

// Draws the top card, applies its effect, cycles it to the bottom. Effects
// are permanent, same as Inflation, since Rule-LK 18 gives no duration.
void drawEconomicEvent(GameState *gamestate) {

    EconomicEventCard drawnCard = economicDeck[0];
    int coastalSquares[6] = { 26, 27, 29, 6, 8, 9 };

    printf("Economic Event\n\n%s\n\n", drawnCard.name);

    if (drawnCard.cardId == 0) {  // Tourism Boom
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].hasHotel == 1) {
                gamestate -> board[i].currentRent = gamestate -> board[i].currentRent * 2;
            }
            i = i + 1;
        }
        int j = 0;
        while (j < 6) {
            gamestate -> board[coastalSquares[j]].currentValue = (gamestate -> board[coastalSquares[j]].currentValue * 115) / 100;
            gamestate -> board[coastalSquares[j]].purchasePrice = (gamestate -> board[coastalSquares[j]].purchasePrice * 115) / 100;
            j = j + 1;
        }

    } else if (drawnCard.cardId == 1) {  // Fuel Crisis
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_RAILWAY) {
                gamestate -> board[i].currentRent = gamestate -> board[i].currentRent * 2;
            }
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 120) / 100;
                gamestate -> board[i].hotelCost = (gamestate -> board[i].hotelCost * 120) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 2) {  // Heavy Monsoon
        // ASSUMPTION: flood risk/insurance premium effects give no
        // number, so only the coastal value drop is modeled.
        int j = 0;
        while (j < 6) {
            gamestate -> board[coastalSquares[j]].currentValue = (gamestate -> board[coastalSquares[j]].currentValue * 90) / 100;
            gamestate -> board[coastalSquares[j]].purchasePrice = (gamestate -> board[coastalSquares[j]].purchasePrice * 90) / 100;
            j = j + 1;
        }

    } else if (drawnCard.cardId == 3) {  // Economic Recession
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 85) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 85) / 100;
                gamestate -> board[i].currentRent = (gamestate -> board[i].currentRent * 90) / 100;
            }
            i = i + 1;
        }
        gamestate -> economy.baseInterestRate = (gamestate -> economy.baseInterestRate * 115) / 100;

    } else if (drawnCard.cardId == 4) {  // Stock Market Boom
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 110) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 110) / 100;
            }
            i = i + 1;
        }
        gamestate -> economy.baseInterestRate = (gamestate -> economy.baseInterestRate * 90) / 100;

    } else if (drawnCard.cardId == 5) {  // Government Housing Programme
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].houseCost = (gamestate -> board[i].houseCost * 75) / 100;
            }
            i = i + 1;
        }

    } else if (drawnCard.cardId == 6) {  // Foreign Investment
        // ASSUMPTION: "commercial properties" read board-wide, same as
        // Foreign Funding (National Event Card).
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].type == SQUARE_PROPERTY) {
                gamestate -> board[i].currentValue = (gamestate -> board[i].currentValue * 120) / 100;
                gamestate -> board[i].purchasePrice = (gamestate -> board[i].purchasePrice * 120) / 100;
            }
            i = i + 1;
        }

    } else {  // cardId == 7: Political Unrest
        // ASSUMPTION: riot probability/claims effects give no number,
        // so only the hotel rent drop is modeled.
        int i = 0;
        while (i < BOARD_SIZE) {
            if (gamestate -> board[i].hasHotel == 1) {
                gamestate -> board[i].currentRent = gamestate -> board[i].currentRent / 2;
            }
            i = i + 1;
        }
    }

    int j = 0;
    while (j < 7) {
        economicDeck[j] = economicDeck[j + 1];
        j = j + 1;
    }
    economicDeck[7] = drawnCard;
}

// Rule-LK 35: ticks down each active effect and marks it inactive when its
// time runs out. Price/rent changes stay permanent, like Inflation (see assumptions.txt).
void revertExpiredEffects(GameState *gamestate) {

    if (gamestate -> economy.regionalCard.isActive == 1) {
        gamestate -> economy.regionalCard.roundsRemaining = gamestate -> economy.regionalCard.roundsRemaining - 1;
        if (gamestate -> economy.regionalCard.roundsRemaining <= 0) {
            gamestate -> economy.regionalCard.isActive = 0;
        }
    }

    if (gamestate -> economy.governmentRegulation.isActive == 1) {
        gamestate -> economy.governmentRegulation.roundsRemaining = gamestate -> economy.governmentRegulation.roundsRemaining - 1;
        if (gamestate -> economy.governmentRegulation.roundsRemaining <= 0) {
            gamestate -> economy.governmentRegulation.isActive = 0;
            gamestate -> economy.antiSpeculationActive = 0;
            gamestate -> economy.insurancePremiumDiscountPercent = 0;
        }
    }

    if (gamestate -> economy.marketBoom.isActive == 1) {
        gamestate -> economy.marketBoom.roundsRemaining = gamestate -> economy.marketBoom.roundsRemaining - 1;
        if (gamestate -> economy.marketBoom.roundsRemaining <= 0) {
            gamestate -> economy.marketBoom.isActive = 0;
        }
    }

    if (gamestate -> economy.marketDecline.isActive == 1) {
        gamestate -> economy.marketDecline.roundsRemaining = gamestate -> economy.marketDecline.roundsRemaining - 1;
        if (gamestate -> economy.marketDecline.roundsRemaining <= 0) {
            gamestate -> economy.marketDecline.isActive = 0;
        }
    }

    // Labour Strike / Political Unrest's construction suspension counts down
    // the same way, even though it isn't stored as an ActiveEffect.
    if (gamestate -> economy.constructionSuspendedRoundsLeft > 0) {
        gamestate -> economy.constructionSuspendedRoundsLeft = gamestate -> economy.constructionSuspendedRoundsLeft - 1;
    }
}

// Small helper for displayMarketConditions(): turns a PropertyGroup enum
// value into its human-readable name for printing.
static char *getGroupName(PropertyGroup group) {
    if (group == GROUP_BROWN) {
        return "Brown";
    } else if (group == GROUP_LIGHT_BLUE) {
        return "Light Blue";
    } else if (group == GROUP_PINK) {
        return "Pink";
    } else if (group == GROUP_ORANGE) {
        return "Orange";
    } else if (group == GROUP_RED) {
        return "Red";
    } else if (group == GROUP_YELLOW) {
        return "Yellow";
    } else if (group == GROUP_GREEN) {
        return "Green";
    } else if (group == GROUP_DARK_BLUE) {
        return "Dark Blue";
    } else {
        return "None";
    }
}

// Rule-LK 36: prints every currently active economy-wide condition, by name,
// for visibility into the simulation's current state.
void displayMarketConditions(GameState *gamestate) {

    printf("---- Current Market Conditions (Round %d) ----\n", gamestate -> economy.round);
    printf("Inflation Rate : %d%%\n", gamestate -> economy.inflationRate);
    printf("Base Loan Interest Rate : %d%%\n", gamestate -> economy.baseInterestRate);

    if (gamestate -> economy.regionalCard.isActive == 1) {
        int cardId = gamestate -> economy.regionalCard.cardId;
        int k = 0;
        while (k < 12) {
            if (regionalDeck[k].cardId == cardId) {
                printf("Regional Development Card Active : %s (%d%%, %d rounds left)\n",
                    regionalDeck[k].name, gamestate -> economy.regionalCard.effectId, gamestate -> economy.regionalCard.roundsRemaining);
            }
            k = k + 1;
        }
    }

    if (gamestate -> economy.governmentRegulation.isActive == 1) {
        printf("Government Regulation Active : %s (%d rounds left)\n",
            governmentRegulationNames[gamestate -> economy.governmentRegulation.cardId], gamestate -> economy.governmentRegulation.roundsRemaining);
    }

    if (gamestate -> economy.marketBoom.isActive == 1) {
        printf("Market Boom Active : %s Group (%d rounds left)\n",
            getGroupName(gamestate -> economy.marketBoom.targetGroup), gamestate -> economy.marketBoom.roundsRemaining);
    }

    if (gamestate -> economy.marketDecline.isActive == 1) {
        printf("Market Decline Active : %s Group (%d rounds left)\n",
            getGroupName(gamestate -> economy.marketDecline.targetGroup), gamestate -> economy.marketDecline.roundsRemaining);
    }

    if (gamestate -> economy.constructionSuspendedRoundsLeft > 0) {
        printf("Construction Suspended : %d rounds left\n", gamestate -> economy.constructionSuspendedRoundsLeft);
    }

    if (gamestate -> economy.antiSpeculationActive == 1) {
        printf("Anti-Speculation Act Active : max 3 undeveloped properties per player\n");
    }

    printf("-----------------------------------------------\n");
}
