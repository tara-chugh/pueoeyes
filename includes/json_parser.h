/*****************************************************************************
  This sample is released as public domain.  It is distributed in the hope it
  will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
  
  This is the sample code for Leopard USB3.0 camera, mainly for loading from 
  config.json to perform group register writes and captures.

  Author: Danyu L
  Last edit: 2019/06
*****************************************************************************/
#pragma once
#include<json-c/json.h>
#include<ctype.h>


void json_parser(int fd, char *json_buffer);

