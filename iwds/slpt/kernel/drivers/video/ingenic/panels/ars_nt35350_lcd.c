#include <config.h>
#include <common.h>
#include <jzfb.h>
#include <jz_dsim.h>
#include <linux/pr_info.h>

#include "../jz_mipi_dsi/jz_mipi_dsih_hal.h"
#include "../jz_mipi_dsi/jz_mipi_dsi_regs.h"

int ars_nt35350_res_init(void) {
	pr_debug("ars_nt35350_res_init called\n");
	return 0;
}

int ars_nt35350_power_on(int on) {
	pr_debug("ars_nt35350_power_on called\n");
	return 0;
}

int ars_nt35350_update_mode(int update) {
	struct dsi_device *dsi = get_default_dsi();

	struct dsi_cmd_packet enter_idle_mode = {0x39, 0x02, 0x00, {0x39, 0x00}}; /* enter idle */
	struct dsi_cmd_packet exit_idle_mode = {0x39, 0x02, 0x00, {0x38, 0x00}}; /* exit idle */

	if(update) {
		if(write_command(dsi, &enter_idle_mode))
			pr_err("write command fail\n");
	} else {
		if(write_command(dsi, &exit_idle_mode))
			pr_err("write command fail\n");
	}

	return 0;
}