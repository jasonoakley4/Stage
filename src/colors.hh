/*
 *  Stage : a multi-robot simulator.
 *  Copyright (C) 2001, 2002 Richard Vaughan, Andrew Howard and Brian Gerkey.
 *
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/*
 * Desc: Some useful color routines.  Define the default colors for all
 * devices here.
 * Author: Andrew Howard, Richard Vaughan
 * Date: 8 Jun 2002
 * CVS info: $Id: colors.hh,v 1.3 2002-09-12 02:21:51 rtv Exp $
 */

#ifndef _STAGECOLORS_H
#define _STAGECOLORS_H 

//#include <stdint.h>
#include "stage_types.hh"

// RTV - moved the typedef to stage_types.hh in an attempt to avoid recompiling
// everything if you change this file - there's still a dependency though...

// Color type
//typedef uint32_t StageColor;

#define GRID_MAJOR_COLOR "gray85"
#define GRID_MINOR_COLOR "gray95"
#define MATRIX_COLOR "dark green"
#define BACKGROUND_COLOR "ivory"
#define GRIPPER_COLOR "blue"
#define POSITION_COLOR "dark red"
#define LASER_COLOR  "blue"
#define SONAR_COLOR  "gray"
#define PTZ_COLOR  "magenta"
#define PUCK_COLOR  "green"
#define BOX_COLOR  "yellow"
#define LBD_COLOR  "green"
#define MISC_COLOR  "gray"
#define GPS_COLOR  "gray"
#define VISION_COLOR  "gray"
#define PLAYER_COLOR  "black"
#define LASERBEACON_COLOR  "cyan"
#define VISIONBEACON_COLOR  "red"
#define WALL_COLOR "black"
#define IDAR_COLOR "DarkRed"
#define DESCARTES_COLOR "DarkBlue"

// Look up the color in a database.  (i.e. transform color name to
// color value).  If the color is not found in the database, a bright
// red color will be returned instead.
StageColor LookupColor(const char *name);

#endif