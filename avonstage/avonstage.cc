/**
  \defgroup stage The Stage standalone robot simulator.

  Here is where I describe the Stage standalone program.
 */

#include <getopt.h>

extern "C" {
#include <avon.h>
}

#include "stage.hh"
#include "config.h"

const char* USAGE = 
  "USAGE:  stage [options] <worldfile1> [worldfile2 ... worldfileN]\n"
  "Available [options] are:\n"
  "  --clock        : print simulation time peridically on standard output\n"
  "  -c             : equivalent to --clock\n"
  "  --gui          : run without a GUI\n"
  "  -g             : equivalent to --gui\n"
  "  --help         : print this message\n"
  "  --args \"str\" : define an argument string to be passed to all controllers\n"
  "  -a \"str\"     : equivalent to --args \"str\"\n"
  "  --host \"str\" : set the http server host name (default: \"localhost\")\n"
  " --port num      : set the http sever port number (default: 8000)\n" 
  " --verbose       : provide lots of informative output\n"
  " -v              : equivalent to --verbose\n"
  "  -?             : equivalent to --help\n";

/* options descriptor */
static struct option longopts[] = {
	{ "gui",  optional_argument,   NULL,  'g' },
	{ "clock",  optional_argument,   NULL,  'c' },
	{ "help",  optional_argument,   NULL,  'h' },
	{ "args",  required_argument,   NULL,  'a' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "port",  required_argument,   NULL,  'p' },
	{ "host",  required_argument,   NULL,  'h' },
	{ "rootdir",  required_argument,   NULL,  'r' },
	{ NULL, 0, NULL, 0 }
};



uint64_t GetTimeWorld( Stg::World* world )
{
  Stg::usec_t stgtime = world->SimTimeNow();  
  return static_cast<uint64_t>(stgtime);
}

uint64_t GetTime( Stg::Model* mod )
{
  assert(mod);	
  return GetTimeWorld( mod->GetWorld() );
}

int GetModelPVA( Stg::Model* mod, av_pva_t* pva  )
{
  assert(mod);
  assert(pva);

  bzero(pva,sizeof(av_pva_t));
  
  pva->time = GetTime(mod);  
  
  Stg::Pose sp = mod->GetPose(); 

  pva->p[0] = sp.x;
  pva->p[1] = sp.y;
  pva->p[2] = sp.z;
  pva->p[3] = 0;
  pva->p[4] = 0;
  pva->p[5] = sp.a;
  
  Stg::Velocity sv = mod->GetVelocity(); 		
  
  pva->v[0] = sv.x;
  pva->v[1] = sv.y;
  pva->v[2] = sv.z;
  pva->v[3] = 0;
  pva->v[4] = 0;
  pva->v[5] = sv.a;
  
  return 0; // ok
}


int SetModelPVA(  Stg::Model* mod, av_pva_t* p  )
{
  assert(mod);
  assert(p);
  
  mod->SetPose( Stg::Pose( p->p[0], // x
									p->p[1], // y
									p->p[2], // z
									p->p[5] )); // a
  
  mod->SetVelocity( Stg::Velocity( p->v[0], // x
											  p->v[1], // y
											  p->v[2], // z
											  p->v[5] )); // a  
  return 0; // ok
}


int SetModelGeom(  Stg::Model* mod, av_geom_t* g )
{	
  assert(mod);
  assert(g);
  
  mod->SetGeom( Stg::Geom( Stg::Pose(g->pose[0], g->pose[1], g->pose[2], g->pose[5] ),
									Stg::Size(g->extent[0], g->extent[1], g->extent[2] ) ));

  // force GUI update to see the change if Stage was paused
  mod->Redraw();
  
  return 0; // ok
}

int GetModelGeom(  Stg::Model* mod, av_geom_t* g )
{	
  assert(mod);
  assert(g);
  
  bzero(g, sizeof(av_geom_t));

  g->time = GetTime(mod);  
  
  Stg::Geom ext(mod->GetGeom());    	
  g->pose[0] = ext.pose.x;
  g->pose[1] = ext.pose.y;
  g->pose[2] = ext.pose.a;
  g->extent[0] = ext.size.x;
  g->extent[1]= ext.size.y;
  g->extent[2] = ext.size.z;

  return 0; // ok
}


int RangerData( Stg::Model* mod, av_data_t* data )
{
  assert(mod);
  assert(data);
  
  /* Will set the data pointer in arg data to point to this static
	  struct, thus avoiding dynamic memory allocation. This is deeply
	  non-reentrant! but fast and simple code. */
  static av_ranger_data_t rd;
  bzero(&rd, sizeof(rd));  
  bzero(data, sizeof(av_data_t));
  
  data->time = GetTime(mod);  
  data->type = AV_MODEL_RANGER;
  data->data = (const void*)&rd;  
  
  Stg::ModelRanger* r = dynamic_cast<Stg::ModelRanger*>(mod);
  
  const std::vector<Stg::ModelRanger::Sensor>& sensors = r->GetSensors();
  
  rd.transducer_count = sensors.size();
  
  assert( rd.transducer_count <= AV_RANGER_TRANSDUCERS_MAX );
  
  for( unsigned int s=0; s<rd.transducer_count; s++ )
	 {
		rd.transducers[s].pose[0] = sensors[s].pose.x;
		rd.transducers[s].pose[1] = sensors[s].pose.y;
		rd.transducers[s].pose[2] = sensors[s].pose.z;
		rd.transducers[s].pose[3] = 0.0;
		rd.transducers[s].pose[4] = 0.0;
		rd.transducers[s].pose[5] = sensors[s].pose.a;		
		
		const std::vector<Stg::meters_t>& ranges = sensors[s].ranges;
		const std::vector<float>& intensities = sensors[s].intensities;

		assert( ranges.size() == intensities.size() );

		rd.transducers[s].sample_count = ranges.size(); 
		
		printf( "ranges size %u\n", ranges.size() );
		
		for( unsigned int r=0; r<rd.transducers[s].sample_count; r++ )
		  {
			 rd.transducers[s].samples[r][AV_SAMPLE_BEARING] = 1.0; // XX
			 rd.transducers[s].samples[r][AV_SAMPLE_AZIMUTH] = 1.0; // XX
			 rd.transducers[s].samples[r][AV_SAMPLE_RANGE] = ranges[r];
			 rd.transducers[s].samples[r][AV_SAMPLE_INTENSITY] = intensities[r];
		  }
	 }
	  
  return 0; //ok
}

