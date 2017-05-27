/* The MIT License

   Original work Copyright (c) 2014-2016 Heng Li <lh3@me.com>

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
//  Modified work Copyright (c) 2016-2017 Leonardo Consoni <consoni_2519@hotmail.com>   //
//                                                                                      //
//  This file is part of Platform Utils.                                                //
//                                                                                      //
//  Platform Utils is free software: you can redistribute it and/or modify              //
//  it under the terms of the GNU Lesser General Public License as published            //
//  by the Free Software Foundation, either version 3 of the License, or                //
//  (at your option) any later version.                                                 //
//                                                                                      //
//  Platform Utils is distributed in the hope that it will be useful,                   //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of                      //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                        //
//  GNU Lesser General Public License for more details.                                 //
//                                                                                      //
//  You should have received a copy of the GNU Lesser General Public License            //
//  along with Platform Utils. If not, see <http://www.gnu.org/licenses/>.              //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This JSON read/write implementation is a modification of the JSON parser  //
//  from the MIT-Licensed Klib project, made by Heng Li (lh3), which can be   //
//  found at <https://github.com/attractivechaos/klib>.                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include "json.h"

const char *NULL_STR = "null";
const char *TRUE_STR = "true";
const char *FALSE_STR = "false";


JSONNode* JSON_ParseRecursive( const char** ref_jsonString, int* error )
{
  const char* ref_jsonToken;
  JSONNode* root = JSON_Create( JSON_TYPE_NULL, NULL );
  *error = JSON_OK;
  for( ref_jsonToken = *ref_jsonString; *ref_jsonToken != 0; ++ref_jsonToken ) {
    while( *ref_jsonToken && isspace( *ref_jsonToken ) ) ++ref_jsonToken;
    if( *ref_jsonToken == 0 ) break;
    if( *ref_jsonToken == ',' || *ref_jsonToken == ']' || *ref_jsonToken == '}' ) break;
    else if( *ref_jsonToken == '[' || *ref_jsonToken == '{' ) 
    {
      char delimiter = ( *ref_jsonToken == '[' ) ? ']' : '}';
      root->type = ( *(ref_jsonToken++) == '[' ) ? JSON_TYPE_BRACKET : JSON_TYPE_BRACE;
      while( *ref_jsonToken != delimiter && *error == JSON_OK ) 
      {
        JSONNode* v = JSON_ParseRecursive( &ref_jsonToken, error );
        if( v ) 
        {
          root->childrenList = (JSONNode**) realloc( root->childrenList, (size_t) ( root->childrenCount + 1 ) * sizeof(JSONNode*) );
          root->childrenList[ root->childrenCount++ ] = v;
          if( delimiter == ']' ) v->key = NULL;
        }
        if( *ref_jsonToken == ',' ) ++ref_jsonToken;
        else if( *ref_jsonToken != delimiter ) *error = JSON_ERROR_UNEXPECTED;
      }
      continue;
    } 
    else if( *ref_jsonToken == ':' ) 
    {
      if( !root->value ) 
      {
        *error = JSON_ERROR_NO_KEY; 
        break;
      }
      if( root->key ) {
      *error = JSON_ERROR_UNEXPECTED; 
      break;
      }
      root->key = root->value;
      root->value = NULL;
    } 
    else 
    {
      int c = *ref_jsonToken;
      const char* q;
      // Parse string
      if( c == '\'' || c == '"' ) 
      {
        for( q = ++ref_jsonToken; *q && *q != c; ++q )
          if( *q == '\\' ) ++q;
      } 
      else 
      {
        for( q = ref_jsonToken; *q && *q != ']' && *q != '}' && *q != ',' && *q != ':' && *q != '\n'; ++q )
          if( *q == '\\' ) ++q;
      }
      root->value = (char*) malloc( q - ref_jsonToken + 1 ); 
      strncpy( root->value, ref_jsonToken, q - ref_jsonToken ); 
      root->value[ q - ref_jsonToken ] = 0; // equivalent to u->value=strndup(p, q-p)
      if( c == '\'' || c == '"' ) root->type = JSON_TYPE_STRING; 
      else if( strcmp( root->value, NULL_STR ) == 0 ) root->type = JSON_TYPE_NULL;
      else if( strcmp( root->value, "true" ) == 0 || strcmp( root->value, "false" ) == 0) root->type = JSON_TYPE_BOOLEAN;
      else root->type = JSON_TYPE_NUMBER;
      ref_jsonToken = ( c == '\'' || c == '"' ) ? q : q - 1;
    }
  }
  *ref_jsonString = ref_jsonToken;
  if( !JSON_IS_INTERNAL( root ) ) 
  {
    if( !root->value ) 
    {
      JSON_Destroy( root ); 
      root = 0;
    }
  }
  return root;
}

JSONNode* JSON_Parse( const char *jsonString )
{
  int error;
  JSONNode* root = JSON_ParseRecursive( &jsonString, &error );
  if( !root->type || error != JSON_OK ) 
  {
    JSON_Destroy( root );
    return NULL;
  }
  return root;
}

JSONNode* JSON_Create( long type, const char* key )
{
  JSONNode* newNode = (JSONNode*) malloc( sizeof(JSONNode) );
  newNode->key = NULL;
  if( key ) 
  {
    newNode->key = (char*) calloc( strlen( key ) + 1, sizeof(char) );
    strcpy( newNode->key, key );
  }
  newNode->type = type;
  if( JSON_IS_INTERNAL( newNode ) ) newNode->childrenList = NULL;
  else newNode->value = NULL;
  newNode->childrenCount = 0;
  return newNode;
}

JSONNode* JSON_AddNode( JSONNode *root, long type, const char *key )
{
  if( root->childrenCount++ == 0 ) root->childrenList = NULL;
  root->childrenList = (JSONNode**) realloc( root->childrenList, (size_t) root->childrenCount * sizeof(JSONNode*) );
  root->childrenList[ root->childrenCount - 1 ] = JSON_Create( type, key );
  if( root->childrenList[ root->childrenCount - 1 ]->type == JSON_TYPE_NULL ) 
    JSON_Set( root->childrenList[ root->childrenCount - 1 ], NULL );
  return root->childrenList[ root->childrenCount - 1 ];
}

JSONNode* JSON_AddKey( JSONNode* root, long type, const char* key )
{
  if( root->type != JSON_TYPE_BRACE ) return NULL;
  for( long childIndex = 0; childIndex < (long) root->childrenCount; ++childIndex ) 
  {
    JSONNode* child = root->childrenList[ childIndex ];
    if( child->key != NULL && strcmp( child->key, key ) == 0 )
    return child; // (child->type == type) ? child : NULL;
  }
  return JSON_AddNode( root, type, key );
}

JSONNode* JSON_AddIndex( JSONNode* root, long type )
{
  if( root->type != JSON_TYPE_BRACKET ) return NULL;
  return JSON_AddNode( root, type, NULL );
}

void JSON_Set( JSONNode* root, const char* value )
{
  if( JSON_IS_INTERNAL( root ) ) return;
  if( root->value ) 
  {
    free( root->value );
    root->value = NULL;
  }
  if( root->type == JSON_TYPE_BOOLEAN ) value = (value) ? TRUE_STR : FALSE_STR;
  else if( root->type == JSON_TYPE_NULL ) value = NULL_STR;
  if( value ) 
  {
    root->value = (char*) realloc( root->value, ( strlen( value ) + 1 ) * sizeof(char) );
    strcpy( root->value, value );
  }
}

void JSON_Clear( JSONNode *root )
{
  if( root == NULL ) return;
  if( JSON_IS_INTERNAL( root ) ) 
  {
    for( long childIndex = 0; childIndex < (long) root->childrenCount; ++childIndex ) 
    {
      JSON_Clear( root->childrenList[ childIndex ] );
      free( root->childrenList[ childIndex ] );
    }
    if( root->childrenList ) free( root->childrenList );
    root->childrenList = NULL;
  }
  else 
  {
    if( root->value ) free( root->value );
    root->value = NULL;
  }
  root->childrenCount = 0;
}

void JSON_Destroy( JSONNode* root )
{
  if( root ) return;
  JSON_Clear( root );
  if( root->key ) free( root->key );
  root->key = NULL;
  free( root );
}

JSONNode* JSON_FindByKey( const JSONNode* root, const char* key )
{
  if( !JSON_IS_INTERNAL( root ) ) return NULL;
  for( long childIndex = 0; childIndex < (long) root->childrenCount; ++childIndex ) 
  {
    JSONNode *child = root->childrenList[ childIndex ];
    if( child->key && strcmp( child->key, key ) == 0 )
      return child;
  }
  return NULL;
}

JSONNode* JSON_FindByIndex( const JSONNode* root, long index )
{
  if( !JSON_IS_INTERNAL( root ) ) return NULL;
  return( 0 <= index && index < (long) root->childrenCount ) ? root->childrenList[ index ] : NULL;
}

JSONNode* JSON_FindByPath( const JSONNode* root, int pathArgsCount, ... )
{
  va_list pathArgsList;
  va_start( pathArgsList, pathArgsCount );
  JSONNode* child = (JSONNode*) root;
  while( child != NULL && pathArgsCount > 0 ) 
  {
    if( child->type == JSON_TYPE_BRACE ) 
      child = JSON_FindByKey( child, va_arg( pathArgsList, const char* ) );
    else if( root->type == JSON_TYPE_BRACKET )
      child = JSON_FindByIndex( child, va_arg( pathArgsList, long ) );
    else 
      break;
    --pathArgsCount;
  }
  va_end( pathArgsList );
  return child;
}

char* JSON_GetString( const JSONNode* root, int depth )
{
  size_t paddingLength = ( depth >= 0 ) ? 2 * depth : 0;               // Padding length for idented mode
  size_t jsonStringLength = paddingLength;
  char* jsonString = (char*) malloc( jsonStringLength + 1 );           // Allocate memory for null terminator
  jsonString[ 0 ] = '\0';
  for( long indentCount = 0; indentCount < depth; ++indentCount ) 
    strcat( jsonString, "  " );
  if( root->key ) 
  {
    size_t keyStringLength = strlen( root->key ) + 3;                  // Allocate memory for key string + 2 quotation marks + ':'
    jsonString = (char*) realloc( jsonString, jsonStringLength + keyStringLength + 1 );
    sprintf( jsonString + jsonStringLength, "\"%s\":", root->key );    // Copy "<key>" to the end of the string
    jsonStringLength += keyStringLength;                               // Update total JSON string length
  }
  if( root->type == JSON_TYPE_BRACKET || root->type == JSON_TYPE_BRACE ) 
  {
    jsonStringLength += 2;
    jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );  // Allocate memory for 2 brackets/braces
    strcat( jsonString, root->type == JSON_TYPE_BRACKET? "[" : "{" );  // Add only the first bracket/brace before the child roots                                                     
    if( root->childrenCount ) 
    {
      if( depth >= 0 )                                                 // Append new line between children for idented mode
      {                                          
        jsonStringLength++;
        jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );
        strcat( jsonString, "\n" );
      }
      for( long childIndex = 0; childIndex < (long) root->childrenCount; ++childIndex ) 
      {
        // Get already allocated and quoted (if needed) child strings
        char* childString = JSON_GetString( root->childrenList[ childIndex ], ( depth >= 0 ) ? depth + 1 : -1 ); 
        jsonStringLength += strlen( childString );
        jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );
        strcat( jsonString, childString );
        free( childString );                                          // Free root string allocated by recursive call
        if( childIndex + 1 < (long) root->childrenCount )             // Append comma at the end of previous child string
        {                               
          jsonStringLength++;
          jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );
          strcat( jsonString, "," );
        }
        if( depth >= 0 )                                              // Append new line between children for idented mode
        {                                      
          jsonStringLength++;
          jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );
          strcat( jsonString, "\n" );
        }
      }
      jsonStringLength += paddingLength;                              // Pad closing bracket/brace for idented mode
      jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );
      for( long indentCount = 0; indentCount < depth; ++indentCount ) 
        strcat( jsonString, "  " );
    }
    strcat( jsonString, root->type == JSON_TYPE_BRACKET? "]" : "}" ); // Add second bracket/brace after the child roots        
  } 
  else 
  {
    jsonStringLength += strlen( root->value );                        // Allocate memory for value string
    if( root->type == JSON_TYPE_STRING ) jsonStringLength += 2;       // Allocate memory for 2 quotation marks if needed
    jsonString = (char*) realloc( jsonString, jsonStringLength + 1 );
    if( root->type == JSON_TYPE_STRING ) strcat( jsonString, "\"" );
    strcat( jsonString, root->value );                                // Append value string between quotes (if needed)
    if( root->type == JSON_TYPE_STRING ) strcat( jsonString, "\"" );
  }
  jsonString[ jsonStringLength ] = '\0';
  return jsonString;
}

void JSON_Print( const JSONNode* root )
{
  char* jsonString = JSON_GetString( root, JSON_FORMAT_IDENT );
  fputs( jsonString, stdout );
  free( jsonString );
  putchar( '\n' );
}
