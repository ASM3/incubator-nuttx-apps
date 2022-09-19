/****************************************************************************
 * apps/examples/myapp/myapp_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>

#include <errno.h>
#include <debug.h>

#include <nuttx/board.h>
#include <nuttx/analog/adc.h>
#include <arch/board/board.h>

#include <sys/types.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <debug.h>

#include "myapp.h"

/****************************************************************************
 * Private Data
 ****************************************************************************/

static struct adc_state_s g_adcstate;

/****************************************************************************
 * Public Functions
 ****************************************************************************/
/****************************************************************************
 * Name: adc_devpath
 ****************************************************************************/

static void adc_devpath(FAR struct adc_state_s *adc, FAR const char *devpath)
{
  /* Get rid of any old device path */

  if (adc->devpath)
    {
      free(adc->devpath);
    }

  /* Then set-up the new device path by copying the string */

  adc->devpath = strdup(devpath);
}

/****************************************************************************
 * Name: adc_help
 ****************************************************************************/

static void adc_help(FAR struct adc_state_s *adc)
{
  printf("Usage: adc [OPTIONS]\n");
  printf("\nArguments are \"sticky\".  "
         "For example, once the ADC device is\n");
  printf("specified, that device will be re-used until it is changed.\n");
  printf("\n\"sticky\" OPTIONS include:\n");
  printf("  [-p devpath] selects the ADC device.  "
         "Default: %s Current: %s\n",
         CONFIG_EXAMPLES_ADC_DEVPATH,
         g_adcstate.devpath ? g_adcstate.devpath : "NONE");
  printf("  [-n count] selects the samples to collect.  "
         "Default: 1 Current: %d\n", adc->count);
  printf("  [-h] shows this message and exits\n");
}


/****************************************************************************
 * Name: arg_string
 ****************************************************************************/

static int arg_string(FAR char **arg, FAR char **value)
{
  FAR char *ptr = *arg;

  if (ptr[2] == '\0')
    {
      *value = arg[1];
      return 2;
    }
  else
    {
      *value = &ptr[2];
      return 1;
    }
}

/****************************************************************************
 * Name: arg_decimal
 ****************************************************************************/

static int arg_decimal(FAR char **arg, FAR long *value)
{
  FAR char *string;
  int ret;

  ret = arg_string(arg, &string);
  *value = strtol(string, NULL, 10);
  return ret;
}

/****************************************************************************
 * Name: parse_args
 ****************************************************************************/

static void parse_args(FAR struct adc_state_s *adc, int argc,
                       FAR char **argv)
{
  FAR char *ptr;
  FAR char *str;
  long value;
  int index;
  int nargs;

  for (index = 1; index < argc; )
    {
      ptr = argv[index];
      if (ptr[0] != '-')
        {
          printf("Invalid options format: %s\n", ptr);
          exit(0);
        }

      switch (ptr[1])
        {
          case 'n':
            nargs = arg_decimal(&argv[index], &value);
            if (value < 0)
              {
                printf("Count must be non-negative: %ld\n", value);
                exit(1);
              }

            adc->count = (uint32_t)value;
            index += nargs;
            break;

          case 'p':
            nargs = arg_string(&argv[index], &str);
            adc_devpath(adc, str);
            index += nargs;
            break;

          case 'h':
            adc_help(adc);
            exit(0);

          default:
            printf("Unsupported option: %s\n", ptr);
            adc_help(adc);
            exit(1);
        }
    }
}

/****************************************************************************
 * myapp_main
 ****************************************************************************/

