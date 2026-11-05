/*
    Copyright (C) 2025-2026, The AROS Development Team. All rights reserved.
*/

/* Functions in this include should be used in rest of posixc.library to check
   if an optional library is available; the library is opened on first use
   and closed when posixc.library is closed. */

struct PosixCIntBase;

int __dos64_available(struct PosixCIntBase *PosixCBase);
