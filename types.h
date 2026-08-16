#ifndef TYPES_H
#define TYPES_H

#define BOARD_SIZE                              40
#define NUM_PLAYERS                              4
#define STARTING_CASH                        30000
#define MAX_ROUNDS                             500
#define PASSING_GO_PAY                        2000
#define BAIL_AMOUNT                            300
#define MAX_NAME_LEN                            50
#define AUCTION_INCREMENT                      250
#define MAX_JAIL_TURNS                           3
#define LOAN_DURATION_ROUNDS                    20
#define INSURANCE_DURATION_ROUNDS               20
#define INSURANCE_WARNING_ROUNDS                 3
#define MAX_HOUSES                               4
#define DEPRECIATION_AGE_LIMIT                  50
#define DEPRECIATION_MAX_PERCENTAGE             30
#define MAINTENANCE_NEGLECT_LIMIT               20
#define BUILDING_DECAY_PERCENTAGE_PER_ROUND      2
#define MARKET_BOOM_COOLDOWN_ROUNDS             30
#define NATIONAL_CARD_DURATION_ROUNDS           15
#define REGIONAL_CARD_DURATION_ROUNDS           15
#define GOVERNMENT_REGULATION_DURATION_ROUNDS   20
#define INCOME_TAX_STARTING_PERCENTAGE          15
#define COMMUNITY_FUND_TAX_PERCENTAGE           10
#define GROUP_COOLDOWN_ROUNDS                   30 /* Rule-LK 33: Boom effected groups can't be 
                                                     selected for 30 rounds */

typedef enum SquareType{
    SQUARE_START,
    SQUARE_PROPERTY,
    SQUARE_RAILWAY,
    SQUARE_UTILITY,
    SQUARE_TAX,
    SQUARE_EVENT,
    SQUARE_JAIL,
    SQUARE_FREE_PARKING,
    SQUARE_GO_TO_JAIL,
    SQUARE_INSURANCE,
    SQUARE_BANK,
} SquareType;

typedef enum PropertyGroup {
    GROUP_NONE, // Square isn't a property
    GROUP_BROWN, // Group 1 (Pettah, Maradana)
    GROUP_LIGHT_BLUE, // Group 2 (Bambalapitiya, Wellawatte, Mount Lavinia)
    GROUP_PINK, // Group 3 (Maharagama, Nugegoda, Kottawa)
    GROUP_ORANGE, // Group 4 (Negombo, Katunayake, Ja-Ela)
    GROUP_RED, // Group 5 (Kandy City, Peradeniya, Katugastota)
    GROUP_YELLOW, //Group 6 (Galle Fort, nawatuna, Hikkaduwa)
    GROUP_GREEN, //Group 7 (Jaffna Town, Nallur, Trincomalee)
    GROUP_DARK_BLUE, //Group 8 (Nuwara Eliya, Galle Face)
} PropertyGroup;

typedef enum StrategyType {
    STRATEGY_AGGRESSIVE, // Aggressive Investor
    STRATEGY_CONSERVATIVE, // Conservative Banker
    STRATEGY_RISK_TAKER, // Risk Taker
    STRATEGY_OPPORTUNISTIC, // Opportunistic Trader
} StrategyType;

typedef enum InsuranceType {
    INSURANCE_NONE = -1,
    INSURANCE_BASIC,
    INSURANCE_COMPREHENSIVE,
    INSURANCE_BUSINESS_INTERRUPTION,
} InsuranceType;

/*Assumed repair cost percentages for each disaster type, 
as a percentage of the property's current value*/
typedef enum DisasterType {
    DISASTER_FIRE, // 20%
    DISASTER_FLOOD, // 15%
    DISASTER_RIOT, // 10%
    DISASTER_COLLAPSE, // 5%
    DISASTER_ELECTRICAL, // 10%
} DisasterType;

