/*
 * Copyright (c) 1998 Rafal Wierzbicki <rafal@mcss.mcmaster.ca>
 * Copyright (c) 1998,2002 Sasha Vasko <sasha at aftercode.net>
 * Copyright (c) 1998 Michal Vitecek <fuf@fuf.sh.cvut.cz>
 * Copyright (c) 1998 Nat Makarevitch <nat@linux-france.com>
 * Copyright (c) 1998 Mike Venaccio <venaccio@aero.und.edu>
 * Copyright (c) 1998 Ethan Fischer <allanon@crystaltokyo.com>
 * Copyright (c) 1998 Mike Venaccio <venaccio@aero.und.edu>
 * Copyright (c) 1998 Chris Ridd <c.ridd@isode.com>
 * Copyright (c) 1997 Raphael Goulais <velephys@hol.fr>
 * Copyright (c) 1997 Guylhem Aznar <guylhem@oeil.qc.ca>
 * Copyright (C) 1996 Frank Fejes
 * Copyright (C) 1995 Bo Yang
 * Copyright (C) 1993 Robert Nation
 *
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/****************************************************************************
 *
 * Configure.c: reads the configuration files, interprets them,
 * and sets up menus, bindings, colors, and fonts as specified
 *
 ***************************************************************************/

#include "../../configure.h"

#include "../../include/asapp.h"
#include "../../include/afterstep.h"
#include "../../include/parse.h"
#include "../../include/mystyle.h"
#include "../../include/decor.h"
#include "../../include/screen.h"
#include "../../include/loadimg.h"
#include "../../include/parser.h"
#include "../../include/confdefs.h"
#include "../../include/balloon.h"
#include "../../libAfterImage/afterimage.h"
#include "../../include/mystyle_property.h"

#include "dirtree.h"
#include "asinternals.h"

/* old look auxilary variables : */
static MyFont StdFont;         /* font structure */
static MyFont WindowFont;      /* font structure for window titles */
static MyFont IconFont;        /* for icon labels */
/*
 * the old-style look variables
 */
static char         *Stdfont;
static char         *Windowfont;
static char         *Iconfont;
static char         *Menustipple;
static char         *Stdback;
static char         *Stdfore;
static char         *Stickyback;
static char         *Stickyfore;
static char         *Hiback;
static char         *Hifore;
static char         *Mtitleback;
static char         *Mtitlefore;
static char         *Menuback;
static char         *Menufore;
static char         *Menuhiback;
static char         *Menuhifore;
static char         *TexTypes = NULL;
static char         *TColor = NULL, *UColor = NULL, *SColor = NULL;
static char         *IColor = NULL, *MHColor = NULL, *MColor = NULL;
static char         *TPixmap = NULL, *UPixmap = NULL, *SPixmap = NULL;
static char         *MTPixmap = NULL, *MPixmap = NULL, *MHPixmap = NULL;
static int           TitleTextType = 0;
static int           TitleTextY = 0;
static int           IconTexType = TEXTURE_BUILTIN;
static char         *IconBgColor;
static char         *IconTexColor;
static char         *IconPixmapFile;



/* parsing handling functions for different data types : */

void          SetInts               (char *text, FILE * fd, char **arg1, int *arg2);
void          SetFlag               (char *text, FILE * fd, char **arg, int *another);
void          SetFlag2              (char *text, FILE * fd, char **arg, int *var);
void          SetBox                (char *text, FILE * fd, char **arg, int *junk);
void          SetCursor             (char *text, FILE * fd, char **arg, int *junk);
void          SetCustomCursor       (char *text, FILE * fd, char **arg, int *junk);
void          SetButtonList         (char *text, FILE * fd, char **arg1, int *arg2);
void          SetTitleText          (char *tline, FILE * fd, char **junk, int *junk2);
void          SetTitleButton        (char *tline, FILE * fd, char **junk, int *junk2);
void          SetFramePart          (char *text, FILE * fd, char **frame, int *id);

void          assign_string         (char *text, FILE * fd, char **arg, int *);
void          assign_path           (char *text, FILE * fd, char **arg, int *);
void          assign_themable_path  (char *text, FILE * fd, char **arg, int *);
void          assign_pixmap         (char *text, FILE * fd, char **arg, int *);
void          obsolete              (char *text, FILE * fd, char **arg, int *);

/* main parsing function  : */
void          match_string (struct config *table, char *text, char *error_msg, FILE * fd);

/* menu loading code : */
int           MeltStartMenu (char *buf);

/* scratch variable : */
static int dummy;

/*
 * Order is important here! if one keyword is the same as the first part of
 * another keyword, the shorter one must come first!
 */
struct config main_config[] = {
	/* base options */
    {"IconPath",   assign_path, &IconPath, (int *)0},
	{"ModulePath", assign_path, &ModulePath, (int *)0},
	{"PixmapPath", assign_themable_path, &PixmapPath, (int *)0},
	{"CursorPath", assign_path, &CursorPath, (int *)0},
    {"FontPath",   assign_path, &FontPath, (int *)0},

	/* database options */
	{"DeskTopScale", SetInts, (char **)&Scr.VScale, &dummy},
    {"DeskTopSize",  SetInts, (char **)&Scr.VxMax, &Scr.VyMax},

