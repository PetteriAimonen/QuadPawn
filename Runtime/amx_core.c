/* Only the most important functions from the official amxcore.c */

#include "amx.h"

typedef unsigned char uchar;

static cell AMX_NATIVE_CALL numargs(AMX *amx,const cell *params)
{
  AMX_HEADER *hdr;
  uchar *data;
  cell bytes;

  (void)params;
  hdr=(AMX_HEADER *)amx->base;
  data=amx->data ? amx->data : amx->base+(int)hdr->dat;
  /* the number of bytes is on the stack, at "frm + 2*cell" */
  bytes= * (cell *)(data+(int)amx->frm+2*sizeof(cell));
  /* the number of arguments is the number of bytes divided
   * by the size of a cell */
  return bytes/sizeof(cell);
}

static cell AMX_NATIVE_CALL getarg(AMX *amx,const cell *params)
{
  AMX_HEADER *hdr;
  uchar *data;
  cell value;

  hdr=(AMX_HEADER *)amx->base;
  data=amx->data ? amx->data : amx->base+(int)hdr->dat;
  /* get the base value */
  value= * (cell *)(data+(int)amx->frm+((int)params[1]+3)*sizeof(cell));
  /* adjust the address in "value" in case of an array access */
  value+=params[2]*sizeof(cell);
  /* get the value indirectly */
  value= * (cell *)(data+(int)value);
  return value;
}

static cell AMX_NATIVE_CALL setarg(AMX *amx,const cell *params)
{
  AMX_HEADER *hdr;
  uchar *data;
  cell value;

  hdr=(AMX_HEADER *)amx->base;
  data=amx->data ? amx->data : amx->base+(int)hdr->dat;
  /* get the base value */
  value= * (cell *)(data+(int)amx->frm+((int)params[1]+3)*sizeof(cell));
  /* adjust the address in "value" in case of an array access */
  value+=params[2]*sizeof(cell);
  /* verify the address */
  if (value<0 || (value>=amx->hea && value<amx->stk))
    return 0;
  /* set the value indirectly */
  * (cell *)(data+(int)value) = params[3];
  return 1;
}

static cell AMX_NATIVE_CALL heapspace(AMX *amx,const cell *params)
{
  (void)params;
  return amx->stk - amx->hea;
}

static cell AMX_NATIVE_CALL funcidx(AMX *amx,const cell *params)
{
  char name[64];
  cell *cstr;
  int index,err;

  cstr=amx_Address(amx,params[1]);
  amx_GetString(name,cstr,0,sizeof name);
  err=amx_FindPublic(amx,name,&index);
  if (err!=AMX_ERR_NONE)
    index=-1;   /* this is not considered a fatal error */
  return index;
}

static cell AMX_NATIVE_CALL amx_memset(AMX *amx,const cell *params)
{
    cell *array = (cell*)params[1];
    cell value = params[2];
    cell count = params[3];
    
    while (count--)
    {
        *array++ = value;
    }
    
    return 0;
}

static cell AMX_NATIVE_CALL amx_memcpy(AMX *amx,const cell *params)
{
    cell *dest = (cell*)params[1];
    cell *src = (cell*)params[2];
    cell count = params[3];
    
    while (count--)
    {
        *dest++ = *src++;
    }
    
    return 0;
}

int amx_CoreInit(AMX *amx)
{
    const AMX_NATIVE_INFO core_Natives[] = {
        { "numargs",       numargs },
        { "getarg",        getarg },
        { "setarg",        setarg },
        { "heapspace",     heapspace },
        { "funcidx",       funcidx },
        { "memset", amx_memset},
        { "memcpy", amx_memcpy},
        {0, 0}
    };
    
    return amx_Register(amx, core_Natives, -1);
}
