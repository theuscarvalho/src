/* $OpenBSD: cmd-respawn-window.c,v 1.38 2019/04/17 14:37:48 nicm Exp $ */

/*
 * Copyright (c) 2008 Nicholas Marriott <nicholas.marriott@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#include "tmux.h"

/*
 * Respawn a window (restart the command). Kill existing if -k given.
 */

static enum cmd_retval	cmd_respawn_window_exec(struct cmd *,
			    struct cmdq_item *);

const struct cmd_entry cmd_respawn_window_entry = {
	.name = "respawn-window",
	.alias = "respawnw",

	.args = { "c:kt:", 0, -1 },
	.usage = "[-c start-directory] [-k] " CMD_TARGET_WINDOW_USAGE
	         " [command]",

	.target = { 't', CMD_FIND_WINDOW, 0 },

	.flags = 0,
	.exec = cmd_respawn_window_exec
};

static enum cmd_retval
cmd_respawn_window_exec(struct cmd *self, struct cmdq_item *item)
{
	struct args		*args = self->args;
	struct spawn_context	 sc;
	struct session		*s = item->target.s;
	struct winlink		*wl = item->target.wl;
	char			*cause = NULL;

	memset(&sc, 0, sizeof sc);
	sc.item = item;
	sc.s = s;
	sc.wl = wl;

	sc.name = NULL;
	sc.argc = args->argc;
	sc.argv = args->argv;

	sc.idx = -1;
	sc.cwd = args_get(args, 'c');

	sc.flags = SPAWN_RESPAWN;
	if (args_has(args, 'k'))
		sc.flags |= SPAWN_KILL;

	if (spawn_window(&sc, &cause) == NULL) {
		cmdq_error(item, "respawn window failed: %s", cause);
		free(cause);
		return (CMD_RETURN_ERROR);
	}

	server_redraw_window(wl->window);

	return (CMD_RETURN_NORMAL);
}
