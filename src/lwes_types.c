/*======================================================================*
 * Copyright (c) 2008, Yahoo! Inc. All rights reserved.                 *
 *                                                                      *
 * Licensed under the New BSD License (the "License"); you may not use  *
 * this file except in compliance with the License.  Unless required    *
 * by applicable law or agreed to in writing, software distributed      *
 * under the License is distributed on an "AS IS" BASIS, WITHOUT        *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     *
 * See the License for the specific language governing permissions and  *
 * limitations under the License. See accompanying LICENSE file.        *
 *======================================================================*/

#include "lwes_types.h"
#include "lwes_event.h"
#include "lwes_hash.h"

#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

/* maximum datagram size for UDP is 64K minus IP layer overhead which is
   20 bytes for IP header, and 8 bytes for UDP header, so this value
   should be

   65535 - 28 = 65507
 */
const size_t MAX_MSG_SIZE = 65507;
const size_t MAX_QUEUED_ELEMENTS = 10000;


#define TYPE_STRINGS(typ,str)                                                            \
  const LWES_SHORT_STRING LWES_##typ##_STRING         = (LWES_SHORT_STRING)str;          \
  const LWES_SHORT_STRING LWES_##typ##_ARRAY_STRING   = (LWES_SHORT_STRING)str "_array"; \
  const LWES_SHORT_STRING LWES_N_##typ##_ARRAY_STRING = (LWES_SHORT_STRING)"nullable_" str "_array";

const LWES_SHORT_STRING LWES_UNDEFINED_STRING         = (LWES_SHORT_STRING)"undef";
TYPE_STRINGS(U_INT_16, "uint16");
TYPE_STRINGS(INT_16,   "int16");
TYPE_STRINGS(U_INT_32, "uint32");
TYPE_STRINGS(INT_32,   "int32");
TYPE_STRINGS(STRING,   "string");
TYPE_STRINGS(IP_ADDR,  "ip_addr");
TYPE_STRINGS(INT_64,   "int64");
TYPE_STRINGS(U_INT_64, "uint64");
TYPE_STRINGS(BOOLEAN,  "boolean");
TYPE_STRINGS(BYTE,     "byte");
TYPE_STRINGS(FLOAT,    "float");
TYPE_STRINGS(DOUBLE,   "double");


const LWES_SHORT_STRING LWES_META_INFO_STRING=(LWES_SHORT_STRING)"MetaEventInfo";

#define TYPE_TO_STR(typ)                                          \
  case LWES_TYPE_##typ:       return LWES_##typ##_STRING;         \
  case LWES_TYPE_##typ##_ARRAY: return LWES_##typ##_ARRAY_STRING; \
  case LWES_TYPE_N_##typ##_ARRAY: return LWES_N_##typ##_ARRAY_STRING;

LWES_CONST_SHORT_STRING
lwes_type_to_string
  (LWES_TYPE type)
{
  switch (type) {
    TYPE_TO_STR(U_INT_16)
    TYPE_TO_STR(INT_16)
    TYPE_TO_STR(U_INT_32)
    TYPE_TO_STR(INT_32)
    TYPE_TO_STR(U_INT_64)
    TYPE_TO_STR(INT_64)
    TYPE_TO_STR(BOOLEAN)
    TYPE_TO_STR(IP_ADDR)
    TYPE_TO_STR(STRING)
    TYPE_TO_STR(BYTE)
    TYPE_TO_STR(FLOAT)
    TYPE_TO_STR(DOUBLE)
    default: return LWES_UNDEFINED_STRING;
  }
}

