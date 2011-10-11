#include <cg/cg.h>
#include "checkForError.h"
#include <stdio.h>
#include <stdlib.h>

void checkForCgError(const char *situation)
{
  CGerror error;
  const char *string = cgGetLastErrorString(&error);

  if (error != CG_NO_ERROR) {
    printf("%s:%s\n",
      situation, string);
    if (error == CG_COMPILER_ERROR) {
      //printf("%s\n", cgGetLastListing(myCgContext));
		printf("complier error");
    }
    exit(1);
  }
}

