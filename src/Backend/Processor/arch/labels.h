//
// Created by ivanbrekman on 24.10.2021.
//

#ifndef SIMPLEPROCESSOR_LABELS_H
#define SIMPLEPROCESSOR_LABELS_H

#include "helper.h"

struct Labels {
    int        labels[MAX_LABELS] = { };        // Labels values
    const char* names[MAX_LABELS] = { };        // Labels names

    int labels_count = 0;
};

// Labels constructor/deconstructor--------------------------------------------
int     labels_ctor(      Labels* lab);
int     labels_dtor(      Labels* lab);
// ----------------------------------------------------------------------------

int    print_labels(const Labels* lab);

// Functions to get label index by some values (label value or name)-----------
int  get_lab_by_val(const Labels* lab, int val);
int get_lab_by_name(const Labels* lab, const char* name);           /* !NOTE! This function works at O(n^2) */
// ----------------------------------------------------------------------------

int   uniq_lab_name(const Labels* lab, const char* name);           /* return 1 if name is unique in lab, else 0 */
int  possible_label(const Labels* lab,       char* string);         /* return 1 if string can be label name in lab, else if it isn`t unique 0, else reasons -1 */

// Functions for interaction with registers------------------------------------
int     write_label(      Labels* lab, const char* name, int label);
int      read_label(const Labels* lab, const char* name);
// ----------------------------------------------------------------------------

//! Function checks opportunity to read value from label
//! \param reg       ptr to Labels object
//! \param reg_index index of label, which we checks
//! \return          1 if we can read from label, else 0
//! \note function checks correct index, then correct label name by this label, yhen correct value by this index
int  can_read_label(const Labels* lab, const int index);

#endif //SIMPLEPROCESSOR_LABELS_H