#define STR_TO_TYPE(typ)                                                                \
  if (0 == strcmp(LWES_##typ##_STRING, type)) { return LWES_TYPE_##typ; }               \
  if (0 == strcmp(LWES_##typ##_ARRAY_STRING, type)) { return LWES_TYPE_##typ##_ARRAY; } \
  if (0 == strcmp(LWES_N_##typ##_ARRAY_STRING, type)) { return LWES_TYPE_N_##typ##_ARRAY; }

LWES_TYPE
lwes_string_to_type
  (LWES_SHORT_STRING type)
{
  STR_TO_TYPE(U_INT_16)
  STR_TO_TYPE(INT_16)
  STR_TO_TYPE(U_INT_32)
  STR_TO_TYPE(INT_32)
  STR_TO_TYPE(U_INT_64)
  STR_TO_TYPE(INT_64)
  STR_TO_TYPE(BOOLEAN)
  STR_TO_TYPE(IP_ADDR)
  STR_TO_TYPE(STRING)
  STR_TO_TYPE(BYTE)
  STR_TO_TYPE(FLOAT)
  STR_TO_TYPE(DOUBLE)
  return LWES_TYPE_UNDEFINED;
}

/* NOTE: this intentionally returns the unit-size for array types */
#define TYPE_TO_SIZE(typ)                                    \
  case LWES_TYPE_##typ:           return sizeof(LWES_##typ); \
  case LWES_TYPE_##typ##_ARRAY:   return sizeof(LWES_##typ); \
  case LWES_TYPE_N_##typ##_ARRAY: return sizeof(LWES_##typ);

int
lwes_type_to_size
  (LWES_TYPE type)
{
  switch (type) {
    TYPE_TO_SIZE(U_INT_16)
    TYPE_TO_SIZE(INT_16)
    TYPE_TO_SIZE(U_INT_32)
    TYPE_TO_SIZE(INT_32)
    TYPE_TO_SIZE(U_INT_64)
    TYPE_TO_SIZE(INT_64)
    TYPE_TO_SIZE(BOOLEAN)
    TYPE_TO_SIZE(IP_ADDR)
    TYPE_TO_SIZE(BYTE)
    TYPE_TO_SIZE(FLOAT)
    TYPE_TO_SIZE(DOUBLE)
    case LWES_TYPE_STRING         : return sizeof(LWES_SHORT_STRING);
    case LWES_TYPE_STRING_ARRAY   : return sizeof(LWES_SHORT_STRING);
    case LWES_TYPE_N_STRING_ARRAY : return sizeof(LWES_SHORT_STRING);
    default: return 0;
  }
}

LWES_BOOLEAN
lwes_type_is_array(LWES_TYPE typ)
{
  return ((typ!= LWES_TYPE_UNDEFINED) &&
          (typ >= LWES_TYPE_U_INT_16_ARRAY) )
          ? TRUE : FALSE;
}

LWES_BOOLEAN
lwes_type_is_nullable_array(LWES_TYPE typ)
{
  return ((typ!= LWES_TYPE_UNDEFINED) &&
          (typ >= LWES_TYPE_N_U_INT_16_ARRAY))
          ? TRUE : FALSE;
}

LWES_TYPE
lwes_array_type_to_base(LWES_TYPE typ)
{
  if (!lwes_type_is_array(typ))
    { 
      return LWES_TYPE_UNDEFINED; 
    }
  if (lwes_type_is_nullable_array(typ))
    {
      return (typ-LWES_TYPE_N_U_INT_16_ARRAY)+LWES_TYPE_U_INT_16;
    }
  else
    {
      return (typ-LWES_TYPE_U_INT_16_ARRAY)+LWES_TYPE_U_INT_16;
    }
}

int
lwes_typed_value_to_stream
  (LWES_TYPE type,
   void* value,
   FILE *stream)
{
  void* v = value;

  if (lwes_type_is_array(type)) {
    /* can't process it without an array size */
    return 0;
  }

  switch(type) {
    case LWES_TYPE_U_INT_16: return fprintf(stream,"%hu",     *(LWES_U_INT_16*)v);
    case LWES_TYPE_INT_16:   return fprintf(stream,"%hd",     *(LWES_INT_16*)v);
    case LWES_TYPE_U_INT_32: return fprintf(stream,"%u",      *(LWES_U_INT_32*)v);
    case LWES_TYPE_INT_32:   return fprintf(stream,"%d",      *(LWES_INT_32*)v);
    case LWES_TYPE_U_INT_64: return fprintf(stream,"%"PRIu64, *(LWES_U_INT_64*)v);
    case LWES_TYPE_INT_64:   return fprintf(stream,"%"PRId64, *(LWES_INT_64*)v);
    case LWES_TYPE_BOOLEAN:  return fprintf(stream,"%s",      (1==*(LWES_BOOLEAN*)v)?"true":"false");
    case LWES_TYPE_BYTE:     return fprintf(stream,"%u",     (unsigned int)*(LWES_BYTE*)v);
    case LWES_TYPE_FLOAT:    return fprintf(stream,"%f",     *(LWES_FLOAT*)v);
    case LWES_TYPE_DOUBLE:   return fprintf(stream,"%lf",    *(LWES_DOUBLE*)v);
    case LWES_TYPE_IP_ADDR:  return fprintf(stream,"%s",      inet_ntoa(*(LWES_IP_ADDR *)v));
    case LWES_TYPE_STRING:   return fprintf(stream,"\"%s\"",  (LWES_LONG_STRING)v);
    default: return 0; //fprintf(stream, "<UNKNOWN_FIELD_TYPE>");
  }
  return 0;
}

int
lwes_typed_array_to_stream
  (LWES_TYPE type,
   void* value,
   int size,
   FILE *stream)
{
  char* v = value;
  int i, skip;
  int ret=0;
  LWES_TYPE baseType;
  LWES_BOOLEAN nullable;

  if (!lwes_type_is_array(type)) {
    return 0;
  }
  nullable = lwes_type_is_nullable_array(type);
  baseType = lwes_array_type_to_base(type);
  skip = lwes_type_to_size(type);
  fprintf(stream,"[ ");
  for (i=0; i<size; ++i)
    {
      if (nullable)
        {
          char* ptr = ((char**)value)[i];
          if (NULL == ptr)
            {
              // TODO print explicit 'null' ?
            }
          else if (LWES_TYPE_STRING == baseType)
            {
                ret += lwes_typed_value_to_stream(baseType, ptr, stream);
            }
          else
            {
              ret += lwes_typed_value_to_stream(baseType, ptr, stream);
            }
        }
      else if (LWES_TYPE_STRING == baseType)
        {
          char** ptr = (char**)(void*)v;
          ret += lwes_typed_value_to_stream(baseType, ptr[i], stream);
        } 
      else
        {
          ret += lwes_typed_value_to_stream(baseType, v+(i*skip), stream);
        }
      if (i<size-1)
        { fprintf(stream,", "); }
    }
  fprintf(stream," ]");
  return ret;
}


int
lwes_event_attribute_to_stream
  (struct lwes_event_attribute *attribute,
   FILE *stream)
{
  void* val = attribute->value;
  if (lwes_type_is_array(attribute->type))
    {
      return lwes_typed_array_to_stream(attribute->type, val, attribute->array_len, stream);
    }
  else
    {
      return lwes_typed_value_to_stream(attribute->type, val, stream);
    }
}

int
lwes_event_to_stream
  (struct lwes_event *event,
   FILE *stream)
{
  struct lwes_event_attribute *tmp;
  struct lwes_hash_enumeration e;

  fprintf (stream, "%s", event->eventName);
  fprintf (stream,"[");
  fflush (stream);
  lwes_typed_value_to_stream (LWES_TYPE_U_INT_16, (void*)&event->number_of_attributes, stream);
  fprintf (stream,"]");
  fflush (stream);
  fprintf (stream,"\n");
  fflush (stream);
  fprintf (stream,"{\n");
  fflush (stream);

  if (lwes_hash_keys (event->attributes, &e))
    {
      while (lwes_hash_enumeration_has_more_elements (&e))
        {
          LWES_SHORT_STRING tmpAttrName =
            lwes_hash_enumeration_next_element (&e);

          tmp =
            (struct lwes_event_attribute *)lwes_hash_get (event->attributes,
                                                          tmpAttrName);

          fprintf (stream,"\t");
          fflush (stream);
          fprintf (stream, "%s", tmpAttrName);
          fflush (stream);
          fprintf (stream," = ");
          fflush (stream);
          lwes_event_attribute_to_stream (tmp, stream);
          fflush (stream);
          fprintf (stream,";\n");
          fflush (stream);
        }
    }
  fprintf (stream,"}\n");
  fflush (stream);
  return 0;
}


