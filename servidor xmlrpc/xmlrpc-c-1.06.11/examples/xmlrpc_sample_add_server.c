/* A simple standalone XML-RPC server written in C. */

#include <stdlib.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>

#include "config.h"  /* information about this build environment */

static xmlrpc_value * sample_add(xmlrpc_env *   const env,xmlrpc_value * const param_array,void * const user_data ATTR_UNUSED)
{

//     xmlrpc_int32 x, y, z;
	
	xmlrpc_value * string;
	/* Parse our argument array. */
	xmlrpc_decompose_value(env, param_array, "s", &string);
	if (env->fault_occurred)
		return NULL;
	/* Add our two numbers. */
// 	z = (x + y) * 2;
	
	/* Sometimes, make it look hard (so client can see what it's like
	to do an RPC that takes a while).
	*/
// 	if (y == 1)
// 		sleep(2);
	
	/* Return our result. */
	return xmlrpc_build_value(env, "s", string);
}



int main(int const argc,const char ** const argv) 
{

    xmlrpc_server_abyss_parms serverparm;
    xmlrpc_registry * registryP;
    xmlrpc_env env;

    if (argc-1 != 1) 
    {
        fprintf(stderr, "You must specify 1 argument:  The TCP port "
                "number on which the server will accept connections "
                "for RPCs.  You specified %d arguments.\n",  argc-1);
        exit(1);
    }
    
    xmlrpc_env_init(&env);

    registryP = xmlrpc_registry_new(&env);

    xmlrpc_registry_add_method(&env, registryP, NULL, "sample.add", &sample_add, NULL);

    /* In the modern form of the Abyss API, we supply parameters in memory
       like a normal API.  We select the modern form by setting
       config_file_name to NULL: 
    */
    serverparm.config_file_name = NULL;
    serverparm.registryP        = registryP;
    serverparm.port_number      = atoi(argv[1]);
    serverparm.log_file_name    = "/tmp/xmlrpc_log";

    printf("Running XML-RPC server...\n");

    xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));

    /* xmlrpc_server_abyss() never returns */

    return 0;
}
