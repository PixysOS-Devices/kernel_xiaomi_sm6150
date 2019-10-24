/*
 * ElementalX msm-sleeper by flar2 <asegaert@gmail.com>
 * 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/notifier.h>
#include <linux/msm_drm_notify.h>
#include <mach/cpufreq.h>

#define MSM_SLEEPER_MAJOR_VERSION	3
#define MSM_SLEEPER_MINOR_VERSION	1

extern uint32_t maxscroff;
extern uint32_t maxscroff_freq;
static int limit_set = 0;

static void msm_sleeper_suspend(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		msm_cpufreq_set_freq_limits(cpu, MSM_CPUFREQ_NO_LIMIT, maxscroff_freq);
		pr_info("Limit max frequency to: %d\n", maxscroff_freq);
	}
	limit_set = 1;

	return; 
}

static void msm_sleeper_resume(void)
{
	int cpu;

	for_each_possible_cpu(cpu) {
		msm_cpufreq_set_freq_limits(cpu, MSM_CPUFREQ_NO_LIMIT, MSM_CPUFREQ_NO_LIMIT);
		pr_info("Restore max frequency to %d\n", MSM_CPUFREQ_NO_LIMIT);
	}
	limit_set = 0;

	return; 

}

static int msm_drm_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct msm_drm_notifier *evdata = data;
	int *blank;

	if (event != MSM_DRM_EVENT_BLANK)
		return 0;

	if (evdata->id != MSM_DRM_PRIMARY_DISPLAY)
		return 0;

	if (evdata && evdata->data) {
		blank = evdata->data;
		switch (*blank) {
			case MSM_DRM_BLANK_UNBLANK:
				//display on
				if (limit_set)
					msm_sleeper_resume();
				break;
			case MSM_DRM_BLANK_POWERDOWN:
				//display off
				if (maxscroff)
					msm_sleeper_suspend();
				break;
		}
	}

	return 0;
}

static struct notifier_block msm_sleeper_msm_drm_notif =
{
	.notifier_call = msm_drm_notifier_callback,
};


static int __init msm_sleeper_init(void)
{
	pr_info("msm-sleeper version %d.%d\n",
		 MSM_SLEEPER_MAJOR_VERSION,
		 MSM_SLEEPER_MINOR_VERSION);

	msm_drm_register_client(&msm_sleeper_msm_drm_notif);

	return 0;
}

MODULE_AUTHOR("flar2 <asegaert@gmail.com>");
MODULE_DESCRIPTION("'msm-sleeper' - Limit max frequency while screen is off");
MODULE_LICENSE("GPL v2");

late_initcall(msm_sleeper_init);

