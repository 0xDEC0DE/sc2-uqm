#ifndef _SIS_H
#define _SIS_H

#include "planets/planets.h"

enum
{
	PM_SCAN = 0,
	PM_STARMAP,
	PM_DEVICES,
	PM_CARGO,
	PM_ROSTER,
	PM_SAVE_LOAD0,
	PM_NAVIGATE,
	
	PM_MIN_SCAN,
	PM_ENE_SCAN,
	PM_BIO_SCAN,
	PM_EXIT_MENU0,
	PM_AUTO_SCAN,
	PM_LAUNCH_LANDER,

	PM_SAVE_GAME,
	PM_LOAD_GAME,
	PM_CHANGE_SETTINGS,
	PM_EXIT_MENU1,

	PM_CONVERSE,
	PM_ATTACK,
	PM_SAVE_LOAD1,
	
	PM_FUEL,
	PM_MODULE,
	PM_SAVE_LOAD2,
	PM_EXIT_MENU2,
	
	PM_CREW,
	PM_SAVE_LOAD3,
	PM_EXIT_MENU3,
	
	PM_SOUND_ON,
	PM_SOUND_OFF,
	PM_MUSIC_ON,
	PM_MUSIC_OFF,
	PM_CYBORG_OFF,
	PM_CYBORG_NORMAL,
	PM_CYBORG_DOUBLE,
	PM_CYBORG_SUPER,
	PM_CHANGE_CAPTAIN,
	PM_CHANGE_SHIP,
	PM_EXIT_MENU4
};

#define CLEAR_SIS_RADAR (1 << 2)
#define DRAW_SIS_DISPLAY (1 << 3)

#define UNDEFINED_DELTA 0x7FFF

#define RADAR_X (4 + (SCREEN_WIDTH - STATUS_WIDTH - SAFE_X))
#define RADAR_WIDTH (STATUS_WIDTH - 8)
#define RADAR_HEIGHT 53
#define RADAR_Y (SIS_ORG_Y + SIS_SCREEN_HEIGHT - RADAR_HEIGHT)
#define NUM_RADAR_SCREENS 12
#define MAG_SHIFT 2 /* driving on planet */

#define SHIP_NAME_WIDTH 60
#define SHIP_NAME_HEIGHT 7

#define NUM_DRIVE_SLOTS 11
#define NUM_JET_SLOTS 8
#define NUM_MODULE_SLOTS 16

#define CREW_POD_CAPACITY 50
#define STORAGE_BAY_CAPACITY 500 /* km cubed */
#define FUEL_TANK_SCALE 100
#define FUEL_TANK_CAPACITY (50 * FUEL_TANK_SCALE)
#define HEFUEL_TANK_CAPACITY (100 * FUEL_TANK_SCALE)
#define MODULE_COST_SCALE 50

#define CREW_EXPENSE_THRESHOLD 1000

#define CREW_PER_ROW 5
#define SBAY_MASS_PER_ROW 50

#define MAX_FUEL_BARS 10
#define FUEL_VOLUME_PER_ROW (HEFUEL_TANK_CAPACITY / MAX_FUEL_BARS)
#define FUEL_RESERVE FUEL_VOLUME_PER_ROW

#define MAX_COMBAT_SHIPS 12
#define MAX_BATTLE_GROUPS 64

#define IP_SHIP_THRUST_INCREMENT 8
#define IP_SHIP_TURN_WAIT 17
#define IP_SHIP_TURN_DECREMENT 2

#define BOGUS_MASS 5

#define BIO_CREDIT_VALUE 2

enum
{
	PLANET_LANDER = 0,
		/* thruster types */
	FUSION_THRUSTER,
		/* jet types */
	TURNING_JETS,
		/* module types */
	CREW_POD,
	STORAGE_BAY,
	FUEL_TANK,
	HIGHEFF_FUELSYS,
	DYNAMO_UNIT,
	SHIVA_FURNACE,
	GUN_WEAPON,
	BLASTER_WEAPON,
	CANNON_WEAPON,
	TRACKING_SYSTEM,
	ANTIMISSILE_DEFENSE,
	
	NUM_PURCHASE_MODULES,

	BOMB_MODULE_0 = NUM_PURCHASE_MODULES,
	BOMB_MODULE_1,
	BOMB_MODULE_2,
	BOMB_MODULE_3,
	BOMB_MODULE_4,
	BOMB_MODULE_5,

	NUM_MODULES = 20 /* must be last entry */
};

#define EMPTY_SLOT NUM_MODULES
#define NUM_BOMB_MODULES 10

#define DRIVE_SIDE_X (31 - 13)
#define DRIVE_SIDE_Y 56
#define DRIVE_TOP_X (33 - 13)
#define DRIVE_TOP_Y (65 + 21)

