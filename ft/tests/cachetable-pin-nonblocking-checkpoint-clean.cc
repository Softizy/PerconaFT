/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- */
// vim: ft=cpp:expandtab:ts=8:sw=4:softtabstop=4:
#ident "$Id$"
/*
COPYING CONDITIONS NOTICE:

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation, and provided that the
  following conditions are met:

      * Redistributions of source code must retain this COPYING
        CONDITIONS NOTICE, the COPYRIGHT NOTICE (below), the
        DISCLAIMER (below), the UNIVERSITY PATENT NOTICE (below), the
        PATENT MARKING NOTICE (below), and the PATENT RIGHTS
        GRANT (below).

      * Redistributions in binary form must reproduce this COPYING
        CONDITIONS NOTICE, the COPYRIGHT NOTICE (below), the
        DISCLAIMER (below), the UNIVERSITY PATENT NOTICE (below), the
        PATENT MARKING NOTICE (below), and the PATENT RIGHTS
        GRANT (below) in the documentation and/or other materials
        provided with the distribution.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

COPYRIGHT NOTICE:

  TokuFT, Tokutek Fractal Tree Indexing Library.
  Copyright (C) 2007-2013 Tokutek, Inc.

DISCLAIMER:

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

UNIVERSITY PATENT NOTICE:

  The technology is licensed by the Massachusetts Institute of
  Technology, Rutgers State University of New Jersey, and the Research
  Foundation of State University of New York at Stony Brook under
  United States of America Serial No. 11/760379 and to the patents
  and/or patent applications resulting from it.

PATENT MARKING NOTICE:

  This software is covered by US Patent No. 8,185,551.
  This software is covered by US Patent No. 8,489,638.

PATENT RIGHTS GRANT:

  "THIS IMPLEMENTATION" means the copyrightable works distributed by
  Tokutek as part of the Fractal Tree project.

  "PATENT CLAIMS" means the claims of patents that are owned or
  licensable by Tokutek, both currently or in the future; and that in
  the absence of this license would be infringed by THIS
  IMPLEMENTATION or by using or running THIS IMPLEMENTATION.

  "PATENT CHALLENGE" shall mean a challenge to the validity,
  patentability, enforceability and/or non-infringement of any of the
  PATENT CLAIMS or otherwise opposing any of the PATENT CLAIMS.

  Tokutek hereby grants to you, for the term and geographical scope of
  the PATENT CLAIMS, a non-exclusive, no-charge, royalty-free,
  irrevocable (except as stated in this section) patent license to
  make, have made, use, offer to sell, sell, import, transfer, and
  otherwise run, modify, and propagate the contents of THIS
  IMPLEMENTATION, where such license applies only to the PATENT
  CLAIMS.  This grant does not include claims that would be infringed
  only as a consequence of further modifications of THIS
  IMPLEMENTATION.  If you or your agent or licensee institute or order
  or agree to the institution of patent litigation against any entity
  (including a cross-claim or counterclaim in a lawsuit) alleging that
  THIS IMPLEMENTATION constitutes direct or contributory patent
  infringement, or inducement of patent infringement, then any rights
  granted to you under this License shall terminate as of the date
  such litigation is filed.  If you or your agent or exclusive
  licensee institute or order or agree to the institution of a PATENT
  CHALLENGE, then Tokutek may terminate any rights granted to you
  under this License.
*/

#ident "Copyright (c) 2007-2013 Tokutek Inc.  All rights reserved."

#include "test.h"
#include "cachetable-test.h"

CACHETABLE ct;

CACHEFILE f1;

static void
run_test (void) {
    const int test_limit = 20;
    int r;
    ct = NULL;
    toku_cachetable_create(&ct, test_limit, ZERO_LSN, nullptr);
    const char *fname1 = TOKU_TEST_FILENAME;
    toku_os_unlink(fname1);
    f1 = NULL;
    r = toku_cachetable_openf(&f1, ct, fname1, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO); assert(r == 0);
    create_dummy_functions(f1);

    void* v1;
    void* v2;
    long s1;
    long s2;
    
    r = toku_cachetable_get_and_pin(f1, make_blocknum(1), 1, &v1, &s1, def_write_callback(NULL), def_fetch, def_pf_req_callback, def_pf_callback, true, NULL);
    r = toku_test_cachetable_unpin(f1, make_blocknum(1), 1, CACHETABLE_CLEAN, make_pair_attr(8)); assert(r==0);

    for (int i = 0; i < 20; i++) {
        r = toku_cachetable_get_and_pin(f1, make_blocknum(2), 2, &v2, &s2, def_write_callback(NULL), def_fetch, def_pf_req_callback, def_pf_callback, true, NULL);
        r = toku_test_cachetable_unpin(f1, make_blocknum(2), 2, CACHETABLE_CLEAN, make_pair_attr(8)); assert(r==0);
    }

    r = toku_cachetable_get_and_pin(f1, make_blocknum(1), 1, &v2, &s2, def_write_callback(NULL), def_fetch, def_pf_req_callback, def_pf_callback, true, NULL);
    CHECKPOINTER cp = toku_cachetable_get_checkpointer(ct);
    toku_cachetable_begin_checkpoint(cp, NULL);
    // mark nodes as pending a checkpoint, so that get_and_pin_nonblocking on block 1 will return TOKUDB_TRY_AGAIN
    r = toku_test_cachetable_unpin(f1, make_blocknum(1), 1, CACHETABLE_CLEAN, make_pair_attr(8)); assert(r==0);

    r = toku_cachetable_get_and_pin_nonblocking(
        f1,
        make_blocknum(1),
        1,
        &v1,
        &s1,
        def_write_callback(NULL),
        def_fetch,
        def_pf_req_callback,
        def_pf_callback,
        PL_WRITE_EXPENSIVE,
        NULL,
        NULL
        );
    assert(r==0);
    r = toku_test_cachetable_unpin(f1, make_blocknum(1), 1, CACHETABLE_CLEAN, make_pair_attr(8)); assert(r==0);
    
    toku_cachetable_end_checkpoint(
        cp, 
        NULL, 
        NULL,
        NULL
        );
    assert(r==0);
    
    toku_cachetable_verify(ct);
    toku_cachefile_close(&f1, false, ZERO_LSN);
    toku_cachetable_close(&ct);
    
    
}

int
test_main(int argc, const char *argv[]) {
  default_parse_args(argc, argv);
  run_test();
  return 0;
}
