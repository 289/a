#ifndef GAMED_UTILITY_LIB_BTLIB_BTUTILITY_H_
#define GAMED_UTILITY_LIB_BTLIB_BTUTILITY_H_


///
/// delete
///
#ifndef BT_DELETE
#define BT_DELETE(ptr) { delete (ptr); (ptr)=NULL; }
#endif // BT_DELETE

#ifndef BT_SAFE_DELETE
#define BT_SAFE_DELETE(ptr) if(NULL != (ptr)) { delete (ptr); (ptr)=NULL; }
#endif // BT_SAFE_DELETE

#ifndef BT_SAFE_DELETE_ARRAY
#define BT_SAFE_DELETE_ARRAY(ptr) if(NULL != (ptr)) { delete[] (ptr); (ptr)=NULL; }
#endif // BT_SAFE_DELETE_ARRAY


#endif // GAMED_UTILITY_LIB_BTLIB_BTUTILITY_H_
