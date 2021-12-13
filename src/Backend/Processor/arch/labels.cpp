//
// Created by ivanbrekman on 24.10.2021.
//

#include <cstdio>
#include <cstring>
#include <cassert>

#include "../../../../libs/baselib.h"

#include "labels.h"

int     labels_ctor(      Labels* lab) {
    assert(VALID_PTR(lab) && "Invalid lab ptr");

    for (int i = 0; i < MAX_LABELS; i++) {
        lab->labels[i] =        poisons::UNINITIALIZED_INT;
        lab->names[i]  = (char*)poisons::UNINITIALIZED_PTR;
    }

    return MAX_LABELS;
}
int     labels_dtor(      Labels* lab) {
    assert(VALID_PTR(lab) && "Invalid lab ptr");
    
    for (int i = 0; i < MAX_LABELS; i++) {
        lab->labels[i] =        poisons::FREED_ELEMENT;
        lab->names[i]  = (char*)poisons::FREED_PTR;
    }

    return MAX_LABELS;
}

int    print_labels(const Labels* lab) {
    assert(VALID_PTR(lab) && "Invalid lab ptr");
    
    for (int i = 0; i < lab->labels_count; i++) {
        printf("%-10s: %d", lab->names[i], lab->labels[i]);

        if      (lab->labels[i] == poisons::UNINITIALIZED_INT) printf(" (uninitialized)");
        else if (lab->labels[i] == poisons::FREED_ELEMENT)     printf(" (freed)");

        printf("\n");
    }

    return lab->labels_count;
}

int  get_lab_by_val(const Labels* lab, int val) {
    assert(VALID_PTR(lab));
    
    for (int i = 0; i < lab->labels_count; i++) {
        if (lab->labels[i] == val) {
            return i;
        }
    }

    return -1;
}
int get_lab_by_name(const Labels* lab, const char* name) {
    assert(VALID_PTR(lab)  && "Invalid lab ptr");
    assert(VALID_PTR(name) && "Invalid name ptr");
    
    for (int i = 0; i < lab->labels_count; i++) {
        if (strcmp(lab->names[i], name) == 0) {
            return i;
        }
    }

    return -1;
}
int   uniq_lab_name(const Labels* lab, const char* name) {
    assert(VALID_PTR(lab)  && "Invalid lab ptr");
    assert(VALID_PTR(name) && "Invalid name ptr");

    for (int i = 0; i < lab->labels_count; i++) {
        if (strcmp(lab->names[i], name) == 0) {
            return 0;
        }
    }
    
    return 1;
}
int  possible_label(const Labels* lab, char* string) {
    assert(VALID_PTR(lab)    && "Invalid lab ptr");
    assert(VALID_PTR(string) && "Invalid string ptr");

    int length = (int)strlen(string);
    if (string[length - 1] != ':') {
        return -1;
    }

    string[length - 1] = '\0';
    int uniq = uniq_lab_name(lab, string);
    string[length - 1] = ':';

    return uniq;
}

int     write_label(      Labels* lab, const char* name, int label) {
    assert(VALID_PTR(lab)  && "Invalid lab ptr");
    assert(VALID_PTR(name) && "Invalid name ptr");
    assert((label >= 0 || label == poisons::UNINITIALIZED_INT) && "Incorrect label (label must be >= 0)");
    
    int lab_index = get_lab_by_name(lab, name);
    if (lab_index == -1) {                              // if it is new label
        lab->names[lab->labels_count]    = name;
        lab->labels[lab->labels_count++] = label;
    } else {
        lab->labels[lab_index]           = label;
    }

    return lab_index == -1 ? lab->labels[lab->labels_count - 1] : lab->labels[lab_index];
}
int      read_label(const Labels* lab, const char* name) {
    assert(VALID_PTR(lab)  && "Invalid lab ptr");
    assert(VALID_PTR(name) && "Invalid name ptr");
    
    int lab_index = get_lab_by_name(lab, name);
    
    return can_read_label(lab, lab_index) ? lab->labels[lab_index] : poisons::UNINITIALIZED_INT;
}
int  can_read_label(const Labels* lab, int index) {
    assert(VALID_PTR(lab) && "Invalid lab ptr");
    
    if (index < 0 || index >= lab->labels_count) {
        return 0;
    }
    if (!VALID_PTR(lab->names[index]) || lab->names[index][0] == '\0') {
        return 0;
    }

    return lab->labels[index] != poisons::UNINITIALIZED_INT && lab->labels[index] != poisons::FREED_ELEMENT;
}
