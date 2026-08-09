#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"


static NationalEventCard nationalDeck[20] = {
    { "Tourism Hype",           0, 200 }, // hotels earn double rent for 5 rounds
    { "Fuel Shortage",          0, 200 }, // railway rent doubles for 5 rounds
    { "Heavy Floods",           0,   0 }, // random coastal property damaged
    { "Political Rally",        0,   0 }, // one random property closed for 2 rounds
    { "Stock Market Rise",      0,   0 }, // all property values increase by 10%
    { "Economic Downturn",      0,   0 }, // property values decrease by 15%
    { "Housing Subsidy",        0,   0 }, // house construction cost reduced by 30%
    { "Interest Rate Cut",      0,   0 }, // loan interest reduced by 2%
    { "Interest Rate Increase", 0,   0 }, // loan interest increased by 2%
    { "Tax Amnesty",         2000,   0 }, // each player receives LKR 2,000
    { "Power Failure",          0,  50 }, // utility income halved for 3 rounds
    { "Foreign Funding",        0,   0 }, // commercial property values increase by 15%
    { "Port Expansion",         0,   0 }, // railway station values increase by 20%
    { "Festival Season",        0, 150 }, // hotels receive 50% additional rent
    { "Labour Strike",          0,   0 }, // construction suspended for 2 rounds
    { "Insurance Discount",     0,   0 }, // premiums reduced by 20%
    { "Property Revaluation",   0,   0 }, // random property group appreciates by 15%
    { "Currency Depreciation",  0,   0 }, // construction costs increase by 10%
    { "Government Grant",    5000,   0 }, // random player receives LKR 5,000
    { "National Disaster",      0,   0 }, // random developed property damaged
};

// Draws the top card (slot 0), applies whatever the NationalEventCard struct can
// represent to the player who drew it, then sends that same card to the bottom of
// the deck, per Appendix A: "the card shall be returned to the bottom of the deck."
void drawNationalEventCard(GameState *gs, int playerIndex) {

    // 1) The top card is always sitting in slot 0.
    NationalEventCard drawnCard = nationalDeck[0];
    Player *player = &gs->players[playerIndex];

    printf("Economic Event\n\n%s\n\n", drawnCard.name);

    // 2) A cash bonus (if the card has one) is credited immediately.
    if (drawnCard.cashBonus != 0) {
        player->cash = player->cash + drawnCard.cashBonus;
    }

    // 3) A rent multiplier (if the card has one) stays active on this player for the
    //    standard national card duration.
    if (drawnCard.rentMultiplierPercentage != 0) {
        player->nationalCard.isActive = 1;
        player->nationalCard.effectId = drawnCard.rentMultiplierPercentage;
        player->nationalCard.targetGroup = GROUP_NONE;
        player->nationalCard.roundsRemaining = NATIONAL_CARD_DURATION_ROUNDS;
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

