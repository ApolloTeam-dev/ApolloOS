/*
    Copyright (C) 1995-2020, The AROS Development Team. All rights reserved.

    Desc:
*/

#include "datatypes_intern.h"
#include <datatypes/datatypes.h>
#include <proto/exec.h>
//#define DEBUG 1
#include <aros/debug.h>

/*****************************************************************************

    NAME */
#include <proto/datatypes.h>

        AROS_LH1(VOID, LockDataType,

/*  SYNOPSIS */
        AROS_LHA(struct DataType *, dt, A0),

/*  LOCATION */
        struct Library *, DataTypesBase, 40, DataTypes)

/*  FUNCTION

    Lock a DataType structure obtained from ObtainDataTypeA() or a data type
    object (DTA_DataType).

    INPUTS

    dt  --  DataType structure; may be NULL

    RESULT

    NOTES

    Calls to LockDataType() and ObtainDataTypeA() must have a corresponding
    ReleaseDataType() call or else problems will arise.

    EXAMPLE

    BUGS

    SEE ALSO

    ObtainDataTypeA(), ReleaseDataType()

    INTERNALS

    HISTORY

    2.8.99  SDuvan  implemented

*****************************************************************************/
{
    AROS_LIBFUNC_INIT

    D(bug("datatypes.library/LockDataType: Entered with datatype(%x)\n", dt));
    
    if(dt == NULL || dt->dtn_Length == 0)
        return;

    ObtainSemaphoreShared(&(GPB(DataTypesBase)->dtb_DTList)->dtl_Lock);

    D(bug("datatypes.library/LockDataType: Datatype %x OpenCount being incremented to %d\n", dt, ((struct CompoundDataType *)dt)->OpenCount + 1));
    ((struct CompoundDataType *)dt)->OpenCount++;
    
    ReleaseSemaphore(&(GPB(DataTypesBase)->dtb_DTList)->dtl_Lock);

    AROS_LIBFUNC_EXIT
} /* LockDataType */
