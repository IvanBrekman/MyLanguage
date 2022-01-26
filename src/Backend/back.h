//
// Created by IvanBrekman on 13.12.2021
//

#ifndef BACKH
#define BACKH

#include "config.h"

#include "libs/baselib.h"
#include "../Frontend/SyntaxAnalyzer/anal.h"

struct BackContext {
    int result = -1;
};

BackContext* Back(const FrontContext* context);

#endif // BACKH