typedef struct Square {
    int squareIndex; // Position 0-39 on the board
    char name[MAX_NAME_LEN];
    SquareType type;
    PropertyGroup group; //GROUP_NONE if not a property

    // Static values
    int purchasePrice;
    int baseRent;
    int houseCost;
    int hotelCost;
    int mortgageValue;

    // Dynamic values, change during the gameplay
    int currentValue;
    int currentRent;

    // ownership
    int ownerId;
    int isMortgaged;
    int isLoanLocked;

    // Development (Rule 8,9,10)
    int numHouses;
    int hasHotel;

    // condition and age
    int buildingCondition;
    int propertyAge; /* Rounds since last purchase, used for depreciation (Rule-LK 27)
                        - properties older than 50 rounds get depreciated by 1% for 5 rounds,
                          upto 30% max*/
    int depreciationPercentage; //Capped at DEPRECIATION_MAX_PERCENTAGE
    int isDamaged;
    int closedRoundsLeft; // Political Rally closure countdown(2 max)

    // maintenance (Rule-LK 27-29)
    int roundsSinceMaintenance;
    int hasStructuralDamage;
    int maintenanceCostPercentage; /* 100 = normal, 105 = house, 108 = hotel,  
                                        150 = after structural damage*/

    // Anti-Speculation Act(Rule-LK 24)-5 rounds to develop after purchase, else development is blocked
    int developmentDeadlineRounds; // 0 = no deadline active

    // insurance
    InsuranceType insurancePolicy;
    int insuranceRoundsLeft;
} Square;

typedef struct ActiveEffect{
    int isActive;
    int effectId;
    int cardId; /* Which card/regulation this is (deck index), then used to look
                 up its name (Rule-LK 36) and to spot one specific card*/
    PropertyGroup targetGroup;
    int roundsRemaining;
} ActiveEffect;

typedef struct Player {
    char name[MAX_NAME_LEN];
    StrategyType strategy;

    int cash;
    int position;
    int lapsCompleted; // Times passed GO - Used for counting rounds (game.c)
    int inJail;
    int jailTurns;
    int isBankrupt;
    //A player may have at most 1 active loan (Rule-LK 1)
    int hasLoan;
    int loanAmount;
    int loanInterestRate;
    int loanRoundsLeft;
    int loanCollateral[BOARD_SIZE]; // Squares pledged to this loan

    ActiveEffect nationalCard; // National event card currently affecting this player
} Player;

typedef struct Economy{
    int round;
    int inflationRate;
    int baseInterestRate;

    ActiveEffect regionalCard; // Regional Development Card currently affecting the economy
    ActiveEffect governmentRegulation; //effectId = regulation index
    ActiveEffect marketBoom;
    ActiveEffect marketDecline;

    //Rule-LK 33: Avoid repeating the same group too soon for events
    PropertyGroup lastBoomGroup;
    PropertyGroup lastDeclineGroup;
    int groupCooldownRounds[9]; // index 1-8 = PropertyGroup

    // fields the various cards/regulations
    int currentIncomeTaxPercentage;   // % of cash charged on Income Tax(Rule 11)
    int communityFundTaxPercentage;   // % of property value charged on Community Development Fund
    int insurancePremiumDiscountPercentage;
    int constructionSuspendedRoundsLeft;
    int antiSpeculationActive;
} Economy;

typedef struct NationalEventCard {
    int cardId; // fixed ID, won't change with the card as the deck reorders
    char name[40];
    int cashBonus;
    int rentMultiplierPercentage;
} NationalEventCard;

typedef struct RegionalDevelopmentCard {
    int cardId;
    char name[50];
    int valuePercentageChange;
} RegionalDevelopmentCard;

typedef struct EconomicEventCard {
    int cardId;
    char name[50];
} EconomicEventCard;

typedef struct GameState{
    Square board[BOARD_SIZE];
    Player players[NUM_PLAYERS];
    Economy economy;
} GameState;

/* Prototypes for functions called across files, grouped by the file that
defines each one. */

// ---- board.c ----
void monopolyBoard(Square board[BOARD_SIZE]);
int movePlayer(Player *player, int diceTotal);
void sendToJail(Player *player);
int getRentMultiplier(Square *square);
void recalculateRentAfterConstruction(Square *square, int oldMultiplier, int newMultiplier);
void reopenClosedSquares(GameState *gamestate);

