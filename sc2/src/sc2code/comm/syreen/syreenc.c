//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "reslib.h"
#include "comm/syreen/resinst.h"
#include "starcon.h"
#include "commglue.h"
#include "comm/syreen/strings.h"

static LOCDATA syreen_desc =
{
	NULL_PTR, /* init_encounter_func */
	NULL_PTR, /* uninit_encounter_func */
	(FRAME)SYREEN_PMAP_ANIM, /* AlienFrame */
	(FONT)SYREEN_FONT, /* AlienFont */
	WHITE_COLOR, /* AlienTextFColor */
	BLACK_COLOR, /* AlienTextBColor */
	0, /* SIS_TEXT_WIDTH - 16, */ /* AlienTextWidth */
	(COLORMAP)SYREEN_COLOR_MAP, /* AlienColorMap */
	SYREEN_MUSIC, /* AlienSong */
	SYREEN_CONVERSATION_PHRASES, /* PlayerPhrases */
	15, /* NumAnimations */
	{
		{
			5, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			7, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			9, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			11, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			13, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			15, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			(1 << 12), /* BlockMask */
		},
		{
			17, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			19, /* StartIndex */
			2, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			(1 << 13),
		},
		{
			21, /* StartIndex */
			6, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			27, /* StartIndex */
			4, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND * 10, ONE_SECOND * 3, /* RestartRate */
			(1 << 14), /* BlockMask */
		},
		{
			31, /* StartIndex */
			6, /* NumFrames */
			CIRCULAR_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
		},
		{
			37, /* StartIndex */
			4, /* NumFrames */
			RANDOM_ANIM, /* AnimFlags */
			8, 8, /* FrameRate */
			8, 8, /* RestartRate */
		},
		{
			41, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			12, 8, /* FrameRate */
			ONE_SECOND * 10, ONE_SECOND * 3, /* RestartRate */
			(1 << 5), /* BlockMask */
		},
		{
			44, /* StartIndex */
			4, /* NumFrames */
			YOYO_ANIM
					| WAIT_TALKING, /* AnimFlags */
			20, 0, /* FrameRate */
			ONE_SECOND * 3, ONE_SECOND, /* RestartRate */
			(1 << 7) | (1 << 14), /* BlockMask */
		},
		{
			48, /* StartIndex */
			3, /* NumFrames */
			YOYO_ANIM
					| WAIT_TALKING, /* AnimFlags */
			16, 8, /* FrameRate */
			ONE_SECOND * 10, ONE_SECOND,/* RestartRate */
			(1 << 9) | (1 << 13), /* BlockMask */
		},
	},
	{
		0, /* StartIndex */
		0, /* NumFrames */
		0, /* AnimFlags */
		0, 0, /* FrameRate */
		0, 0, /* RestartRate */
	},
	{
		1, /* StartIndex */
		4, /* NumFrames */
		0, /* AnimFlags */
		8, 0, /* FrameRate */
		10, 0, /* RestartRate */
	},
};

