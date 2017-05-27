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


#ifndef JSON_H
#define JSON_H

#include <string.h>

#define JSON_TYPE_NULL      0
#define JSON_TYPE_BOOLEAN   1
#define JSON_TYPE_NUMBER    2
#define JSON_TYPE_STRING    3
#define JSON_TYPE_BRACKET   4
#define JSON_TYPE_BRACE     5

#define JSON_OK                0
#define JSON_ERROR_UNEXPECTED  1
#define JSON_ERROR_NO_KEY      2
#define JSON_ERROR_NO_VALUE    3

#define JSON_FORMAT_SERIAL   -1
#define JSON_FORMAT_IDENT    0


// JSON data tree node structure/object
typedef struct _JSONNode {
  unsigned long long type:3, childrenCount:61;
  char* key;
  union 
  {
    struct _JSONNode** childrenList;
    char *value;
  };
} 
JSONNode;

#define JSON_IS_INTERNAL( node ) ( (node)->type == JSON_TYPE_BRACKET || (node)->type == JSON_TYPE_BRACE )

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generate JSON tree data structure from a serialized JSON string
 * 
 * @param jsonString serialized JSON string
 * 
 * @return reference/pointer to root node of generated JSON tree data structure
 */
JSONNode* JSON_Parse( const char* jsonString );

/**
 * Create root/base JSON node of given type
 * 
 * @param type enum value defining node type (JSON_TYPE_{NULL,BOOLEAN,NUMBER,STRING,BRACKET,BRACE})
 * @param key string key to index the node. NULL for node without key
 * 
 * @return reference/pointer to created node. NULL on errors
 */
JSONNode* JSON_Create( long type, const char* key );

/**
 * Set value of given JSON node
 * 
 * @param root node for which the value will be set
 * @param value node value in string form
 */
void JSON_Set( JSONNode* root, const char* value );
    
/**
 * Clear value of given node or destroy its children, if internal
 * 
 * @param root node from which the clearing starts
 */
void JSON_Clear( JSONNode* root );
    
/**
 * Destroy given node and its children, if any
 * 
 * @param root node to be destroyed
 */
void JSON_Destroy( JSONNode* root );
    
/**
 * Find node (inside a BRACE type node) by its key
 * 
 * @param root pointer to the node (BRACE type) where search will be performed
 * @param key key of searched node
 * 
 * @return reference/pointer to found node. NULL if nothing is found
 */
JSONNode* JSON_FindByKey( const JSONNode* root, const char* key );
    
/**
 * Find node (inside a BRACKET type node) by its index
 * 
 * @param root pointer to the node (BRACKET type) where search will be performed
 * @param i index of searched node
 * 
 * @return reference/pointer to found node. NULL if nothing is found
 */
JSONNode* JSON_FindByIndex( const JSONNode* root, long i );
    
/**
 * Find node following a sequence of keys and/or indexes
 * 
 * @param root pointer to the node from where search will be performed
 * @param pathArgsCount number of parameters for the subsequent variable lenght arguments list
 * 
 * @return reference/pointer to found node. NULL if nothing is found
 */
JSONNode* JSON_FindByPath( const JSONNode* root, int pathArgsCount, ... );
    
/**
 * Display JSON data tree as a formatted string
 * 
 * @param root root/base node of the tree to be displayed
 */
void JSON_Print( const JSONNode* root );
    
/**
 * Write JSON data tree to a string
 * 
 * @param root root/base node of the tree to be written
 * @param mode format of the string representation. Serialized (JSON_FMT_SERIAL) or idented (JSON_FMT_IDENT)
 * 
 * @return reference/pointer to allocated string containing JSON data. Must be freed manually
 */
char* JSON_GetString( const JSONNode* root, int mode );
    
/**
 * Add JSON node to BRACE type node
 * 
 * @param root parent BRACE type node to which new node will be added
 * @param type type of new child node to be added
 * @param key key of new child node to be added
 * 
 * @return reference/pointer to newly created child node. NULL on errors
 */
JSONNode* JSON_AddKey( JSONNode* root, long type, const char* key );
    
/**
 * Append JSON node to BRACKET type node
 * 
 * @param root parent BRACKET type node to which new node will be appended
 * @param type type of new child node to be appended
 * 
 * @return reference/pointer to newly created child node. NULL on errors
 */
JSONNode* JSON_AddIndex( JSONNode* root, long type );

#ifdef __cplusplus
}
#endif

#endif
