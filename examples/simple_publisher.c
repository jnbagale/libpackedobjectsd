/* Copyright (C) 2009-2011 The Clashing Rocks Team */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU General Public License for more details. */

/* A sample ZeroMQ node which can act as publisher */
/* Publisher connects to broker's inbound socket */

#include <glib.h>       /* for GOptions*/
#include <pthread.h>   /* for threads */
#include <string.h>   /* for strlen()*/
#include <stdlib.h>  /* for exit()  */
#include <unistd.h> /* for read()  */
#include <fcntl.h> /* for open()  */

#include "config.h" /* Auto generated by the program */
#include "packedobjectsd.h"

static int global_send_counter = 0;

static void *publish_data(void *pub_obj)
{
  int ret;
  int size;
  int encode_type = ENCODED;
  char msg[20] = "test message";
  char *message;
  pubObject *pub_object;
  pub_object =  (pubObject *) pub_obj; /* Casting void * pointer back to pubObject pointer */

  while(1) {
    size =  strlen(msg) + sizeof(int);
    message = malloc (size + 1);
    sprintf(message, "%s%d", msg ,global_send_counter++); /* Preparing message */

    ret = send_data(pub_object, message, strlen(message), encode_type); 
    if(ret  != -1) {
      printf("Message sent %s: %s\n",((!encode_type) ? "with encoding" : "without encoding"),  message);
    }
    else {
      printf("Message could not be sent: %s\n", message);
    }

    free(message);
    usleep(1000 * 1000); /* Control send frequency in microseconds. Also allows other thread to run.. */
  }

  /* We should never reach here unless something goes wrong! */
  return pub_object;  
}

int main (int argc, char *argv [])
{
  pthread_t thread_pub;
  gboolean verbose = FALSE;
  char *address = DEFAULT_SERVER_ADDRESS;
  char *pub_schema_path = "schema.xsd";
  int port = DEFAULT_SERVER_PORT;
  int send_freq = DEFAULT_SEND_FREQ;
  pubObject *pub_obj = NULL;

  /* For command line arguments */
  GError *error = NULL;
  GOptionContext *context;
  GOptionEntry entries[] = 
    {
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Verbose output", NULL },
      { "address", 'h', 0, G_OPTION_ARG_STRING, &address, "Lookup server address", NULL },
      { "port", 'p', 0, G_OPTION_ARG_INT, &port, "Lookup server port", "N" },
      { "send-freq", 's', 0, G_OPTION_ARG_INT, &send_freq, "Sending frequency for publisher", "N" },
      { "pub-schema-path", 'x', 0, G_OPTION_ARG_STRING, &pub_schema_path, "File path for Publisher xml schema", NULL },
      { NULL }
    };
 
  context = g_option_context_new ("- node");
  g_option_context_add_main_entries (context, entries, PACKAGE_NAME);
  
  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    printf("option parsing failed: %s\n", error->message);
    exit (EXIT_FAILURE);
  }

  /* Initialise objects & variables */
  pub_obj = make_pub_object();  
  
  pub_obj->port = port;
  pub_obj->address = malloc (strlen(address) + 1);
  sprintf(pub_obj->address, "%s",address);
  


  /* Connects to PUB socket, program quits if connect fails * */
  pub_obj = publish_to_broker(pub_obj, pub_schema_path);
  sleep(1); /* Sleep for a second to allow broker to run if it's not running */

  /* Create thread which will execute publish_data() function */
  if (pthread_create( &thread_pub, NULL, publish_data,(void *) pub_obj)) {
    fprintf(stderr, "Error creating publisher thread \n");
    exit(EXIT_FAILURE);
  }
  

  /* Join the thread to start the server */
  if(pthread_join( thread_pub, NULL)) {
    fprintf(stderr, "Error joining publisher thread \n");
    exit(EXIT_FAILURE);
  }

  /* We should never reach here unless something goes wrong! */
  return EXIT_FAILURE;
}
/* End of simple_publisher.c */