static void
FriendlyExit (RESPONSE_REF R)
{
	SET_GAME_STATE (BATTLE_SEGUE, 0);

	if (PLAYER_SAID (R, bye))
		NPCPhrase (GOODBYE);
	else if (PLAYER_SAID (R, im_on_my_way)
			|| PLAYER_SAID (R, doing_this_for_you)
			|| PLAYER_SAID (R, if_i_die))
		NPCPhrase (GOOD_LUCK);
	else if (PLAYER_SAID (R, bye_before_vault))
		NPCPhrase (GOODBYE_BEFORE_VAULT);
	else if (PLAYER_SAID (R, bye_after_vault))
		NPCPhrase (GOODBYE_AFTER_VAULT);
	else if (PLAYER_SAID (R, bye_before_ambush))
		NPCPhrase (GOODBYE_BEFORE_AMBUSH);
	else if (PLAYER_SAID (R, bye_after_ambush))
		NPCPhrase (GOODBYE_AFTER_AMBUSH);
	else
	{
		if (PLAYER_SAID (R, hands_off))
		{
			NPCPhrase (OK_WONT_USE_HANDS0);
			NPCPhrase (OK_WONT_USE_HANDS1);
			NPCPhrase (OK_WONT_USE_HANDS2);
			NPCPhrase (OK_WONT_USE_HANDS3);

			syreen_desc.AlienTextTemplate.align = ALIGN_RIGHT;
			syreen_desc.AlienTextTemplate.baseline.x = SIS_SCREEN_WIDTH - TEXT_X_OFFS;
			AlienTalkSegue (1);

			syreen_desc.AlienTextTemplate.baseline.x = SIS_TEXT_WIDTH * 3 / 4;
			syreen_desc.AlienTextTemplate.baseline.y = 120 * 3 / 4;
			AlienTalkSegue (2);

			syreen_desc.AlienTextTemplate.baseline.x = SIS_TEXT_WIDTH * 2 / 4;
			syreen_desc.AlienTextTemplate.baseline.y = 120 * 2 / 4;
			AlienTalkSegue (3);

			syreen_desc.AlienTextTemplate.baseline.x = SIS_TEXT_WIDTH * 1 / 4;
			syreen_desc.AlienTextTemplate.baseline.y = 120 * 1 / 4;
			AlienTalkSegue ((COUNT)~0);

			syreen_desc.AlienTextTemplate.baseline.x =
					TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
			syreen_desc.AlienTextTemplate.baseline.y = 0;
			syreen_desc.AlienTextTemplate.align = ALIGN_CENTER;
			syreen_desc.AlienTextWidth = SIS_TEXT_WIDTH - 16;
		}
		else if (PLAYER_SAID (R, not_much_more_to_say))
			NPCPhrase (THEN_STOP_TALKING);
		NPCPhrase (LATER);
		NPCPhrase (SEX_GOODBYE);

		AlienTalkSegue (1);
		AlienTalkSegue (2);
		XFormPLUT (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 0)
				), ONE_SECOND / 2);
		AlienTalkSegue ((COUNT)~0);

		SET_GAME_STATE (PLAYER_HAD_SEX, 1);
		SET_GAME_STATE (PLAYER_HAVING_SEX, 0);
	}
}

static void
Sex (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, in_the_spirit))
	{
		NPCPhrase (OK_SPIRIT0);
		NPCPhrase (OK_SPIRIT1);
		NPCPhrase (OK_SPIRIT2);
		NPCPhrase (OK_SPIRIT3);

		syreen_desc.AlienTextTemplate.align = ALIGN_LEFT;
		syreen_desc.AlienTextTemplate.baseline.x = TEXT_X_OFFS;
		AlienTalkSegue (1);

		syreen_desc.AlienTextTemplate.baseline.x = SIS_TEXT_WIDTH * 1 / 4;
		syreen_desc.AlienTextTemplate.baseline.y = 120 * 1 / 4;
		AlienTalkSegue (2);

		syreen_desc.AlienTextTemplate.baseline.x = SIS_TEXT_WIDTH * 2 / 4;
		syreen_desc.AlienTextTemplate.baseline.y = 120 * 2 / 4;
		AlienTalkSegue (3);

		syreen_desc.AlienTextTemplate.baseline.x = SIS_TEXT_WIDTH * 3 / 4;
		syreen_desc.AlienTextTemplate.baseline.y = 120 * 3 / 4;
		AlienTalkSegue ((COUNT)~0);

		syreen_desc.AlienTextTemplate.baseline.x =
				TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
		syreen_desc.AlienTextTemplate.baseline.y = 0;
		syreen_desc.AlienTextTemplate.align = ALIGN_CENTER;
		syreen_desc.AlienTextWidth = SIS_TEXT_WIDTH - 16;
	}
	else if (PLAYER_SAID (R, what_in_mind))
	{
		NPCPhrase (SOMETHING_LIKE_THIS0);
		NPCPhrase (SOMETHING_LIKE_THIS1);
		NPCPhrase (SOMETHING_LIKE_THIS2);
		NPCPhrase (SOMETHING_LIKE_THIS3);

		syreen_desc.AlienTextTemplate.align = ALIGN_LEFT;
		syreen_desc.AlienTextTemplate.baseline.x = TEXT_X_OFFS;
		AlienTalkSegue (1);

		syreen_desc.AlienTextTemplate.baseline.y = 120 * 3 / 4;
		AlienTalkSegue (2);

		syreen_desc.AlienTextTemplate.align = ALIGN_RIGHT;
		syreen_desc.AlienTextTemplate.baseline.x = SIS_SCREEN_WIDTH - TEXT_X_OFFS;
		AlienTalkSegue (3);

		syreen_desc.AlienTextTemplate.baseline.y = 0;
		AlienTalkSegue ((COUNT)~0);

		syreen_desc.AlienTextTemplate.baseline.x =
				TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
		syreen_desc.AlienTextTemplate.baseline.y = 0;
		syreen_desc.AlienTextTemplate.align = ALIGN_CENTER;
		syreen_desc.AlienTextWidth = SIS_TEXT_WIDTH - 16;
	}
	else if (PLAYER_SAID (R, disease))
		NPCPhrase (JUST_RELAX);
	else if (PLAYER_SAID (R, what_happens_if_i_touch_this))
	{
		NPCPhrase (THIS_HAPPENS0);
		NPCPhrase (GLOBAL_PLAYER_NAME);
		NPCPhrase (THIS_HAPPENS1);

		DISABLE_PHRASE (what_happens_if_i_touch_this);
	}
	else if (PLAYER_SAID (R, are_you_sure_this_is_ok))
	{
		NPCPhrase (YES_SURE);

		DISABLE_PHRASE (are_you_sure_this_is_ok);
	}
	else if (PLAYER_SAID (R, boy_they_never_taught))
	{
		NPCPhrase (THEN_LET_ME_TEACH);

		DISABLE_PHRASE (boy_they_never_taught);
	}

	if (!PHRASE_ENABLED (what_happens_if_i_touch_this)
			&& !PHRASE_ENABLED (are_you_sure_this_is_ok)
			&& !PHRASE_ENABLED (boy_they_never_taught))
		Response (not_much_more_to_say, FriendlyExit);
	else
	{
		if (PHRASE_ENABLED (what_happens_if_i_touch_this))
			Response (what_happens_if_i_touch_this, Sex);
		if (PHRASE_ENABLED (are_you_sure_this_is_ok))
			Response (are_you_sure_this_is_ok, Sex);
		if (PHRASE_ENABLED (boy_they_never_taught))
			Response (boy_they_never_taught, Sex);
	}
}