int RangerCmd( Stg::Model* mod, av_cmd_t* data )
{
  assert(mod);
  assert(data);  
  puts( "ranger command does nothing" );  
  return 0; //ok
}

int RangerCfgSet( Stg::Model* mod, av_cfg_t* data )
{
  assert(mod);
  assert(data);  
  puts( "ranger setcfg does nothing" );  
  return 0; //ok
}

int RangerCfgGet( Stg::Model* mod, av_cfg_t* data )
{
  assert(mod);
  assert(data);  
  puts( "ranger getcfg does nothing" );  
  return 0; //ok
}

int RegisterModel( Stg::Model* mod, void* dummy )
{ 
  //printf( "[AvonStage] registering %s\n", mod->Token() );

  av_type_t type = AV_MODEL_GENERIC;
  
  std::string str = mod->GetModelType();
  
  if( str == "position" )
	 type = AV_MODEL_POSITION2D;
  else if( str == "ranger" )
	 type = AV_MODEL_RANGER;
  
  Stg::Model* parent = mod->Parent();
  const char* parent_name = parent ? parent->Token() : NULL;
  
  av_register_model( mod->Token(), type, parent_name, dynamic_cast<void*>(mod) );
  
  return 0; // ok
}

int main( int argc, char* argv[] )
{
  // initialize libstage - call this first
  Stg::Init( &argc, &argv );

  printf( "%s %s ", PROJECT, VERSION );
  
  int ch=0, optindex=0;
  bool usegui = true;
  bool showclock = false;
  
  std::string host = "localhost";
  std::string rootdir = ".";
  unsigned short port = AV_DEFAULT_PORT;
  bool verbose = false;

  while ((ch = getopt_long(argc, argv, "cvrgh?p?", longopts, &optindex)) != -1)
	 {
		switch( ch )
		  {
		  case 0: // long option given
			 printf( "option %s given\n", longopts[optindex].name );
			 break;

			 // stage options
		  case 'a':
			 Stg::World::ctrlargs = std::string(optarg);
			 break;
		  case 'c': 
			 showclock = true;
			 printf( "[Clock enabled]" );
			 break;
		  case 'g': 
			 usegui = false;
			 printf( "[GUI disabled]" );
			 break;
			 
			 // avon options
		  case 'p':
			 port = atoi(optarg);
			 break;
		  case 'h':
			 host = std::string(optarg);
			 break;
			 // 		  case 'f':
			 // 			 fedfilename = optarg;
			 // 			 usefedfile = true;
			 // 			 break;
		  case 'r':
			 rootdir = std::string(optarg);
			 break;
		  case 'v':
			 verbose = true;
			 break;
			 // help options
		  case '?':  
			 puts( USAGE );
			 exit(0);
			 break;
		  default:
			 printf("unhandled option %c\n", ch );
			 puts( USAGE );
			 //exit(0);
		  }
	 }
  
  const char* worldfilename = argv[optind];

  if( worldfilename == NULL )
	 {	
		puts( "[AvonStage] no worldfile specified on command line. Quit.\n" );
		exit(-1);
	 }

  puts("");// end the first start-up line
  
  printf( "[AvonStage] host %s:%u world %s\n",
			 host.c_str(), 
			 port, 
			 worldfilename );
  
  // avon
  av_init( host.c_str(), port, rootdir.c_str(), verbose, PROJECT, VERSION );

  av_install_generic_callbacks( (av_pva_set_t)SetModelPVA,
										  (av_pva_get_t)GetModelPVA,
										  (av_geom_set_t)SetModelGeom,
										  (av_geom_get_t)GetModelGeom );
  
  av_install_typed_callbacks( AV_MODEL_RANGER, 
										(av_data_get_t)RangerData,
										(av_cmd_set_t)RangerCmd,
										(av_cfg_set_t)RangerCfgSet,
										(av_cfg_get_t)RangerCfgGet );
  
  // arguments at index [optindex] and later are not options, so they
  // must be world file names
  

  
  Stg::World* world = ( usegui ? 
						 new Stg::WorldGui( 400, 300, worldfilename ) : 
						 new Stg::World( worldfilename ) );

  world->Load( worldfilename );
  world->ShowClock( showclock );

	// now we have a world object, we can install a clock callback
	av_install_clock_callbacks( (av_clock_get_t)GetTimeWorld, world );
	
  // start the http server
  av_startup();
  // register all models here  
  world->ForEachDescendant( RegisterModel, NULL );
 
  if( ! world->paused ) 
	 world->Start();
    
  while( 1 )
	 {
		// TODO - loop properly

		Fl::check(); 
		av_check();	 

		usleep(100); // TODO - loop sensibly here
	 }
  
  puts( "\n[AvonStage: done]" );
  
  return EXIT_SUCCESS;
}
