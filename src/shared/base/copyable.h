#ifndef SHARED_BASE_COPYABLE_H_
#define SHARED_BASE_COPYABLE_H_

namespace shared {

/// A tag class emphasises the objects are copyable.
/// The empty base class optimization applies.
/// Any derived class of copyable should be a value type.
class copyable
{
};

};

#endif  // SHARED_BASE_COPYABLE_H_
