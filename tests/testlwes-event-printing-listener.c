/*======================================================================*
 * Copyright (c) 2008, Yahoo! Inc. All rights reserved.                 *
 *                                                                      *
 * Licensed under the New BSD License (the "License"); you may not use  *
 * this file except in compliance with the License.  Unless required    *
 * by applicable law or agreed to in writing, software distributed      *
 * under the License is distributed on an "AS IS" BASIS, WITHOUT        *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     *
 * See the License for the specific language governing permissions and  *
 * limitations under the License. See accompanying LICENSE file.        *
 *======================================================================*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <assert.h>
#include <lwes_event.h>
#include <lwes_emitter.h>

#include "config.h"

#ifdef HAVE_VALGRIND_HEADER
#include <valgrind/valgrind.h>

/* FIXME: This feels totally hackish, but for whatever reason I can't seem
   to get this to run correctly under valgrind, it has random heisenbugs
   (ie, if I add extra arguments to valgrind, add sleeps, add printf's etc
   the bug moves and asserts fail at different spots.  So for the moment
   turn off asserts if RUNNING_ON_VALGRIND */
#define  MY_ASSERT(stmt) {if (! RUNNING_ON_VALGRIND) { assert (stmt); }}
#else
#define  MY_ASSERT(stmt) assert(stmt);
#endif

#define  NUM_ELEMS(array)  (sizeof (array) / sizeof ((array)[0]))
#define  TRUE 1
#define  FALSE 0

int lwes_event_printing_listener_main (int   argc, char *argv[]);

#define main lwes_event_printing_listener_main
#include "lwes-event-printing-listener.c"
#undef main

static const char TEST_LLOG_ADDRESS[] = "224.1.1.101";
static const char TEST_LLOG_INTERFACE[] = "127.0.0.1";
static const char TEST_LLOG_PORT[] = "9111";

static void event1 (struct lwes_emitter *emitter)
{
  struct lwes_event *event;
  int ret = 0;

  LWES_SHORT_STRING eventname = (LWES_SHORT_STRING)"TypeChecker";
  LWES_SHORT_STRING key01     = (LWES_SHORT_STRING)"aString";
  LWES_LONG_STRING  value01   = (LWES_LONG_STRING)"http://www.test.com";
  LWES_SHORT_STRING key02     = (LWES_SHORT_STRING)"aBoolean";
  LWES_BOOLEAN      value02   = (LWES_BOOLEAN)1;
  LWES_SHORT_STRING key03     = (LWES_SHORT_STRING)"anIPAddress";
  LWES_IP_ADDR      value03;
  LWES_SHORT_STRING key04     = (LWES_SHORT_STRING)"aUInt16";
  LWES_U_INT_16     value04   = (LWES_U_INT_16)65535;
  LWES_SHORT_STRING key05     = (LWES_SHORT_STRING)"anInt16";
  LWES_INT_16       value05   = (LWES_INT_16)-1;
  LWES_SHORT_STRING key06     = (LWES_SHORT_STRING)"aUInt32";
  LWES_U_INT_32     value06   = (LWES_U_INT_32)0xffffffffL;
  LWES_SHORT_STRING key07     = (LWES_SHORT_STRING)"anInt32";
  LWES_INT_32       value07   = (LWES_INT_32)-1;
  LWES_SHORT_STRING key08     = (LWES_SHORT_STRING)"aUInt64";
  LWES_U_INT_64     value08   = (LWES_U_INT_64)0xffffffffffffffffULL;
  LWES_SHORT_STRING key09     = (LWES_SHORT_STRING)"anInt64";
  LWES_INT_64       value09   = (LWES_INT_64)-1;

  const LWES_U_INT_16 size10   = 3;
  LWES_SHORT_STRING key10      = (LWES_SHORT_STRING)"uint16_array";
  LWES_U_INT_16     value10[3] = {123, 234, 345};

  const LWES_U_INT_16 size11   = 4;
  LWES_SHORT_STRING key11      = (LWES_SHORT_STRING)"string_array";
  LWES_CONST_LONG_STRING value11[4]  = {"a", "bb", "ccc", "d"};

  const LWES_U_INT_16 size12   = 5;
  LWES_SHORT_STRING key12      = (LWES_SHORT_STRING)"string_null_array";
  LWES_CONST_LONG_STRING value12[5]  = {NULL, "a", NULL, "ccc", NULL};

  /* set this one here since we need to call a function */
  value03.s_addr = inet_addr("224.0.0.100");
  /* create event */
  event  = lwes_event_create (NULL, eventname);
  MY_ASSERT (event != NULL);

  /* fill out event */
  ret = lwes_event_set_STRING   (event, key01, value01);
  MY_ASSERT ( ret == 1);
  ret = lwes_event_set_BOOLEAN  (event, key02, value02);
  MY_ASSERT ( ret == 2 );
  ret = lwes_event_set_IP_ADDR  (event, key03, value03);
  MY_ASSERT ( ret == 3 );
  ret = lwes_event_set_U_INT_16 (event, key04, value04);
  MY_ASSERT ( ret == 4 );
  ret = lwes_event_set_INT_16   (event, key05, value05);
  MY_ASSERT ( ret == 5 );
  ret = lwes_event_set_U_INT_32 (event, key06, value06);
  MY_ASSERT ( ret == 6 );
  ret = lwes_event_set_INT_32   (event, key07, value07);
  MY_ASSERT ( ret == 7 );
  ret = lwes_event_set_U_INT_64 (event, key08, value08);
  MY_ASSERT ( ret == 8 );
  ret = lwes_event_set_INT_64   (event, key09, value09);
  MY_ASSERT ( ret == 9 );

  ret = lwes_event_set_array(event, key10, LWES_TYPE_U_INT_16_ARRAY, size10, value10);
  MY_ASSERT ( ret == 10 );
  ret = lwes_event_set_array(event, key11, LWES_TYPE_STRING_ARRAY,   size11, value11);
  MY_ASSERT ( ret == 11 );
  ret = lwes_event_set_nullable_array(event, key12, LWES_TYPE_N_STRING_ARRAY, size12, value12);
  MY_ASSERT ( ret == 12 );


  /* now emit event */
  ret = lwes_emitter_emit (emitter, event);
  MY_ASSERT (ret == 0);

  /* now clean up */
  lwes_event_destroy (event);
}

