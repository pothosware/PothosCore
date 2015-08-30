/*
Copyright Redshift Software, Inc. 2008-2013
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/
#include <Pothos/serialization/impl/predef/detail/test.h>

#ifndef POTHOS_PREDEF_MAKE_H
#define POTHOS_PREDEF_MAKE_H

/*
Shorthands for the common version number formats used by vendors...
*/

/*`
[heading `POTHOS_PREDEF_MAKE_..` macros]

These set of macros decompose common vendor version number
macros which are composed version, revision, and patch digits.
The naming convention indicates:

* The base of the specified version number. "`POTHOS_PREDEF_MAKE_0X`" for
  hexadecimal digits, and "`POTHOS_PREDEF_MAKE_10`" for decimal digits.
* The format of the vendor version number. Where "`V`" indicates the version digits,
  "`R`" indicates the revision digits, "`P`" indicates the patch digits, and "`0`"
  indicates an ignored digit.

Macros are:
*/
/*` `POTHOS_PREDEF_MAKE_0X_VRP(V)` */
#define POTHOS_PREDEF_MAKE_0X_VRP(V) POTHOS_VERSION_NUMBER((V&0xF00)>>8,(V&0xF0)>>4,(V&0xF))
/*` `POTHOS_PREDEF_MAKE_0X_VVRP(V)` */
#define POTHOS_PREDEF_MAKE_0X_VVRP(V) POTHOS_VERSION_NUMBER((V&0xFF00)>>8,(V&0xF0)>>4,(V&0xF))
/*` `POTHOS_PREDEF_MAKE_0X_VRPP(V)` */
#define POTHOS_PREDEF_MAKE_0X_VRPP(V) POTHOS_VERSION_NUMBER((V&0xF000)>>12,(V&0xF00)>>8,(V&0xFF))
/*` `POTHOS_PREDEF_MAKE_0X_VVRR(V)` */
#define POTHOS_PREDEF_MAKE_0X_VVRR(V) POTHOS_VERSION_NUMBER((V&0xFF00)>>8,(V&0xFF),0)
/*` `POTHOS_PREDEF_MAKE_0X_VRRPPPP(V)` */
#define POTHOS_PREDEF_MAKE_0X_VRRPPPP(V) POTHOS_VERSION_NUMBER((V&0xF000000)>>24,(V&0xFF0000)>>16,(V&0xFFFF))
/*` `POTHOS_PREDEF_MAKE_0X_VVRRP(V)` */
#define POTHOS_PREDEF_MAKE_0X_VVRRP(V) POTHOS_VERSION_NUMBER((V&0xFF000)>>12,(V&0xFF0)>>4,(V&0xF))
/*` `POTHOS_PREDEF_MAKE_0X_VRRPP000(V)` */
#define POTHOS_PREDEF_MAKE_0X_VRRPP000(V) POTHOS_VERSION_NUMBER((V&0xF0000000)>>28,(V&0xFF00000)>>20,(V&0xFF000)>>12)
/*` `POTHOS_PREDEF_MAKE_10_VPPP(V)` */
#define POTHOS_PREDEF_MAKE_10_VPPP(V) POTHOS_VERSION_NUMBER(((V)/1000)%10,0,(V)%1000)
/*` `POTHOS_PREDEF_MAKE_10_VRP(V)` */
#define POTHOS_PREDEF_MAKE_10_VRP(V) POTHOS_VERSION_NUMBER(((V)/100)%10,((V)/10)%10,(V)%10)
/*` `POTHOS_PREDEF_MAKE_10_VRP000(V)` */
#define POTHOS_PREDEF_MAKE_10_VRP000(V) POTHOS_VERSION_NUMBER(((V)/100000)%10,((V)/10000)%10,((V)/1000)%10)
/*` `POTHOS_PREDEF_MAKE_10_VRPP(V)` */
#define POTHOS_PREDEF_MAKE_10_VRPP(V) POTHOS_VERSION_NUMBER(((V)/1000)%10,((V)/100)%10,(V)%100)
/*` `POTHOS_PREDEF_MAKE_10_VRR(V)` */
#define POTHOS_PREDEF_MAKE_10_VRR(V) POTHOS_VERSION_NUMBER(((V)/100)%10,(V)%100,0)
/*` `POTHOS_PREDEF_MAKE_10_VRRPP(V)` */
#define POTHOS_PREDEF_MAKE_10_VRRPP(V) POTHOS_VERSION_NUMBER(((V)/10000)%10,((V)/100)%100,(V)%100)
/*` `POTHOS_PREDEF_MAKE_10_VRR000(V)` */
#define POTHOS_PREDEF_MAKE_10_VRR000(V) POTHOS_VERSION_NUMBER(((V)/100000)%10,((V)/1000)%100,0)
/*` `POTHOS_PREDEF_MAKE_10_VV00(V)` */
#define POTHOS_PREDEF_MAKE_10_VV00(V) POTHOS_VERSION_NUMBER(((V)/100)%100,0,0)
/*` `POTHOS_PREDEF_MAKE_10_VVRR(V)` */
#define POTHOS_PREDEF_MAKE_10_VVRR(V) POTHOS_VERSION_NUMBER(((V)/100)%100,(V)%100,0)
/*` `POTHOS_PREDEF_MAKE_10_VVRRPP(V)` */
#define POTHOS_PREDEF_MAKE_10_VVRRPP(V) POTHOS_VERSION_NUMBER(((V)/10000)%100,((V)/100)%100,(V)%100)
/*` `POTHOS_PREDEF_MAKE_10_VVRR0PP00(V)` */
#define POTHOS_PREDEF_MAKE_10_VVRR0PP00(V) POTHOS_VERSION_NUMBER(((V)/10000000)%100,((V)/100000)%100,((V)/100)%100)
/*` `POTHOS_PREDEF_MAKE_10_VVRR0PPPP(V)` */
#define POTHOS_PREDEF_MAKE_10_VVRR0PPPP(V) POTHOS_VERSION_NUMBER(((V)/10000000)%100,((V)/100000)%100,(V)%10000)
/*` `POTHOS_PREDEF_MAKE_10_VVRR00PP00(V)` */
#define POTHOS_PREDEF_MAKE_10_VVRR00PP00(V) POTHOS_VERSION_NUMBER(((V)/100000000)%100,((V)/1000000)%100,((V)/100)%100)
/*`
[heading `POTHOS_PREDEF_MAKE_*..` date macros]

Date decomposition macros return a date in the relative to the 1970
Epoch date. If the month is not available, January 1st is used as the month and day.
If the day is not available, but the month is, the 1st of the month is used as the day.
*/
/*` `POTHOS_PREDEF_MAKE_DATE(Y,M,D)` */
#define POTHOS_PREDEF_MAKE_DATE(Y,M,D) POTHOS_VERSION_NUMBER((Y)%10000-1970,(M)%100,(D)%100)
/*` `POTHOS_PREDEF_MAKE_YYYYMMDD(V)` */
#define POTHOS_PREDEF_MAKE_YYYYMMDD(V) POTHOS_PREDEF_MAKE_DATE(((V)/10000)%10000,((V)/100)%100,(V)%100)
/*` `POTHOS_PREDEF_MAKE_YYYY(V)` */
#define POTHOS_PREDEF_MAKE_YYYY(V) POTHOS_PREDEF_MAKE_DATE(V,1,1)
/*` `POTHOS_PREDEF_MAKE_YYYYMM(V)` */
#define POTHOS_PREDEF_MAKE_YYYYMM(V) POTHOS_PREDEF_MAKE_DATE((V)/100,(V),1)

#endif