	/* feel options */
	{"StubbornIcons", SetFlag, (char **)StubbornIcons, (int *)0},
	{"StubbornPlacement", SetFlag, (char **)StubbornPlacement, (int *)0},
	{"StubbornIconPlacement", SetFlag, (char **)StubbornIconPlacement, (int *)0},
	{"StickyIcons", SetFlag, (char **)StickyIcons, (int *)0},
	{"IconTitle", SetFlag, (char **)IconTitle, (int *)0},
	{"KeepIconWindows", SetFlag, (char **)KeepIconWindows, (int *)0},
	{"NoPPosition", SetFlag, (char **)NoPPosition, (int *)0},
	{"CirculateSkipIcons", SetFlag, (char **)CirculateSkipIcons, (int *)0},
    {"EdgeScroll", SetInts, (char **)&Scr.Feel.EdgeScrollX, &Scr.Feel.EdgeScrollY},
	{"RandomPlacement", SetFlag, (char **)RandomPlacement, (int *)0},
	{"SmartPlacement", SetFlag, (char **)SMART_PLACEMENT, (int *)0},
	{"DontMoveOff", SetFlag, (char **)DontMoveOff, (int *)0},
	{"DecorateTransients", SetFlag, (char **)DecorateTransients, (int *)0},
	{"CenterOnCirculate", SetFlag, (char **)CenterOnCirculate, (int *)0},
    {"AutoRaise", SetInts, (char **)&Scr.Feel.AutoRaiseDelay, &dummy},
    {"ClickTime", SetInts, (char **)&Scr.Feel.ClickTime, &dummy},
    {"OpaqueMove", SetInts, (char **)&Scr.Feel.OpaqueMove, &dummy},
    {"OpaqueResize", SetInts, (char **)&Scr.Feel.OpaqueResize, &dummy},
    {"XorValue", SetInts, (char **)&Scr.Feel.XorValue, &dummy},
	{"Mouse", ParseMouseEntry, (char **)1, (int *)0},
    {"Popup", ParseMenuEntry, (char **)1, (int *)0},
    {"Function", ParseFunctionEntry, (char **)1, (int *)0},
	{"Key", ParseKeyEntry, (char **)1, (int *)0},
	{"ClickToFocus", SetFlag, (char **)ClickToFocus, (int *)EatFocusClick},
    {"ClickToRaise", SetButtonList, (char **)&Scr.Feel.RaiseButtons, (int *)0},
	{"MenusHigh", SetFlag, (char **)MenusHigh, (int *)0},
	{"SloppyFocus", SetFlag, (char **)SloppyFocus, (int *)0},
	{"Cursor", SetCursor, (char **)0, (int *)0},
	{"CustomCursor", SetCustomCursor, (char **)0, (int *)0},
    {"PagingDefault", SetFlag, (char **)DoHandlePageing, NULL},
    {"EdgeResistance", SetInts, (char **)&Scr.Feel.EdgeResistanceScroll, &Scr.Feel.EdgeResistanceMove},
	{"BackingStore", SetFlag, (char **)BackingStore, (int *)0},
	{"AppsBackingStore", SetFlag, (char **)AppsBackingStore, (int *)0},
	{"SaveUnders", SetFlag, (char **)SaveUnders, (int *)0},
    {"Xzap", SetInts, (char **)&Scr.Feel.Xzap, (int *)&dummy},
    {"Yzap", SetInts, (char **)&Scr.Feel.Yzap, (int *)&dummy},
    {"AutoReverse", SetInts, (char **)&Scr.Feel.AutoReverse, (int *)&dummy},
    {"AutoTabThroughDesks", SetFlag, (char **)AutoTabThroughDesks, NULL},
	{"MWMFunctionHints", SetFlag, (char **)MWMFunctionHints, NULL},
	{"MWMDecorHints", SetFlag, (char **)MWMDecorHints, NULL},
	{"MWMHintOverride", SetFlag, (char **)MWMHintOverride, NULL},
	{"FollowTitleChanges", SetFlag, (char **)FollowTitleChanges, (int *)0},
	/* look options */
	{"Font", assign_string, &Stdfont, (int *)0},
	{"WindowFont", assign_string, &Windowfont, (int *)0},
	{"MTitleForeColor", assign_string, &Mtitlefore, (int *)0},
	{"MTitleBackColor", assign_string, &Mtitleback, (int *)0},
	{"MenuForeColor", assign_string, &Menufore, (int *)0},
	{"MenuBackColor", assign_string, &Menuback, (int *)0},
	{"MenuHiForeColor", assign_string, &Menuhifore, (int *)0},
	{"MenuHiBackColor", assign_string, &Menuhiback, (int *)0},
	{"MenuStippleColor", assign_string, &Menustipple, (int *)0},
	{"StdForeColor", assign_string, &Stdfore, (int *)0},
	{"StdBackColor", assign_string, &Stdback, (int *)0},
	{"StickyForeColor", assign_string, &Stickyfore, (int *)0},
	{"StickyBackColor", assign_string, &Stickyback, (int *)0},
	{"HiForeColor", assign_string, &Hifore, (int *)0},
	{"HiBackColor", assign_string, &Hiback, (int *)0},
	{"IconBox", SetBox, (char **)0, (int *)0},
	{"IconFont", assign_string, &Iconfont, (int *)0},
	{"MyStyle", mystyle_parse, &PixmapPath, NULL},

#ifndef NO_TEXTURE
	{"TextureTypes", assign_string, &TexTypes, (int *)0},
    {"TextureMaxColors", obsolete, NULL, (int *)0},
	{"TitleTextureColor", assign_string, &TColor, (int *)0},	/* title */
	{"UTitleTextureColor", assign_string, &UColor, (int *)0},	/* unfoc tit */
	{"STitleTextureColor", assign_string, &SColor, (int *)0},	/* stic tit */
	{"MTitleTextureColor", assign_string, &MColor, (int *)0},	/* menu title */
	{"MenuTextureColor", assign_string, &IColor, (int *)0},	/* menu items */
	{"MenuHiTextureColor", assign_string, &MHColor, (int *)0},	/* sel items */
	{"MenuPixmap", assign_string, &MPixmap, (int *)0},	/* menu entry */
	{"MenuHiPixmap", assign_string, &MHPixmap, (int *)0},	/* hil m entr */
	{"MTitlePixmap", assign_string, &MTPixmap, (int *)0},	/* menu title */
    {"MenuPinOn", assign_pixmap, (char **)&Scr.Look.MenuPinOn, NULL},    /* menu pin */
    {"MenuPinOff", assign_pixmap, (char **)&Scr.Look.MenuPinOff, NULL},
    {"MArrowPixmap", assign_pixmap, (char **)&Scr.Look.MenuArrow, NULL},   /* menu arrow */
	{"TitlePixmap", assign_string, &TPixmap, (int *)0},	/* foc tit */
	{"UTitlePixmap", assign_string, &UPixmap, (int *)0},	/* unfoc tit */
	{"STitlePixmap", assign_string, &SPixmap, (int *)0},	/* stick tit */
    {"TexturedHandle", SetFlag2, (char **)TexturedHandle, (int *)&Scr.Look.flags},
    {"TitlebarNoPush", SetFlag2, (char **)TitlebarNoPush, (int *)&Scr.Look.flags},

	/* these are obsolete : */
    {"TextGradientColor", obsolete, (char **)NULL, (int *)0}, /* title text */
    {"GradientText", obsolete, (char **)NULL, (int *)0},

