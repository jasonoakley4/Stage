#ifndef STG_H
#define STG_H
/*
 *  Stage : a multi-robot simulator.  
 * 
 *  Copyright (C) 2001-2004 Richard Vaughan, Andrew Howard and Brian
 *  Gerkey for the Player/Stage Project
 *  http://playerstage.sourceforge.net
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

/* File: stage.h
 * Desc: External header file for the Stage library
 * Authors: Richard Vaughan vaughan@sfu.ca 
 *          Andrew Howard ahowards@usc.edu
 *          Brian Gerkey gerkey@stanford.edu
 * Date: 1 June 2003
 * CVS: $Id: stage.h,v 1.126 2005-02-26 08:39:46 rtv Exp $
 */


/*! \file stage.h 
  Stage library header file

  This header file contains the external interface for the Stage
  library
*/


/** @defgroup libstage libstage
    A C library for creating robot simulations
    @{
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> // for portable int types eg. uint32_t
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include <glib.h> // we use GLib's data structures extensively
#include <rtk.h> // and graphics stuff pulled from Andrew Howard's RTK2 library

#ifdef __cplusplus
extern "C" {
#endif 

#include "config.h"
#include "replace.h"


  /** @addtogroup stg_model
      @{ */

  typedef enum
    {
      STG_MODEL_SIMULATION=0,
      STG_MODEL_BASIC,
      STG_MODEL_POSITION,
      STG_MODEL_TEST,
      STG_MODEL_LASER,
      STG_MODEL_FIDUCIAL,
      STG_MODEL_RANGER,
      STG_MODEL_BLOB,
      STG_MODEL_ENERGY,
      STG_MODEL_COUNT // this must be the last entry - it counts the entries
      
    } stg_model_type_t;
  
  
  /** any integer value other than this is a valid fiducial ID 
   */
  // TODO - fix this up
#define FiducialNone 0
  

  // Basic self-describing measurement types. All packets with real
  // measurements are specified in these terms so changing types here
  // should work throughout the code If you change these, be sure to
  // change the byte-ordering macros below accordingly.
  typedef int stg_id_t;
  typedef double stg_meters_t;
  typedef double stg_radians_t;
  typedef unsigned long stg_msec_t;
  typedef double stg_kg_t; // Kilograms (mass)
  typedef double stg_joules_t; // Joules (energy)
  typedef double stg_watts_t; // Watts (Joules per second) (energy expenditure)
  typedef int stg_bool_t;
  typedef double stg_friction_t;
  typedef uint32_t stg_color_t;
  typedef int stg_obstacle_return_t;
  typedef int stg_blob_return_t;
  typedef int stg_fiducial_return_t;
  
  /** specify a rectangular size 
   */
  typedef struct 
  {
    stg_meters_t x, y;
  } stg_size_t;
  
  /** specify a 3 axis position, in x, y and heading.
   */
  typedef struct
  {
    stg_meters_t x, y, a;
  } stg_pose_t;
  
  /** specify a 3 axis velocity in x, y and heading.
   */
  typedef stg_pose_t stg_velocity_t;  

  /** specify an object's geometry: position and rectangular size
   */
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_geom_t;

  // ENERGY --------------------------------------------------------------
  
  /** energy data packet */
  typedef struct
  {
    /** estimate of current energy stored */
    stg_joules_t stored;

    /** maximum storage capacity */
    stg_joules_t capacity;

    /** total joules received */
    stg_joules_t input_joules;

    /** total joules supplied */
    stg_joules_t output_joules;

    /** estimate of current energy output */
    stg_watts_t input_watts;

    /** estimate of current energy input */
    stg_watts_t output_watts;

    /** TRUE iff the device is receiving energy from a charger */
    stg_bool_t charging;

    /** the range to the charger, if attached, in meters */
    stg_meters_t range;
  } stg_energy_data_t;

  /** energy config packet (use this to set or get energy configuration)*/
  typedef struct
  {
    /** maximum storage capacity */
    stg_joules_t capacity;

    /** when charging another device, supply this many joules/sec at most*/
    stg_watts_t give;

    /** when charging from another device, receive this many
	joules/sec at most*/
    stg_watts_t take;

    /** length of the charging probe */
    stg_meters_t probe_range;
  } stg_energy_config_t;

  // there is currently no energy command packet

  // BLINKENLIGHT -------------------------------------------------------

  //typedef struct
  //{
  //int enable;
  //stg_msec_t period;
  //} stg_blinkenlight_t;

  // GRIPPER ------------------------------------------------------------

  // Possible Gripper return values
  //typedef enum 
  //{
  //  GripperDisabled = 0,
  //  GripperEnabled
  //} stg_gripper_return_t;

  // GUIFEATURES -------------------------------------------------------
  
  // Movement masks for figures