// ---- finance.c ----
int calculateMaxLoan(GameState *gamestate, int playerIndex);
void takeLoan(GameState *gamestate, int playerIndex, int amount, int collateralChoice[BOARD_SIZE]);
void repayLoan(GameState *gamestate, int playerIndex, int amount);
void applyLoanInterest(GameState *gamestate);
void getInsurance(GameState *gamestate, int playerIndex, int squareIndex, InsuranceType policy);
void payRent(GameState *gamestate, int playerIndex, int squareIndex);
int calculateRailwayRent(GameState *gamestate, int squareIndex);
int calculateUtilityRent(GameState *gamestate, int squareIndex, int diceTotal);
void collectTax(GameState *gamestate, int playerIndex);
void applyCommunityDevelopmentFundTax(GameState *gamestate, int playerIndex);
int calculateNetWorth(GameState *gamestate, int playerIndex);
void checkBankruptcy(GameState *gamestate);
void depreciateProperties(GameState *gamestate);
void decayBuildingCondition(GameState *gamestate);
void countdownInsurance(GameState *gamestate);
void calculateRecurringLuxuryTax(GameState *gamestate);
void decrementGroupCooldowns(GameState *gamestate);
void retryDamageRepairs(GameState *gamestate);
void runFinanceUpdates(GameState *gamestate);
void doMaintenance(GameState *gamestate, int playerIndex, int squareIndex);
void checkMaintenanceNeglect(GameState *gamestate);
void renovateStructuralDamage(GameState *gamestate, int playerIndex, int squareIndex);
void renovateDepreciation(GameState *gamestate, int playerIndex, int squareIndex);
void extendLoan(GameState *gamestate, int playerIndex);
void increaseLoan(GameState *gamestate, int playerIndex, int amount);
void sellProperty(GameState *gamestate, int playerIndex, int squareIndex);
void enforceDevelopmentDeadlines(GameState *gamestate);

// ---- events.c ----
void drawNationalEventCard(GameState *gamestate, int playerIndex);
void drawRegionalDevelopmentCard(GameState *gamestate);
void inflationChange(GameState *gamestate);
void propertyMarketChange(GameState *gamestate);
void governmentRegulationChange(GameState *gamestate);
void triggerDisaster(GameState *gamestate);
void drawEconomicEvent(GameState *gamestate);
void revertExpiredEffects(GameState *gamestate);
void displayMarketConditions(GameState *gamestate);

// ---- players.c ----
int monopolyPlay(GameState *gamestate, int playerIndex, PropertyGroup group);
int decideToPurchase(GameState *gamestate, int playerIndex, int squareIndex);
int decideAuctionBid(GameState *gamestate, int playerIndex, int squareIndex, int currentBid);
int decideLoanAmount(GameState *gamestate, int playerIndex);
int decideLoanRepaymentAmount(GameState *gamestate, int playerIndex);
int decideInsuranceTarget(GameState *gamestate, int playerIndex);
InsuranceType decideInsurancePolicy(GameState *gamestate, int playerIndex, int squareIndex);
int decideConstruction(GameState *gamestate, int playerIndex);
int decideMortgage(GameState *gamestate, int playerIndex);
int decidePayOffMortgage(GameState *gamestate, int playerIndex);
int decideSellProperty(GameState *gamestate, int playerIndex);
int decideToMaintain(GameState *gamestate, int playerIndex);
int decideRenovateStructuralDamage(GameState *gamestate, int playerIndex);
int decideRenovateDepreciation(GameState *gamestate, int playerIndex, int squareIndex);
int decideLoanExtension(GameState *gamestate, int playerIndex);
int decideLoanIncrease(GameState *gamestate, int playerIndex);
int decideJailOrOut(GameState *gamestate, int playerIndex);

// ---- game.c ----
void initializingPlayers(GameState *gamestate);
void determineTurnOrder(GameState *gamestate, int order[NUM_PLAYERS]);
void playTurn(GameState *gamestate, int playerIndex);
void runAuction(GameState *gamestate, int squareIndex);
void endOfRoundProcessing(GameState *gamestate);
void printRoundSummary(GameState *gamestate);
int isGameOver(GameState *gamestate);
int findWinner(GameState *gamestate);
int findSlowestLaps(GameState *gamestate);
void runGame(GameState *gamestate);

#endif