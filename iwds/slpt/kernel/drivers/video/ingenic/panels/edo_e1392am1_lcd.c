#include <config.h>
#include <common.h>
#include <jzfb.h>
#include <jz_dsim.h>
#include <linux/pr_info.h>

#include "../jz_mipi_dsi/jz_mipi_dsih_hal.h"
#include "../jz_mipi_dsi/jz_mipi_dsi_regs.h"

int edo_e1392am1_res_init(void) {
	pr_debug("edo_e1392am1_res_init called\n");
	return 0;
}

int edo_e1392am1_power_on(int on) {
	pr_debug("edo_e1392am1_power_on called\n");
	return 0;
}

int edo_e1392am1_update_mode(int update) {
	struct dsi_device *dsi = get_default_dsi();

	struct dsi_cmd_packet enter_idle_mode = {0x39, 0x02, 0x00, {0x39, 0x00}}; /* enter idle */
	struct dsi_cmd_packet exit_idle_mode = {0x39, 0x02, 0x00, {0x38, 0x00}}; /* exit idle */

	pr_debug("edo_e1392am1_update_mode called\n");

	if(update) {
		if(write_command(dsi, &enter_idle_mode))
			pr_err("write command fail\n");
	} else {
		if(write_command(dsi, &exit_idle_mode))
			pr_err("write command fail\n");
	}

	return 0;
}
