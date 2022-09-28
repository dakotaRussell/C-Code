#include <limits.h>
#include "BitUnpacker.h"
#include "MyLib.h"
#include "CodeSet.h"
#include "LZWExp.h"

#define DEFAULT_RECYCLE_CODE 4096
#define EOD 256
#define INIT_BIT_COUNT 9

#define BAD_CODE -1
#define MISSING_EOD -2

void LZWExpInit(LZWExp *exp, DataSink sink, void *sinkState, int recycleCode) {
   int initDigit = 0;
   
   exp->dict = CreateCodeSet(recycleCode + 1); 
 
   while (initDigit <= EOD) {
      exp->lastCode = NewCode(exp->dict, initDigit++);
   }

   exp->numBits = INIT_BIT_COUNT;
   exp->sink = sink;
   exp->sinkState = sinkState;
   exp->recycleCode = recycleCode;
   exp->maxCode = (1 << INIT_BIT_COUNT) - 1;
   BuInit(&(exp->bitUnpacker));
   exp->EODSeen = 0;
}

/* Break apart compressed data in "bits" into one or more codes and send 
 * the corresponding symbol sequences to the DataSink.  Save any leftover 
 * compressed bits to combine with the bits from the next call of 
 * LZWExpEncode.  Return 0 on success or BAD_CODE if you receive a code not
 * in the dictionary.
 * 
 * For this and all other methods, a code is "invalid" if it either could not
 * have been sent (e.g. is too high) or if it is a nonzero code following
 * the detection of an EOD code.
 */
int LZWExpDecode(LZWExp *exp, UInt bits) {
   UInt temp = 0;
   Code code;
   int status = 0;
   UInt recycledData = 0;
   DataSink sink = exp->sink;
   int recycleCode = exp->recycleCode;

   BuTakeData(&(exp->bitUnpacker), bits);
   
   while (BuUnpack(&(exp->bitUnpacker), exp->numBits, &temp) && status == 0) {
      if (temp == EOD) {
         exp->EODSeen = 1;
         exp->numBits = exp->bitUnpacker.bitsLeft;
      }
      else {
         if (!exp->EODSeen) {
            if (temp <= exp->lastCode) {
               code = GetCode(exp->dict, temp);
         
               if (exp->lastCode != EOD)
                  SetSuffix(exp->dict, exp->lastCode, code.data[0]);
         
               exp->sink(NULL, code.data, code.size);
               exp->lastCode = ExtendCode(exp->dict, temp);
               FreeCode(exp->dict, temp);
               
               if (exp->lastCode == exp->recycleCode) {
                  BuUnpack(&(exp->bitUnpacker), exp->bitUnpacker.bitsLeft, 
                   &recycledData);
                  DestroyCodeSet(exp->dict);
                  LZWExpInit(exp, sink, NULL, recycleCode);
                  exp->bitUnpacker.curData = recycledData;
               }

            }
            else
               status = BAD_CODE;

            if (exp->lastCode > exp->maxCode) {
               exp->numBits++;
               exp->maxCode = (1 << exp->numBits) - 1;
            }
         }
         else if (temp == 0 && exp->numBits != sizeof(UInt)) {
            exp->numBits = sizeof(UInt);
         }
         else if (temp != 0 || exp->numBits == sizeof(UInt))
            status = BAD_CODE;
      }   
   }

   return status;
   
}

/* Called after EOD has been seen to peform any error checking and/or
 ** housekeeping that should be performed at the end of decoding. Returns 0 if
 ** all is OK, NO_EOD if no terminating EOD code has been found, or BAD_CODE
 ** if either non-zero bits follow the EOD or if one or more extra intextra an int-block(s) 
 * */
int LZWExpStop(LZWExp *exp) {
   int status = 0;
   
   if (!exp->EODSeen) 
      status = MISSING_EOD;

   return status;
}

/* Free all storage associated with LZWExp (not the sinkState, though,
 *  * which is "owned" by the caller.  Must be called even if LZWExpInit
 *   * returned an error.
 *    */
void LZWExpDestruct(LZWExp *exp) {
   DestroyCodeSet(exp->dict); 
}

