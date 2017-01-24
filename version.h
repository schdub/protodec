#pragma once

#define _VERSION_MAJOR_        0
#define _VERSION_MINOR_        6
#define _VERSION_BUILDS_       2
#define _VERSION_REVISION_     38

#define _PRODVERSION_MAJOR_    1
#define _PRODVERSION_MINOR_    0
#define _PRODVERSION_BUILDS_   0
#define _PRODVERSION_REVISION_ 0

#define _LOCAL_TEST_ENV_       0

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define _VERSION_     TOSTRING(_VERSION_MAJOR_)      \
                  "." TOSTRING(_VERSION_MINOR_)      \
                  "." TOSTRING(_VERSION_BUILDS_)     \
                  "." TOSTRING(_VERSION_REVISION_)

#define _PRODVERSION_ TOSTRING(_PRODVERSION_MAJOR_)  \
                  "." TOSTRING(_PRODVERSION_MINOR_)

#define _PROD_NAME_           "protodec"
#define _PROD_DESCRIPTION_    "protobuf ver2 decompiler"
#define _ORGANIZATION_NAME_   "Oleg V. Polivets"
#define _PROD_COPYRIGHT_      "(c) " _ORGANIZATION_NAME_ ". All rights reserved."
