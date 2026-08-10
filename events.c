#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"


static NationalEventCard nationalDeck[20] = {
    { "Tourism Hype", 0, 200 },     // Hotels earn double rent for 5 rounds
    { "Fuel Shortage", 0, 200 },     // Railway rent doubles for 5 rounds
    { "Heavy Floods", 0, 0 },     // Random coastal property damaged
    { "Political Rally", 0, 0 },     // One random property closed for 2 rounds
    { "Stock Market Rise", 0, 0 },     // All property values increase by 10%
    { "Economic Downturn", 0, 0 },     // Property values decrease by 15%
    { "Housing Subsidy", 0, 0 },     //House construction cost reduced by 30%
    { "Interest Rate Cut", 0, 0 },     // Loan interest reduced by 2%
    { "Interest Rate Increase", 0, 0 },     // Loan interest increased by 2%
    { "Tax Amnesty", 2000, 0 },    // Each player receives LKR 2,000
    { "Power Failure", 0, 50 },    // Utility income halved for 3 rounds
    { "Foreign Funding", 0, 0 },      // Commercial property values increase by 15%
    { "Port Expansion", 0, 0 },     // railway station values increase by 20%
    { "Festival Season", 0, 150 },    // Hotels receive 50% additional rent
    { "Labour Strike", 0, 0 },      // Construction suspended for 2 rounds
    { "Insurance Discount", 0, 0 },    // Premiums reduced by 20%
    { "Property Revaluation", 0, 0 },    // Random property group appreciates by 15%
    { "Currency Depreciation", 0, 0 },   // Construction costs increase by 10%
    { "Government Grant", 5000, 0 },     // Random player receives LKR 5000
    { "National Disaster", 0, 0 },      // Random developed property gets damaged
};

// Draws the top card (slot 0), applies whatever the NationalEventCard struct can
// represent to the player who drew it, then sends that same card to the bottom of
// the deck, per Appendix A: "the card shall be returned to the bottom of the deck."
void drawNationalEventCard(GameState *gamestate, int playerIndex) {

    // 1) The top card is always sitting in slot 0.
    NationalEventCard drawnCard = nationalDeck[0];
    Player *player = &gamestate->players[playerIndex];

    printf("Economic Event \n\n%s\n\n ", drawnCard.name);

    // 2) A cash bonus (if the card has one) is credited immediately.
    int hasCashBonus = 0;
    if (drawnCard.cashBonus != 0) {
        hasCashBonus = 1;
    }

    if (hasCashBonus == 1) {
        player -> cash = player -> cash + drawnCard.cashBonus;
    }

    // 3) A rent multiplier (if the card has one) stays active on this player for the
    //    standard national card duration.
    int hasRentMultiplier = 0;
    if (drawnCard.rentMultiplierPercentage != 0) {
        hasRentMultiplier = 1;
    }

    if (hasRentMultiplier == 1) {
        player -> nationalCard.isActive = 1;
        player -> nationalCard.effectId = drawnCard.rentMultiplierPercentage;
        player -> nationalCard.targetGroup = GROUP_NONE;
        player -> nationalCard.roundsRemaining = NATIONAL_CARD_DURATION_ROUNDS;
    }
    // ...still to do: property value, construction cost, insurance premium and damage
    // effects for the remaining cards, once those systems exist in finance.c/board.c.

    // 4) Send the drawn card to the bottom of the deck: shift every other card up one
    //    slot, then place the drawn card in the now-empty last slot.
    int i = 0;
    while (i < 19) {
        nationalDeck[i] = nationalDeck[i + 1];
        i = i + 1;
    }
    nationalDeck[19] = drawnCard;
}

// Table 4 — all 12 Regional Development Cards.
// NOTE: the struct only models a single flat percentage, so "Water Shortage" (which has
// two different percentages for two different things) is recorded with its primary
// number here — the comment on each line still states the real Table 4 effect in full.
static RegionalDevelopmentCard regionalDeck[12] = {
    { "Southern Tourism Boom", 40 },     // Galle Fort, Unawatuna, Hikkaduwa rental income +40%
    { "Port City Expansion", 25 },     // Pettah, Maradana, Colombo Fort Station values +25%
    { "IT Industry Growth", 20 },     // Maharagama, Nugegoda, Kottawa values +20%
    { "Northern Development Programme", 30 },    // Jaffna Town, Nallur, Trincomalee values +30%
    { "Tea Export Boom", 35 },     // Nuwara Eliya value +35%
    { "Airport Expansion", 30 },     // Negombo, Katunayake, Ja-Ela rents +30%
    { "University City Growth", 20 },     // Peradeniya, Kandy City values +20%
    { "Beach Pollution", -30 },     // Southern coastal rents -30%
    { "Flood Damage", -20 },     // Low-lying coastal properties lose 20% value
    { "Transport Strike", -40 },     // Railway revenue reduced by 40%
    { "Electricity Tariff Increase", 25 },     // Utility rent +25%
    { "Water Shortage", 20 },     // Water utility revenue +20%; surrounding properties -10%
};

