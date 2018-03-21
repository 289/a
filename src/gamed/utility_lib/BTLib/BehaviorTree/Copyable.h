#ifndef GAMED_UTILITY_LIB_BTLIB_COPYABLE_H_
#define GAMED_UTILITY_LIB_BTLIB_COPYABLE_H_


namespace BTLib {

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_COPYABLE_H_