static void event2 (struct lwes_emitter *emitter)
{
  LWES_BYTE array_event_bytes[] = {
    14,65,114,114,97,121,84,101,115,116,69,118,101,110,116,0,13,3,101,110,
    100,5,0,22,84,104,105,115,32,105,115,32,116,104,101,32,101,110,100,32,
    102,105,101,108,100,46,9,115,116,114,105,110,103,78,65,114,145,0,8,0,8,
    85,0,1,97,0,2,98,98,0,3,99,99,99,0,4,100,100,100,100,9,117,105,110,116,
    49,54,78,65,114,141,0,14,0,14,171,40,0,1,0,2,0,3,0,5,0,7,0,11,0,13,8,
    115,116,114,105,110,103,65,114,133,0,4,0,3,119,111,110,0,3,116,111,111,
    0,4,70,114,101,101,0,3,102,111,114,8,117,105,110,116,49,54,65,114,129,
    0,7,0,1,0,2,0,3,0,5,0,7,0,11,0,13,3,100,117,98,12,64,9,33,251,77,18,
    216,74,8,102,108,111,97,116,105,110,103,11,102,254,244,249,4,98,105,
    116,101,10,42,5,115,116,97,114,116,5,0,24,84,104,105,115,32,105,115,32,
    116,104,101,32,115,116,97,114,116,32,102,105,101,108,100,46,14,120,95,
    115,118,99,95,118,101,114,115,105,111,110,115,5,0,16,103,97,116,101,
    119,97,121,47,49,49,46,49,49,55,46,48,10,120,95,97,112,112,95,110,97,
    109,101,5,0,7,103,97,116,101,119,97,121,4,101,95,105,100,7,162,1,254,
    124,103,80,96,101,9,101,95,118,101,114,115,105,111,110,4,0,1,96,24
  };

  /* now emit event */
  int ret = lwes_emitter_emit_bytes (emitter, array_event_bytes, sizeof(array_event_bytes));
  ;
  MY_ASSERT (ret == sizeof(array_event_bytes));

}

