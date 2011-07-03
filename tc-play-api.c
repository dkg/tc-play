/*
 * Copyright (c) 2011 Alex Hornung <alex@alexhornung.com>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <errno.h>

#include "tc-play.h"
#include "tc-play-api.h"

int
tc_api_init(int verbose)
{
	tc_internal_verbose = verbose;

	tc_play_init();
	return TC_OK;
}

int
tc_api_uninit(void)
{
	check_and_purge_safe_mem();
	return TC_OK;
}

char *
tc_api_get_error_msg(void)
{
	return (tc_internal_verbose) ? "" : tc_internal_log_buffer;
}

int
tc_api_create_volume(tc_api_op *api_opts)
{
	int nkeyfiles, n_hkeyfiles;
	int create_hidden;
	int err, i;

	if ((api_opts == NULL) ||
	    (api_opts->tc_device == NULL)) {
		errno = EFAULT;
		return TC_ERR;
	}

	if ((err = tc_api_check_cipher(api_opts)) != TC_OK)
		return TC_ERR;

	if ((err = tc_api_check_prf_hash(api_opts)) != TC_OK)
		return TC_ERR;

	for (nkeyfiles = 0; (nkeyfiles < MAX_KEYFILES) &&
	    (api_opts->tc_keyfiles[nkeyfiles] != NULL); nkeyfiles++)
		;

	create_hidden = 0;

	if (tc_size_hidden_in_blocks > 0) {
		create_hidden = 1;
		for (n_hkeyfiles = 0; (n_hkeyfiles < MAX_KEYFILES) &&
		    (api_opts->tc_keyfiles_hidden[n_hkeyfiles] != NULL);
		    n_hkeyfiles++)
			;
	}

	err = create_volume(api_opts->tc_device, create_hidden,
	    api_opts->tc_keyfiles, nkeyfiles,
	    api_opts->tc_keyfiles_hidden, n_hkeyfiles,
	    check_prf_algo(api_opts->tc_prf_hash, 1),
	    check_cipher_chain(api_opts->tc_cipher, 1),
	    api_opts->tc_passphrase, api_opts->tc_passphrase_hidden,
	    tc_size_hidden_in_blocks, 0 /* non-interactive */);

	return (err) ? TC_ERR : TC_OK;
}

int
tc_api_map_volume(tc_api_op *api_opts)
{
	int nkeyfiles;
	int err;

	if ((api_opts == NULL) ||
	    (api_opts->tc_device == NULL)) {
		errno = EFAULT;
		return TC_ERR;
	}

	for (nkeyfiles = 0; (nkeyfiles < MAX_KEYFILES) &&
	    (api_opts->tc_keyfiles[nkeyfiles] != NULL); nkeyfiles++)
		;

	err = map_volume(api_opts->tc_map_name, api_opts->device,
	    /* sflag */ 0, /* sys_dev */ NULL,
	    /* protect_hidden */ 0, api_opts->tc_keyfiles, nkeyfiles,
	    /* h_keyfiles[] */ NULL, /* n_hkeyfiles */ 0,
	    api_opts->passphrase, /* passphrase_hidden */ NULL,
	    api_opts->tc_interactive_prompt, api_opts->tc_password_retries);

	return (err) ? TC_ERR : TC_OK;
}

int
tc_api_check_cipher(tc_api_op *api_opts)
{
	struct tc_cipher_chain *chain;

	if (api_opts == NULL || api_opts->tc_cipher == NULL)
		errno = EFAULT;
		return TC_ERR;
	}

	if ((chain = check_cipher_chain(api_opts->tc_cipher, 1)) != NULL)
		return TC_OK;

	errno = ENOENT;
	return TC_ERR;
}

int
tc_api_check_prf_hash(tc_api_op *api_opts)
{
	struct pbkdf_prf_algo *prf_hash;

	if (api_opts == NULL || api_opts->tc_prf_hash == NULL)
		errno = EFAULT;
		return TC_ERR;
	}

	if ((prf_hash = check_prf_algo(api_opts->tc_prf_hash, 1)) != NULL)
		return TC_OK;

	errno = ENOENT;
	return TC_ERR;
}