static void
Foreplay (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, whats_my_reward)
			|| PLAYER_SAID (R, what_about_us))
	{
		if (PLAYER_SAID (R, whats_my_reward))
			NPCPhrase (HERES_REWARD);
		else
			NPCPhrase (ABOUT_US);
		NPCPhrase (MORE_COMFORTABLE0);
		NPCPhrase (GLOBAL_PLAYER_NAME);
		NPCPhrase (MORE_COMFORTABLE1);
		AlienTalkSegue (1);
		XFormPLUT (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 1)
				), ONE_SECOND);
		AlienTalkSegue ((COUNT)~0);

		SET_GAME_STATE (PLAYER_HAVING_SEX, 1);
	}
	else if (PLAYER_SAID (R, why_lights_off))
	{
		NPCPhrase (LIGHTS_OFF_BECAUSE0);
		NPCPhrase (LIGHTS_OFF_BECAUSE1);
		NPCPhrase (LIGHTS_OFF_BECAUSE2);
		NPCPhrase (LIGHTS_OFF_BECAUSE3);

		AlienTalkSegue (1);
		syreen_desc.AlienTextWidth >>= 1;
		syreen_desc.AlienTextTemplate.baseline.x =
				TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 2);
		syreen_desc.AlienTextTemplate.baseline.y = 50;
		AlienTalkSegue (2);
		syreen_desc.AlienTextTemplate.baseline.x =
				(TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1))
				+ (SIS_TEXT_WIDTH >> 2);
		AlienTalkSegue (3);
		syreen_desc.AlienTextTemplate.baseline.x =
				TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
		syreen_desc.AlienTextTemplate.baseline.y = 90;
		syreen_desc.AlienTextWidth = SIS_TEXT_WIDTH - 16;
		AlienTalkSegue ((COUNT)~0);

		syreen_desc.AlienTextTemplate.baseline.x =
				TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
		syreen_desc.AlienTextTemplate.baseline.y = 0;

		DISABLE_PHRASE (why_lights_off);
	}
	else if (PLAYER_SAID (R, evil_monster))
	{
		NPCPhrase (NOT_EVIL_MONSTER);

		DISABLE_PHRASE (evil_monster);
	}

	if (PHRASE_ENABLED (why_lights_off))
		Response (why_lights_off, Foreplay);
	else if (PHRASE_ENABLED (evil_monster))
		Response (evil_monster, Foreplay);
	else
		Response (disease, Sex);
	Response (in_the_spirit, Sex);
	Response (what_in_mind, Sex);
	Response (hands_off, FriendlyExit);
}