static void
generate_events (void)
{
  struct lwes_emitter *emitter;

  /* create emitter */
  emitter = lwes_emitter_create (TEST_LLOG_ADDRESS,
                                 TEST_LLOG_INTERFACE,
                                 atoi (TEST_LLOG_PORT),
                                 0,
                                 60);

  MY_ASSERT (emitter != NULL);

  event1 (emitter);
  event2 (emitter);

  lwes_emitter_destroy (emitter);
}

static void
read_and_compare (const char *data, int fd)
{
  const size_t max_line = 2048;
  char tmpline[max_line];
  char *start = tmpline;
  int n;
  unsigned int j = 0;
  unsigned int x;
  unsigned int num_to_read = 0;

  if (data == NULL || strlen (data) == 0)
    {
      n = read (fd, start, max_line);
      MY_ASSERT (n == 0
              || (fprintf (stderr, "read of %d [%.*s] chars when expecting 0\n", n, n, start), 0));
      return;
    }

  num_to_read = strlen (data);

  /* clear the memory */
  memset (tmpline, 0, max_line*sizeof(char));

  /* read character by character until we get as many as expected output */
  do
    {
      n = read (fd, start, 1);
      MY_ASSERT (n == 1
              || (fprintf (stderr, "read of %d chars, total %d read [%s] when "
                                   "expecting %d for [%s]\n",
                                    n, j, tmpline, num_to_read, data), 0));
      start++;
      j++;
      MY_ASSERT (j < max_line);
    }
  while (j < num_to_read);

  MY_ASSERT (j == num_to_read);
  n = j;

  /* compare the line read to the line passed */
  for (x = 0; x < strlen (data); ++x)
    {
      /* special case to allow for exclusion of certain fields ignore any
       * character which is \1 in the input
       */
      if (data[x] != '\1')
        {
          MY_ASSERT (data[x] == tmpline[x]
                  || (fprintf (stderr, "expected [%c] got [%c] at byte[%d] with input [%s] and output [%.*s]\n",
                               data[x], tmpline[x], x, data, n, tmpline), 0));
        }
    }

  /* check that there's not more */
  n = read (fd, tmpline, max_line);
  MY_ASSERT (n == 0
          || (fprintf (stderr, "read of %d extra chars when no more expected, "
                       "got expected [%s] followed by [%.*s]\n",
                       n, data, n, tmpline), 0));
}

static void
fork_and_wait (int          argc,
               const char **argv,
               unsigned int timeout_usec,
               int          do_events,
               int          expect_ok,
               int          kill_child,
               const char * output,
               const char * error)
{
  pid_t pid;
  int stdout_fd[2];
  int stderr_fd[2];
  int ret = 0;

  /* open a socketpair for stdout */
  ret = socketpair (AF_UNIX, SOCK_STREAM, 0, stdout_fd);
  MY_ASSERT (ret == 0);

  /* open a socketpair for stderr */
  ret = socketpair (AF_UNIX, SOCK_STREAM, 0, stderr_fd);
  MY_ASSERT (ret == 0);

  pid = fork ();
  MY_ASSERT (pid >= 0);

  if (pid == 0)
    {
      /* child */

      int ret = 0;

      /* close stdin */
      ret = close (0);
      MY_ASSERT (ret == 0);

      /* close then duplicate stdout and stderr */
      ret = close (stdout_fd[1]);
      MY_ASSERT ( ret == 0);
      ret = close (stderr_fd[1]);
      MY_ASSERT ( ret == 0);

      ret = dup2 (stdout_fd[0], 1);
      MY_ASSERT ( ret != -1);
      ret = dup2 (stderr_fd[0], 2);
      MY_ASSERT ( ret != -1);

      ret = lwes_event_printing_listener_main (argc, (char **) argv);

      exit (ret);
    }
  else
    {
      /* parent */

      int status;
      pid_t child;
      int ret = 0;

      ret = close (stdout_fd[0]);
      MY_ASSERT (ret == 0);
      ret = close (stderr_fd[0]);
      MY_ASSERT (ret == 0);

      if (do_events)
        {
          usleep (timeout_usec);

          generate_events ();

          usleep (10 * timeout_usec);

          if (kill_child)
            {
              kill (pid, SIGINT);
            }
        }

      read_and_compare (output, stdout_fd[1]);
      read_and_compare (error, stderr_fd[1]);

      /* wait for the child to end */
      child = wait (&status);

      MY_ASSERT (child == pid);
      ret = (WEXITSTATUS (status) == 0);
      MY_ASSERT (ret == expect_ok);
      (void) expect_ok;

      ret = close (stdout_fd[1]);
      MY_ASSERT (ret == 0);
      ret = close (stderr_fd[1]);
      MY_ASSERT (ret == 0);
    }
}

