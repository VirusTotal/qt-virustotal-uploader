/*
Copyright 2014 VirusTotal S.L. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef VT_LOG_H
#define VT_LOG_H

#ifdef  __cplusplus
extern "C" {
#endif

enum {
     VT_LOG_EMERG,   // system is unusable
     VT_LOG_ALERT,   // action must be taken immediately
     VT_LOG_CRIT,    // critical conditions
     VT_LOG_ERR,     // error conditions
     VT_LOG_WARNING, // warning conditions
     VT_LOG_NOTICE,  // normal but significant condition
     VT_LOG_INFO,    // informational
     VT_LOG_DEBUG };

#endif

#ifdef  __cplusplus
}
#endif /*cplusplus*/
