/*
 * (C) Copyright 2014
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 * 	on behalf of ifm electronic GmbH
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <string.h>
#include "swupdate.h"
#include "handler.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "util.h"
#include "lua_util.h"

static void shell_handler(void);

static int start_shell_script(struct img_type *img, void *data)
{
	int ret;
	const char *fnname;
	char shellscript[MAX_IMAGE_FNAME +
		strlen(SCRIPTS_DIR) + strlen("postinst") + 2];
	script_fn scriptfn;

	if (!data)
		return -1;

	scriptfn = *(script_fn *)data;

	switch (scriptfn) {
	case PREINSTALL:
		fnname="preinst";
		break;
	case POSTINSTALL:
		fnname="postinst";
		break;
	default:
		/* no error, simply no call */
		return 0;
	}

	snprintf(shellscript, sizeof(shellscript),
		"%s%s", TMPDIR, img->fname);
	if (chmod(shellscript, S_IRUSR | S_IWUSR | S_IXUSR)) {
		ERROR("Execution bit cannot be set for %s", shellscript);
		return -1;
	}
	snprintf(shellscript, sizeof(shellscript),
		"%s%s %s", TMPDIR, img->fname, fnname);

	ret = system(shellscript);
	TRACE("Calling shell script %s: return with %d", shellscript, ret);
	if (ret == WIFEXITED(ret))
		return 0;
	if (ret) {
		ERROR("%s returns '%s'", img->fname, strerror(errno));
	}

	return ret;

}

 __attribute__((constructor))
static void shell_handler(void)
{
	register_handler("shellscript", start_shell_script, NULL);
}