static void
AfterAmbush (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, what_now_after_ambush))
	{
		NPCPhrase (DO_THIS_AFTER_AMBUSH);

		ActivateStarShip (SYREEN_SHIP, 0);
	}
	else if (PLAYER_SAID (R, what_about_you))
	{
		NPCPhrase (ABOUT_ME);

		DISABLE_PHRASE (what_about_you);
	}
	else if (PLAYER_SAID (R, whats_up_after_ambush))
	{
		BYTE NumVisits;

		NumVisits = GET_GAME_STATE (SYREEN_INFO);
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (GENERAL_INFO_AFTER_AMBUSH_1);
				break;
			case 1:
				NPCPhrase (GENERAL_INFO_AFTER_AMBUSH_2);
				break;
			case 2:
				NPCPhrase (GENERAL_INFO_AFTER_AMBUSH_3);
				break;
			case 3:
				NPCPhrase (GENERAL_INFO_AFTER_AMBUSH_4);
				--NumVisits;
				break;
		}
		SET_GAME_STATE (SYREEN_INFO, NumVisits);

		DISABLE_PHRASE (whats_up_after_ambush);
	}

	if (PHRASE_ENABLED (what_about_you))
		Response (what_about_you, AfterAmbush);
	else if (!GET_GAME_STATE (PLAYER_HAD_SEX))
	{
		Response (what_about_us, Foreplay);
	}
	if (!(ActivateStarShip (SYREEN_SHIP, CHECK_ALLIANCE) & GOOD_GUY))
		Response (what_now_after_ambush, AfterAmbush);
	if (PHRASE_ENABLED (whats_up_after_ambush))
		Response (whats_up_after_ambush, AfterAmbush);
	Response (bye_after_ambush, FriendlyExit);
}

static void
AmbushReady (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, repeat_plan))
	{
		NPCPhrase (OK_REPEAT_PLAN);

		DISABLE_PHRASE (repeat_plan);
	}

	if (PHRASE_ENABLED (repeat_plan))
		Response (repeat_plan, AmbushReady);
	Response (bye_before_ambush, FriendlyExit);
}

static void
SyreenShuttle (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, whats_next_step))
	{
		NPCPhrase (OPEN_VAULT);

		DISABLE_PHRASE (whats_next_step);
	}
	else if (PLAYER_SAID (R, what_do_i_get_for_this))
	{
		NPCPhrase (GRATITUDE);

		DISABLE_PHRASE (what_do_i_get_for_this);
	}
	else if (PLAYER_SAID (R, not_sure))
	{
		NPCPhrase (PLEASE);

		DISABLE_PHRASE (not_sure);
	}
	else if (PLAYER_SAID (R, where_is_it))
	{
		NPCPhrase (DONT_KNOW_WHERE);
		NPCPhrase (GIVE_SHUTTLE);

		SET_GAME_STATE (SYREEN_SHUTTLE, 1);
		SET_GAME_STATE (SYREEN_SHUTTLE_ON_SHIP, 1);

		DISABLE_PHRASE (where_is_it);
	}
	else if (PLAYER_SAID (R, been_there))
	{
		NPCPhrase (GREAT);
		NPCPhrase (GIVE_SHUTTLE);

		SET_GAME_STATE (SYREEN_SHUTTLE, 1);
		SET_GAME_STATE (SYREEN_SHUTTLE_ON_SHIP, 1);

		DISABLE_PHRASE (been_there);
	}

	if (PHRASE_ENABLED (whats_next_step))
		Response (whats_next_step, SyreenShuttle);
	else
	{
		if (!GET_GAME_STATE (KNOW_SYREEN_VAULT))
		{
			if (PHRASE_ENABLED (where_is_it))
				Response (where_is_it, SyreenShuttle);
		}
		else
		{
			if (PHRASE_ENABLED (been_there))
				Response (been_there, SyreenShuttle);
		}
		if (!PHRASE_ENABLED (where_is_it)
				|| !PHRASE_ENABLED (been_there))
		{
			Response (im_on_my_way, FriendlyExit);
			Response (doing_this_for_you, FriendlyExit);
			Response (if_i_die, FriendlyExit);
		}
	}
	if (PHRASE_ENABLED (what_do_i_get_for_this))
		Response (what_do_i_get_for_this, SyreenShuttle);
	if (PHRASE_ENABLED (not_sure))
		Response (not_sure, SyreenShuttle);
}