    {"ButtonTextureType", SetInts, (char **)&IconTexType, &dummy},
	{"ButtonBgColor", assign_string, &IconBgColor, (int *)0},
	{"ButtonTextureColor", assign_string, &IconTexColor, (int *)0},
    {"ButtonMaxColors", obsolete, (char **)NULL, NULL},
	{"ButtonPixmap", assign_string, &IconPixmapFile, (int *)0},
    {"ButtonNoBorder", SetFlag2, (char **)IconNoBorder, (int *)&Scr.Look.flags},
    {"TextureMenuItemsIndividually", SetFlag2, (char **)TxtrMenuItmInd,(int *)&Scr.Look.flags},
    {"MenuMiniPixmaps", SetFlag2, (char **)MenuMiniPixmaps, (int *)&Scr.Look.flags},
    {"FrameNorth", SetFramePart, NULL, (int *)FR_N},
    {"FrameSouth", SetFramePart, NULL, (int *)FR_S},
    {"FrameEast",  SetFramePart, NULL, (int *)FR_E},
    {"FrameWest",  SetFramePart, NULL, (int *)FR_W},
    {"FrameNW", SetFramePart, NULL, (int *)FR_NW},
    {"FrameNE", SetFramePart, NULL, (int *)FR_NE},
    {"FrameSW", SetFramePart, NULL, (int *)FR_SW},
    {"FrameSE", SetFramePart, NULL, (int *)FR_SE},
    {"DecorateFrames", SetFlag2, (char **)DecorateFrames, (int *)&Scr.Look.flags},
#endif /* NO_TEXTURE */
    {"TitleTextAlign", SetInts, (char **)&Scr.Look.TitleTextAlign, &dummy},
    {"TitleButtonSpacing", SetInts, (char **)&Scr.Look.TitleButtonSpacing, (int *)&dummy},
    {"TitleButtonStyle", SetInts, (char **)&Scr.Look.TitleButtonStyle, (int *)&dummy},
	{"TitleButton", SetTitleButton, (char **)1, (int *)0},
	{"TitleTextMode", SetTitleText, (char **)1, (int *)0},
    {"ResizeMoveGeometry", assign_geometry, &Scr.Look.resize_move_geometry, (int *)0},
    {"StartMenuSortMode", SetInts, (char **)&Scr.Look.StartMenuSortMode, (int *)&dummy},
    {"DrawMenuBorders", SetInts, (char **)&Scr.Look.DrawMenuBorders, (int *)&dummy},
    {"ButtonSize", SetInts, (char **)&Scr.Look.ButtonWidth, (int *)&Scr.ButtonHeight},
    {"SeparateButtonTitle", SetFlag2, (char **)SeparateButtonTitle, (int *)&Scr.Look.flags},
    {"RubberBand", SetInts, (char **)&Scr.Look.RubberBand, &dummy},
    {"DefaultStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSWindow[BACK_DEFAULT], NULL},
    {"FWindowStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSWindow[BACK_FOCUSED], NULL},
    {"UWindowStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSWindow[BACK_UNFOCUSED], NULL},
    {"SWindowStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSWindow[BACK_STICKY], NULL},
    {"MenuItemStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSMenu[MENU_BACK_ITEM], NULL},
    {"MenuTitleStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSMenu[MENU_BACK_TITLE], NULL},
    {"MenuHiliteStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSMenu[MENU_BACK_HILITE], NULL},
    {"MenuStippleStyle", mystyle_parse_set_style, (char **)&Scr.Look.MSMenu[MENU_BACK_STIPPLE], NULL},
    {"ShadeAnimationSteps", SetInts, (char **)&Scr.Look.ShadeAnimationSteps, (int *)&dummy},
	{"", 0, (char **)0, (int *)0}
};

#define PARSE_BUFFER_SIZE 	MAXLINELENGTH
char         *orig_tline = NULL;

/* the following values must not be reset other then by main
   config reading routine
 */
int           curr_conf_line = -1;
char         *curr_conf_file = NULL;

void
error_point ()
{
	fprintf (stderr, "AfterStep");
	if (curr_conf_file)
		fprintf (stderr, "(%s:%d)", curr_conf_file, curr_conf_line);
	fprintf (stderr, ":");
}

void
tline_error (const char *err_text)
{
	error_point ();
	fprintf (stderr, "%s in [%s]\n", err_text, orig_tline);
}

void
str_error (const char *err_format, const char *string)
{
	error_point ();
	fprintf (stderr, err_format, string);
}

/***************************************************************
 **************************************************************/
void
obsolete (char *text, FILE * fd, char **arg, int *i)
{
    tline_error ("This option is obsolete. ");
}

/***************************************************************
 * get an icon
 **************************************************************/
Bool
GetIconFromFile (char *file, MyIcon * icon, int max_colors)
{
    if( Scr.image_manager == NULL )
	{
		char *ppath = PixmapPath ;
		if( ppath == NULL )
			ppath = getenv( "IMAGE_PATH" );
		if( ppath == NULL )
			ppath = getenv( "PATH" );
		Scr.image_manager = create_image_manager( NULL, 2.2, ppath, getenv( "IMAGE_PATH" ), getenv( "PATH" ), NULL );
	}

    memset( icon, 0x00, sizeof(icon_t));
    return load_icon(icon, file, Scr.image_manager );
}

/*
 * Copies a string into a new, malloc'ed string
 * Strips all data before the second quote. and strips trailing spaces and
 * new lines
 */

char         *
stripcpy3 (const char *source, const Bool Warn)
{
	while ((*source != '"') && (*source != 0))
		source++;
	if (*source != 0)
		source++;
	while ((*source != '"') && (*source != 0))
		source++;
	if (*source == 0)
	{
		if (Warn)
			tline_error ("bad binding");
		return 0;
	}
	source++;
	return stripcpy (source);
}


/*
 * initialize the old-style look variables
 */
void
init_old_look_variables (Bool free_resources)
{
	if (free_resources)
	{
        /* the fonts */
		if (Stdfont != NULL)
			free (Stdfont);
		if (Windowfont != NULL)
			free (Windowfont);
		if (Iconfont != NULL)
			free (Iconfont);

		/* the colors */
		if (Stdback != NULL)
			free (Stdback);
		if (Stdfore != NULL)
			free (Stdfore);
		if (Hiback != NULL)
			free (Hiback);
		if (Hifore != NULL)
			free (Hifore);
		if (Stickyback != NULL)
			free (Stickyback);
		if (Stickyfore != NULL)
			free (Stickyfore);
		if (Mtitlefore != NULL)
			free (Mtitlefore);
		if (Mtitleback != NULL)
			free (Mtitleback);
		if (Menuback != NULL)
			free (Menuback);
		if (Menufore != NULL)
			free (Menufore);
		if (Menuhifore != NULL)
			free (Menuhifore);
		if (Menuhiback != NULL)
			free (Menuhiback);
		if (Menustipple != NULL)
			free (Menustipple);

#ifndef NO_TEXTURE
		if (UColor != NULL)
			free (UColor);
		if (TColor != NULL)
			free (TColor);
		if (SColor != NULL)
			free (SColor);
		if (MColor != NULL)
			free (MColor);
		if (IColor != NULL)
			free (IColor);
		if (MHColor != NULL)
			free (MHColor);

		/* the pixmaps */
		if (UPixmap != NULL)
			free (UPixmap);
		if (TPixmap != NULL)
			free (TPixmap);
		if (SPixmap != NULL)
			free (SPixmap);
		if (MTPixmap != NULL)
			free (MTPixmap);
		if (MPixmap != NULL)
			free (MPixmap);
		if (MHPixmap != NULL)
			free (MHPixmap);

		/* miscellaneous stuff */
		if (TexTypes != NULL)
			free (TexTypes);
#endif
	}

	/* the fonts */
	Stdfont = NULL;
	Windowfont = NULL;
	Iconfont = NULL;

	/* the text type */
	Scr.TitleTextType = 0;

	/* the colors */
	Stdback = NULL;
	Stdfore = NULL;
	Hiback = NULL;
	Hifore = NULL;
	Stickyback = NULL;
	Stickyfore = NULL;
	Mtitlefore = NULL;
	Mtitleback = NULL;
	Menuback = NULL;
	Menufore = NULL;
	Menuhifore = NULL;
	Menuhiback = NULL;
	Menustipple = NULL;

#ifndef NO_TEXTURE
	/* the gradients */
	UColor = NULL;
	TColor = NULL;
	SColor = NULL;
	MColor = NULL;
	IColor = NULL;
	MHColor = NULL;

	/* the pixmaps */
	UPixmap = NULL;
	TPixmap = NULL;
	SPixmap = NULL;
	MTPixmap = NULL;
	MPixmap = NULL;
	MHPixmap = NULL;

	/* miscellaneous stuff */
	TexTypes = NULL;
#endif
}


void
merge_old_look_colors (MyStyle * style, int type, char *fore, char *back,
					   char *gradient, char *pixmap)
{
	if ((fore != NULL) && !((*style).user_flags & F_FORECOLOR))
	{
		if (parse_argb_color (fore, &((*style).colors.fore)) != fore)
			(*style).user_flags |= F_FORECOLOR;
	}
	if ((back != NULL) && !((*style).user_flags & F_BACKCOLOR))
	{
		if (parse_argb_color (back, &((*style).colors.back)) != back)
		{
			(*style).relief.fore = GetHilite ((*style).colors.back);
			(*style).relief.back = GetShadow ((*style).colors.back);
			(*style).user_flags |= F_BACKCOLOR;
		}
	}
#ifndef NO_TEXTURE
    if (type >= 0)
	{
		switch (type)
		{
		 case TEXTURE_GRADIENT:
			 style->texture_type = TEXTURE_GRADIENT_TL2BR;
			 break;
		 case TEXTURE_HGRADIENT:
			 style->texture_type = TEXTURE_GRADIENT_L2R;
			 break;
		 case TEXTURE_HCGRADIENT:
			 style->texture_type = TEXTURE_GRADIENT_L2R;
			 break;
		 case TEXTURE_VGRADIENT:
			 style->texture_type = TEXTURE_GRADIENT_T2B;
			 break;
		 case TEXTURE_VCGRADIENT:
			 style->texture_type = TEXTURE_GRADIENT_T2B;
			 break;
		 default:
			 style->texture_type = type;
			 break;
		}
	}
	if ((type > 0) && (type < TEXTURE_PIXMAP) && !((*style).user_flags & F_BACKGRADIENT))
	{
		if (gradient != NULL)
		{
			ARGB32        c1, c2 = 0;
			ASGradient    grad;
			char         *ptr;

			ptr = (char *)parse_argb_color (gradient, &c1);
			parse_argb_color (ptr, &c2);
			if (ptr != gradient && (type = mystyle_parse_old_gradient (type, c1, c2, &grad)) >= 0)
			{
				if (style->user_flags & F_BACKGRADIENT)
				{
					free (style->gradient.color);
					free (style->gradient.offset);
				}
				style->gradient = grad;
				grad.type = mystyle_translate_grad_type (type);
				style->texture_type = type;
				style->user_flags |= F_BACKGRADIENT;
			} else
                show_error ("bad gradient definition in look file: %s", gradient);
		}
	} else if ((type == TEXTURE_PIXMAP) && !((*style).user_flags & F_BACKPIXMAP))
	{
		if (pixmap != NULL)
		{
			int           colors = -1;

			if ((*style).set_flags & F_MAXCOLORS)
				colors = (*style).max_colors;

			if( GetIconFromFile (pixmap, &(style->back_icon), 0) )
				(*style).user_flags |= F_BACKPIXMAP;
        }
	}
#endif
	(*style).set_flags = (*style).user_flags | (*style).inherit_flags;
}

static void
merge_old_look_font (MyStyle * style, MyFont * font)
{
	/* NOTE: these should have inherit_flags set, so the font is only
	 *       unloaded once */
	if (style != NULL && !(style->set_flags & F_FONT))
	{
		style->font = *font;
		style->inherit_flags |= F_FONT;
		style->user_flags &= ~F_FONT;		   /* to prevent confusion */
		style->set_flags = style->user_flags | style->inherit_flags;
	}
}

/*
 * merge the old variables into the new styles
 * the new styles have precedence
 */
void
merge_old_look_variables (void)
{
	MyStyle      *button_title_focus = mystyle_find ("ButtonTitleFocus");
	MyStyle      *button_title_sticky = mystyle_find ("ButtonTitleSticky");
	MyStyle      *button_title_unfocus = mystyle_find ("ButtonTitleUnfocus");

	/* the fonts */
	if (Stdfont != NULL)
	{
        if (load_font (Stdfont, &StdFont) == False)
		{
			fprintf (stderr, "%s: unable to load font %s\n", MyName, Stdfont);
			exit (1);
		}
        merge_old_look_font (Scr.MSMenuItem, &StdFont);
        merge_old_look_font (Scr.MSMenuHilite, &StdFont);
        merge_old_look_font (Scr.MSMenuStipple, &StdFont);
	}
	if (Windowfont != NULL)
	{
        if (load_font (Windowfont, &WindowFont) == False)
		{
			fprintf (stderr, "%s: unable to load font %s\n", MyName, Windowfont);
			exit (1);
		}
        merge_old_look_font (Scr.MSUWindow, &WindowFont);
        merge_old_look_font (Scr.MSFWindow, &WindowFont);
        merge_old_look_font (Scr.MSSWindow, &WindowFont);
        merge_old_look_font (Scr.MSMenuTitle, &WindowFont);
	}
	if (Iconfont != NULL)
	{
        if (load_font (Iconfont, &IconFont) == False)
		{
			fprintf (stderr, "%s: unable to load font %s\n", MyName, Iconfont);
			exit (1);
		}
        merge_old_look_font (button_title_focus, &IconFont);
        merge_old_look_font (button_title_sticky, &IconFont);
        merge_old_look_font (button_title_unfocus, &IconFont);
	}
	/* the text type */
	if (Scr.TitleTextType != 0)
	{
		if (((*Scr.MSUWindow).set_flags & F_TEXTSTYLE) == 0)
		{
			(*Scr.MSUWindow).text_style = Scr.TitleTextType;
			(*Scr.MSUWindow).user_flags |= F_TEXTSTYLE;
			(*Scr.MSUWindow).set_flags |= F_TEXTSTYLE;
		}
		if (((*Scr.MSFWindow).set_flags & F_TEXTSTYLE) == 0)
		{
			(*Scr.MSFWindow).text_style = Scr.TitleTextType;
			(*Scr.MSFWindow).user_flags |= F_TEXTSTYLE;
			(*Scr.MSFWindow).set_flags |= F_TEXTSTYLE;
		}
		if (((*Scr.MSSWindow).set_flags & F_TEXTSTYLE) == 0)
		{
			(*Scr.MSSWindow).text_style = Scr.TitleTextType;
			(*Scr.MSSWindow).user_flags |= F_TEXTSTYLE;
			(*Scr.MSSWindow).set_flags |= F_TEXTSTYLE;
		}
	}
	/* the colors */
	/* for black and white - ignore user choices */
	/* for color - accept user choices */
	if (Scr.d_depth > 1)
	{
		int           utype, ftype, stype, mttype, mhtype, mitype;	/* texture types */

		utype = ftype = stype = mttype = mhtype = mitype = -1;
#ifndef NO_TEXTURE
		if (TexTypes != NULL)
			sscanf (TexTypes, "%i %i %i %i %i %i", &ftype, &utype, &stype, &mttype, &mitype,
					&mhtype);

		if (IconTexType == TEXTURE_BUILTIN)
			IconTexType = -1;
#endif /* !NO_TEXTURE */

		/* check for missing 1.4.5.x keywords */
		if (Mtitlefore == NULL && Hifore != NULL)
			Mtitlefore = mystrdup (Hifore);
		if (Mtitleback == NULL && Hiback != NULL)
			Mtitleback = mystrdup (Hiback);
		if (Menuhifore == NULL && Hifore != NULL)
			Menuhifore = mystrdup (Hifore);
		if (Menuhiback == NULL && Menuback != NULL)
		{
			mhtype = mitype;
            Menuhiback = mystrdup (Menuback);
			if ((MHColor == NULL) && (IColor != NULL))
				MHColor = mystrdup (IColor);
			if ((MHPixmap == NULL) && (MPixmap != NULL))
				MHPixmap = mystrdup (MPixmap);
		}
        merge_old_look_colors (Scr.MSUWindow, utype, Stdfore, Stdback, UColor, UPixmap);
        merge_old_look_colors (Scr.MSFWindow, ftype, Hifore, Hiback, TColor, TPixmap);
        merge_old_look_colors (Scr.MSSWindow, stype, Stickyfore, Stickyback, SColor, SPixmap);
        merge_old_look_colors (Scr.MSMenuTitle, mttype, Mtitlefore, Mtitleback, MColor, MTPixmap);
        merge_old_look_colors (Scr.MSMenuItem, mitype, Menufore, Menuback, IColor, MPixmap);
        merge_old_look_colors (Scr.MSMenuHilite, mhtype, Menuhifore, Menuhiback, MHColor, MHPixmap);
        merge_old_look_colors (Scr.MSMenuStipple, mitype, Menustipple, Menuback, IColor,  MPixmap);

#ifndef NO_TEXTURE
		{
			MyStyle      *button_pixmap = mystyle_find ("ButtonPixmap");

			/* icon styles automagically inherit from window title styles */
			if (button_pixmap != NULL)
			{
				mystyle_merge_styles (Scr.MSFWindow, button_pixmap, 0, 0);
                merge_old_look_colors (button_pixmap, IconTexType, NULL, IconBgColor, IconTexColor, IconPixmapFile);
			}
		}
#endif /* !NO_TEXTURE */
		if (button_title_focus != NULL)
			mystyle_merge_styles (Scr.MSFWindow, button_title_focus, 0, 0);
		if (button_title_sticky != NULL)
			mystyle_merge_styles (Scr.MSSWindow, button_title_sticky, 0, 0);
		if (button_title_unfocus != NULL)
			mystyle_merge_styles (Scr.MSUWindow, button_title_unfocus, 0, 0);
	}
    init_old_look_variables (True);            /* no longer need those strings !!!! */
}

/*
 * Initialize base.#bpp variables
 */
void
InitBase (Bool free_resources)
{
	if (free_resources)
	{
		if (IconPath != NULL)
			free (IconPath);
		if (ModulePath != NULL)
			free (ModulePath);
        if (CursorPath != NULL)
            free (CursorPath);
        if (PixmapPath != NULL)
			free (PixmapPath);
        if (FontPath != NULL)
            free (FontPath);
    }

	IconPath = NULL;
	ModulePath = NULL;
	PixmapPath = NULL;
    CursorPath = NULL;
    FontPath = NULL;

    Scr.VxMax = 1;
    Scr.VyMax = 1;
    Scr.VScale = 1;
}

/*
 * Initialize look variables
 */
void
InitLook (MyLook *look, Bool free_resources)
{
	int           i;

    balloon_init (free_resources);
    if (free_resources)
	{
		/* styles/textures */
		while (mystyle_first != NULL)
			mystyle_delete (mystyle_first);

        if( Scr.DefaultFrame )
            destroy_myframe( &(Scr.DefaultFrame) );
		/* GCs */
		if (Scr.DrawGC != None)
			XFreeGC (dpy, Scr.DrawGC);

#ifndef NO_TEXTURE
		/* icons */
		if (Scr.MenuArrow.pix != None)
			free_icon_resources( Scr.MenuArrow );
		if (Scr.MenuPinOn.pix != None)
			free_icon_resources( Scr.MenuPinOn );
		if (Scr.MenuPinOff.pix != None)
			free_icon_resources( Scr.MenuPinOff );

		/* cached gradients */
		if (Scr.TitleGradient != None)
			XFreePixmap (dpy, Scr.TitleGradient);
#endif /* !NO_TEXTURE */

		/* titlebar buttons */
		for (i = 0; i < 10; i++)
		{
			free_icon_resources( Scr.buttons[i].unpressed );
			free_icon_resources( Scr.buttons[i].pressed );
		}
#ifndef NO_TEXTURE
        if( Scr.configured_icon_areas )
            free( Scr.configured_icon_areas );
        if( Scr.default_icon_box )
            destroy_asiconbox( &(Scr.default_icon_box));
        if( Scr.icon_boxes )
            destroy_ashash( &(Scr.icon_boxes));
        /* iconized window background */
		if (IconBgColor != NULL)
			free (IconBgColor);
		if (IconTexColor != NULL)
			free (IconTexColor);
		if (IconPixmapFile != NULL)
			free (IconPixmapFile);
#endif /* !NO_TEXTURE */

		/* resize/move window geometry */
		if (RMGeom != NULL)
			free (RMGeom);
        /* temporary old-style fonts : */
        unload_font (&StdFont);
        unload_font (&WindowFont);
        unload_font (&IconFont);
    }
    /* styles/textures */
	mystyle_first = NULL;
	Scr.MSDefault = NULL;
	Scr.MSFWindow = NULL;
	Scr.MSUWindow = NULL;
	Scr.MSSWindow = NULL;
	Scr.MSMenuTitle = NULL;
	Scr.MSMenuItem = NULL;
	Scr.MSMenuHilite = NULL;
	Scr.MSMenuStipple = NULL;

    Scr.DefaultFrame = create_default_myframe();

	/* GCs */
	Scr.DrawGC = None;

#ifndef NO_TEXTURE
	/* icons */
	memset(&(Scr.MenuArrow), 0x00, sizeof(MyIcon));
	memset(&(Scr.MenuPinOn), 0x00, sizeof(MyIcon));
	memset(&(Scr.MenuPinOff), 0x00, sizeof(MyIcon));

	/* cached gradients */
	Scr.TitleGradient = None;

#endif /* !NO_TEXTURE */
	Scr.TitleTextAlign = 0;

	/* titlebar buttons */
	Scr.TitleButtonSpacing = 2;
	Scr.TitleButtonStyle = 0;
	for (i = 0; i < 10; i++)
		memset(&(Scr.buttons[i]), 0x00, sizeof(MyButton));

#ifndef NO_TEXTURE
	/* iconized window background */
	IconTexType = TEXTURE_BUILTIN;
	IconBgColor = NULL;
	IconTexColor = NULL;
	IconPixmapFile = NULL;
#endif /* !NO_TEXTURE */
	Scr.ButtonWidth = 0;
	Scr.ButtonHeight = 0;

	/* resize/move window geometry */
	RMGeom = NULL;

#ifndef NO_TEXTURE
	/* frames */
    if( Scr.DefaultFrame )
        destroy_myframe( &(Scr.DefaultFrame) );
    if( Scr.FramesList )
        destroy_ashash( &(Scr.FramesList));
#endif /* !NO_TEXTURE */

	/* miscellaneous stuff */
	RubberBand = 0;
	DrawMenuBorders = 1;
	TextureMenuItemsIndividually = 1;
    Scr.look_flags = SeparateButtonTitle;
    Scr.configured_icon_areas_num = 0;
    Scr.configured_icon_areas = NULL ;
    Scr.default_icon_box = NULL ;
    Scr.icon_boxes = NULL ;

    look->StartMenuSortMode = DEFAULTSTARTMENUSORT;

    look->supported_hints = create_hints_list();
    enable_hints_support( look->supported_hints, HINTS_ICCCM );
    enable_hints_support( look->supported_hints, HINTS_Motif );
    enable_hints_support( look->supported_hints, HINTS_Gnome );
    enable_hints_support( look->supported_hints, HINTS_ExtendedWM );
    enable_hints_support( look->supported_hints, HINTS_ASDatabase );
    enable_hints_support( look->supported_hints, HINTS_GroupLead );
    enable_hints_support( look->supported_hints, HINTS_Transient );

    /* temporary old-style fonts : */
    memset(&StdFont, 0x00, sizeof(MyFont));
    memset(&WindowFont, 0x00, sizeof(MyFont));
    memset(&IconFont, 0x00, sizeof(MyFont));


}

/*
 * Initialize feel variables
 */
void
InitFeel (ASFeel *feel, Bool free_resources)
{
    if (free_resources && feel)
	{
        while (feel->MouseButtonRoot != NULL)
		{
            MouseButton  *mb = feel->MouseButtonRoot;

            feel->MouseButtonRoot = mb->NextButton;
            if (mb->fdata)
            {
                free_func_data( mb->fdata);
                free (mb->fdata);
            }
			free (mb);
		}
        while (feel->FuncKeyRoot != NULL)
		{
            FuncKey      *fk = feel->FuncKeyRoot;

            feel->FuncKeyRoot = fk->next;
			if (fk->name != NULL)
				free (fk->name);
            if (fk->fdata != NULL)
            {
                free_func_data(fk->fdata);
                free (fk->fdata);
            }
			free (fk);
		}
	}

    feel->buttons2grab = 7;
    feel->AutoReverse = 0;
    feel->Xzap = 12;
    feel->Yzap = 12;
    feel->EdgeScrollX = Scr.Feel.EdgeScrollY = -100000;
    feel->ScrollResistance = Scr.Feel.MoveResistance = 0;
    feel->OpaqueSize = 5;
    feel->OpaqueResize = 0;
    feel->ClickTime = 150;
    feel->AutoRaiseDelay = 0;
    feel->RaiseButtons = 0;
    feel->flags = DoHandlePageing;

    feel->MouseButtonRoot = NULL;
    feel->FuncKeyRoot = NULL;
}

/*
 * Initialize database variables
 */

void
InitDatabase (Bool free_resources)
{
	if (free_resources)
    {
        destroy_asdb( &Database );
        /* XResources : */
        destroy_user_database();
    }else
        Database = NULL ;
}

void
ParseDatabase (const char *file)
{
    struct name_list *list = NULL ;
	char         *realfilename;

	/* memory management for parsing buffer */
	if (file == NULL)
		return;

	realfilename = make_file_name (as_dirs.after_dir, file);
	if (CheckFile (realfilename) != 0)
	{
		free (realfilename);
		realfilename = make_file_name (as_dirs.after_sharedir, file);
		if (CheckFile (realfilename) != 0)
		{
            show_progress( "database file \"%s\" does not exists or is not readable.", realfilename );
			free (realfilename);
			return;
		}
	}
    if (realfilename)
	{
        list = ParseDatabaseOptions (realfilename, "afterstep");
		free (realfilename);
        if( list )
        {
            Database = build_asdb( list );
            if( is_output_level_under_threshold( OUTPUT_LEVEL_DATABASE ) )
                print_asdb( NULL, NULL, Database );
            while (list != NULL)
                delete_name_list (&(list));
        }else
            show_progress( "no database records loaded." );
    }else
        show_progress( "no database file available." );
    /* XResources : */
    load_user_database();
}

/*
void
InitDatabase (Bool free_resources)
{
	if (free_resources)
	{
		while (Scr.TheList != NULL)
			style_delete (Scr.TheList);
		if (Scr.DefaultIcon != NULL)
			free (Scr.DefaultIcon);
	}
	Scr.TheList = NULL;
	Scr.DefaultIcon = NULL;
}
*/
/*
 * Create/destroy window titlebar/buttons as necessary.
 */
Bool
redecorate_aswindow_iter_func(void *data, void *aux_data)
{
    ASWindow *asw = (ASWindow*)data;
    if(asw )
	{
        redecorate_window( asw, False );
        on_window_status_changed( asw, True, False );
    }
    return True;
}


void
make_styles (void)
{
/* make sure the globals are defined */
	if (Scr.MSDefault == NULL)
	{
		if ((Scr.MSDefault = mystyle_find ("default")) == NULL)
			Scr.MSDefault = mystyle_new_with_name ("default");
	}
	/* for now, the default style must be named "default" */
	else if (mystrcasecmp ((*Scr.MSDefault).name, "default"))
	{
		free ((*Scr.MSDefault).name);
		(*Scr.MSDefault).name = mystrdup ("default");
	}
	if (Scr.MSFWindow == NULL)
		Scr.MSFWindow = mystyle_new_with_name ("FWindow");
	if (Scr.MSUWindow == NULL)
		Scr.MSUWindow = mystyle_new_with_name ("UWindow");
	if (Scr.MSSWindow == NULL)
		Scr.MSSWindow = mystyle_new_with_name ("SWindow");
	if (Scr.MSMenuTitle == NULL)
		Scr.MSMenuTitle = mystyle_new_with_name ("MenuTitle");
	if (Scr.MSMenuItem == NULL)
		Scr.MSMenuItem = mystyle_new_with_name ("MenuItem");
	if (Scr.MSMenuHilite == NULL)
		Scr.MSMenuHilite = mystyle_new_with_name ("MenuHilite");
	if (Scr.MSMenuStipple == NULL)
		Scr.MSMenuStipple = mystyle_new_with_name ("MenuStipple");
	if (mystyle_find ("ButtonPixmap") == NULL)
		mystyle_new_with_name ("ButtonPixmap");
	if (mystyle_find ("ButtonTitleFocus") == NULL)
		mystyle_new_with_name ("ButtonTitleFocus");
	if (mystyle_find ("ButtonTitleSticky") == NULL)
		mystyle_new_with_name ("ButtonTitleSticky");
	if (mystyle_find ("ButtonTitleUnfocus") == NULL)
		mystyle_new_with_name ("ButtonTitleUnfocus");
}

int
ParseConfigFile (const char *file, char **tline)
{
	char         *realfilename;
	FILE         *fp = NULL;
	register char *ptr;

	/* memory management for parsing buffer */
	if (file == NULL)
		return -1;

	realfilename = make_file_name (as_dirs.after_dir, file);
	if (CheckFile (realfilename) != 0)
	{
		free (realfilename);
		realfilename = make_file_name (as_dirs.after_sharedir, file);
		if (CheckFile (realfilename) != 0)
		{
			free (realfilename);
			return -1;
		}
	}
	/* this should not happen, but still checking */
	if ((fp = fopen (realfilename, "r")) == (FILE *) NULL)
	{
        show_error("can't open config file [%s] - skipping it for now.\nMost likely you have incorrect permissions on the AfterStep configuration dir.",
             file);
        return -1;
	}
	free (realfilename);

	if (*tline == NULL)
		*tline = safemalloc (MAXLINELENGTH + 1);

	curr_conf_file = (char *)file;
	curr_conf_line = 0;
	while (fgets (*tline, MAXLINELENGTH, fp))
	{
		curr_conf_line++;
		/* prventing buffer overflow */
		*((*tline) + MAXLINELENGTH) = '\0';
		/* remove comments from the line */
		ptr = stripcomments (*tline);
		/* parsing the line */
		orig_tline = ptr;
		if (*ptr != '\0' && *ptr != '#' && *ptr != '*')
			match_string (main_config, ptr, "error in config:", fp);
	}
	curr_conf_file = NULL;
	fclose (fp);
	return 1;
}

/*****************************************************************************
 *
 * This routine is responsible for reading and parsing the config file
 *
 ****************************************************************************/
/* MakeMenus - for those who can't remember LoadASConfig's real name */
void
LoadASConfig (int thisdesktop, Bool parse_menu, Bool parse_look, Bool parse_feel)
{
	int           parse_base = 1, parse_database = 1;
	char         *tline = NULL;

	ASImageManager *old_image_manager = Scr.image_manager ;

#ifndef DIFFERENTLOOKNFEELFOREACHDESKTOP
	/* only one look & feel should be used */
	thisdesktop = 0;
#endif /* !DIFFERENTLOOKNFEELFOREACHDESKTOP */

	/* kludge: make sure functions get updated */
	if (parse_menu)
		parse_feel = True;

	/* always parse database */
	InitDatabase (True);

	/* base.* variables */
	if (parse_base || shall_override_config_file)
        InitBase (True);

	if (parse_look || shall_override_config_file)
        InitLook (True);

	if (parse_feel || shall_override_config_file)
		InitFeel (True);

	XORvalue = (((unsigned long)1) << Scr.d_depth) - 1;

	/* initialize some lists */
    Scr.Look.DefaultIcon = NULL;

	/* free pixmaps that are no longer in use */
	pixmap_ref_purge ();

	fprintf (stderr, "Detected colordepth : %d. Loading configuration ", Scr.d_depth);
	if (!shall_override_config_file)
	{
		char          configfile[64];

		CheckASTree (thisdesktop, parse_look, parse_feel);
		if (parse_base)
		{
			char * old_pixmap_path = PixmapPath ;
			PixmapPath = NULL ;
			sprintf (configfile, "%s.%dbpp", BASE_FILE, Scr.d_depth);
			ParseConfigFile (configfile, &tline);
			/* Save base filename to pass to modules */
			if (global_base_file != NULL)
				free (global_base_file);
			global_base_file = mystrdup (configfile);
			if( (old_pixmap_path == NULL && PixmapPath != NULL )||
			    (old_pixmap_path != NULL && PixmapPath == NULL )||
				(old_pixmap_path != NULL && PixmapPath != NULL && strcmp(old_pixmap_path, PixmapPath) == 0 ) )
			{
				Scr.image_manager = create_image_manager( NULL, 2.2, PixmapPath, getenv( "IMAGE_PATH" ), getenv( "PATH" ), NULL );
				if( !parse_look )
				{
					InitLook (True);
					parse_look = True ;
				}
				parse_menu = True ;
				if( !parse_feel )
				{
					parse_feel = True;
					InitFeel (True);
				}
			}
			free( old_pixmap_path );
		}
		fprintf (stderr, ".");
		if (parse_look)
		{
			Bool          done = False;

            if (Scr.screen != 0)
			{
				sprintf (configfile, LOOK_FILE ".scr%ld", thisdesktop, Scr.d_depth, Scr.screen);
				done = (ParseConfigFile (configfile, &tline) > 0);
			}
			if (!done)
			{
				sprintf (configfile, LOOK_FILE, thisdesktop, Scr.d_depth);
/*fprintf( stderr, "screen = %ld, look :[%s]\n", Scr.screen, configfile );*/
				ParseConfigFile (configfile, &tline);
			}
        }
		fprintf (stderr, ".");
		if (parse_menu)
		{
			if (tline == NULL)
				tline = safemalloc (MAXLINELENGTH + 1);
			MeltStartMenu (tline);
		}
		fprintf (stderr, ".");
		if (parse_feel)
		{
			Bool          done = False;

			if (Scr.screen != 0)
			{
				sprintf (configfile, FEEL_FILE ".scr%ld", thisdesktop, Scr.d_depth, Scr.screen);
				done = (ParseConfigFile (configfile, &tline) > 0);
			}
			if (!done)
			{
				sprintf (configfile, FEEL_FILE, thisdesktop, Scr.d_depth);
				ParseConfigFile (configfile, &tline);
			}
		}
		if (parse_feel || parse_look)
		{
			Bool          done = False;

			if (Scr.screen != 0)
			{
				sprintf (configfile, THEME_FILE ".scr%ld", thisdesktop, Scr.d_depth, Scr.screen);
				done = (ParseConfigFile (configfile, &tline) > 0);
			}
			if (!done)
			{
				sprintf (configfile, THEME_FILE, thisdesktop, Scr.d_depth);
				ParseConfigFile (configfile, &tline);
			}
			ParseConfigFile (THEME_OVERRIDE_FILE, &tline);
        }
		fprintf (stderr, ".");
		ParseConfigFile (AUTOEXEC_FILE, &tline);
		fprintf (stderr, ".");
		ParseDatabase (DATABASE_FILE);
		/* ParseConfigFile (DATABASE_FILE, &tline); */
		fprintf (stderr, ".");
	} else
	{
		Scr.image_manager = NULL ;
		/* Yes, override config file */
		ParseConfigFile (config_file_to_override, &tline);
        fprintf (stderr, "......");
	}
	free_func_hash ();

	/* let's free the memory used for parsing */
	if (tline)
		free (tline);
	fprintf (stderr, ". Done.\n");

	if (parse_base || shall_override_config_file)
	{
		Scr.VxMax = Scr.VxMax * Scr.MyDisplayWidth - Scr.MyDisplayWidth;
		Scr.VyMax = Scr.VyMax * Scr.MyDisplayHeight - Scr.MyDisplayHeight;
		if (Scr.VxMax < 0)
			Scr.VxMax = 0;
		if (Scr.VyMax < 0)
			Scr.VyMax = 0;

		if (Scr.VxMax == 0)
			Scr.flags &= ~EdgeWrapX;
		if (Scr.VyMax == 0)
			Scr.flags &= ~EdgeWrapY;
	}

	if (parse_look || shall_override_config_file)
	{
		/* make sure all needed styles are created */
		make_styles ();

		/* merge pre-1.5 compatibility keywords */
		merge_old_look_variables ();

		/* fill in remaining members with the default style */
		mystyle_fix_styles ();

		mystyle_set_property (dpy, Scr.Root, _AS_STYLE, XA_INTEGER);

#ifndef NO_TEXTURE
        /* update frame geometries */
        if (get_flags( Scr.look_flags, DecorateFrames))
        {
            if( Scr.DefaultFrame )
                myframe_load ( Scr.DefaultFrame, Scr.image_manager );
            else
                Scr.DefaultFrame = create_default_myframe();
            /* need to load the list as well (if we have any )*/
        }
#endif /* ! NO_TEXTURE */
	}

	/* update the resize/move window geometry */
	if (parse_look || shall_override_config_file)
	{
		int           invalid_RMGeom = 0;
		int           x = 0, y = 0;
		int           width, height;

		height = (*Scr.MSFWindow).font.height + SIZE_VINDENT * 2;
		Scr.SizeStringWidth = XTextWidth ((*Scr.MSFWindow).font.font, " +8888 x +8888 ", 15);
		XSetWindowBorder (dpy, Scr.SizeWindow, (*Scr.MSFWindow).colors.fore);
		XSetWindowBackground (dpy, Scr.SizeWindow, (*Scr.MSFWindow).colors.back);

		width = Scr.SizeStringWidth + SIZE_HINDENT * 2;

		if ((RMGeom != NULL) && (strlen (RMGeom) == 2))
		{
			if (RMGeom[0] == '+')
				x = 0;
			else if (RMGeom[0] == '-')
				x = DisplayWidth (dpy, Scr.screen) - width;
			else
				invalid_RMGeom = 1;

			if (RMGeom[1] == '+')
				y = 0;
			else if (RMGeom[1] == '-')
				y = DisplayHeight (dpy, Scr.screen) - height;
			else
				invalid_RMGeom = 1;
		} else
			invalid_RMGeom = 1;				   /* not necessarily invalid--maybe unspecified */

		if (invalid_RMGeom)
		{
			/* the default case is, of course, to center the R/M window */
			x = (DisplayWidth (dpy, Scr.screen) - width) / 2;
			y = (DisplayHeight (dpy, Scr.screen) - height) / 2;
		}
		XMoveResizeWindow (dpy, Scr.SizeWindow, x, y, width, height);
	}

	if (parse_feel || shall_override_config_file)
	{
		/* If no edge scroll line is provided in the setup file, use a default */
		if (Scr.EdgeScrollX == -100000)
			Scr.EdgeScrollX = 25;
		if (Scr.EdgeScrollY == -100000)
			Scr.EdgeScrollY = Scr.EdgeScrollX;

		if ((Scr.flags & ClickToRaise) && (Scr.AutoRaiseDelay == 0))
			Scr.AutoRaiseDelay = -1;

		/* if edgescroll >1000 and < 100000m
		 * wrap at edges of desktop (a "spherical" desktop) */
		if (Scr.EdgeScrollX >= 1000)
		{
			Scr.EdgeScrollX /= 1000;
			Scr.flags |= EdgeWrapX;
		}
		if (Scr.EdgeScrollY >= 1000)
		{
			Scr.EdgeScrollY /= 1000;
			Scr.flags |= EdgeWrapY;
		}
		Scr.EdgeScrollX = Scr.EdgeScrollX * Scr.MyDisplayWidth / 100;
		Scr.EdgeScrollY = Scr.EdgeScrollY * Scr.MyDisplayHeight / 100;
	}

    /* TODO: update the menus */
	if (parse_look || parse_feel || parse_menu || shall_override_config_file)
	{
    }

	/* setup the titlebar buttons */
	if (parse_look || shall_override_config_file)
	{
		balloon_setup (dpy);
		balloon_set_style (dpy, mystyle_find_or_default ("TitleButtonBalloon"));
	}

    /* force update of window frames */
	if (parse_look || parse_base || parse_database || shall_override_config_file)
        iterate_asbidirlist( Scr.Windows->clients, redecorate_aswindow_iter_func, NULL, NULL, False );

	/* redo icons in case IconBox, ButtonSize, SeparateButtonTitle, or one
	 * of the Icon definitions in database changed */
	if (parse_database || parse_look || shall_override_config_file)
	{

    }

    if( old_image_manager && old_image_manager != Scr.image_manager )
		destroy_image_manager( old_image_manager, False );
}

/*****************************************************************************
 *
 * Copies a text string from the config file to a specified location
 *
 ****************************************************************************/

void
assign_string (char *text, FILE * fd, char **arg, int *junk)
{
	*arg = stripcpy (text);
}

/*****************************************************************************
 *
 * Copies a PATH string from the config file to a specified location
 *
 ****************************************************************************/

void
assign_themable_path (char *text, FILE * fd, char **arg, int *junk)
{
	char         *as_theme_data = make_file_name (as_dirs.after_dir, THEME_DATA_DIR);
	char         *tmp = stripcpy (text);
	int           tmp_len;

	replaceEnvVar (&tmp);
	tmp_len = strlen (tmp);
	*arg = safemalloc (tmp_len + 1 + strlen (as_theme_data) + 1);
	strcpy (*arg, tmp);
	(*arg)[tmp_len] = ':';
	strcpy ((*arg) + tmp_len + 1, as_theme_data);
	free (tmp);
	free (as_theme_data);
}


void
assign_path (char *text, FILE * fd, char **arg, int *junk)
{
	*arg = stripcpy (text);
	replaceEnvVar (arg);
}

/*****************************************************************************
 * Loads a pixmap to the assigned location
 ****************************************************************************/
void
assign_pixmap (char *text, FILE * fd, char **arg, int *junk)
{
    char *fname = NULL ;
    if( parse_filename(text, &fname) != text )
    {
        GetIconFromFile (fname, (MyIcon *) arg, -1);
        free (fname);
    }
}

/****************************************************************************
 *  Read TitleText Controls
 ****************************************************************************/

void
SetTitleText (char *tline, FILE * fd, char **junk, int *junk2)
{
#ifndef NO_TEXTURE
	int           n;
	int           ttype, y;

    sscanf (tline, "%d %d", &ttype, &y);
    TitleTextType = ttype;
    TitleTextY = y;
#endif /* !NO_TEXTURE */
}

/****************************************************************************
 *
 *  Read Titlebar pixmap button
 *
 ****************************************************************************/

void
SetTitleButton (char *tline, FILE * fd, char **junk, int *junk2)
{
	int           num;
	char          file[256], file2[256];
	int           fnamelen = 0, offset = 0, linelen;
	int           n;

	if (balloon_parse (tline, fd))
		return;

	linelen = strlen (tline);
	if ((n = sscanf (tline, "%d", &num)) <= 0)
	{
		fprintf (stderr, "wrong number of parameters given with TitleButton\n");
		return;
	}
	if (num < 0 || num > 9)
	{
		fprintf (stderr, "invalid Titlebar button number: %d\n", num);
		return;
	}

	num = translate_title_button(num);

	/* going the hard way to prevent buffer overruns */
	while (isspace (*(tline + offset)) && offset < linelen)
		offset++;
	while (isdigit (*(tline + offset)) && offset < linelen)
		offset++;
	while (isspace (*(tline + offset)) && offset < linelen)
		offset++;
	for (; !isspace (*(tline + offset)) && offset < linelen; offset++)
		if (fnamelen < 254)
			file[fnamelen++] = *(tline + offset);

	file[fnamelen] = '\0';
	if (fnamelen)
	{
		while (isspace (*(tline + offset)) && offset < linelen)
			offset++;
		for (fnamelen = 0; !isspace (*(tline + offset)) && offset < linelen; offset++)
			if (fnamelen < 254)
				file2[fnamelen++] = *(tline + offset);
		file2[fnamelen] = '\0';
	}
	if (fnamelen == 0)
	{
		fprintf (stderr, "wrong number of parameters given with TitleButton\n");
		return;
	}

	GetIconFromFile (file, &(Scr.buttons[num].unpressed), 0);
	GetIconFromFile (file2, &(Scr.buttons[num].pressed), 0);

	Scr.buttons[num].width = 0 ;
	Scr.buttons[num].height = 0 ;

	if( Scr.buttons[num].unpressed.image )
	{
		Scr.buttons[num].width = Scr.buttons[num].unpressed.image->width ;
		Scr.buttons[num].height = Scr.buttons[num].unpressed.image->height ;
	}
	if( Scr.buttons[num].pressed.image )
	{
		if( Scr.buttons[num].pressed.image->width > Scr.buttons[num].width )
			Scr.buttons[num].width = Scr.buttons[num].pressed.image->width ;
		if( Scr.buttons[num].pressed.image->height > Scr.buttons[num].height )
			Scr.buttons[num].height = Scr.buttons[num].pressed.image->height ;
	}
}

/*****************************************************************************
 *
 * Changes a cursor def.
 *
 ****************************************************************************/

void
SetCursor (char *text, FILE * fd, char **arg, int *junk)
{
	int           num, cursor_num, cursor_style;

	num = sscanf (text, "%d %d", &cursor_num, &cursor_style);
	if ((num != 2) || (cursor_num >= MAX_CURSORS) || (cursor_num < 0))
		tline_error ("bad Cursor");
	else
		Scr.ASCursors[cursor_num] = XCreateFontCursor (dpy, cursor_style);
}

void
SetCustomCursor (char *text, FILE * fd, char **arg, int *junk)
{
	int           num, cursor_num;
	char          f_cursor[1024], f_mask[1024];
	Pixmap        cursor = None, mask = None;
	int           width, height, x, y;
	XColor        fore, back;
	char         *path;

	num = sscanf (text, "%d %s %s", &cursor_num, f_cursor, f_mask);
	if ((num != 3) || (cursor_num >= MAX_CURSORS) || (cursor_num < 0))
	{
		tline_error ("bad Cursor");
		return;
	}

	path = findIconFile (f_mask, CursorPath, R_OK);
	if (path)
	{
		XReadBitmapFile (dpy, Scr.Root, path, &width, &height, &mask, &x, &y);
		free (path);
	} else
	{
		tline_error ("Cursor mask not found");
		return;
	}

	path = findIconFile (f_cursor, CursorPath, R_OK);
	if (path)
	{
		XReadBitmapFile (dpy, Scr.Root, path, &width, &height, &cursor, &x, &y);
		free (path);
	} else
	{
		tline_error ("cursor bitmap not found");
		return;
	}

	fore.pixel = Scr.asv->black_pixel;
	back.pixel = Scr.asv->white_pixel;
	XQueryColor (dpy, Scr.asv->colormap, &fore);
	XQueryColor (dpy, Scr.asv->colormap, &back);

	if (cursor == None || mask == None)
	{
		tline_error ("unrecognized format for cursor");
		return;
	}
	Scr.ASCursors[cursor_num] = XCreatePixmapCursor (dpy, cursor, mask, &fore, &back, x, y);
	XFreePixmap (dpy, mask);
	XFreePixmap (dpy, cursor);
}

/*****************************************************************************
 *
 * Sets a boolean flag to true
 *
 ****************************************************************************/

void
SetFlag (char *text, FILE * fd, char **arg, int *another)
{
    Scr.Feel.flags |= (unsigned long)arg;
	if (another)
	{
		long          i = strtol (text, NULL, 0);
		if (i)
            Scr.Feel.flags |= (unsigned long)another;
	}
}

void
SetFlag2 (char *text, FILE * fd, char **arg, int *var)
{
	unsigned long *flags = (unsigned long *)var;
	char         *ptr;
	int           val = strtol (text, &ptr, 0);

	if (flags == NULL)
        flags = &Scr.Feel.flags;
	if (ptr != text && val == 0)
		*flags &= ~(unsigned long)arg;
	else
		*flags |= (unsigned long)arg;
}

/*****************************************************************************
 *
 * Reads in one or two integer values
 *
 ****************************************************************************/

void
SetInts (char *text, FILE * fd, char **arg1, int *arg2)
{
	sscanf (text, "%d%*c%d", (int *)arg1, (int *)arg2);
}

/*****************************************************************************
 *
 * Reads in a list of mouse button numbers
 *
 ****************************************************************************/

void
SetButtonList (char *text, FILE * fd, char **arg1, int *arg2)
{
	int           i, b;
	char         *next;

	for (i = 0; i < MAX_MOUSE_BUTTONS; i++)
	{
		b = (int)strtol (text, &next, 0);
		if (next == text)
			break;
		text = next;
		if (*text == ',')
			text++;
		if ((b > 0) && (b <= MAX_MOUSE_BUTTONS))
			Scr.RaiseButtons |= 1 << b;
	}
	Scr.flags |= ClickToRaise;
}


/*****************************************************************************
 *
 * Reads Dimensions for an icon box from the config file
 *
 ****************************************************************************/

void
SetBox (char *text, FILE * fd, char **arg, int *junk)
{
    int x1 = 0, y1 = 0, x2 = Scr.MyDisplayWidth, y2 = Scr.MyDisplayHeight ;
    int num;

    /* not a standard X11 geometry string :*/
	num = sscanf (text, "%d%d%d%d", &x1, &y1, &x2, &y2);

	/* check for negative locations */
	if (x1 < 0)
		x1 += Scr.MyDisplayWidth;
	if (y1 < 0)
		y1 += Scr.MyDisplayHeight;

	if (x2 < 0)
		x2 += Scr.MyDisplayWidth;
	if (y2 < 0)
		y2 += Scr.MyDisplayHeight;

    if (x1 >= x2 || y1 >= y2 ||
		x1 < 0 || x1 > Scr.MyDisplayWidth || x2 < 0 || x2 > Scr.MyDisplayWidth ||
		y1 < 0 || y1 > Scr.MyDisplayHeight || y2 < 0 || y2 > Scr.MyDisplayHeight)
    {
        show_error("invalid IconBox '%s'", text);
    }else
	{
        int box_no = Scr.configured_icon_areas_num;
        Scr.configured_icon_areas = realloc( Scr.configured_icon_areas, (box_no+1)*sizeof(ASGeometry));
        Scr.configured_icon_areas[box_no].x = x1 ;
        Scr.configured_icon_areas[box_no].y = y1 ;
        Scr.configured_icon_areas[box_no].width = x2-x1 ;
        Scr.configured_icon_areas[box_no].height = y2-y1 ;
        Scr.configured_icon_areas[box_no].flags = XValue|YValue|WidthValue|HeightValue ;
        if( x1 > Scr.MyDisplayWidth-x2 )
            Scr.configured_icon_areas[box_no].flags |= XNegative ;
        if( y1 > Scr.MyDisplayHeight-y2 )
            Scr.configured_icon_areas[box_no].flags |= YNegative ;
        ++Scr.configured_icon_areas_num;
    }
}

void
SetFramePart (char *text, FILE * fd, char **frame, int *id)
{
    char *fname = NULL;
    MyFrame *pframe  = (MyFrame*)frame;
    if( parse_filename (text, &fname) != text )
    {
        if( pframe == NULL )
        {
            if( Scr.DefaultFrame == NULL )
                Scr.DefaultFrame = create_myframe();
            pframe = Scr.DefaultFrame;
        }
        filename2myframe_part (pframe, (int)id, fname);
        free( fname );
    }
}

/****************************************************************************
 *
 * These routines put together files from start directory
 *
 ***************************************************************************/

/* buf must be at least MAXLINELENGTH chars long */
int
MeltStartMenu (char *buf)
{
	char         *as_start = NULL;
	dirtree_t    *tree;

	switch (StartMenuSortMode)
	{
	 case SORTBYALPHA:
		 dirtree_compar_list[0] = dirtree_compar_order;
		 dirtree_compar_list[1] = dirtree_compar_type;
		 dirtree_compar_list[2] = dirtree_compar_alpha;
		 dirtree_compar_list[3] = NULL;
		 break;

	 case SORTBYDATE:
		 dirtree_compar_list[0] = dirtree_compar_order;
		 dirtree_compar_list[1] = dirtree_compar_type;
		 dirtree_compar_list[2] = dirtree_compar_mtime;
		 dirtree_compar_list[3] = NULL;
		 break;

	 default:
		 dirtree_compar_list[0] = NULL;
		 break;
	}

	/*
	 *    Here we test the existence of various
	 *    directories used for the generation.
	 */

	if (CheckDir (as_dirs.after_dir) == 0)
	{
		as_start = make_file_name (as_dirs.after_dir, START_DIR);
		if (CheckDir (as_start) != 0)
		{
			free (as_start);
			as_start = NULL;
		}
	}
	if (as_start == NULL)
	{
		printf ("Using system wide defaults from '%s'", as_dirs.after_sharedir);
		as_start = make_file_name (as_dirs.after_sharedir, START_DIR);
		if (CheckDir (as_start) != 0)
		{
			free (as_start);
			perror ("unable to locate the menu directory");
			Done (0, NULL);
			return 0;
		}
	}
	tree = dirtree_new_from_dir (as_start);
	free (as_start);

#ifdef FIXED_DIR
	{
		char         *as_fixeddir = make_file_name (as_dirs.after_sharedir, FIXED_DIR);

		if (CheckDir (as_fixeddir) == 0)
		{
			dirtree_t    *fixed_tree = dirtree_new_from_dir (as_fixeddir);

			free (as_fixeddir);
			dirtree_move_children (tree, fixed_tree);
			dirtree_delete (fixed_tree);
		} else
			perror ("unable to locate the fixed menu directory");
		free (as_fixeddir);
	}
#endif /* FIXED_DIR */

	dirtree_parse_include (tree);
	dirtree_remove_order (tree);
	dirtree_merge (tree);
	dirtree_sort (tree);
	dirtree_set_id (tree, 0);
	/* make sure one copy of the root menu uses the name "0" */
	(*tree).flags &= ~DIRTREE_KEEPNAME;

	dirtree_make_menu2 (tree, buf);
	/* to keep backward compatibility, make a copy of the root menu with
	 * the name "start" */
	{
		if ((*tree).name != NULL)
			free ((*tree).name);
		(*tree).name = mystrdup ("start");
		(*tree).flags |= DIRTREE_KEEPNAME;
		dirtree_make_menu2 (tree, buf);
	}
	/* cleaning up cache of the searcher */
	is_executable_in_path (NULL);

	dirtree_delete (tree);
	return 0;
}

/****************************************************************************
 *
 * Matches text from config to a table of strings, calls routine
 * indicated in table.
 *
 ****************************************************************************/

void
match_string (struct config *table, char *text, char *error_msg, FILE * fd)
{
    register int i ;
    table = find_config (table, text);
	if (table != NULL)
	{
        i = strlen (table->keyword);
        while(isspace(text[i])) ++i;
        table->action (&(text[i]), fd, table->arg, table->arg2);
	} else
		tline_error (error_msg);
}


