
/*
Copyright (c) 2009, Schuyler Erle.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <pebble.h>
#include "morpheuz.h"

#ifdef VOICE_SUPPORTED

# ifdef LEV_CASE_INSENSITIVE
# define eq(x, y) (tolower(x) == tolower(y))
# else
# define eq(x, y) ((x) == (y))
# endif

#define min(x, y) ((x) < (y) ? (x) : (y))

unsigned int levenshtein (const char *word1, const char *word2) {
    unsigned int len1 = strlen(word1),
                 len2 = strlen(word2);
    unsigned int *v = calloc(len2 + 1, sizeof(unsigned int));
    unsigned int i, j, current, next = 0, cost;

    /* strip common prefixes */
    while (len1 > 0 && len2 > 0 && eq(word1[0], word2[0]))
        word1++, word2++, len1--, len2--;

    /* handle degenerate cases */
    if (!len1) return len2;
    if (!len2) return len1;

    /* initialize the column vector */
    for (j = 0; j < len2 + 1; j++)
        v[j] = j;

    for (i = 0; i < len1; i++) {
        /* set the value of the first row */
        current = i + 1;
        /* for each row in the column, compute the cost */
        for (j = 0; j < len2; j++) {
            /*
             * cost of replacement is 0 if the two chars are the same, or have
             * been transposed with the chars immediately before. otherwise 1.
             */
            cost = !(eq(word1[i], word2[j]) || (i && j &&
                     eq(word1[i-1], word2[j]) && eq(word1[i],word2[j-1])));
            /* find the least cost of insertion, deletion, or replacement */
            next = min(min( v[j+1] + 1,
                            current + 1 ),
                            v[j] + cost );
            /* stash the previous row's cost in the column vector */
            v[j] = current;
            /* make the cost of the next transition current */
            current = next;
        }
        /* keep the final cost at the bottom of the column */
        v[len2] = next;
    }
    free(v);
    return next;
}

#endif

