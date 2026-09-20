#pragma once

#define VERSION_MAJOR 2
#define VERSION_MINOR 1
#define VERSION_BUILD 0

#define PLUGIN_NAME		"Mod Explorer"
#define PLUGIN_AUTHOR	"Perchik71"

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)

#define VER_FILE_VERSION_STR        STRINGIZE(VERSION_MAJOR)        \
"." STRINGIZE(VERSION_MINOR)    \
"." STRINGIZE(0) \
"." STRINGIZE(VERSION_BUILD)