static void
NormalSyreen (RESPONSE_REF R)
{
	BYTE i, LastStack;
	RESPONSE_REF pStr[4];

	LastStack = 0;
	pStr[0] = pStr[1] = pStr[2] = pStr[3] = 0;
	if (PLAYER_SAID (R, we_here_to_help))
		NPCPhrase (NO_NEED_HELP);
	else if (PLAYER_SAID (R, we_need_help))
		NPCPhrase (CANT_GIVE_HELP);
	else if (PLAYER_SAID (R, know_about_deep_children))
	{
		NPCPhrase (WHAT_ABOUT_DEEP_CHILDREN);

		DISABLE_PHRASE (know_about_deep_children);
	}
	else if (PLAYER_SAID (R, mycons_involved))
	{
		NPCPhrase (WHAT_PROOF);

		SET_GAME_STATE (KNOW_ABOUT_SHATTERED, 3);
	}
	else if (PLAYER_SAID (R, have_no_proof))
	{
		NPCPhrase (NEED_PROOF);

		SET_GAME_STATE (SYREEN_WANT_PROOF, 1);
	}
	else if (PLAYER_SAID (R, have_proof))
	{
		NPCPhrase (SEE_PROOF);

		DISABLE_PHRASE (have_proof);
	}
	else if (PLAYER_SAID (R, what_doing_here))
	{
		NPCPhrase (OUR_NEW_WORLD);

		SET_GAME_STATE (SYREEN_STACK0, 1);
		LastStack = 1;
	}
	else if (PLAYER_SAID (R, what_about_war))
	{
		NPCPhrase (ABOUT_WAR);

		SET_GAME_STATE (SYREEN_STACK0, 2);
		LastStack = 1;
	}
	else if (PLAYER_SAID (R, help_us))
	{
		NPCPhrase (WONT_HELP);

		SET_GAME_STATE (SYREEN_STACK0, 3);
	}
	else if (PLAYER_SAID (R, what_about_history))
	{
		NPCPhrase (BEFORE_WAR);

		SET_GAME_STATE (SYREEN_STACK1, 1);
		LastStack = 2;
	}
	else if (PLAYER_SAID (R, what_about_homeworld))
	{
		NPCPhrase (ABOUT_HOMEWORLD);

		SET_GAME_STATE (SYREEN_STACK1, 2);
		LastStack = 2;
	}
	else if (PLAYER_SAID (R, what_happened))
	{
		NPCPhrase (DONT_KNOW_HOW);

		SET_GAME_STATE (KNOW_SYREEN_WORLD_SHATTERED, 1);
		SET_GAME_STATE (SYREEN_STACK1, 3);
	}
	else if (PLAYER_SAID (R, what_about_outfit))
	{
		NPCPhrase (HOPE_YOU_LIKE_IT);

		SET_GAME_STATE (SYREEN_STACK2, 1);
		LastStack = 3;
	}
	else if (PLAYER_SAID (R, where_mates))
	{
		NPCPhrase (MATES_KILLED);

		SET_GAME_STATE (SYREEN_STACK2, 2);
		LastStack = 3;
	}
	else if (PLAYER_SAID (R, get_lonely))
	{
		NPCPhrase (MAKE_OUT_ALL_RIGHT);

		SET_GAME_STATE (SYREEN_STACK2, 3);
	}
	else if (PLAYER_SAID (R, look_at_egg_sacks))
	{
		NPCPhrase (HORRIBLE_TRUTH);

		SET_GAME_STATE (BATTLE_SEGUE, 0);
		SET_GAME_STATE (SYREEN_HOME_VISITS, 0);
		SET_GAME_STATE (SYREEN_KNOW_ABOUT_MYCON, 1);

		SyreenShuttle ((RESPONSE_REF)0);
		goto ExitSyreen;
	}

	if (GET_GAME_STATE (KNOW_ABOUT_SHATTERED) < 3)
	{
		if (GET_GAME_STATE (KNOW_ABOUT_SHATTERED) == 2
				&& GET_GAME_STATE (KNOW_SYREEN_WORLD_SHATTERED))
		{
			if (PHRASE_ENABLED (know_about_deep_children))
				pStr[0] = know_about_deep_children;
			else
				pStr[0] = mycons_involved;
		}
	}
	else
	{
		if (GET_GAME_STATE (EGG_CASE0_ON_SHIP)
				|| GET_GAME_STATE (EGG_CASE1_ON_SHIP)
				|| GET_GAME_STATE (EGG_CASE2_ON_SHIP))
		{
			if (PHRASE_ENABLED (have_proof))
				pStr[0] = have_proof;
			else
				pStr[0] = look_at_egg_sacks;
		}
		else if (!GET_GAME_STATE (SYREEN_WANT_PROOF))
		{
			pStr[0] = have_no_proof;
		}
	}
	switch (GET_GAME_STATE (SYREEN_STACK0))
	{
		case 0:
			pStr[1] = what_doing_here;
			break;
		case 1:
			pStr[1] = what_about_war;
			break;
		case 2:
			pStr[1] = help_us;
			break;
	}
	switch (GET_GAME_STATE (SYREEN_STACK1))
	{
		case 0:
			pStr[2] = what_about_history;
			break;
		case 1:
			pStr[2] = what_about_homeworld;
			break;
		case 2:
			pStr[2] = what_happened;
			break;
	}
	switch (GET_GAME_STATE (SYREEN_STACK2))
	{
		case 0:
			pStr[3] = what_about_outfit;
			break;
		case 1:
			pStr[3] = where_mates;
			break;
		case 2:
			pStr[3] = get_lonely;
			break;
	}
	if (pStr[LastStack])
		Response (pStr[LastStack], NormalSyreen);
	for (i = 0; i < 4; ++i)
	{
		if (i != LastStack && pStr[i])
			Response (pStr[i], NormalSyreen);
	}
	Response (bye, FriendlyExit);
ExitSyreen:
	;
}