#define STG_MOVE_TRANS (1 << 0)
#define STG_MOVE_ROT   (1 << 1)
#define STG_MOVE_SCALE (1 << 2)
  
  typedef int stg_movemask_t;
  
  typedef struct
  {
    uint8_t show_data;
    uint8_t show_cfg;
    uint8_t show_cmd;
    
    uint8_t nose;
    uint8_t grid;
    uint8_t boundary;
    stg_movemask_t movemask;
  } stg_guifeatures_t;


  // LASER ------------------------------------------------------------

  /// laser return value
  typedef enum 
    {
      LaserTransparent, ///<not detected by laser model 
      LaserVisible, ///< detected by laser with a reflected intensity of 0 
      LaserBright  ////< detected by laser with a reflected intensity of 1 
    } stg_laser_return_t;

  /**@}*/
  

  /// returns the real (wall-clock) time in seconds
  stg_msec_t stg_timenow( void );
  
  /** initialize the stage library - optionally pass in the arguments
      to main, so Stage or rtk or gtk or xlib can read the options. 
  */
  int stg_init( int argc, char** argv );
  
  /** Get a string identifying the version of stage. The string is
      generated by autoconf 
  */
  const char* stg_get_version_string( void );
  
  /// if stage wants to quit, this will return non-zero
  int stg_quit_test( void );

  /// set stage's quit flag
  void stg_quit_request( void );

  /// report an error
  void stg_err( char* err );


  // UTILITY STUFF ----------------------------------------------------

  /** @defgroup util Utilities
      @{
  */
  
  void stg_pose_sum( stg_pose_t* result, stg_pose_t* p1, stg_pose_t* p2 );

  // ROTATED RECTANGLES -------------------------------------------------

  /** @defgroup rotrect Rotated Rectangles
      @{ 
  */
  
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
  } stg_rotrect_t; // rotated rectangle
  
  /** normalizes the set [rects] of [num] rectangles, so that they fit
      exactly in a unit square.
  */
  void stg_normalize_rects( stg_rotrect_t* rects, int num );
  
  /** load the image file [filename] and convert it to an array of
      rectangles, filling in the number of rects, width and
      height. Caller must free the rectangle array.
   */
  int stg_load_image( const char* filename, 
		      stg_rotrect_t** rects,
		      int* rect_count,
		      int* widthp, int* heightp );
  
  /**@}*/

  /** print human-readable description of a geometry struct on stdout 
   */
  void stg_print_geom( stg_geom_t* geom );
  
  /** Look up the color in the X11 database.  (i.e. transform color name to
      color value).  If the color is not found in the database, a bright
      red color (0xF00) will be returned instead.
  */
  stg_color_t stg_lookup_color(const char *name);

  // POINTS ---------------------------------------------------------

  /** @defgroup stg_point Points
      Creating and manipulating points
      @{
  */

  typedef struct
  {
    stg_meters_t x, y;
  } stg_point_t;

  /// create an array of [count] points. Caller must free the space.
  stg_point_t* stg_points_create( size_t count );

  /// create a single point structure. Caller must free the space.
  stg_point_t* stg_point_create( void );

  /// frees a point array
  void stg_points_destroy( stg_point_t* pts );

  /// frees a single point
  void stg_point_destroy( stg_point_t* pt );

  /**@}*/

  // POLYGONS ---------------------------------------------------------

  /** @defgroup stg_polygon Polygons
      Creating and manipulating polygons
      @{
  */

  typedef struct
  {
    /// pointer to an array of points
    GArray* points;

    /// if TRUE, this polygon is drawn filled
    stg_bool_t filled; 

    /// render color of this polygon - TODO  - implement color rendering
    stg_color_t color;
  } stg_polygon_t; 

  
  /// return an array of [count] polygons. Caller must free() the space.
  stg_polygon_t* stg_polygons_create( int count );
  
  /// destroy an array of [count] polygons
  void stg_polygons_destroy( stg_polygon_t* p, size_t count );
  
  /// return a single polygon structure. Caller  must free() the space.
  stg_polygon_t* stg_polygon_create( void );
  
  /** creates a unit square polygon
   */
  stg_polygon_t* unit_polygon_create( void );
  
  /** load [filename], an image format understood by gdk-pixbuf, and
      return a set of rectangles that approximate the image. Caller
      must free the array of rectangles. If width and height are
      non-null, they are filled in with the size of the image in pixels 
  */

  /// destroy a single polygon
  void stg_polygon_destroy( stg_polygon_t* p );
  
  /// Copies [count] points from [pts] into polygon [poly], allocating
  /// memory if mecessary. Any previous points in [poly] are
  /// overwritten.
  void stg_polygon_set_points( stg_polygon_t* poly, stg_point_t* pts, size_t count );			       
  /// Appends [count] points from [pts] into polygon [poly],
  /// allocating memory if mecessary.
  void stg_polygon_append_points( stg_polygon_t* poly, stg_point_t* pts, size_t count );			       

  stg_polygon_t* stg_rects_to_polygons( stg_rotrect_t* rects, size_t count );
  
  /// scale the array of [num] polygons so that all its points fit
  /// exactly in a rectagle of pwidth] by [height] units
  void stg_normalize_polygons( stg_polygon_t* polys, int num, 
			       double width, double height );
  
  /**@}*/

  // end util documentation group
  /**@}*/


  // end property typedefs -------------------------------------------------


  
  // get property structs with default values filled in --------------------
  
  /** @defgroup defaults Defaults
      @{ */
  
  void stg_get_default_pose( stg_pose_t* pose );
  void stg_get_default_geom( stg_geom_t* geom );
  
  /**@}*/

  // end defaults --------------------------------------------
  

  // forward declare struct types
  struct _stg_world; 
  struct _stg_model;
  struct _stg_matrix;
  struct _gui_window;

  /** @addtogroup stg_model
      @{ */

  /** opaque data structure implementing a model
   */
  typedef struct _stg_model stg_model_t;

  /**@}*/

  //  WORLD --------------------------------------------------------

  /** @defgroup stg_world Worlds
     Implements a world - a collection of models and a matrix.   
     @{
  */
  
  /** opaque data structure implementing a world
   */
  typedef struct _stg_world stg_world_t;
  
  /** Create a new world, to be configured and populated by user code
   */
  stg_world_t* stg_world_create( stg_id_t id, 
				 const char* token, 
				 int sim_interval, 
				 int real_interval,
				 double ppm_high,
				 double ppm_med,
				 double ppm_low );
  
  /** Create a new world as described in the worldfile [worldfile_path] 
   */
  stg_world_t* stg_world_create_from_file( char* worldfile_path );

  /** Destroy a world and everything it contains
   */
  void stg_world_destroy( stg_world_t* world );
  
  /** Run one simulation step. Returns 0 if all is well, or a positive error code 
   */
  int stg_world_update( stg_world_t* world, int sleepflag );

  /** configure the world by reading from the current world file */
  void stg_world_load( stg_world_t* mod );

  /** save the state of the world to the current world file */
  void stg_world_save( stg_world_t* mod );

  /** print human-readable information about the world on stdout 
   */
  void stg_world_print( stg_world_t* world );
    
  /// get a model pointer from its ID
  stg_model_t* stg_world_get_model( stg_world_t* world, stg_id_t mid );
  
  /// get a model pointer from its name
  stg_model_t* stg_world_model_name_lookup( stg_world_t* world, const char* name );
  
  /**@}*/



  //  MODEL --------------------------------------------------------
    
  // group the docs of all the model types
  /** @defgroup stg_models Models
      @{ */
  
  /** @defgroup stg_model Basic model
      Implements the basic object
      @{ */

  
  /// create a new model
  stg_model_t* stg_model_create(  stg_world_t* world,
				  stg_model_t* parent, 
				  stg_id_t id, 
				  stg_model_type_t type,
				  char* token );

  /// create a new model with [extra_len] bytes on the end of the struct.
  stg_model_t* stg_model_create_extended( stg_world_t* world, 
					  stg_model_t* parent,
					  stg_id_t id, 
					  stg_model_type_t type,
					  char* token,
					  size_t extra_len );
    
  /** destroy a model, freeing its memory */
  void stg_model_destroy( stg_model_t* mod );

  /** get the pose of a model in the global CS */
  void stg_model_get_global_pose( stg_model_t* mod, stg_pose_t* pose );

  /** get the velocity of a model in the global CS */
  void stg_model_get_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /* set the velocity of a model in the global coordinate system */
  //void stg_model_set_global_velocity( stg_model_t* mod, stg_velocity_t* gvel );

  /** subscribe to a model's data */
  void stg_model_subscribe( stg_model_t* mod );

  /** unsubscribe from a model's data */
  void stg_model_unsubscribe( stg_model_t* mod );

  /** configure a model by reading from the current world file */
  void stg_model_load( stg_model_t* mod );

  /** save the state of the model to the current world file */
  void stg_model_save( stg_model_t* mod );

  /** get a human-readable string for the model's type */
  const char* stg_model_type_string( stg_model_type_t type );

  // SET properties - use these to set props, don't set them directly

  /** set the pose of a model in its parent's coordinate system */
  int stg_model_set_pose( stg_model_t* mod, stg_pose_t* pose );
  
  /** set the pose of model in global coordinates */
  int stg_model_set_global_pose( stg_model_t* mod, stg_pose_t* gpose );
  
  /** set a model's odometry */
  //int stg_model_set_odom( stg_model_t* mod, stg_pose_t* pose );

  /** set a model's velocity in it's parent's coordinate system */
  int stg_model_set_velocity( stg_model_t* mod, stg_velocity_t* vel );
  
  /** set a model's size */
  int stg_model_set_size( stg_model_t* mod, stg_size_t* sz );

  /** set a model's  */
  int stg_model_set_color( stg_model_t* mod, stg_color_t* col );

  /** set a model's geometry */
  int stg_model_set_geom( stg_model_t* mod, stg_geom_t* geom );

  /** set a model's mass */
  int stg_model_set_mass( stg_model_t* mod, stg_kg_t* mass );

  /** set a model's GUI features */
  int stg_model_set_guifeatures( stg_model_t* mod, stg_guifeatures_t* gf );

  // TODO
  /* set a model's energy configuration */
  // int stg_model_set_energy_config( stg_model_t* mod, stg_energy_config_t* gf );

  /* set a model's energy data*/
  // int stg_model_set_energy_data( stg_model_t* mod, stg_energy_data_t* gf );

  /** set a model's polygon array*/
  int stg_model_set_polygons( stg_model_t* mod, 
			      stg_polygon_t* polys, size_t poly_count );
  
  /** get a model's polygon array */
  stg_polygon_t* stg_model_get_polygons( stg_model_t* mod, size_t* count);
  
  /** set a model's obstacle return value */
  int stg_model_set_obstaclereturn( stg_model_t* mod, stg_obstacle_return_t* ret );

  /** set a model's laser return value */
  int stg_model_set_laserreturn( stg_model_t* mod, stg_laser_return_t* val );

  /** set a model's fiducial return value */
  int stg_model_set_fiducialreturn( stg_model_t* mod, stg_fiducial_return_t* val );

  /** set a model's friction*/
  // int stg_model_set_friction( stg_model_t* mod, stg_friction_t* fricp );

  // GET properties - use these to get props - don't get them directly

  // todo - make all of these copy data into a buffer!
  void stg_model_get_velocity( stg_model_t* mod, stg_velocity_t* dest );
  void stg_model_get_geom( stg_model_t* mod, stg_geom_t* dest );
  void stg_model_get_color( stg_model_t* mod, stg_color_t* dest );
  void stg_model_get_pose( stg_model_t* mod, stg_pose_t* dest );
  void stg_model_get_mass( stg_model_t* mod, stg_kg_t* dest );
  void stg_model_get_guifeatures( stg_model_t* mod, stg_guifeatures_t* dest );
  void stg_model_get_obstaclereturn( stg_model_t* mod, stg_obstacle_return_t* dest  );
  void stg_model_get_laserreturn( stg_model_t* mod, stg_laser_return_t* dest );
  void stg_model_get_fiducialreturn( stg_model_t* mod,stg_fiducial_return_t* dest );
  //void stg_model_get_friction( stg_model_t* mod,stg_friction_t* dest );

  //stg_energy_data_t*     stg_model_get_energy_data( stg_model_t* mod );
  //stg_energy_config_t*   stg_model_get_energy_config( stg_model_t* mod );

  // wrappers for polymorphic functions
  int stg_model_set_command( stg_model_t* mod, void* cmd, size_t len );
  int stg_model_set_data( stg_model_t* mod, void* data, size_t len );
  int stg_model_set_config( stg_model_t* mod, void* cmd, size_t len );
  int stg_model_get_command( stg_model_t* mod, void* dest, size_t len );
  int stg_model_get_data( stg_model_t* mod, void* dest, size_t len );
  int stg_model_get_config( stg_model_t* mod, void* dest, size_t len );
  
  /// associate an arbitrary data item with this model, referenced by the string 'name'.
  void  stg_model_set_prop( stg_model_t* mod, char* name, void* data );
  /// retrieve a data item from the model, referenced by the string "name".
  void* stg_model_get_prop( stg_model_t* mod, char* name );
  
  /** print human-readable information about the model on stdout
   */
  void stg_model_print( stg_model_t* mod );
  
  /** returns TRUE iff [testmod] exists above [mod] in a model tree 
   */
  int stg_model_is_antecedent( stg_model_t* mod, stg_model_t* testmod );
  
  /** returns TRUE iff [testmod] exists below [mod] in a model tree 
   */
  int stg_model_is_descendent( stg_model_t* mod, stg_model_t* testmod );
  
  /** returns TRUE iff [mod1] and [mod2] both exist in the same model
      tree 
  */
  int stg_model_is_related( stg_model_t* mod1, stg_model_t* mod2 );

  /** return the top-level model above mod */
  stg_model_t* stg_model_root( stg_model_t* mod );

  /** add a pointer to each model in the tree starting at root to the
      array. Returns the number of model pointers added */
  int stg_model_tree_to_ptr_array( stg_model_t* root, GPtrArray* array );

  /** initialize a model - called when a model goes from zero to one subscriptions */
  int stg_model_startup( stg_model_t* mod );

  /** finalize a model - called when a model goes from one to zero subscriptions */
  int stg_model_shutdown( stg_model_t* mod );


  int stg_model_update( stg_model_t* model );

  /** convert a global pose into the model's local coordinate system */
  void stg_model_global_to_local( stg_model_t* mod, stg_pose_t* pose );
  void stg_model_local_to_global( stg_model_t* mod, stg_pose_t* pose );

 
  /**@}*/


  // BLOBFINDER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_blobfinder Blobfinder
      Implements the blobfinder  model.
      @{ */
  
#define STG_BLOB_CHANNELS_MAX 16
  
  /** blobfinder config packet
   */
  typedef struct
  {
    int channel_count; // 0 to STG_BLOBFINDER_CHANNELS_MAX
    stg_color_t channels[STG_BLOB_CHANNELS_MAX];
    int scan_width;
    int scan_height;
    stg_meters_t range_max;
    stg_radians_t pan, tilt, zoom;
  } stg_blobfinder_config_t;
  
  /** blobfinder data packet 
   */
  typedef struct
  {
    int channel;
    stg_color_t color;
    int xpos, ypos;   // all values are in pixels
    //int width, height;
    int left, top, right, bottom;
    int area;
    stg_meters_t range;
  } stg_blobfinder_blob_t;

  /** Create a new blobfinder model 
   */
  stg_model_t* stg_blobfinder_create( stg_world_t* world,	
				      stg_model_t* parent, 
				      stg_id_t id,  
				      char* token );   
  /**@}*/

  // LASER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_laser Laser range scanner
      Implements the laser model: emulates a scanning laser rangefinder
      @{ */
  
  /** laser sample packet
   */
  typedef struct
  {
    uint32_t range; ///< range to laser hit in mm
    char reflectance; ///< intensity of the reflection 0-4
  } stg_laser_sample_t;
  
  /** laser configuration packet
   */
  typedef struct
  {
    //stg_geom_t geom;
    stg_radians_t fov; ///< field of view 
    stg_meters_t range_max; ///< the maximum range
    stg_meters_t range_min; ///< the miniimum range
    int samples; ///< the number of range measurements (and thus the size
    ///< of the array of stg_laser_sample_t's returned)
  } stg_laser_config_t;
  
  /** print human-readable version of the laser config struct
   */
  void stg_print_laser_config( stg_laser_config_t* slc );
  
  /** Create a new laser model 
   */
  stg_model_t* stg_laser_create( stg_world_t* world, 
				 stg_model_t* parent, 
				 stg_id_t id,
				 char* token );
  /**@}*/

  // FIDUCIAL MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_fiducial Fidicial detector
      Implements the fiducial detector model
      @{ */

  /** fiducial config packet 
   */
  typedef struct
  {
    stg_meters_t max_range_anon;
    stg_meters_t max_range_id;
    stg_meters_t min_range;
    stg_radians_t fov; // field of view 
    stg_radians_t heading; // center of field of view

  } stg_fiducial_config_t;
  
  /** fiducial data packet 
   */
  typedef struct
  {
    stg_meters_t range; // range to the target
    stg_radians_t bearing; // bearing to the target 
    stg_pose_t geom; // size and relative angle of the target
    int id; // the identifier of the target, or -1 if none can be detected.
    
  } stg_fiducial_t;

  /** Create a new fiducial model 
   */
  stg_model_t* stg_fiducial_create( stg_world_t* world,  
				    stg_model_t* parent,  
				    stg_id_t id, 
				    char* token );  
  /**@}*/
  
  // RANGER MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_ranger Range finder
      Implements the ranger model: emulates
      sonar, IR, and non-scanning laser range sensors 
      @{ */

  typedef struct
  {
    stg_meters_t min, max;
  } stg_bounds_t;
  
  typedef struct
  {
    stg_bounds_t range; // min and max range of sensor
    stg_radians_t angle; // viewing angle of sensor
  } stg_fov_t;
  
  typedef struct
  {
    stg_pose_t pose;
    stg_size_t size;
    stg_bounds_t bounds_range;
    stg_radians_t fov;
  } stg_ranger_config_t;
  
  typedef struct
  {
    stg_meters_t range;
    //double error; // TODO
  } stg_ranger_sample_t;
  
  /** Create a new ranger model 
   */
  stg_model_t* stg_ranger_create( stg_world_t* world,  
				  stg_model_t* parent, 
				  stg_id_t id, 
				  char* token );
  /**@}*/
  
  // POSITION MODEL --------------------------------------------------------
  
  /** @defgroup stg_model_position Position
      Implements the position model: a mobile robot base
      @{ */
  
#define STG_MM_POSITION_RESETODOM 77
  
  typedef enum
    { STG_POSITION_CONTROL_VELOCITY, STG_POSITION_CONTROL_POSITION }
    stg_position_control_mode_t;
  
  typedef enum
    { STG_POSITION_STEER_DIFFERENTIAL, STG_POSITION_STEER_INDEPENDENT }
    stg_position_steer_mode_t;
  
  typedef struct
  {
    stg_meters_t x,y,a;
    stg_position_control_mode_t mode; 
  } stg_position_cmd_t;
  
  typedef struct
  {
    stg_position_steer_mode_t steer_mode;
    //stg_bool_t motor_disable; // if non-zero, the motors are disabled
    //stg_pose_t odometry
  } stg_position_cfg_t;
  
  typedef struct
  {
    stg_pose_t pose; // current position estimate
    stg_velocity_t velocity; // current velocity estimate
    stg_bool_t stall; // motors stalled flag
  } stg_position_data_t;

  typedef struct
  {
    stg_pose_t odom_origin; // odometry origin
    stg_pose_t odom; // current odom value
    double x_error, y_error, a_error; // params for odom error model (not yet implemented)
  } stg_model_position_t;
  
  /// create a new position model
  stg_model_t* stg_position_create( stg_world_t* world,  stg_model_t* parent,  stg_id_t id, char* token );
  
  /// set the current odometry estimate 
  void stg_model_position_set_odom( stg_model_t* mod, stg_pose_t* odom );
  
  /**@}*/
  
  // end the group of all models
  /**@}*/
  


// MACROS ------------------------------------------------------
// Some useful macros


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MILLION 1e6
#define BILLION 1e9

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef TWOPI
#define TWOPI (2.0*M_PI)
#endif
  
#ifndef RTOD
  /// Convert radians to degrees
#define RTOD(r) ((r) * 180.0 / M_PI)
#endif
  
#ifndef DTOR
  /// Convert degrees to radians
#define DTOR(d) ((d) * M_PI / 180.0)
#endif
  
#ifndef NORMALIZE
  /// Normalize angle to domain -pi, pi
#define NORMALIZE(z) atan2(sin(z), cos(z))
#endif


#ifdef __cplusplus
}
#endif 

// end documentation group libstage
/**@}*/

#endif