#define JET_SIDE_X (71 - 13)
#define JET_SIDE_Y 48
#define JET_TOP_X (70 - 13)
#define JET_TOP_Y (73 + 21)

#define MODULE_SIDE_X (17 - 13)
#define MODULE_SIDE_Y 14
#define MODULE_TOP_X (17 - 13)
#define MODULE_TOP_Y (96 + 21)

#define SHIP_PIECE_OFFSET 12

#define MAX_BUILT_SHIPS 12
#define MAX_LANDERS 10

#define SUPPORT_SHIP_PTS \
	{3 +  0, 30 + (2 * 16)}, \
	{3 + 42, 30 + (2 * 16)}, \
	{3 +  0, 30 + (3 * 16)}, \
	{3 + 42, 30 + (3 * 16)}, \
	{3 +  0, 30 + (1 * 16)}, \
	{3 + 42, 30 + (1 * 16)}, \
	{3 +  0, 30 + (4 * 16)}, \
	{3 + 42, 30 + (4 * 16)}, \
	{3 +  0, 30 + (0 * 16)}, \
	{3 + 42, 30 + (0 * 16)}, \
	{3 +  0, 30 + (5 * 16)}, \
	{3 + 42, 30 + (5 * 16)},

#define SIS_MESSAGE_WIDTH (SIS_SCREEN_WIDTH - 69 - 2)
#define SIS_MESSAGE_HEIGHT 8
#define SIS_TITLE_WIDTH 55
#define SIS_TITLE_HEIGHT 8
#define STATUS_MESSAGE_WIDTH 60
#define STATUS_MESSAGE_HEIGHT 7

typedef struct
{
	long log_x, log_y;

	DWORD ResUnits;

	DWORD FuelOnBoard;
	COUNT CrewEnlisted;
	COUNT TotalElementMass, TotalBioMass;

	BYTE ModuleSlots[NUM_MODULE_SLOTS];
	BYTE DriveSlots[NUM_DRIVE_SLOTS];
	BYTE JetSlots[NUM_JET_SLOTS];

	BYTE NumLanders;

	COUNT ElementAmounts[NUM_ELEMENT_CATEGORIES];

	UNICODE ShipName[16];
	UNICODE CommanderName[16];
	UNICODE PlanetName[16];
} SIS_STATE;
typedef SIS_STATE *PSIS_STATE;

#define MAX_EXCLUSIVE_DEVICES 16

typedef struct
{
	SIS_STATE SS;
	BYTE Activity;
	BYTE Flags;
	BYTE day_index, month_index;
	COUNT year_index;
	BYTE MCreditLo, MCreditHi;
	BYTE NumShips, NumDevices;
	BYTE ShipList[MAX_BUILT_SHIPS];
	BYTE DeviceList[MAX_EXCLUSIVE_DEVICES];
} SUMMARY_DESC;

#define OVERRIDE_LANDER_FLAGS (1 << 7)
#define AFTER_BOMB_INSTALLED (1 << 7)

extern BOOLEAN InitSIS (void);
extern void UninitSIS (void);

extern void SeedUniverse (void);
extern BOOLEAN LoadHyperspace (void);
extern BOOLEAN FreeHyperspace (void);
extern void MoveSIS (PSIZE pdx, PSIZE pdy);
extern void RepairSISBorder (void);
extern void InitSISContexts (void);
extern void DrawSISFrame (void);
extern void ClearSISRect (BYTE ClearFlags);
extern void DrawHyperCoords (POINT puniverse);
extern void DrawSISTitle (UNICODE *pStr);
extern void DrawSISMessage (UNICODE *pStr);
extern void DrawGameDate (void);
extern void DateToString (UNICODE *buf, BYTE month_index, BYTE day_index,
		COUNT year_index);
extern void DrawStatusMessage (UNICODE *pStr);
extern void DrawLanders (void);
extern void DrawStorageBays (BOOLEAN Refresh);
extern void DeltaSISGauges (SIZE crew_delta, SIZE fuel_delta, int
		resunit_delta);
extern COUNT GetCrewCount (void);
extern COUNT GetCPodCapacity (PPOINT ppt);
extern COUNT GetLBayCapacity (PPOINT ppt);
extern COUNT GetSBayCapacity (PPOINT ppt);
extern DWORD GetFTankCapacity (PPOINT ppt);
extern COUNT CountSISPieces (BYTE piece_type);
extern void DrawMenuStateStrings (BYTE beg_index, BYTE NewState);
extern void DoMenuOptions (void);
extern void DrawFlagshipName (BOOLEAN InStatusArea);
extern void DrawCaptainsName (void);

#endif /* _SIS_H */