static void
InitialSyreen (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, we_are_vice_squad))
	{
		NPCPhrase (OK_VICE);
		NPCPhrase (HOW_CAN_YOU_BE_HERE);
	}
	else if (PLAYER_SAID (R, we_are_the_one_for_you_baby))
	{
		NPCPhrase (MAYBE_CAPTAIN);
		NPCPhrase (HOW_CAN_YOU_BE_HERE);
	}
	else if (PLAYER_SAID (R, we_are_vindicator0))
	{
		NPCPhrase (WELCOME_VINDICATOR0);
		NPCPhrase (GLOBAL_PLAYER_NAME);
		NPCPhrase (WELCOME_VINDICATOR1);
		NPCPhrase (GLOBAL_SHIP_NAME);
		NPCPhrase (WELCOME_VINDICATOR2);
		NPCPhrase (HOW_CAN_YOU_BE_HERE);
	}
	else if (PLAYER_SAID (R, we_are_impressed))
	{
		NPCPhrase (SO_AM_I_CAPTAIN);
		NPCPhrase (HOW_CAN_YOU_BE_HERE);
	}
	else if (PLAYER_SAID (R, i_need_you))
	{
		NPCPhrase (OK_NEED);

		DISABLE_PHRASE (i_need_you);
	}
	else if (PLAYER_SAID (R, i_need_touch_o_vision))
	{
		NPCPhrase (TOUCH_O_VISION);

		DISABLE_PHRASE (i_need_touch_o_vision);
	}

	Response (we_here_to_help, NormalSyreen);
	Response (we_need_help, NormalSyreen);
	if (PHRASE_ENABLED (i_need_you))
		Response (i_need_you, InitialSyreen);
	if (PHRASE_ENABLED (i_need_touch_o_vision))
		Response (i_need_touch_o_vision, InitialSyreen);
}

static void
PlanAmbush (RESPONSE_REF R)
{
	NPCPhrase (OK_FOUND_VAULT);

	SET_GAME_STATE (MYCON_AMBUSH, 1);
	SET_GAME_STATE (SYREEN_HOME_VISITS, 0);

	Response (whats_my_reward, Foreplay);
	Response (bye_after_vault, FriendlyExit);
}