// Called every 15 rounds (Section 2.10). Picks a random card out of the 12 and marks
// it active for REGIONAL_CARD_DURATION_ROUNDS rounds, for the whole economy (unlike a
// national card, which only affects the one player who drew it).
void drawRegionalDevelopmentCard(GameState *gamestate) {

    // 1) Pick a random card.
    int cardIndex = rand() % 12;
    RegionalDevelopmentCard drawnCard = regionalDeck[cardIndex];

    printf("Regional Development Card \n\n%s\n\n ", drawnCard.name);

    // 2) Mark that card as the active regional effect.
    gamestate -> economy.regionalCard.isActive = 1;
    gamestate -> economy.regionalCard.effectId = cardIndex;
    gamestate -> economy.regionalCard.targetGroup = GROUP_NONE;
    gamestate -> economy.regionalCard.roundsRemaining = REGIONAL_CARD_DURATION_ROUNDS;

    // 3) Work out which squares this card affects (Table 4) and whether its percentage
    //    changes rental income or property/utility value. Up to 4 squares per card;
    //    unused slots stay -1.
    int targetSquares[4] = { -1, -1, -1, -1 };
    int isRentEffect = 0;

    if (cardIndex == 0) {
        // Southern Tourism Boom: Galle Fort, Unawatuna, Hikkaduwa rental income +40%
        targetSquares[0] = 26;
        targetSquares[1] = 27;
        targetSquares[2] = 29;
        isRentEffect = 1;
    } else if (cardIndex == 1) {
        // Port City Expansion: Pettah, Maradana, Colombo Fort Station values +25%
        targetSquares[0] = 1;
        targetSquares[1] = 3;
        targetSquares[2] = 5;
        isRentEffect = 0;
    } else if (cardIndex == 2) {
        // IT Industry Growth: Maharagama, Nugegoda, Kottawa values +20%
        targetSquares[0] = 13;
        targetSquares[1] = 11;
        targetSquares[2] = 14;
        isRentEffect = 0;
    } else if (cardIndex == 3) {
        // Northern Development Programme: Jaffna Town, Nallur, Trincomalee values +30%
        targetSquares[0] = 31;
        targetSquares[1] = 32;
        targetSquares[2] = 34;
        isRentEffect = 0;
    } else if (cardIndex == 4) {
        // Tea Export Boom: Nuwara Eliya value +35%
        targetSquares[0] = 37;
        isRentEffect = 0;
    } else if (cardIndex == 5) {
        // Airport Expansion: Negombo, Katunayake, Ja-Ela rents +30%
        targetSquares[0] = 16;
        targetSquares[1] = 18;
        targetSquares[2] = 19;
        isRentEffect = 1;
    } else if (cardIndex == 6) {
        // University City Growth: Peradeniya, Kandy City values +20%
        targetSquares[0] = 23;
        targetSquares[1] = 21;
        isRentEffect = 0;
    } else if (cardIndex == 7) {
        // Beach Pollution: Southern coastal rents -30%.
        // ASSUMPTION - Table 4 doesn't name exact squares, so this targets the same
        // southern coastal group as Southern Tourism Boom (Galle Fort, Unawatuna,
        // Hikkaduwa).
        targetSquares[0] = 26;
        targetSquares[1] = 27;
        targetSquares[2] = 29;
        isRentEffect = 1;
    } else if (cardIndex == 8) {
        // Flood Damage: low-lying coastal properties lose 20% value.
        // ASSUMPTION - Table 4 doesn't name exact squares, so this targets the Light
        // Blue group (Bambalapitiya, Wellawatte, Mount Lavinia), the low-lying Colombo
        // coast.
        targetSquares[0] = 6;
        targetSquares[1] = 8;
        targetSquares[2] = 9;
        isRentEffect = 0;
    } else if (cardIndex == 9) {
        // Transport Strike: railway revenue reduced by 40% - all 4 stations.
        targetSquares[0] = 5;
        targetSquares[1] = 15;
        targetSquares[2] = 25;
        targetSquares[3] = 35;
        isRentEffect = 1;
    } else if (cardIndex == 10) {
        // Electricity Tariff Increase: Ceylon Electricity Board rent +25%
        targetSquares[0] = 12;
        isRentEffect = 1;
    } else {
        // cardIndex == 11: Water Shortage - National Water Supply and Drainage Board
        // revenue +20%. The surrounding-properties -10% is a second number that the
        // deck can't store, so it's applied separately in step 5 below.
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
    if (cardIndex == 11) {
        gamestate -> board[27].currentValue = (gamestate -> board[27].currentValue * 90) / 100;
        gamestate -> board[29].currentValue = (gamestate -> board[29].currentValue * 90) / 100;
    }

    // ...still to do (Rule-LK 35): once this card's 15 rounds expire, these squares
    // should return to their normal market-adjusted values unless another active event
    // is still influencing them. That needs a round-tick handler in game.c/finance.c
    // that remembers each square's pre-event baseline - not part of drawing the card.
}



