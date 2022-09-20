/****************************************************************************
 * apps/examples/examples/volapp/volapp.h
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

#ifndef __APPS_ADC_ADC_H
#define __APPS_ADC_ADC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <uORB/uORB.h>
#include <sensor/adc.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* Configuration ************************************************************/
/* CONFIG_NSH_BUILTIN_APPS - Build the ADC test as an NSH built-in function.
 *  Default: Built as a standalone program
 * CONFIG_ADC_DEVPATH - The default path to the ADC device. Default: /dev/adc0
 * CONFIG_ADC_NSAMPLES - This number of samples is
 *   collected and the program terminates.  Default:  Samples are collected
 *   indefinitely.
 * CONFIG_ADC_GROUPSIZE - The number of samples to read at once.
 *   Default: 4
 */

#ifndef CONFIG_ADC_NSAMPLES
#  define CONFIG_ADC_NSAMPLES 10
#endif

#ifndef CONFIG_ADC
#  error "ADC device support is not enabled (CONFIG_ADC)"
#endif

#ifndef CONFIG_ADC_DEVPATH
#  define CONFIG_ADC_DEVPATH "/dev/adc0"
#endif

#ifndef CONFIG_ADC_GROUPSIZE
#  define CONFIG_ADC_GROUPSIZE 2
#endif

#ifndef STM32L4_ADC1_SAMPLE_FREQUENCY
#  define STM32L4_ADC1_SAMPLE_FREQUENCY 100
#endif

/****************************************************************************
 * Public Types
 ****************************************************************************/

struct adc_state_s
{
  bool      initialized;
  FAR char *devpath;
  int       count;
};

struct orb_adc_s
{
  uint64_t timestamp;
  int32_t adc0;
  int32_t adc1;
  int32_t adc2;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#endif /* __APPS_ADC_ADC_H */