static void
SyreenVault (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, vault_hint))
	{
		NPCPhrase (OK_HINT);

		DISABLE_PHRASE (vault_hint);
	}

	if (PHRASE_ENABLED (vault_hint))
	{
		Response (vault_hint, SyreenVault);
	}
	Response (bye_before_vault, FriendlyExit);
}

static void
Intro (void)
{
	BYTE NumVisits;

	if (LOBYTE (GLOBAL (CurrentActivity)) == WON_LAST_BATTLE)
	{
// NPCPhrase (OUT_TAKES);

		SET_GAME_STATE (BATTLE_SEGUE, 0);
		goto ExitIntro;
	}

	NumVisits = GET_GAME_STATE (SYREEN_HOME_VISITS);
	if (GET_GAME_STATE (MYCON_FELL_FOR_AMBUSH))
	{
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (HELLO_AFTER_AMBUSH_1);
				break;
			case 1:
				NPCPhrase (HELLO_AFTER_AMBUSH_2);
				break;
			case 2:
				NPCPhrase (HELLO_AFTER_AMBUSH_3);
				break;
			case 3:
				NPCPhrase (HELLO_AFTER_AMBUSH_3);
				--NumVisits;
				break;
		}

		AfterAmbush ((RESPONSE_REF)0);
	}
	else if (GET_GAME_STATE (MYCON_AMBUSH))
	{
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (READY_FOR_AMBUSH);
				--NumVisits;
				break;
		}

		AmbushReady ((RESPONSE_REF)0);
	}
	else if (!GET_GAME_STATE (SYREEN_KNOW_ABOUT_MYCON))
	{
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (HELLO_BEFORE_AMBUSH_1);
				break;
			case 1:
				NPCPhrase (HELLO_BEFORE_AMBUSH_2);
				break;
			case 2:
				NPCPhrase (HELLO_BEFORE_AMBUSH_3);
				break;
			case 3:
				NPCPhrase (HELLO_BEFORE_AMBUSH_4);
				--NumVisits;
				break;
		}

		if (NumVisits > 1)
			NormalSyreen ((RESPONSE_REF)0);
		else
		{
			construct_response (shared_phrase_buf,
					we_are_vindicator0,
					GLOBAL_SIS (CommanderName),
					we_are_vindicator1,
					GLOBAL_SIS (ShipName),
					we_are_vindicator2,
					0);
			Response (we_are_vice_squad, InitialSyreen);
			Response (we_are_the_one_for_you_baby, InitialSyreen);
			DoResponsePhrase (we_are_vindicator0, InitialSyreen, shared_phrase_buf);
			Response (we_are_impressed, InitialSyreen);
		}
	}
#ifdef NEVER
	else if (!GET_GAME_STATE (SYREEN_SHUTTLE))
	{
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (MUST_ACT);
				--NumVisits;
				break;
		}

		SyreenShuttle ((RESPONSE_REF)0);
	}
#endif /* NEVER */
	else if (GET_GAME_STATE (SHIP_VAULT_UNLOCKED))
	{
		PlanAmbush ((RESPONSE_REF)0);
	}
	else
	{
		switch (NumVisits++)
		{
			case 0:
				NPCPhrase (FOUND_VAULT_YET_1);
				break;
			case 1:
				NPCPhrase (FOUND_VAULT_YET_2);
				--NumVisits;
				break;
		}

		SyreenVault ((RESPONSE_REF)0);
	}
	SET_GAME_STATE (SYREEN_HOME_VISITS, NumVisits);
ExitIntro:
	;
}

static COUNT
uninit_syreen (void)
{
	return (0);
}

LOCDATAPTR
init_syreen_comm (void)
{
	LOCDATAPTR retval;

	syreen_desc.init_encounter_func = Intro;
	syreen_desc.uninit_encounter_func = uninit_syreen;

	syreen_desc.AlienTextTemplate.baseline.x =
			TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
	syreen_desc.AlienTextTemplate.baseline.y = 0;
	syreen_desc.AlienTextTemplate.align = ALIGN_CENTER;
	syreen_desc.AlienTextWidth = SIS_TEXT_WIDTH - 16;

	SET_GAME_STATE (BATTLE_SEGUE, 0);
	retval = &syreen_desc;

	return (retval);
}
