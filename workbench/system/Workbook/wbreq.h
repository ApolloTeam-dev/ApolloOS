//
// Created by jarokuczi on 28/04/2019.
//

#ifndef WORKBOOK_WBREQ_H
#define WORKBOOK_WBREQ_H

#include "workbook_intern.h"

#define GAD_TEXT    (0)
#define GAD_PROCEED    (1)
#define GAD_CANCEL    (2)

char *RequestText(char windowTitle[], char inputLabel[], char buttonTitle[], struct WorkbookBase *wbBase);

#endif //WORKBOOK_WBREQ_H