int myapp_main(int argc, FAR char *argv[])
{
  printf("Voliro PB V2 app!!\n");

  struct adc_msg_s sample[CONFIG_EXAMPLES_ADC_GROUPSIZE];
  size_t readsize;
  ssize_t nbytes;
  int fd;
  int errval = 0;
  int ret;
  int i;

  UNUSED(ret);

  /* advertise attitude topic */
  struct orb_adc_s adc;
  //int pubsub_task;
  int instance = 0;
  int adc_pub_fd;

  adc.adc0       = 0;
  adc.adc1       = 0;
  adc.adc2       = 0;
  adc.timestamp  = orb_absolute_time();

  adc_pub_fd = orb_advertise_multi_queue_persist(ORB_ID(sensor_adc),
                                         &adc, &instance, 1);
  if (adc_pub_fd < 0)
    {
      return printf("orb_advertise failed");
    }
////////////// Temp check subscribe message
  int sfd;

  if ((sfd = orb_subscribe(ORB_ID(sensor_adc))) < 0)
    {
      printf("subscribe failed\n");
    }
//////////////////
  /* Check if we have initialized */

  if (!g_adcstate.initialized)
    {
      /* Initialization of the ADC hardware must be performed by
       * board-specific logic prior to running this test.
       */

      /* Set the default values */

      adc_devpath(&g_adcstate, CONFIG_EXAMPLES_ADC_DEVPATH);

      g_adcstate.initialized = true;
    }

  g_adcstate.count = CONFIG_EXAMPLES_ADC_NSAMPLES;

  /* Parse the command line */

  parse_args(&g_adcstate, argc, argv);

  /* If this example is configured as an NX add-on, then limit the number of
   * samples that we collect before returning.  Otherwise, we never return
   */

  printf("adc_main: g_adcstate.count: %d\n", g_adcstate.count);

  /* Open the ADC device for reading */

  printf("adc_main: Hardware initialized. Opening the ADC device: %s\n",
         g_adcstate.devpath);

  fd = open(g_adcstate.devpath, O_RDONLY);
  if (fd < 0)
    {
      printf("adc_main: open %s failed: %d\n", g_adcstate.devpath, errno);
      errval = 2;
      goto errout;
    }

  /* Now loop the appropriate number of times, displaying the collected
   * ADC samples.
   */

  for (; ; )
    {
      /* Flush any output before the loop entered or from the previous pass
       * through the loop.
       */

      fflush(stdout);

      /* Read up to CONFIG_EXAMPLES_ADC_GROUPSIZE samples */

      readsize = CONFIG_EXAMPLES_ADC_GROUPSIZE * sizeof(struct adc_msg_s);
      nbytes = read(fd, sample, readsize);

      /* Handle unexpected return values */

      if (nbytes < 0)
        {
          errval = errno;
          if (errval != EINTR)
            {
              printf("adc_main: read %s failed: %d\n",
                     g_adcstate.devpath, errval);
              errval = 3;
              goto errout_with_dev;
            }

          printf("adc_main: Interrupted read...\n");
        }
      else if (nbytes == 0)
        {
          printf("adc_main: No data read, Ignoring\n");
        }

      /* Print the sample data on successful return */

      else
        {
          int nsamples = nbytes / sizeof(struct adc_msg_s);
          if (nsamples * sizeof(struct adc_msg_s) != nbytes)
            {
              printf("adc_main: read size=%ld is not a multiple of "
                     "sample size=%d, Ignoring\n",
                     (long)nbytes, sizeof(struct adc_msg_s));
            }
          else
            {
              //printf("Sample:\n");
              for (i = 0; i < nsamples; i++)
                {
#if 0
            	  printf("%d: channel: IN_%d ADC value: %" PRId32 "\n",
                         i + 1, sample[i].am_channel, sample[i].am_data);
#else if
            	  memset(&adc, 0, sizeof(adc));

            	  adc.adc0       = sample[0].am_data;
            	  adc.adc1       = sample[1].am_data;
            	  adc.adc2       = sample[2].am_data;
            	  adc.timestamp = orb_absolute_time();

                  if (OK != orb_publish(ORB_ID(sensor_adc), adc_pub_fd, &adc))
                    {
                	  printf("publish fail!\n");
                      return 0;
                    }
                  //usleep(1000); /* simulate >800 Hz system operation */
                  ///////////////////// Temp remove from here to another app! //////////////
                  bool updated;
            	  memset(&adc, 0, sizeof(adc));


                  orb_check(sfd, &updated);
                  if (updated)
                    {
                      orb_copy(ORB_ID(sensor_adc), sfd, &adc);
                	  printf("published: %"PRIu64":  ADC value: %" PRId32 "\n",
                			  adc.timestamp, adc.adc0);
                    }
                  ///////////////////// ------------------------------------ //////////////

#endif
                }
            }
        }

      if (g_adcstate.count && --g_adcstate.count <= 0)
        {
          break;
        }
    }

  close(fd);

  return 0;

  /* Error exits */

errout_with_dev:
  close(fd);

errout:
  printf("Terminating!\n");
  fflush(stdout);
  return errval;
}