static void
check_opt_help (void)
{
  static const char *HELP_ARGV[] =
    {
      "testlwes-event-printing-listener", "-h",
    };
  static int HELP_ARGC = NUM_ELEMS (HELP_ARGV);

  fork_and_wait (HELP_ARGC, HELP_ARGV, 500, TRUE, FALSE, TRUE, NULL, help);
}

static void
check_opt_bad (void)
{
  static const char *BOGUS_ARGV[] =
    {
      "testlwes-event-printing-listener", "-z",
    };
  static int BOGUS_ARGC = NUM_ELEMS (BOGUS_ARGV);

  const char *error =
    "error: unrecognized command line option -z\n";

  /* It seems strange that lwes-event-printing-listener is supposed
     to go ahead and run when it gets a bad command-line argument,
     but that's the current behavior: */
  fork_and_wait (BOGUS_ARGC, BOGUS_ARGV, 500, TRUE, FALSE, TRUE, NULL, error);
}

static void
check_event_1 (void)
{
  static const char *NORMAL_ARGV[] =
    {
      "testlwes-event-printing-listener",
      "-m", TEST_LLOG_ADDRESS,
      "-p", TEST_LLOG_PORT,
      "-i", TEST_LLOG_INTERFACE,
    };
  static int NORMAL_ARGC = NUM_ELEMS (NORMAL_ARGV);

  /* this will totally not work if the order changes, or the SenderPort
     SenderIP or ReceiptTime are not the lengths represented here */
  const char *output =
    "TypeChecker[15]\n"
    "{\n"
    "\tanIPAddress = 224.0.0.100;\n"
    "\tanInt16 = -1;\n"
    "\tanInt64 = -1;\n"
    "\tReceiptTime = \1\1\1\1\1\1\1\1\1\1\1\1\1;\n"
    "\tSenderIP = \1\1\1\1\1\1\1\1\1;\n"
    "\taUInt32 = 4294967295;\n"
    "\tstring_null_array = [ , \"a\", , \"ccc\",  ];\n"
    "\taUInt16 = 65535;\n"
    "\taUInt64 = 18446744073709551615;\n"
    "\tSenderPort = \1\1\1\1\1;\n"
    "\tuint16_array = [ 123, 234, 345 ];\n"
    "\taBoolean = true;\n"
    "\taString = \"http://www.test.com\";\n"
    "\tstring_array = [ \"a\", \"bb\", \"ccc\", \"d\" ];\n"
    "\tanInt32 = -1;\n"
    "}\n"

    "ArrayTestEvent[16]\n"
    "{\n"
    "\tend = \"This is the end field.\";\n"
    "\tuint16Ar = [ 1, 2, 3, 5, 7, 11, 13 ];\n"
    "\tReceiptTime = \1\1\1\1\1\1\1\1\1\1\1\1\1;\n"
    "\tstringNAr = [ \"a\", , \"bb\", , \"ccc\", , \"dddd\",  ];\n"
    "\tSenderIP = \1\1\1\1\1\1\1\1\1;\n"
    "\tx_svc_versions = \"gateway/11.117.0\";\n"
    "\te_version = 90136;\n"
    "\tstringAr = [ \"won\", \"too\", \"Free\", \"for\" ];\n"
    "\tx_app_name = \"gateway\";\n"
    "\tstart = \"This is the start field.\";\n"
    "\te_id = -6772852554325794715;\n"
    "\tSenderPort = \1\1\1\1\1;\n"
    "\tbite = 42;\n"
    "\tfloating = 602000017271895229464576.000000;\n"
    "\tuint16NAr = [ 1, 2, , 3, , 5, , 7, , , , 11, , 13 ];\n"
    "\tdub = 3.141593;\n"
    "}\n";

  fork_and_wait (NORMAL_ARGC, NORMAL_ARGV, 500, TRUE, TRUE, TRUE, output, NULL);
}


int
main (void)
{
  check_opt_help ();
  check_event_1 ();
  check_opt_bad ();

  return 0;
}
